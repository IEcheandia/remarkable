#include "calibration/CalibrationOCT_TCP.h"
#include "calibration/calibrationManager.h"
#include "calibration/CalibrationOCTConfigurationIDM.h"
#include "calibration/CalibrationOCTData.h"


namespace precitec
{
namespace calibration
{


CalibrationOCT_TCP::CalibrationOCT_TCP( CalibrationManager& p_rCalibrationManager )
    : CalibrationProcedure ( p_rCalibrationManager )
    , m_oScanWideArea(false)
{
};

CalibrationOCT_TCP::~CalibrationOCT_TCP() {}

bool CalibrationOCT_TCP::calibrate()
{
    using precitec::interface::Sensor;
    
    if ( !m_rCalibrationManager.isOCTTrackApplication() )
    {
        wmLog(eError, "Cannot run Calibration OCT TCP, device not enabled \n");
        return false;
    }
    
    //get configuration from IDM device
    
    bool validSystemConfig = false;    
    const CalibrationOCTConfigurationIDM oInitialOCTConfiguration = [this, &validSystemConfig] () 
    {
        CalibrationOCTConfigurationIDM oOCTConfiguration;
        validSystemConfig = oOCTConfiguration.loadCurrentIDMConfiguration(m_rCalibrationManager, false);
        return oOCTConfiguration;
    }();
    
    if (!validSystemConfig)
    {
        wmLog(eError, "Cannot run Calibration OCT TCP, invalid configuration \n");
        return false;
    }
    int oInitialRescaleFactor = getRescaleFactor();
    
    geo2d::DPoint Center1DScan_Newson =  oInitialOCTConfiguration.getOCTScanCenter();
    
    const auto oMeasurementModel = m_rCalibrationManager.getCalibrationOCTData()->getMeasurementModel();

    geo2d::DPoint Center2DScan_Newson = {0,0}; //by construction , see newsoncommunicator.searchTCp()
    // define OCT configuration for 2D scan
    {
        IDM_Values_t scanParameters = oInitialOCTConfiguration.getConfiguration();
        scanParameters[IDM_Parameter::eRescaleIDMValue] = 1;
        if (m_oScanWideArea )
        {
            scanParameters[IDM_Parameter::eJumpSpeed] = 10000;
            scanParameters[IDM_Parameter::eMarkSpeed] = 10000;
            scanParameters[IDM_Parameter::eScanWidth] = 100;
        }
        m_oScanOCTConfiguration = CalibrationOCTConfigurationIDM{scanParameters, Center2DScan_Newson};
        m_oScanOCTConfiguration.configureIDM(m_rCalibrationManager, oInitialOCTConfiguration);
        usleep(30 * 1000);
        setTCPSearchLoopCount(350); 
        usleep(30 * 1000);
    }
    
    setRescaleFactor(1); //use the full range of IDM value, the resulting image will be rescaled according to the actual intensity 
    usleep(30 * 1000);
    
    // start scan
    wmLog(eInfo, "Start OCT 2D Scan \n");
    geo2d::Size oCanvasSize(1024, 1024);
    
    m_rCalibrationManager.clearCanvas();
    m_rCalibrationManager.drawText(255, 255, "2D Scan in progress ...  ",  Color::Red());
    BImage background(oCanvasSize);
    m_rCalibrationManager.renderImage(background);
    usleep(200*1000);

    std::vector<image::Sample> oSamples = m_rCalibrationManager.getTCPSearchSamples();
    wmLog(eDebug, "End OCT 2D Scan\n");
    
    m_numScanLines = oSamples.size();

    bool validImage = oSamples.size() > 0;

    if (m_numScanLines > 0)
    {
        // build image from accumulated samples
        m_numPointsScanLine =0; 
        int minIDMValue = (oSamples.front().numElements() > 0) ? oSamples.front()[0] : 0 ;
        int maxIDMValue = minIDMValue;
        usleep(200*1000);
        
        interface::TriggerContext ctx = m_rCalibrationManager.getTriggerContext();
        
        for (unsigned int i = 0; i < oSamples.size(); i++)
        {
            auto & rSample =oSamples[i];
            unsigned int n = rSample.numElements();
            
            m_numPointsScanLine = n > m_numPointsScanLine ? n : m_numPointsScanLine;
            if (n==0)
            {
                std::cout << "Empty line received at row " << i << ", invalid image \n";
                validImage = false;
            }
            
            if (rSample.data() + rSample.numElements() -1 != &(rSample[rSample.numElements()-1]))
            {
                wmLog(eWarning, "rSample of numElements %d does not seem continuos \n");
            }
            
            std::pair<int*, int*> bounds = std::minmax_element(rSample.data(), rSample.data()+rSample.numElements());
            minIDMValue =  *(bounds.first) < minIDMValue  ?  *(bounds.first) : minIDMValue;
            maxIDMValue =  *(bounds.second) > maxIDMValue ? *(bounds.second) : maxIDMValue;            
            
#ifndef NDEBUG
            std::cout << i << ") " << n << " elements " ;
            std::cout << " range " << *(bounds.first) << "  " << *(bounds.second) ;
#endif

        }            
#ifndef NDEBUG            
        std::cout << std::endl;
#endif
        
        wmLog(eInfo, "2DScan: num points in scan line: %d,  num lines %d \n", m_numPointsScanLine, m_numScanLines);
        wmLog(eInfo, "Suggested rescaling for image (->255) %f  - IDM range %d %d  \n", (maxIDMValue - minIDMValue)/255.0 , minIDMValue, maxIDMValue);
        
        validImage = m_oCanvasImage.normalize(oSamples, m_numPointsScanLine/double(m_numScanLines), m_numScanLines, minIDMValue, maxIDMValue, oCanvasSize);
    }
    
    if (validImage)
    {
        
        //draw area of valid image
        m_rCalibrationManager.clearCanvas();
        
        interface::ImageContext ctx{m_rCalibrationManager.getTriggerContext()};

        ctx.HW_ROI_x0 = 0;
        ctx.HW_ROI_y0 = 0;

        m_rCalibrationManager.renderImage(m_oCanvasImage.image, ctx);
    
        m_rCalibrationManager.drawRect(m_oCanvasImage.topBorder, m_oCanvasImage.leftBorder, m_oCanvasImage.validWidth, m_oCanvasImage.validHeight, Color::Red());
        
        /* 
         * here we need to convert different reference systems: 
         * - 1D  : pixels (sample index) corresponding to  InitialOCTConfiguration (lineScan - used in automatic cycle)
         * - 2D  : pixels (sample index) corresponding to  ScanOCTConfiguration  (imageScan - used in TCP calibration)
         * - Newson:  Newson Unit
         * - CanvasImage: pixel on the image on  Screen (cf 2D) Origin is at top left corner
         * - Coax: pixel in the InitialOCTConfiguration  (cf 1D)
        */
        
        
        //compute conversion factors
        m_NewsonToPixel2D = m_numPointsScanLine / double(m_oScanOCTConfiguration.get(IDM_Parameter::eScanWidth));        

        //define positions in the different reference systems
        m_CenterCanvasImage = { m_oCanvasImage.image.width() / 2.0, m_oCanvasImage.image.height() / 2.0};
        geo2d::DPoint Center2DScan_CanvasImage = newsonToPixelScan2D(Center2DScan_Newson);
        
        geo2d::DPoint Center1DScan_CanvasImage = newsonToPixelScan2D(Center1DScan_Newson);
        
        geo2d::DPoint TCP_Newson = getTCPNewson();
        geo2d::DPoint TCP_CanvasImage = newsonToPixelScan2D(TCP_Newson);

        int ScanWidth_CanvasImage = std::round(scanWidth_CanvasImage(oInitialOCTConfiguration));
#ifndef NDEBUG        
        std::cout << "After Scan: " << std::endl;
        std::cout << "TCP  " ;
        std::cout << " Newson " <<  TCP_Newson ;
        std::cout << " On image " << TCP_CanvasImage << std::endl;
        std::cout << " Scan Center ";
        std::cout << " Newson " << Center1DScan_Newson << " on Image " << Center1DScan_CanvasImage << std::endl;
        std::cout << " Scan width ";
        std::cout << " Newson " << oInitialOCTConfiguration.get(IDM_Parameter::eScanWidth) << " on Image " << ScanWidth_CanvasImage << std::endl;
#endif
        std::string oTCPtxt = "TCP " + std::to_string(TCP_Newson.x) + " " + std::to_string(TCP_Newson.y);        
        m_rCalibrationManager.drawCross( TCP_CanvasImage.x, TCP_CanvasImage.y, 20, Color::Blue());
        m_rCalibrationManager.drawText(TCP_CanvasImage.x, TCP_CanvasImage.y, oTCPtxt, Color::Blue());
        
        std::string oScanCenter = "Scan " + std::to_string(Center1DScan_Newson.x) + " " + std::to_string(Center1DScan_Newson.y);        
        m_rCalibrationManager.drawCross( Center1DScan_CanvasImage.x, Center1DScan_CanvasImage.y, 20, Color::Green());   
        m_rCalibrationManager.drawText(Center1DScan_CanvasImage.x, Center1DScan_CanvasImage.y, oScanCenter, Color::Green());  
        
        m_rCalibrationManager.drawLine( Center1DScan_CanvasImage.x - ScanWidth_CanvasImage/2.0, Center1DScan_CanvasImage.y, 
                                        Center1DScan_CanvasImage.x + ScanWidth_CanvasImage/2.0, Center1DScan_CanvasImage.y, Color::Green());
    
        if (oMeasurementModel.isInitialized())
        {
            //2D Configuration
            auto oLateralResolution2D = oMeasurementModel.getLateralResolution(m_oScanOCTConfiguration); 
            
            std::ostringstream oSizeInfo2D;
            oSizeInfo2D <<  "Scan 2D lateral Resolution [um] " << oLateralResolution2D << " Scan width [mm] " << m_numPointsScanLine*oLateralResolution2D/1000;
            wmLog(eInfo, oSizeInfo2D.str().c_str()); 
            

            //1D  Configuration
            auto oLateralResolution1D = oMeasurementModel.getLateralResolution(oInitialOCTConfiguration); 
        
            std::ostringstream oSizeInfo1D;
            oSizeInfo1D <<  "Scan 1D lateral Resolution [um] " << oLateralResolution1D << " Scan width [mm] " << ScanWidth_CanvasImage*oLateralResolution2D/1000;
            wmLog(eInfo, oSizeInfo1D.str().c_str());
        
            double newson_to_mm = oMeasurementModel.getNewsonLineWidthCalibrated(1.0)/1000.0;
            
            //this values are different when the numMeasurements in calibrationoctdata is wrong
            
            wmLog(eDebug, "Distance Scan Center to TCP: %f, %f [mm] (using lateral resolution) \n", 
                  std::abs(TCP_CanvasImage.x - Center1DScan_CanvasImage.x)*oLateralResolution2D/1000.0,  
                  std::abs(TCP_CanvasImage.y - Center1DScan_CanvasImage.y)*oLateralResolution2D/1000.0);
            
            wmLog(eDebug, "Distance Scan Center to TCP: %f, %f [mm] (using getNewsonLineWidthCalibrated) \n", 
                  std::abs(TCP_Newson.x - Center1DScan_Newson.x)*newson_to_mm,
                  std::abs(TCP_Newson.y - Center1DScan_Newson.y)*newson_to_mm);
            
            
            wmLog(eDebug, "Distance Scan Center to Newson Origin: %f, %f [mm]\n", 
                  std::abs(Center1DScan_CanvasImage.x - m_CenterCanvasImage.x)*oLateralResolution2D/1000.0,  
                  std::abs(Center1DScan_CanvasImage.y - m_CenterCanvasImage.y)*oLateralResolution2D/1000.0);


            {                 
                double desiredDistance_mm = getDesiredDistanceFromTCP();
                double scanY = TCP_Newson.y - oMeasurementModel.getNewsonPositionFromDistance(desiredDistance_mm);
                geo2d::DPoint suggestedScanCenter =  {TCP_Newson.x, scanY};    
                wmLog(eInfo, "suggestedScanCenter %f %f [Newson] \n", suggestedScanCenter.x, suggestedScanCenter.y);
                geo2d::DPoint newCenter1DScan_CanvasImage = newsonToPixelScan2D(suggestedScanCenter);
                m_rCalibrationManager.drawCross( newCenter1DScan_CanvasImage.x, newCenter1DScan_CanvasImage.y, 20, Color::Yellow());
                m_rCalibrationManager.drawLine( newCenter1DScan_CanvasImage.x - ScanWidth_CanvasImage/2.0, newCenter1DScan_CanvasImage.y, 
                                            newCenter1DScan_CanvasImage.x + ScanWidth_CanvasImage/2.0, newCenter1DScan_CanvasImage.y, Color::Yellow());
                
                m_rCalibrationManager.drawText( newCenter1DScan_CanvasImage.x, newCenter1DScan_CanvasImage.y, std::to_string(desiredDistance_mm), Color::Yellow());
            }

            m_rCalibrationManager.renderImage(m_oCanvasImage.image, ctx);

        }
        else
        {
            wmLog(eError, "Resolution info not available because OCT System is not calibrated\n");
        }        
    }
    else
    {
        wmLog(eWarning, "Error during 2DScan image processing\n");
        m_rCalibrationManager.renderImage(background);
        m_NewsonToPixel2D = 0;
    }
    

    usleep(200*1000);

    //restore previous configuration
    
    wmLog(eInfo, "Reset OCT configureIDM\n");
    oInitialOCTConfiguration.configureIDM(m_rCalibrationManager, m_oScanOCTConfiguration);
  
    setRescaleFactor(oInitialRescaleFactor);
    usleep(200*1000);

    return validImage;
}

bool CalibrationOCT_TCP::NormalizedImage::normalize(const image::BImage & r_ScanImage, unsigned int lengthScanLine, unsigned int numScanLines, geo2d::Size outputSize)
{       
    auto & rImage = image;
    //sanity check
    if (int( lengthScanLine) > r_ScanImage.width() || int(numScanLines) > r_ScanImage.height() )
    {
        rImage.clear();
        return false;
    }
    
    if ( lengthScanLine < numScanLines)
    {
        wmLog(eWarning, "Horizontal resolution too low \n");
        rImage.clear();
        return false;
        
    }
    
    rImage.resizeFill(outputSize, 0);
        
    //write the content of scanImage into normalizedImage, such that they have the same center
    //each row of scanImage is just translated into the new image (repeated if necessary)
    //no interpolation is performed
    assert(scaleX == 1.0);
    scaleY= lengthScanLine/double(numScanLines);
    
    validWidth = lengthScanLine;
    validHeight = std::floor(scaleY * numScanLines);
    leftBorder = (rImage.width() - validWidth) /2;
    topBorder = ( rImage.height() - validHeight) /2;
    
    for (unsigned int scanRow = 0 ; scanRow <numScanLines; ++scanRow)
    {
        unsigned int firstRow = topBorder + std::floor(scaleY * scanRow);
        unsigned int lastRow = topBorder + std::floor(scaleY * (scanRow+1));
        for (unsigned int outRow = firstRow; outRow < lastRow;  ++outRow )
        {
            auto pPixel = rImage.rowBegin(outRow) + leftBorder;
            memcpy(pPixel, r_ScanImage.rowBegin(scanRow), lengthScanLine);
        }        
    }
    
    assert(image.isValid());
    return true;
}



bool CalibrationOCT_TCP::NormalizedImage::normalize ( const std::vector< precitec::image::Sample >& r_samples, double p_ScaleY, unsigned int numScanLines, int minIntensity, int maxIntensity, geo2d::Size outputSize )
{
    std::cout << "Normalize : scale y " << p_ScaleY << "  numScanLines " << numScanLines << " " << minIntensity << " " << maxIntensity << std::endl;
    if (numScanLines > r_samples.size() || numScanLines == 0)
    {
        return false;
    }
    image.resizeFill(outputSize, 0);
    
    assert(scaleX == 1.0);
    scaleY= p_ScaleY;
    
    validWidth = std::floor(numScanLines*scaleY);
    validHeight = std::floor(scaleY * numScanLines);
    leftBorder = (image.width() - validWidth) /2;
    topBorder = ( image.height() - validHeight) /2;
    double intensityRange = maxIntensity - minIntensity;
    
    for (unsigned int scanRow = 0 ; scanRow <numScanLines; ++scanRow)
    {
        auto & rLineValues = r_samples[scanRow];
        unsigned int numSamples = rLineValues.numElements();
        
        unsigned int firstRow = topBorder + std::floor(scaleY * scanRow);
        unsigned int lastRow = topBorder + std::floor(scaleY * (scanRow+1));
        auto localLeftBorder =( image.width() - numSamples)/2;
#ifndef NDEBUG        
        if ((int)(localLeftBorder) < leftBorder ||  (int)(numSamples) > validWidth)
        {
            std::cout << "Error length   "
            << " numSamples " << numSamples 
            << " valid width " << validWidth
            << " localLeftBorder " << leftBorder << " " << localLeftBorder
            << std::endl;
        }
#endif

        for (unsigned int outRow = firstRow; outRow < lastRow;  ++outRow )
        {
            auto pPixelOut = image.rowBegin(outRow) + localLeftBorder;
            for (unsigned int i=0; i < numSamples; i++)
            {
                (*pPixelOut) =  double( rLineValues[i] - minIntensity) / intensityRange *255+0; 
                ++pPixelOut;
                
            }
        }        
    }
    
    assert(image.isValid());
    return true;
}


void CalibrationOCT_TCP::setTCPSearchLoopCount ( int p_oValue )
{
    interface::SmpKeyValue pKeyValue = new interface::TKeyValue<int> ( "TCP Search Loop Count", p_oValue );
    m_rCalibrationManager.setIDMKeyValue ( pKeyValue );
}


void CalibrationOCT_TCP::setRescaleFactor ( int p_oValue )
{
    interface::SmpKeyValue pKeyValue = new interface::TKeyValue<int> ( "Rescale IDM Line", p_oValue );
    m_rCalibrationManager.setIDMKeyValue ( pKeyValue );
}


int CalibrationOCT_TCP::getRescaleFactor() const 
{
    return m_rCalibrationManager.getIDMKeyValue ( "Rescale IDM Line" )->value<int>();
}


double CalibrationOCT_TCP::getScanCenterX() const
{
    return  m_rCalibrationManager.getIDMKeyValue ( "ScanCenterX" )->value<double>();
}


double CalibrationOCT_TCP::getScanCenterY() const
{
    auto kv =  m_rCalibrationManager.getIDMKeyValue ( "ScanCenterY" );
    assert(!kv.isNull() &&  kv->isHandleValid() &&  kv->type() == TDouble);
    return m_rCalibrationManager.getIDMKeyValue ( "ScanCenterY" )->value<double>();
}

void CalibrationOCT_TCP::setScanCenter(double x, double y)
{
    interface::SmpKeyValue pKeyValueX = new interface::TKeyValue<double> ( "ScanCenterX", x );
    interface::SmpKeyValue pKeyValueY = new interface::TKeyValue<double> ( "ScanCenterY", y );
    m_rCalibrationManager.setIDMKeyValue ( pKeyValueX );
    m_rCalibrationManager.setIDMKeyValue ( pKeyValueY );
}

double CalibrationOCT_TCP::getDesiredDistanceFromTCP() const
{
    auto smpKeyValue = m_rCalibrationManager.getCalibrationOCTKeyValue("DesiredDistanceFromTCP");
    if (smpKeyValue.isNull())
    {
        wmLog(eWarning, "Error in DesiredDistanceFromTCP \n");
        return 0.0;
    }
    return smpKeyValue->value<double>();

}


geo2d::DPoint CalibrationOCT_TCP::getTCPNewson() const
{
    auto smpKeyValueX =  m_rCalibrationManager.getCalibrationOCTKeyValue("X_TCP_Newson");
    assert(smpKeyValueX->isHandleValid() && smpKeyValueX->type() == TDouble );
    auto smpKeyValueY =  m_rCalibrationManager.getCalibrationOCTKeyValue("Y_TCP_Newson");
    assert(smpKeyValueY->isHandleValid() && smpKeyValueY->type() == TDouble);
    return {smpKeyValueX->value<double>(), smpKeyValueY->value<double>()};
}



geo2d::DPoint CalibrationOCT_TCP::pixelScan2DtoNewson ( geo2d::DPoint p_PositionImage ) const
{
    return ( p_PositionImage - m_CenterCanvasImage ) / m_NewsonToPixel2D;
}

geo2d::DPoint CalibrationOCT_TCP::newsonToPixelScan2D ( geo2d::DPoint p_PositionNewson ) const
{
    auto ret =  p_PositionNewson * m_NewsonToPixel2D + m_CenterCanvasImage;
    return ret;
}


double CalibrationOCT_TCP::scanWidth_CanvasImage ( CalibrationOCTConfigurationIDM p_oOCTConfigurationScan1D ) const
{
    return m_numPointsScanLine * p_oOCTConfigurationScan1D.get ( IDM_Parameter::eScanWidth ) / double ( m_oScanOCTConfiguration.get ( IDM_Parameter::eScanWidth ) );
}


} // namespace calibration

} // namespace precitec
