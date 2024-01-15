#include "util/ScanFieldImageParameters.h"
#include <fstream>

using precitec::image::ImageFillMode;

namespace {
    //helper functions
    precitec::geo2d::DPoint applyMirroringToPixelCoordinates(ImageFillMode mode, precitec::geo2d::DPoint point, int w, int h)
    {
        switch(mode)
        {
            case ImageFillMode::Direct:
                return point;
            case ImageFillMode::FlippedHorizontal:
                return {(double)(w) - point.x - 1, point.y};
            case ImageFillMode::FlippedVertical:
                return {point.x, (double)(h) - point.y -1};
            case ImageFillMode::Reverse:
                return {(double)(w) - point.x - 1, (double)(h) - point.y -1};
        }
        assert(false && "case not handled");
        return {};
    }
}

namespace precitec{
namespace calibration{    


const std::array<std::string, ScanFieldImageParameters::key_id::NUM_IDs> ScanFieldImageParameters::s_keys = 
    {
        "xMinLeft_mm",  "yMinTop_mm", "ScanFieldImageWidth",  "ScanFieldImageHeight"
    };

interface::Configuration ScanFieldImageParameters::makeConfiguration() const
{
    auto oConfiguration = m_ScanMasterData.makeConfiguration();
    for (auto & key : s_keys)
    {
        oConfiguration.emplace_back(getKeyValue(key));
    }
    return oConfiguration;
}

interface::SmpKeyValue ScanFieldImageParameters::getKeyValue ( std::string p_key ) const
{
    auto it = std::find(s_keys.begin(), s_keys.end(), p_key);
    if (it == s_keys.end())
    {
        //key not found,  try with m_ScanMasterData
        return m_ScanMasterData.getKeyValue(p_key);
    }
    
    key_id id = static_cast<key_id>(std::distance(s_keys.begin(), it));
    
    switch (id)
    {
        case (e_xMinLeft_mm ):
            return interface::SmpKeyValue { new interface::TKeyValue<double> ( p_key, m_xMinLeft_mm, -1000, +1000, 0.0)};
        case (e_yMinTop_mm ) :
            return interface::SmpKeyValue {new interface::TKeyValue<double> ( p_key, m_yMinTop_mm, -1000, +1000, 0.0)};
        case (e_Width ) :
            return interface::SmpKeyValue {new interface::TKeyValue<int> ( p_key, m_scanfieldimageSize.width, 0, 10000, 0.0)};
        case (e_Height) :
            return interface::SmpKeyValue {new interface::TKeyValue<int> ( p_key, m_scanfieldimageSize.height, 0, 10000, 0.0)};
        case (NUM_IDs):
            return {};
    }
    return {};
}

interface::KeyHandle ScanFieldImageParameters::setKeyValue ( interface::SmpKeyValue p_oSmpKeyValue )
{
    assert(!p_oSmpKeyValue.isNull());
    auto it = std::find(s_keys.begin(), s_keys.end(), p_oSmpKeyValue->key());
    
    if (it == s_keys.end())
    {
        //key not found,  try with m_ScanMasterData
        auto oHandle = m_ScanMasterData.setKeyValue(p_oSmpKeyValue);
        if (oHandle.handle() == -1)
        {
            wmLog(eWarning, "ScanFieldImageCalculator: Unknown key  %s \n",  p_oSmpKeyValue->key().c_str());
        }
        return oHandle;
    }
    
    key_id id = static_cast<key_id>(std::distance(s_keys.begin(), it));
    
    switch(p_oSmpKeyValue->type())
    {
        case(TDouble): 
        {
            double value = p_oSmpKeyValue->value<double>();
            if ( id == e_xMinLeft_mm ) 
            {
                m_xMinLeft_mm = value;
                return {1};
            }
            if ( id == e_yMinTop_mm ) 
            {
                m_yMinTop_mm = value;
                return {1};
            }
        }
            break;
        case(TInt): 
        {
            int value = p_oSmpKeyValue->value<int>();
            if ( id == e_Width ) 
            {
                m_scanfieldimageSize.width = value;
                return {1};
            }
            if ( id == e_Height ) 
            {
                m_scanfieldimageSize.height = value;
                return {1};
            }
        }
            break;
        default:
            wmLog(eDebug, "ScanFieldImageCalculator: Unkwown key  %s of type %d \n",  p_oSmpKeyValue->key().c_str(), int(p_oSmpKeyValue->type()) );
            break;
    }
    return {-1};
}

void ScanFieldImageParameters::load ( const Poco::Util::AbstractConfiguration* pConfIn )
{
    assert ( pConfIn );

    m_ScanMasterData.load(pConfIn);
    
    for (auto & key: s_keys)
    {
        auto smpKeyValue = getKeyValue(key);
        
        try 
        {
            switch ( smpKeyValue->type() ) 
            {
                case TDouble:
                    smpKeyValue->setValue ( pConfIn->getDouble ( key, smpKeyValue->defValue<double>() ) );
                    break;
                    
                case TInt:
                    smpKeyValue->setValue ( pConfIn->getInt( key, smpKeyValue->defValue<int>() ) );
                    break;
                    
                case TBool:
                    smpKeyValue->setValue( pConfIn->getBool(key, smpKeyValue->defValue<bool>()));
                    break;
                    
                default:
                    std::ostringstream oMsg;
                    oMsg << "Invalid value type: '" << smpKeyValue->type() << "'\n";
                    wmLogTr ( eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str() );
                    
                    break;
                    
            } // switch
        } 
        catch ( const Poco::Exception &p_rException ) 
        {

            wmLog ( eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str() );
            std::ostringstream oMsg;
            oMsg << "Parameter '" << key.c_str() << "' of type '" << smpKeyValue->type() << "' could not be converted. Reset to default value.\n";
            wmLog ( eDebug, oMsg.str() );
            wmLogTr ( eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str() );
            
            smpKeyValue->resetToDefault();

        } // catch
        catch (...)
        {
            std::cout << "Uncaught Exception " << std::endl;
            std::ostringstream oMsg;
            oMsg << smpKeyValue->toString();
            wmLogTr ( eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str() );
            
            smpKeyValue->resetToDefault();
        }
        setKeyValue ( smpKeyValue );
    }
}

/*static*/ ScanFieldImageParameters ScanFieldImageParameters::computeParameters(const ScanMasterCalibrationData& rScanMasterCalibrationData, 
                                                                                    image::Size2d imageSize, ScanFieldGridParameters gridParameters)
{
    return computeParameters(rScanMasterCalibrationData, imageSize.width, imageSize.height, gridParameters.xMin, gridParameters.xMax, gridParameters.yMin, gridParameters.yMax);
}

/*static*/ ScanFieldImageParameters ScanFieldImageParameters::computeParameters(const ScanMasterCalibrationData& rScanMasterCalibrationData, 
                                                                                        int imageWidth, int imageHeight, double xMin, double xMax, double yMin, double yMax) 
{                                       
    ScanFieldImageParameters oParameters;

    oParameters.m_ScanMasterData = rScanMasterCalibrationData;
    

    double halfWidth_mm = std::abs(1.0 / oParameters.m_ScanMasterData.m_XmmToPixel * imageWidth / 2.0) ;
    double halfHeight_mm = std::abs(1.0 /oParameters.m_ScanMasterData.m_YmmToPixel * imageHeight/ 2.0) ;

    
    // center the scan field image in 0,0
    xMax = std::max(std::abs(xMax), std::abs(xMin));
    xMin = -xMax;
    yMax = std::max(std::abs(yMax), std::abs(yMin));
    yMin = -yMax;
    
    double p_xMinLeft_mm = xMin-halfWidth_mm;
    double p_yMinTop_mm = yMin-halfHeight_mm;
    double p_xMaxRight_mm = xMax+halfWidth_mm;
    double p_yMaxBottom_mm =  yMax+halfHeight_mm;
    
    assert((p_xMaxRight_mm - p_xMinLeft_mm) > 0);
    assert((p_yMaxBottom_mm - p_yMinTop_mm) > 0);
    
    int expectedWidth = std::abs(std::ceil(p_xMaxRight_mm - p_xMinLeft_mm) * oParameters.m_ScanMasterData.m_XmmToPixel);
    double deltaHeight = expectedWidth * oParameters.m_ScanMasterData.m_Slope;
    double deltaHeight_mm = deltaHeight / oParameters.m_ScanMasterData.m_YmmToPixel;
    int expectedHeight = std::abs(std::ceil(p_yMaxBottom_mm - p_yMinTop_mm) * oParameters.m_ScanMasterData.m_YmmToPixel) + std::ceil(std::abs(deltaHeight));

    oParameters.m_xMinLeft_mm = oParameters.m_ScanMasterData.m_XmmToPixel > 0 ? p_xMinLeft_mm : p_xMaxRight_mm ;
    oParameters.m_yMinTop_mm = oParameters.m_ScanMasterData.m_YmmToPixel> 0 ? p_yMinTop_mm : p_yMaxBottom_mm;
    if (deltaHeight < 0)
    {
        oParameters.m_yMinTop_mm += deltaHeight_mm;
    }
        
    oParameters.m_scanfieldimageSize = image::Size2d{expectedWidth, expectedHeight};
    return oParameters;
}

/*static*/ ScanFieldImageParameters ScanFieldImageParameters::computeParameters(const math::CalibrationParamMap & rCalibrationParameters) 
{
    double xMin = rCalibrationParameters.getDouble("SM_X_min");
    double xMax = rCalibrationParameters.getDouble("SM_X_max");
    double yMin = rCalibrationParameters.getDouble("SM_Y_min");
    double yMax = rCalibrationParameters.getDouble("SM_Y_max");
    int imageWidth = rCalibrationParameters.getInt("sensorWidth");
    int imageHeight = rCalibrationParameters.getInt("sensorHeight");
    return computeParameters(ScanMasterCalibrationData::load(rCalibrationParameters), imageWidth,  imageHeight, 
      xMin, xMax,  yMin, yMax);
}

/*static*/ ScanFieldImageParameters ScanFieldImageParameters::loadConfiguration(interface::Configuration p_configuration)
{
    ScanFieldImageParameters oResult;
    for (auto & rSmpKeyValue : p_configuration)
    {
        oResult.setKeyValue(rSmpKeyValue);
    }
    return oResult;
}


precitec::geo2d::DPoint ScanFieldImageParameters::getCenterInScanFieldImageBeforeMirroring(double xCenter_mm, double yCenter_mm) const
{
    double x_pix = ((xCenter_mm - m_xMinLeft_mm) * m_ScanMasterData.m_XmmToPixel);
    double y_pix = ((yCenter_mm - m_yMinTop_mm) * m_ScanMasterData.m_YmmToPixel) + m_ScanMasterData.m_Slope *x_pix;
    return {x_pix, y_pix};            
}

precitec::geo2d::DPoint ScanFieldImageParameters::getCenterInScanFieldImage(double xCenter_mm, double yCenter_mm) const
{
    auto center_pix = getCenterInScanFieldImageBeforeMirroring(xCenter_mm, yCenter_mm);
    return applyMirroringToPixelCoordinates(m_ScanMasterData.getImageFillMode(), center_pix, m_scanfieldimageSize.width, m_scanfieldimageSize.height);
}


geo2d::DPoint ScanFieldImageParameters::getScannerPositionFromScanFieldImage(double xCenter_pix, double yCenter_pix) const
{
    auto correctedPosition = applyMirroringToPixelCoordinates(m_ScanMasterData.getImageFillMode(), {xCenter_pix, yCenter_pix}, m_scanfieldimageSize.width, m_scanfieldimageSize.height);
    double xCenter_mm = correctedPosition.x / m_ScanMasterData.m_XmmToPixel + m_xMinLeft_mm;
    double yCenter_mm = (correctedPosition.y - m_ScanMasterData.m_Slope * correctedPosition.x ) / m_ScanMasterData.m_YmmToPixel + m_yMinTop_mm;
    return {xCenter_mm,  yCenter_mm};
}


geo2d::Point ScanFieldImageParameters::getTopLeftCornerInScanFieldImageBeforeMirroring(const precitec::image::Size2d & rLocalImageSize, double xCenter_mm, double yCenter_mm) const
{
    auto center_pix = getCenterInScanFieldImageBeforeMirroring(xCenter_mm, yCenter_mm);
    int xTopLeft_pix = center_pix.x - rLocalImageSize.width/2;
    int yTopLeft_pix = center_pix.y - rLocalImageSize.height/2;

    return {xTopLeft_pix, yTopLeft_pix};
}


bool ScanFieldImageParameters::saveToIni(std::string filename) const
{
    using interface::TKeyValue;
    
    std::ofstream oIniStream(filename);
    if (!oIniStream.is_open())
    {
        return false;
    }
        
    auto p_rConfiguration = makeConfiguration();
    
    //oIniStream << "[key_value_configuration]\n";           // abitrary root node name - not needed for reading

    for(auto it(std::begin(p_rConfiguration)); it != std::end(p_rConfiguration); ++it) {
        const Types oType	(it->get()->type());
        oIniStream << it->get()->key() <<  "=";
        switch (oType) { 
        case TBool: {
            const auto pKvInt	(static_cast<const TKeyValue<bool>*>(it->get()));
            oIniStream <<  pKvInt->value() <<  "\n";
            break;
        }
        case TInt: {
            auto pKvInt	(static_cast<const TKeyValue<int>*>(it->get()));
            oIniStream <<  pKvInt->value() <<  "\n";
            break;	
        }
        case TUInt: {
            const auto pKvInt	(static_cast<const TKeyValue<uint32_t>*>(it->get()));
            oIniStream <<  pKvInt->value() <<  "\n"; // NOTE: uint not possible, needs cast when read with getInt()
            break;
        }
        case TString: {
            const auto pKvString	(static_cast<const TKeyValue<std::string>*>(it->get()));
            oIniStream <<  pKvString->value() <<  "\n";
            break;
        }
        case TDouble: {
            const auto pKvDouble(static_cast<const TKeyValue<double>*>(it->get()));
            oIniStream << std::setprecision(15) <<  pKvDouble->value() <<  "\n";
            break;
        }
        default:
            std::ostringstream oMsg;
            oMsg << __FUNCTION__ << ": invalid value type: " << oType;
            return false;
        } // switch
    } // for
    
    oIniStream.close();

    return true;
}


bool ScanFieldGridParameters::includesOrigin() const
{
    return !( yMin > 0 || yMax < 0 || xMin > 0 || xMax < 0);
}


void ScanFieldGridParameters::forceDelta(double delta)
{
    if ( deltaX != delta || deltaY != delta)
    {
        wmLog(eWarning, "Bypassing provided delta %f %f, set to %f \n", deltaX, deltaY, delta);
        deltaX = delta;
        deltaY = delta;
    }
}


void ScanFieldGridParameters::forceSymmetry()
{
    if (yMin != -yMax || xMin != -xMax)
    {
        //force a symmetric scan
        double newX = std::min(std::abs(xMin), std::abs(xMax));
        double newY = std::min(std::abs(yMin), std::abs(yMax));
        std::stringstream oMsg;
        oMsg << "Bypassing provided bounds x " << xMin << " " << xMax << " y " << yMin << " " << yMax
             << ", set to " << -newX << " " << newX << " " << -newY << " " << newY << "\n";
        wmLog(eWarning, oMsg.str());
        xMin = -newX;
        xMax = newX;
        yMin = -newY;
        yMax = newY;
    }
}


void ScanFieldGridParameters::forceAcquireOnOrigin()
{
    // adjust the search, so that the camera will always see the circle on the target at the same position as in the reference image (0,0)

    int newX =  std::floor(-xMin/deltaX) * deltaX;
    int newY =  std::floor(-yMin/deltaY) * deltaY;
    if (-newX != xMin || -newY != yMin)
    {
        std::stringstream oMsg;
        oMsg << "Bypassing provided bounds x " << xMin << " " << xMax << " y " << yMin << " " << yMax
             << ", set to " << -newX << " " << newX << " " << -newY << " " << newY << "\n";
        wmLog(eWarning, oMsg.str());
        xMin = -newX;
        xMax = newX;
        yMin = -newY;
        yMax = newY;
    }
}


int ScanFieldGridParameters::numCols() const
{
    return (xMax - xMin)/deltaX;
}


int ScanFieldGridParameters::numRows() const
{
    return (yMax - yMin)/deltaY;
}


ScanFieldGridParameters::ScannerPositions ScanFieldGridParameters::computeScannerPositions(bool minimizeJump) const
{
    std::vector<ScannerPosition> scannerPositions;
    scannerPositions.reserve((numRows() +1 )* (numCols()+1));

    bool towardsRight = true;
    
    for (auto y = yMin, row=0.0; y <= yMax; y += deltaY, ++row)
    {
        std::vector<ScannerPosition> currentRow;
        
    
        for (auto x = xMin, column = 0.0; x <= xMax; x += deltaX, ++column)
        {
            currentRow.push_back({x,y,int(row),int(column)});
        }
        if (towardsRight)
        {
            for (auto itRow = currentRow.begin(); itRow != currentRow.end(); itRow++)
            {
                scannerPositions.push_back(*itRow);
            }
        }
        else
        {
            for (auto itRow = currentRow.rbegin(); itRow != currentRow.rend(); itRow++)
            {
                scannerPositions.push_back(*itRow);
            }
            
        }
        if (minimizeJump)
        {
            towardsRight = !towardsRight;
        }
            
    }
    return scannerPositions;
}


double ScanFieldGridParameters::xRange() const
{
    return xMax - xMin;
}


double ScanFieldGridParameters::yRange() const
{
    return yMax - yMin;
}




}
}
