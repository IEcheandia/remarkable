#include "calibration/CalibrationOCTLine.h"
#include "calibration/calibrationManager.h"

#include <common/connectionConfiguration.h>
#include <numeric> //accumulate
#include <iomanip> //set_precision, setw

#include "calibration/CalibrationOCTData.h"



namespace precitec
{
namespace calibration
{


//IDM settings to use during calibration

const std::vector< IDM_Values_t>  oCalibrationConfigurations
{
    IDM_Values_t {3018, 3018, 30, 70000, 10},
    IDM_Values_t {3018, 3018, 25, 70000, 10},
   // IDM_Values_t  {3018, 3018, 35, 70000, 10},
    //IDM_Values_t {3048, 3048, 25, 70000, 10},
};


CalibrationOCTLine::CalibrationOCTLine ( CalibrationManager& p_rCalibrationManager )
    : CalibrationProcedure ( p_rCalibrationManager )
    , m_oRepetitions ( 10 ) 
    , m_yMagnify(1)     //rescaling for paint: we expect a canvas height ot 512 and a signal height of 200, we magnify oof (512/200) 
   
{
    assert(m_rCalibrationManager.getCalibrationOCTData() != nullptr);
    m_profileRecognitionParameters = m_rCalibrationManager.getCalibrationOCTData()->getCalibrationOCTLineParameters();    
};

CalibrationOCTLine::~CalibrationOCTLine() {}



CalibrationOCTLine::ProfileFeatures CalibrationOCTLine::computeProfileFeatures ( const precitec::image::Sample& rSample, const CalibrationOCTLineParameters & rParameters ) 
{
        
    ProfileFeatures oResult;
    oResult.numMeasurements = rSample.numElements();
    if ( oResult.numMeasurements < 2 * rParameters.m_minNumValuesPerLayer )
    {
        wmLog ( eWarning, "Insufficient number of measurements %d\n", oResult.numMeasurements );
        oResult.valid = false;
        return oResult; 
    } 
    
    //copy to vector, (just to ust stl algotihms)
    std::vector<int> oLine( oResult.numMeasurements );
    auto  it = oLine.begin();
    for ( int i = 0; i <  oResult.numMeasurements; ++i, ++it )
    {
        assert ( it != oLine.end() );
        ( *it ) = rSample[i];
    }

    /////////////////////////////////////////////////////////////
    //compute profile features
    /////////////////////////////////////////////////////////////

    //assuming horizontal line
    const double valueLeft = std::accumulate ( oLine.begin(), oLine.begin() + rParameters.m_minNumValuesPerLayer,0 )
                                / double ( rParameters.m_minNumValuesPerLayer );

    int indexBottomLayerLeft = 0;
    int indexBottomLayerRight = 0;

    double valueBottomLayer = valueLeft;
    for ( int i = 0, end = oLine.size(); i < end; i += rParameters.m_minNumValuesPerLayer )
    {
        double curMean = std::accumulate ( oLine.begin()+i, oLine.begin()+i + rParameters.m_minNumValuesPerLayer,0 ) / double ( rParameters.m_minNumValuesPerLayer );

        if ( ( curMean  > valueBottomLayer ) && ( curMean > ( valueLeft + rParameters.m_minJumpPixel ) ) )
        {
            valueBottomLayer = curMean;
            if ( indexBottomLayerLeft == 0 )
            {
                //update index only the first time
                indexBottomLayerLeft = i;
            }
        }


        if ( std::abs ( curMean - valueBottomLayer ) < rParameters.m_maxRangePerLayer )
        {
            indexBottomLayerRight = i;
        }
    }
    
    //save the position found so far for debug, and improve the position
    oResult.m_oCandidateXLeft = indexBottomLayerLeft;
    oResult.m_oCandidateXRight = indexBottomLayerRight;
    
    for ( int index = std::min<int> ( indexBottomLayerLeft + rParameters.m_minNumValuesPerLayer, oLine.size()-1 ),
        lastIndex = std::max<int> ( indexBottomLayerLeft - rParameters.m_minNumValuesPerLayer/2,0 );
        index > lastIndex && std::abs ( ( oLine[index] + oLine[index+1] ) /2.0 - valueBottomLayer ) < rParameters.m_maxRangePerLayer ; --index )
    {
        indexBottomLayerLeft  = index;
    }

    for ( int index =std::max<int> ( indexBottomLayerRight - rParameters.m_minNumValuesPerLayer/2,1 ),
            lastIndex = std::min<int> ( indexBottomLayerRight + 3 * rParameters.m_minNumValuesPerLayer/2, oLine.size() );
            index <lastIndex && std::abs ( ( oLine[index] + oLine[index-1] ) /2.0 -valueBottomLayer ) < rParameters.m_maxRangePerLayer ; ++index )
    {
        indexBottomLayerRight = index;
    }
    
    
    if ( (indexBottomLayerRight - indexBottomLayerLeft) < rParameters.m_minNumValuesPerLayer
        || (valueBottomLayer - valueLeft) < rParameters.m_minJumpPixel
        || (indexBottomLayerRight - indexBottomLayerLeft) > rParameters.m_maxWidthPixel
    )
    {
        oResult.valid = false;
        return oResult;
    }
    

    oResult.m_oXLeft = indexBottomLayerLeft;
    oResult.m_oXGap = indexBottomLayerRight - indexBottomLayerLeft;
    oResult.m_oZTop = valueLeft;
    oResult.m_oZGap = valueBottomLayer - valueLeft;
    oResult.valid = true;
    return oResult;

}

bool CalibrationOCTLine::calibrate()
{
    using precitec::interface::Sensor;
    
    if ( !m_rCalibrationManager.isOCTTrackApplication() )
    {
        return false;
    }
       
    CalibrationOCTConfigurationIDM oInitialConfiguration, oPreviousConfiguration;
    bool validIDMConfiguration = oInitialConfiguration.loadCurrentIDMConfiguration(m_rCalibrationManager, true);
    oPreviousConfiguration = oInitialConfiguration;

    
    if (!validIDMConfiguration)
    {
        wmLog(eWarning, "Could not perform calibration: IDM configuration could not be read or it's invalid\n");
        return false;
    }
    
    geo2d::DPoint oScanCenter = oInitialConfiguration.getOCTScanCenter();
    
    m_rCalibrationManager.clearCanvas();
    BImage oIdmDummyImage {{1024,512}};

    int index = 0;
    const int total_tests  = oCalibrationConfigurations.size() * m_oRepetitions;
    
    std::vector<CalibrationOCTConfigurationIDM> oTestedConfigurations;
    std::vector<ProfileFeatures> oTestedProfiles;
    
    oTestedConfigurations.reserve(total_tests);
    oTestedProfiles.reserve(total_tests);
    
    for ( unsigned int configuration_item = 0; configuration_item < oCalibrationConfigurations.size(); ++configuration_item )
    {
        CalibrationOCTConfigurationIDM  oConfiguration { oCalibrationConfigurations[configuration_item], oScanCenter};
        
        bool ok = oConfiguration.configureIDM(m_rCalibrationManager, oPreviousConfiguration);
        oPreviousConfiguration = oConfiguration;
        
        if (!ok)
        {
            wmLog(eWarning, "Could not perform calibration: error during IDM configuration \n");
            return false;
        }

        
    
        showInfoConfiguration(oConfiguration, index, total_tests );

        wmLog ( eDebug, "Requesting sample config %d repetition\n", configuration_item);
        
        auto oSamples = m_rCalibrationManager.getSamples ( Sensor::eIDMTrackingLine, m_oRepetitions, 1, 5000);
        
        for (const auto & rSample : oSamples)
        {
        
            ProfileFeatures oProfileFeatures = computeProfileFeatures(rSample, m_profileRecognitionParameters);
            
            showInfoProfile(rSample, oProfileFeatures, index, total_tests );

            if (oProfileFeatures.valid)
            {
                oTestedConfigurations.push_back(oConfiguration);
                oTestedProfiles.push_back(oProfileFeatures);
            }   
            index++;
        }
        
        //m_rCalibrationManager.cancelTrigger(std::vector<int>(1,0));
        usleep(1000*1000);
            
                
        m_rCalibrationManager.renderImage(oIdmDummyImage);        
    }

    //after all the tests, restore the initial configuration
    oInitialConfiguration.configureIDM(m_rCalibrationManager);

    if (oTestedProfiles.size() == 0)
    {
        wmLog(eWarning, "Not enough measurements for calibration \n");
        return false;
    }
    
    auto oModel = computeModel(oTestedConfigurations, oTestedProfiles, m_profileRecognitionParameters.m_gapWidth,  m_profileRecognitionParameters.m_gapHeight);
    if (!oModel.isInitialized())
    {
        wmLog(eWarning, "Not enough measurements for calibration \n");
        return false;
    }

    // save calibration data
    return m_rCalibrationManager.updateOCTCalibrationData(oModel);
}


geo2d::Point CalibrationOCTLine::coordinatesForPaint ( int x, int y, int x_offset, int index_subplot, int total_subplots )
{
    int subplot_height = 512/total_subplots;
    int y_offset = subplot_height * index_subplot;
    return {x+ x_offset, y_offset + ( y * m_yMagnify / total_subplots ) };
}

void CalibrationOCTLine::showInfoConfiguration(const CalibrationOCTConfigurationIDM & rConfiguration,  int index, int total_subplots)
{
    Color oColor = chooseColor(index);
    geo2d::Point oTextBox = coordinatesForPaint ( 0,0, 0, index, total_subplots );
    std::stringstream oMsg;
    oMsg << "IDM Config: ";
    for ( int i = 0; i < IDM_Parameter::eNumParameters; ++i )
    {
        oMsg << rConfiguration.get ( IDM_Parameter ( i ) ) << "  ";
    }
    m_rCalibrationManager.drawText ( oTextBox.x, oTextBox.y, oMsg.str(),oColor);
}

Color CalibrationOCTLine::chooseColor(int index) const 
{
        switch(index%3)
        {
            case 0: return Color::Orange();
            case 1:  return Color::Magenta();
            default:  return Color::Yellow();
        };
}
    

void CalibrationOCTLine::showInfoProfile ( const image::Sample& rSample, const ProfileFeatures& rProfileFeatures, int index, int total_subplots )
{
    
    Color oColor = chooseColor(index);
    
    int x_offset = 512 - rSample.numElements() /2;

    //draw frame (slightly shrinked)
    {
        Color oFrameColor = oColor;
        oFrameColor.alpha = 125;
        geo2d::Point oTopLeft = coordinatesForPaint(10,10,0, index, total_subplots);
        geo2d::Point oSize = coordinatesForPaint(1014,502,0, index, total_subplots);
        m_rCalibrationManager.drawRect(oTopLeft.x, oTopLeft.y, oSize.x, oSize.y, oFrameColor);
    }
    
    for ( int i= 0; i < rSample.numElements(); ++i )
    {
        geo2d::Point oPoint = coordinatesForPaint ( i, rSample[i], x_offset, index, total_subplots );
        m_rCalibrationManager.drawPixel ( oPoint.x, oPoint.y, oColor );
    }

    geo2d::Point oTextBox = coordinatesForPaint ( 200,0,0, index, total_subplots );
    if ( rProfileFeatures.valid )
    {
        geo2d::Point oTopLeft = coordinatesForPaint ( rProfileFeatures.m_oXLeft, rProfileFeatures.m_oZTop, x_offset, index, total_subplots );
        geo2d::Point oBottomRight = coordinatesForPaint ( rProfileFeatures.m_oXLeft + rProfileFeatures.m_oXGap, rProfileFeatures.m_oZTop + rProfileFeatures.m_oZGap,
                                    x_offset, index, total_subplots );
        m_rCalibrationManager.drawCross( oTopLeft.x, oTopLeft.y, 10, Color::Red() );
        m_rCalibrationManager.drawCross( oBottomRight.x, oBottomRight.y,10, Color::Cyan() );
        
        double oLateralResolution = rProfileFeatures.m_oXGap / m_profileRecognitionParameters.m_gapWidth;
        double oDepthResolution = rProfileFeatures.m_oZGap / m_profileRecognitionParameters.m_gapHeight;

        

        std::stringstream oMsg;
        oMsg << std::setprecision ( 4 ) << std::setw ( 5 ) ;
        oMsg << "Lateral " << oLateralResolution << " Depth " << oDepthResolution;
        m_rCalibrationManager.drawText ( oTextBox.x, oTextBox.y, oMsg.str(), oColor );
        oMsg << "\n";
        wmLog(eInfo, std::to_string(index) + ": " + oMsg.str());
    }
    else
    {
        std::stringstream oMsg;
        oMsg << "Invalid features " << rProfileFeatures.numMeasurements << " " << rProfileFeatures.m_oXGap << " " << rProfileFeatures.m_oZGap; 
        m_rCalibrationManager.drawText ( oTextBox.x, oTextBox.y, oMsg.str(), oColor );        
        oMsg<< "\n";
        wmLog(eInfo, std::to_string(index) + ": " + oMsg.str());
    }
}



//simple linear model, taking as a reference the first valid configuration
// it makes strong assumptions on oCalibrationConfigurations and on the validity of the profile features, there must be at least one valid feature
// it does not check input, it must be called only when the input is valid
CalibrationOCTMeasurementModel CalibrationOCTLine::computeModel ( const std::vector< precitec::calibration::CalibrationOCTConfigurationIDM >& r_configurations, 
                                                       const std::vector< precitec::calibration::CalibrationOCTLine::ProfileFeatures >& r_profileFeatures,
                                                       double p_oGapWidth_um, double p_oGapHeigth_um
                                                     )
{
    assert(r_configurations.size() == r_profileFeatures.size() && "caller has not checked inputs");
    assert(r_configurations.size() > 0);
    
    int reference_index = -1;
    
    double validValues = 0; // avoid integer division when computing averages
    double sum_x = 0;
    double sum_z = 0;
    double sum_lateralResolution = 0;
    double sum_depthResolution = 0;
        
    for (unsigned int i = 0; i < r_configurations.size(); ++i)
    {
        auto & rFeature = r_profileFeatures[i];
        auto & rConfig = r_configurations[i];
        
        if (!rFeature.valid)
        {
            continue;
        }
        
        //check if oCalibrationConfigurations still has valid configurations
        if ( ( reference_index >= 0 && rConfig.get(eSamplingFrequency) !=  r_configurations[reference_index].get(eSamplingFrequency)) 
            || rConfig.get(eJumpSpeed) != rConfig.get(eMarkSpeed)
            || rConfig.get(eJumpSpeed) == 0
            || rConfig.get(eScanWidth) == 0
        )
        {
            std::string oMsg =  "Configuration not supported in the current computation\n";
            wmLog(eInfo, oMsg.c_str());
            std::cout << oMsg;
            continue;
        }
        
        if (reference_index < 0)
        {
            //we just found the first valid pair of configurations and profiles
            reference_index = i;
        }
        
        auto & rReference =  r_configurations[reference_index];
    
        double xFromCenter = double(rFeature.m_oXLeft) - rFeature.numMeasurements/2;
        if (  rConfig.get(eJumpSpeed) == rReference.get(eJumpSpeed) )
        {
            sum_x += xFromCenter;
            sum_lateralResolution +=  (p_oGapWidth_um/ double(rFeature.m_oXGap)); 
        }
        else
        {   
            double speedRatio = double(rConfig.get(eJumpSpeed)) / double(rReference.get(eJumpSpeed)) ;
   
            //normalize to reference speed
            sum_x += ( xFromCenter*  speedRatio);
            sum_lateralResolution +=  (p_oGapWidth_um/ double(speedRatio* rFeature.m_oXGap)); 
        }
        
        sum_z += rFeature.m_oZTop;
        sum_depthResolution +=  (p_oGapHeigth_um/ double(rFeature.m_oZGap* rConfig.get(eRescaleIDMValue)));
        
        validValues ++;
    }
    if (validValues == 0)
    {
        // return an invalid model
        return {};
    }
    
    geo2d::Point oReferencePosition( std::round(sum_x / validValues) ,  std::round(sum_z / validValues));
    return  {
            r_configurations[reference_index], 
            oReferencePosition, 
            r_profileFeatures[reference_index].numMeasurements, 
            sum_lateralResolution / validValues, 
            sum_depthResolution / validValues
        };
    
}



} //namespace calibration
} //namespace precitec
