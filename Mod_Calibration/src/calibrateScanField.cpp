#include "calibration/calibrateScanField.h"

#include "calibration/ScanFieldImageCalculator.h"
#include "calibration/chessboardRecognition.h" 
#include "math/calibrationCornerGrid.h"
#include "math/calibration3DCoordsInterpolator.h"
#include <calibration/calibrationManager.h>
#include <common/systemConfiguration.h>
#include <common/bitmap.h>
#include "common/systemConfiguration.h"
#include <calibration/calibrationGraph.h>
#include "util/CalibrationCorrectionContainer.h"
#include "filter/algoImage.h"
#include "coordinates/fieldDistortionMapping.h"
#include "coordinates/octCalibrationModel.h"

#include <fstream>
#include <filesystem>
#include <regex>

#include "CalibrationLibrary.h"
#define SLCL_ACTIVATE_PASSWORD 0xBFBE0E531E10CE76

namespace precitec
{
namespace calibration
{

class AcquireScanfieldRoutine 
{
public:
    AcquireScanfieldRoutine(ScanFieldImageParameters p_ScanfieldImageParameter, std::string p_outFolder, std::string p_scanfieldFolder);
    void processCurrentImage(const image::BImage & rImage, const ScanFieldGridParameters::ScannerPosition & rScannerPosition);
    BImage exportScanFieldImage();
    
private:
    struct ScanImageInfo 
    {
        ScanFieldGridParameters::ScannerPosition m_scannerPosition;
        geo2d::Point m_positionInScanFieldImage;
        std::string m_filename;
        bool m_saved;
        double mm_to_pix; // pixel per mm
        double error_pix;
        static void printHeader(std::ostream & os);
        void printCurrentRow(std::ostream & os);
    };
    
    std::string getImageFilename(const ScanFieldGridParameters::ScannerPosition & rScannerPosition ) const;  
    
    std::string m_debugFolder;
    std::string m_scanfieldImageFolder;
    bool m_exportDebugInfo;
    ScanFieldImageCalculator m_ScanFieldImageCalculator;
    std::ofstream m_info;
                                
    
};

class IDMMeasurements
{
    public:
        
        IDMMeasurements(const ScanFieldGridParameters & oScanFieldGrid, bool minimizeJump);
        int getNumberOfPositions() const;
        const ScanFieldGridParameters::ScannerPosition & getScannerPosition(int index) const;
        void renderBackgroundImage(calibration::CalibrationManager & m_rCalibrationManager) const;
        void addMeasurements(int index, std::vector<double> oIDMRawMeasurement);
        void renderMeasurementPoint(ScanFieldGridParameters::ScannerPosition rScannerPosition, calibration::CalibrationManager & m_rCalibrationManager,
                                    std::string labelTop, std::string labelBottom ) const;

        BImage computeDepthImageFromCalibratedValues(CalibrationManager * pCalibrationManager = nullptr,byte rangeGray = 255, double rangeZ = std::numeric_limits<double>::signaling_NaN()) const;
        BImage computeDepthImageFromRawValues(CalibrationManager * pCalibrationManager = nullptr) const;

        void computeMedianValues();
        void computeCalibratedValues(const math::CalibrationData & rCalibrationData);
        void computeCalibratedValues(coordinates::CalibrationIDMCorrectionContainer & rCalibrationIDMCorrectionContainer);
        image::BImage renderDepthImage(bool m_showCorrectedIDM_Z, calibration::CalibrationManager & m_rCalibrationManager)  const;
        coordinates::CalibrationIDMCorrectionContainer computeCorrectionContainer();
        bool saveIDMCorrectionFile(std::string filename) const;

        static double computeMedian(IDMAcquisition::RawIdmMeasurements values);
    private:
        double xDepthImageToScanner(int col) const;
        double yDepthImageToScanner(int row) const;
        
        double xScannerToDepthImage(double x) const;
        double yScannerToDepthImage(double y) const;
        BImage computeDepthImageWithInterpolation(
            const std::vector<ScanFieldGridParameters::ScannerPosition> & rScannerPositions, const std::vector<double> & values) const;
        BImage computeDepthImageWithoutInterpolation(
            const std::vector<ScanFieldGridParameters::ScannerPosition> & rScannerPositions, const std::vector<double> & values,
                byte rangeGray = 255, double rangeZ = std::numeric_limits<double>::signaling_NaN()) const;

        
        const ScanFieldGridParameters m_oScanFieldGridParameters;
        const ScanFieldGridParameters::ScannerPositions m_scannerPositions;
        std::vector<IDMAcquisition::RawIdmMeasurements> m_rawMeasurements;
        
        std::vector<double> m_calibratedValues;
        std::vector<double> m_medianValues;
        std::vector<std::pair<geo2d::DPoint, coordinates::CalibrationIDMCorrection>> m_offsets;
        image::Size2d m_imageSize;
        BImage m_oBackgroundImage;

};


CalibrationGraphScanmaster::CalibrationGraphScanmaster(CalibrationManager & rCalibrationManager)
    : m_oCircleDetectionGraph(rCalibrationManager, "GraphCalibrationScanField.xml")
{
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            m_hasCamera = interface::SystemConfiguration::instance().getBool("HasCamera", true);
        }
        else
        {
            m_hasCamera = false;
        }
    }
    else
    {
        m_hasCamera = false;
    }
    printf("m_hasCamera: %d\n", m_hasCamera);
}


void CalibrationGraphScanmaster::setUseGridRecognition(bool value)
{
    m_useGridRecognition = value;
}


void CalibrationGraphScanmaster::resetIndexImageFromDisk()
{
    m_oCircleDetectionGraph.m_rCalibrationManager.setIndexForNextImageFromDisk(0);
}
    
bool CalibrationGraphScanmaster::init(double tol_pix, std::array<int,4> searchROI, int borderGridSearch, int sideGridSearch, int expectedRadius, int numRepetitions)
{

    // 1. Set parameters
    m_treshold = 0.9;
    m_tol_pix = tol_pix;
    m_numRepetitions = numRepetitions;
    int radiusMin = 0.8*expectedRadius;
    int radiusMax = 1.2*expectedRadius;
    bool ok = setRoi(searchROI[0], searchROI[1], searchROI[2], searchROI[3])
        && setGridSearch(borderGridSearch, sideGridSearch) 
        && setSearchRadius(radiusMin, radiusMax, 10);
    if (!ok)
    {
        return false;
    }
       
    // 2. Iteratively recognize the radius size [pixel] from image 
    
    resetIndexImageFromDisk(); //no camera: evaluate the first image in wm_inst/images
 
    m_oReferenceCircle = fitCircle();
    resetIndexImageFromDisk(); //no camera: graph processing has increased the image index, but we need re-evaluate the first image (00_center.bmp) in wm_inst/images 
    
    radiusMin = std::max( 0.0, m_oReferenceCircle.r -15);
    radiusMax = m_oReferenceCircle.r + 15;
    ok = setSearchRadius(radiusMin, radiusMax, 1);
    m_oReferenceCircle = fitCircle();
    resetIndexImageFromDisk(); //no camera: graph processing has increased the image index, but we need re-evaluate the first image (00_center.bmp) in wm_inst/images 
    
    if (m_extraDebug)
    {
        wmLog(eDebug, "Candidate reference Circle at %f, %f with radius %f (%f)\n", 
          m_oReferenceCircle.x, m_oReferenceCircle.y, m_oReferenceCircle.r, m_oReferenceCircle.p);
    }   
    
    
    if (m_oReferenceCircle.p < m_treshold)
    {    
        return false;
    }
        
    ok = setSearchRadius(m_oReferenceCircle.r - 5, m_oReferenceCircle.r + 5, 1);
    
    // at this point the circle size and position in pixels is known
    //refine the reference position with the correct search radius
    resetIndexImageFromDisk(); //no camera: evaluate the first image in wm_inst/images
    m_oReferenceCircle = fitCircle();
    return true;
}

std::pair<bool, coordinates::CalibrationCameraCorrection>  CalibrationGraphScanmaster::evaluateTranslation()
{
    setSearchOutsideROI(true);
    auto oEvaluationResult = evaluateCircleFit();
    setSearchOutsideROI(false);
    
    return oEvaluationResult;
}

std::pair<bool, IDMAcquisition::RawIdmMeasurements> IDMAcquisition::evaluateIDMWeldingDepth(int trigger_ms)
{    
    if (m_extraDebug)
    {
        wmLog(eDebug, "evaluateIDMWeldingDepth(trigger_ms=%d)\n", trigger_ms);
    }
    RawIdmMeasurements measuredZ(m_numRepetitions,0);
    int numValidResults = 0;
    
    int sampleSensorId = interface::eIDMWeldingDepth;
    
    std::vector<image::Sample> oIDMSamples;
    oIDMSamples = m_rCalibrationManager.getSamples(sampleSensorId, m_numRepetitions, trigger_ms, 5000 );
    if ((int)(oIDMSamples.size()) != m_numRepetitions)
    {
        wmLogTr(eError, "QnxMsg.Calib.NoSample", "Calibration failed, no sample available -- check hardware.\n");
        return {false, {}};
    }
    
    for (int i = 0; i < m_numRepetitions; i++)
    {
        wmLog(eDebug, "Prepare repetition %d/%d\n", i+1, m_numRepetitions);
        std::string title = "Measure IDM " + std::to_string(i);
        
        Sample oSample = oIDMSamples[i];
        if (oSample.getSize() == 0)
        {
            wmLogTr(eError, "QnxMsg.Calib.NoSample", "Calibration failed, no sample available -- check hardware.\n");
            return {false, {}};
        }
        
        measuredZ[i] = oSample[0];
        numValidResults ++;
    
        if (m_extraDebug)
        {
            wmLog(eDebug, "Repetition %d/%d: measuredZ = %f \n", i+1, m_numRepetitions, measuredZ[i]);
        }
    }
    if (numValidResults == 0)
    { 
        return {false, {} };
    }
    return {true, measuredZ };
}

std::pair<bool, coordinates::CalibrationCameraCorrection> CalibrationGraphScanmaster::evaluateCircleFit()
{
    auto curCircle = fitCircle();
    if (curCircle.p < m_treshold)
    {
        return {false, {}};
    }
    return {true, { curCircle.x - m_oReferenceCircle.x, curCircle.y - m_oReferenceCircle.y} };
}

std::tuple<bool, coordinates::CalibrationCameraCorrection, CalibrationGraphScanmaster::CircleResult> CalibrationGraphScanmaster::evaluateAll()
{
    auto curCircle = fitCircle();
    if (curCircle.p < m_treshold)
    {
        return {false, {}, curCircle};
    }

    return {true, { curCircle.x - m_oReferenceCircle.x, curCircle.y - m_oReferenceCircle.y}, curCircle};
}

double CalibrationGraphScanmaster::getRatio_pix_mm_from_Grid(double gridSide_mm)  const
{
    if (!m_useGridRecognition)
    {
        wmLog(eWarning, "Grid measurement requested, but grid recognition was not active \n");
    }
    double meanLength = (geo2d::distance(m_oReferenceCircle.getTopLeftCorner(), m_oReferenceCircle.getTopRightCorner())
                    + geo2d::distance(m_oReferenceCircle.getTopRightCorner(), m_oReferenceCircle.getBottomRightCorner())
                    + geo2d::distance(m_oReferenceCircle.getBottomRightCorner(), m_oReferenceCircle.getBottomLeftCorner())
                    + geo2d::distance(m_oReferenceCircle.getBottomLeftCorner(), m_oReferenceCircle.getTopLeftCorner()))/4.0;
    return meanLength / gridSide_mm;
}


double CalibrationGraphScanmaster::getRatio_pix_mm_from_Circle(double circleRadius_mm)  const
{
    return m_oReferenceCircle.r / circleRadius_mm;
}

bool CalibrationGraphScanmaster::setRoi(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    using fliplib::Parameter;

    std::shared_ptr<fliplib::FilterGraph> pGraph = m_oCircleDetectionGraph.getGraph();
    fliplib::BaseFilter* pFilter = pGraph->find(Poco::UUID(m_GraphFilterInstancesUUID.m_searchROI)); 
    if(pFilter== nullptr)
    {
        wmLog(eWarning,  "instance filter id of search ROI missing in ScanCalibrationGraph \n");
        return false;
    }

    pFilter->getParameters().update("X", Parameter::TYPE_UInt32, x);
    pFilter->getParameters().update("Y", Parameter::TYPE_UInt32, y);
    pFilter->getParameters().update("Width", Parameter::TYPE_UInt32, w);
    pFilter->getParameters().update("Height", Parameter::TYPE_UInt32, h);

    analyzer::ParameterSetter oParamSetter;
    pGraph->control(oParamSetter);
    return true;

}

bool CalibrationGraphScanmaster::setSearchRadius(int radiusMin, int radiusMax, int step)
{
    wmLog(eInfo, "Set search radius %d %d \n", radiusMin, radiusMax );
    using fliplib::Parameter;

    std::shared_ptr<fliplib::FilterGraph> pGraph = m_oCircleDetectionGraph.getGraph();
    fliplib::BaseFilter* pFilterCircle = pGraph->find(Poco::UUID(m_GraphFilterInstancesUUID.m_circleDetection)); 
    fliplib::BaseFilter* pFilterRadiusStart = pGraph->find(Poco::UUID(m_GraphFilterInstancesUUID.m_radiusStart)); 
    fliplib::BaseFilter* pFilterRadiusEnd = pGraph->find(Poco::UUID(m_GraphFilterInstancesUUID.m_radiusEnd)); 
    if(pFilterCircle== nullptr || pFilterRadiusStart == nullptr || pFilterRadiusEnd == nullptr)
    {
        wmLog(eWarning,  "instance filter id of circle detection missing in ScanCalibrationGraph \n");
        return false;
    }

    pFilterRadiusStart->getParameters().update("scalar", Parameter::TYPE_double, double(radiusMin));
    pFilterRadiusEnd->getParameters().update("scalar", Parameter::TYPE_double, double(radiusMax));
    pFilterCircle->getParameters().update("RadiusStep", Parameter::TYPE_double, double(step));
    
    analyzer::ParameterSetter oParamSetter;
    pGraph->control(oParamSetter);
    return true;
}


bool CalibrationGraphScanmaster::setSearchOutsideROI(bool value)
{
    using fliplib::Parameter;

    std::shared_ptr<fliplib::FilterGraph> pGraph = m_oCircleDetectionGraph.getGraph();
    fliplib::BaseFilter* pFilterCircle = pGraph->find(Poco::UUID(m_GraphFilterInstancesUUID.m_circleDetection)); 
    if(pFilterCircle== nullptr)
    {
        wmLog(eWarning,  "instance filter id of circle detection missing in ScanCalibrationGraph \n");
        return false;
    }

    pFilterCircle->getParameters().update("SearchOutsideROI", Parameter::TYPE_bool, value);
    
    analyzer::ParameterSetter oParamSetter;
    pGraph->control(oParamSetter);
    return true;
}


bool CalibrationGraphScanmaster::setGridSearch(int border, int roiShortSide)
{
    wmLog(eInfo, "Set grid radius border %d roi short side %d \n", border, roiShortSide );
    using fliplib::Parameter;

    std::shared_ptr<fliplib::FilterGraph> pGraph = m_oCircleDetectionGraph.getGraph();
    fliplib::BaseFilter* pFilterBorder = pGraph->find(Poco::UUID(m_GraphFilterInstancesUUID.m_border)); 
    fliplib::BaseFilter* pFilterShortSide = pGraph->find(Poco::UUID(m_GraphFilterInstancesUUID.m_roiShortSide)); 
    if(pFilterBorder== nullptr || pFilterShortSide == nullptr)
    {
        wmLog(eWarning,  "instance filter id of grid search filters missing in ScanCalibrationGraph \n");
        return false;
    }

    pFilterBorder->getParameters().update("scalar", Parameter::TYPE_double, double(border));
    pFilterShortSide->getParameters().update("scalar", Parameter::TYPE_double, double(roiShortSide));

    
    analyzer::ParameterSetter oParamSetter;
    pGraph->control(oParamSetter);
    return true;
}

void CalibrationGraphScanmaster::setRoiAroundCircle(CircleResult p_circle, int border)
{
    auto radius = p_circle.r + border;
    auto x = p_circle.x - radius;
    auto y = p_circle.y - radius;
    setRoi(x, y, 2*radius, 2*radius);
}

CalibrationGraphScanmaster::CircleResult CalibrationGraphScanmaster::fitCircle() 
{
    bool oShowImage = true;
    
    auto f_convertResults = [this](std::vector< interface::ResultArgs* > & p_results){
        
        CircleResult oCircle;
        auto & rSquare = oCircle.square;
        int numValidResults = 0;
        int numValidCircleResults = 0;
        for (auto & rResult : p_results)
        {
            if (!rResult->isValid() || rResult->type() != interface::RegDoubleArray )
            {
                continue;
            }
            
            auto values = rResult->value<double>();
            if (values.size() != 1)
            {
                continue;
            }
            //take only the first value 
            auto value = values[0];
            if (rResult->rank()[0] != filter::eRankMax)
            {
                continue;
            }
            numValidResults++;
            switch( (int)(rResult->resultType()))
            {
                case GraphResult::id_x : oCircle.x = value;  numValidCircleResults++; break;
                case GraphResult::id_y : oCircle.y = value; numValidCircleResults++; break;
                case GraphResult::id_r : oCircle.r = value; numValidCircleResults++; break;   
                case GraphResult::id_x1 : rSquare[0].x = value; break;
                case GraphResult::id_y1 : rSquare[0].y = value; break;   
                case GraphResult::id_x2 : rSquare[1].x = value; break;
                case GraphResult::id_y2 : rSquare[1].y = value; break;   
                case GraphResult::id_x3 : rSquare[2].x = value; break;
                case GraphResult::id_y3 : rSquare[2].y = value; break;   
                case GraphResult::id_x4 : rSquare[3].x = value; break;
                case GraphResult::id_y4 : rSquare[3].y = value; break;   
                default: numValidResults--; break;
            }            
        }
        
        if ( numValidCircleResults == 3 && (!m_useGridRecognition || numValidResults == 11))
        {
            if (m_useGridRecognition)
            {
                // compare the center of the fitted circle with the center of the enclosing square
                
                geo2d::DPoint oCenterEnclosingSquare = {
                    (rSquare[0].x + rSquare[1].x + rSquare[2].x + rSquare[3].x) / 4.0,
                    (rSquare[0].y + rSquare[1].y + rSquare[2].y + rSquare[3].y) / 4.0,
                };
                double maxNorm = std::max(std::abs(oCenterEnclosingSquare.x - oCircle.x), std::abs(oCenterEnclosingSquare.y - oCircle.y));
                
                oCircle.p = maxNorm >= m_tol_pix ? 0.0 : 1.0; 
                if (oCircle.p < m_treshold)
                {
                    wmLog(eInfo, "Too much distance  ( score %f) between circle center %f %f and grid center %f %f\n",
                        oCircle.p , oCircle.x, oCircle.y, oCenterEnclosingSquare.x, oCenterEnclosingSquare.y);
                }                
            }
            else
            {                
                oCircle.p = 1.0;
            }
        }
        else
        {
            oCircle.p = 0.0;
        }
        return std::pair<CircleResult, EnclosingSquare>{oCircle, rSquare};
        
    };

    CircleResult bestMatch;
    bestMatch.p = 0.0;
    for (int i = 0; i < m_numRepetitions; i++)
    {
        auto results = m_oCircleDetectionGraph.execute(oShowImage, "Fit circle " + std::to_string(i), interface::eImageSensorDefault);
        auto curResult = f_convertResults(results);
        auto & curCircle = curResult.first;
        if (curCircle.p > bestMatch.p)
        {
            bestMatch = curCircle;
        }
        //TODO average?
    }
    return bestMatch;
};


geo2d::DPoint CalibrationGraphScanmaster::getReferencePosition() const
{
    return {m_oReferenceCircle.x, m_oReferenceCircle.y};
}

CalibrateScanField::CalibrateScanField( CalibrationManager& p_rCalibrationManager )
    : CalibrationProcedure ( p_rCalibrationManager )
{
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            m_oHasCamera = interface::SystemConfiguration::instance().getBool("HasCamera", true);
        }
        else
        {
            m_oHasCamera = false;
        }
    }
    else
    {
        m_oHasCamera = false;
    }
}

CalibrateScanField::~CalibrateScanField()
{
}

bool CalibrateScanField::calibrate()
{
    assert(!m_rCalibrationManager.isSimulation() && "Weldhead device proxy not activated");
    if (!interface::SystemConfiguration::instance().get(interface::SystemConfiguration::BooleanKey::Scanner2DEnable))
    {
        wmLog(eWarning, "CalibrationProcedureScanField requested, but %s is false\n", interface::SystemConfiguration::keyToString(interface::SystemConfiguration::BooleanKey::Scanner2DEnable));
    }
        
    bool valid = prepareExportFolder();

    if (!valid)
    {
        return false;
    }
    
    double systemJumpSpeed = m_rCalibrationManager.getJumpSpeed();
    m_rCalibrationManager.setJumpSpeed(0.1);
    bool result = false;
    bool calibDataChanged = false;
    switch (m_calibrationType)
    {
        case interface::CalibType::eAcquireScanFieldImage:
            result = acquireScanFieldImage();
            break;
        case interface::CalibType::eCalibrateScanFieldTarget:
            result = calibrateCameraTarget();
            calibDataChanged = result;
            break;
        case interface::CalibType::eCalibrateScanFieldIDM_Z:
            result = calibrateIDMSurface(true);
            calibDataChanged = result;
            break;
        case interface::CalibType::eVerifyScanFieldIDM_Z:
            result = calibrateIDMSurface(false);
            break;
        case interface::CalibType::eScannerCalibrationMeasure:
            result = calibrateScanner();
            calibDataChanged = result;
            break;
        case interface::CalibType::eScanmasterCameraCalibration:
            result = calibrateCamera();
            calibDataChanged = result;
            break;
        default:
            assert(false);
            wmLog(eError, "Unexpected calibration type %d \n", m_calibrationType);
            result = false;
            break;
    }
    m_rCalibrationManager.setJumpSpeed(systemJumpSpeed);
    
    if (calibDataChanged)
    {
        int oSensorID = 0;
        m_rCalibrationManager.sendCalibDataChangedSignal(oSensorID, true);
        wmLogTr(eInfo, "QnxMsg.Calib.CalibOK", "Calibration successfully completed!\n");
    }
    return result;
}


/*static*/ void AcquireScanfieldRoutine::ScanImageInfo::printHeader(std::ostream & os)
{
    os << "scan x [mm];scan y [m];filename;saved;"
        << "x top left in scan image[pix];y top left in scan field[pix];"
        <<"mm_to_pix; error chessboard [pix] \n";
}

void AcquireScanfieldRoutine::ScanImageInfo::printCurrentRow(std::ostream & os)
{
    os << m_scannerPosition.x << ";"<< m_scannerPosition.y <<";" << m_filename << ";" << m_saved << ";" 
    << m_positionInScanFieldImage.x << ";" << m_positionInScanFieldImage.y << ";"
    << mm_to_pix << ";" << error_pix << ";" 
    << "\n";       
};
        
std::string AcquireScanfieldRoutine::getImageFilename(const ScanFieldGridParameters::ScannerPosition & rScannerPosition ) const
{
    
    std::ostringstream oFilename;
    oFilename << std::setfill('0') << std::setw(2) << m_debugFolder << "row_" << rScannerPosition.row << "_col_" << rScannerPosition.column << ".bmp";
    return oFilename.str();
}
        
void AcquireScanfieldRoutine::processCurrentImage( const BImage & rImage, const ScanFieldGridParameters::ScannerPosition & rScannerPosition)
{
    ScanImageInfo oCurrentImageInfo{rScannerPosition};
  
    oCurrentImageInfo.m_filename = getImageFilename(rScannerPosition);
    oCurrentImageInfo.m_saved = false;
    if (m_exportDebugInfo)
    {
        fileio::Bitmap oBitmap(oCurrentImageInfo.m_filename, rImage.width(), rImage.height(),false);
        oCurrentImageInfo.m_saved =  oBitmap.isValid() & oBitmap.save(rImage.data());
        
    }
    
    auto oPositionInScanfieldImage = m_ScanFieldImageCalculator.getParameters().getTopLeftCornerInScanFieldImageBeforeMirroring(rImage.size() ,rScannerPosition.x,rScannerPosition.y);

    bool ok = m_ScanFieldImageCalculator.pasteImage(rImage,oPositionInScanfieldImage.x, oPositionInScanfieldImage.y);
    if (!ok)
    {
        wmLog(eInfo, "Error pasting image %d %d \n",oPositionInScanfieldImage.x, oPositionInScanfieldImage.y);
    }

    oCurrentImageInfo.m_positionInScanFieldImage = oPositionInScanfieldImage;
    
    if (m_exportDebugInfo)
    {
        oCurrentImageInfo.printCurrentRow(m_info);
    }
        
}


AcquireScanfieldRoutine::AcquireScanfieldRoutine(ScanFieldImageParameters p_ScanfieldImageParameter, 
                                                 std::string p_outFolder, std::string p_scanfieldFolder)
    : m_debugFolder(p_outFolder), 
    m_scanfieldImageFolder(p_scanfieldFolder),
    m_exportDebugInfo(!m_debugFolder.empty()), 
    m_ScanFieldImageCalculator(p_ScanfieldImageParameter)
    {
        if (m_exportDebugInfo)
        {
            m_info.open(m_debugFolder + "info.txt");
            ScanImageInfo::printHeader(m_info);
        }
    }

BImage AcquireScanfieldRoutine::exportScanFieldImage()
{
    if (m_scanfieldImageFolder.empty())
    {
        wmLog(eDebug, "Debug or ScanfieldImagePath not selected, scanfield image will not be exported\n");    
    }

    wmLog(eDebug, std::string("Save final image to ") + m_scanfieldImageFolder + "\n");
    
    BImage oImage;
    std::string outputFolder;
    std::string configFile;
    std::tie(oImage, outputFolder, configFile) = m_ScanFieldImageCalculator.computeAndWriteScanFieldImage(m_scanfieldImageFolder);
    
    if (outputFolder.empty())
    {
        wmLog(eWarning, std::string("Could not save final image to ") + m_scanfieldImageFolder + "\n");
    }
    
    return oImage;
}

bool CalibrateScanField::prepareExportFolder()
{
    int oSensorId = 0;
    auto & rCalibrationParameters = m_rCalibrationManager.getCalibrationData(oSensorId).getParameters();
    
    std::string oProvidedImageFolderPath = m_rCalibrationManager.getScanFieldPath();
    if (m_calibrationType == interface::CalibType::eCalibrateScanFieldTarget)
    {
        oProvidedImageFolderPath = ""; //avoid overwriting the last seamseries image
    }
    bool isPreviewScanfieldImage = oProvidedImageFolderPath == "preview"; // see ScanfieldCalibrationController::startAcquireScanFieldImage()
    bool exportDebugInfo = rCalibrationParameters.getInt("Debug") > 0
                            || m_calibrationType == interface::CalibType::eVerifyScanFieldIDM_Z
                            || isPreviewScanfieldImage;
        
    std::string oWMBaseDir;
    if ( getenv( "WM_BASE_DIR" ) )
    {
        oWMBaseDir = std::string( getenv( "WM_BASE_DIR" ) );
    }
    else
    {
        oWMBaseDir = std::string( "/tmp" );
    }

    std::string oDebugFolderBase = oWMBaseDir + "/data/scanfieldimage/";
    std::string oConfigFolderBase = oWMBaseDir + "/config/scanfieldimage/";

    //create folder for intermediate images    
    m_oDebugFolder = "";
    if (exportDebugInfo)
    {
        std::stringstream ssTime;
        {
            // get date
            char dateString[32];
            time_t m_TheTime = time(0);
            strftime(dateString, 30, "%Y%m%d_%H%M%S", localtime(&m_TheTime));
            ssTime << dateString;
        }
        
        Poco::File	oDestDir(oDebugFolderBase + ssTime.str() +"/");
        if ( !oDestDir.exists())
        {
            oDestDir.createDirectories();
            std::ostringstream oMsg;
            oMsg << __FUNCTION__ << ": Directory created: " << "'" << oDestDir.path() << "'\n";
            wmLog( eDebug, oMsg.str() );
        } 
        if (!oDestDir.exists())
        {
            wmLog( eError, "Could not create folder %s \n", oDestDir.path().c_str());
            return false;
        }
        m_oDebugFolder = oDestDir.path().c_str();
        if (m_oDebugFolder.back() != '/')
        {
            m_oDebugFolder += "/";
        }
    }
    

    //create folder for ScanFieldImage:
    
    //if an image folder path has been explicitly provided, save the image in config folder
    //if not, use a the debug folder
    if (oProvidedImageFolderPath.length() > 0)
    {
        if (oProvidedImageFolderPath.back() !=  '/')
        {
            oProvidedImageFolderPath +=  "/";
        }
        m_oScanFieldImageFolder = oConfigFolderBase + oProvidedImageFolderPath; 
        
        wmLog(eDebug, "Create folder " + m_oScanFieldImageFolder + " \n");
        
        Poco::File oImageFolder(m_oScanFieldImageFolder);
        if (!oImageFolder.exists())
        {
            oImageFolder.createDirectories();
            std::ostringstream oMsg;
            oMsg << __FUNCTION__ << ": Directory created: " << "'" << oImageFolder.path() << "'\n";
            wmLog( precitec::eDebug, oMsg.str().c_str() );
        }
        if (!oImageFolder.exists())
        {
            wmLog( eError, "Could not create folder %s \n", oImageFolder.path().c_str());
            m_oDebugFolder = "";
            m_oScanFieldImageFolder = "";
            return false;
        }
    }
    else
    {
       m_oScanFieldImageFolder = m_oDebugFolder;
    }
    
    return true;
}

ScanFieldGridParameters CalibrateScanField::loadScanFieldGridParameters() const
{
    using namespace interface;
    const int oSensorID = 0;
    const auto & rCalibrationParameters = m_rCalibrationManager.getCalibrationData(oSensorID).getParameters();
    ScanFieldGridParameters result;
    result.xMin = rCalibrationParameters.getDouble("SM_X_min");
    result.xMax = rCalibrationParameters.getDouble("SM_X_max");
    result.yMin = rCalibrationParameters.getDouble("SM_Y_min");
    result.yMax = rCalibrationParameters.getDouble("SM_Y_max");
    switch (m_calibrationType)
    {
        case CalibType::eAcquireScanFieldImage:
        case CalibType::eCalibrateScanFieldTarget:
        case CalibType::eVerifyScanFieldIDM_Z:
            result.deltaX = rCalibrationParameters.getDouble("SM_deltaX");
            result.deltaY = rCalibrationParameters.getDouble("SM_deltaY");
            break;
        case CalibType::eCalibrateScanFieldIDM_Z:
            result.deltaX = rCalibrationParameters.getDouble("SM_IDMdeltaX");
            result.deltaY = rCalibrationParameters.getDouble("SM_IDMdeltaY");
            break;
        default:
            wmLog(eWarning,"Unexpected calibration type %d in loadScanFieldGridParameters \n", m_calibrationType);
            assert(false);
            break;
    }
    return result;
}

bool CalibrateScanField::calibrateCameraTarget() const
{
    const int oSensorID = 0;
    const auto & rCalibrationDataParams = m_rCalibrationManager.getCalibrationData(oSensorID).getParameters();
    
    bool extraDebug = rCalibrationDataParams.getInt("Debug") > 0;
    
    CalibrationGraphScanmaster oCalibrationGraph(m_rCalibrationManager);
    oCalibrationGraph.setExtraDebug(extraDebug);
    
    oCalibrationGraph.setNumRepetitions(rCalibrationDataParams.getInt("SM_CalibRoutineRepetitions"));
    
    //acquire the first image just to get the size
    auto oImageSize = m_rCalibrationManager.getImage().size();
    
    if (oImageSize.area() == 0)
    {
        wmLog(eError, "Invalid image, interrupt calibration \n"); 
        return false;
    }
    m_rCalibrationManager.setIndexForNextImageFromDisk(0);
    

    ////////////////////////////////////////////////////////////////////////////
    // 1 check the calibration parameters
    ///////////////////////////////////////////////////////////////////////////
    const bool useGridRecognitionParameter = rCalibrationDataParams.getBool("SM_UseGridRecognition");
    if (!useGridRecognitionParameter)
    {
        wmLog(eWarning, "Using debug setting: grid recognition disabled \n");
    }

    oCalibrationGraph.setUseGridRecognition(useGridRecognitionParameter);

    ScanFieldGridParameters oScanFieldGrid = loadScanFieldGridParameters();

    if (!oScanFieldGrid.includesOrigin())
    {
        wmLog(eError, "The scanner origin 0,0 must be included in the calibration procedure in order to update the system calibration\n");
        return false;
    }
    
    double gridSide_mm = rCalibrationDataParams.getDouble("SM_EnclosingSquareSize_mm");
    oScanFieldGrid.forceDelta(gridSide_mm);
    oScanFieldGrid.forceSymmetry();
    
    // adjust the search, so that the camera will always see the circle on the target at the same position as in the reference image (0,0)
    oScanFieldGrid.forceAcquireOnOrigin();
    
    ////////////////////////////////////////////////////////////////////////////
    // 2 use the image at the scanner center for reference
    ///////////////////////////////////////////////////////////////////////////
    
    m_rCalibrationManager.setScannerPosition(0.0, 0.0); 

    m_rCalibrationManager.setIndexForNextImageFromDisk(0);
    auto oImage = m_rCalibrationManager.getImage();
    auto oImageContext(m_rCalibrationManager.getTriggerContext());

    m_rCalibrationManager.clearCanvas();
    m_rCalibrationManager.drawText(10, 10, "Scanner origin 0,0", Color::m_oOrangeDark);
    m_rCalibrationManager.renderImage(oImage);
    
    if (!oImage.isValid())
    {
        wmLog(eError, "Invalid image, interrupt calibration \n"); 
        return false;
    }
    
    //compute reference position for the calibration
    std::array<int,4> searchROI = { rCalibrationDataParams.getInt("SM_searchROI_X"),
        rCalibrationDataParams.getInt("SM_searchROI_Y"),
        rCalibrationDataParams.getInt("SM_searchROI_W"),
        rCalibrationDataParams.getInt("SM_searchROI_H")};

    //the keyvalues are defined on the full sensor image, correct them according to the current HW ROI
    searchROI[0] -= oImageContext.HW_ROI_x0;
    searchROI[1] -= oImageContext.HW_ROI_y0;
            
    bool referenceCircleFound =  oCalibrationGraph.init(rCalibrationDataParams.getInt("SM_DetectionTolerance_pix"), 
                                                searchROI, 
                                                rCalibrationDataParams.getInt("SM_searchGridBorder"), 
                                                rCalibrationDataParams.getInt("SM_searchGridSide"),
                                                rCalibrationDataParams.getInt("SM_CircleRadius_pix"),
                                                1);

    saveDebugImage("00_center");

    if (!referenceCircleFound)
    {
        wmLog(eWarning, "No reference circle found \n");
        return false;
    }
    


    const auto oColorReference = Color::Magenta();

    auto drawOffset = [this, &oCalibrationGraph, & oColorReference](double dx, double dy)
    {
        auto oReferencePosition = oCalibrationGraph.getReferencePosition();

        geo2d::Point oPaintReferencePosition (std::round(oReferencePosition.x), std::round(oReferencePosition.y)); ;
        geo2d::Point oPaintTranslatedPosition (std::round(oReferencePosition.x + dx), std::round(oReferencePosition.y + dy)) ;
        m_rCalibrationManager.drawCross(oPaintReferencePosition.x, oPaintReferencePosition.y, 10, oColorReference );
        m_rCalibrationManager.drawLine(oPaintReferencePosition.x, oPaintReferencePosition.y, oPaintTranslatedPosition.x, oPaintTranslatedPosition.y, oColorReference );
    };


    //no camera: initialization could have increased the image index multiple times during the iterative search,
    // but we need to be sure to evaluate the second image in wm_inst/images in the next step (01_x_*.bmp)
    m_rCalibrationManager.setIndexForNextImageFromDisk(1);

    ////////////////////////////////////////////////////////////////////////////
    // 3 evaluate the scanner direction
    ////////////////////////////////////////////////////////////////////////////

    bool invertX = true;
    bool invertY = true;
    double slope = 0.0;
    double pix_mm_x = 0;
    double pix_mm_y = 0;

    if (rCalibrationDataParams.getBool("SM_CalibrateAngle"))
    {
        auto renderLastStepResult = [this](std::string title)
        {
            m_rCalibrationManager.drawText(10, 30, title, Color::m_oOrangeDark);
            m_rCalibrationManager.renderCanvas();
        };
        
        
        enum class ScannerDirection {x,y};
        struct ScannerDirectionEvaluation
        {
            bool valid;
            double dx_pix;
            double dy_pix;
            double pix_mm;
            
        };
        
        auto evaluateScannerDirection = [this, &oCalibrationGraph, &extraDebug, &gridSide_mm,
                            &renderLastStepResult, &drawOffset, & oColorReference] (ScannerDirection dir, double k, std::string label)
        {
            
            double kx = 0;
            double ky = 0;
            switch (dir)
            {
                case ScannerDirection::x:
                    kx = k; 
                    ky = 0;
                    break;
                case ScannerDirection::y:
                    kx = 0; 
                    ky = k;
                    break;
            }
            double scanner_x = kx * gridSide_mm;
            double scanner_y = ky * gridSide_mm;
            if (kx != 0)
            {
                label += "x_" + std::to_string(scanner_x);
            }
            if (ky != 0)
            {
                label += "y_" + std::to_string(scanner_y);
            }
            
            m_rCalibrationManager.clearCanvas();
            
            
            m_rCalibrationManager.setScannerPosition(scanner_x, scanner_y); 
            
            auto oEvaluationResult = oCalibrationGraph.evaluateTranslation();
    
            saveDebugImage(label);
                    
            renderLastStepResult("Calibrate Angle " + label);
            
            
            if (extraDebug)
            {
                wmLog(eDebug, " (scanner %f %f, target movement %f %f) \n",
                    scanner_x, scanner_y, oEvaluationResult.second.m_oTCPXOffset, oEvaluationResult.second.m_oTCPYOffset ); 
            }
            
            std::ostringstream oMsg;
            oMsg << "Scanner " << scanner_x << " " << scanner_y ;
            m_rCalibrationManager.drawText(10, 10, oMsg.str(), Color::m_oOrangeDark);
            
            
            if (!oEvaluationResult.first)
            {
                m_rCalibrationManager.renderCanvas();
                wmLog(eError, "Could not process image at %f %f, interrupt calibration \n", scanner_x, scanner_y); 
                return ScannerDirectionEvaluation{false, 0,0};
            } 
            
        
            auto dx = oEvaluationResult.second.m_oTCPXOffset;
            auto dy = oEvaluationResult.second.m_oTCPYOffset;
            
            //show the reference position too
            drawOffset(dx,dy);
            m_rCalibrationManager.renderCanvas();

            bool invalidOrientation = true;
            switch (dir)
            {
                case ScannerDirection::x:
                    invalidOrientation = (std::abs(dy) >= std::abs(dx) || std::abs(dx) < 1 );
                    break;
                case ScannerDirection::y:
                    invalidOrientation = (std::abs(dx) >= std::abs(dy)|| std::abs(dy) < 1 );
                    break;
            }
            if (invalidOrientation)
            {
                
                wmLog(eError, "Unexpected camera orientation, interrupt calibration (scanner %f %f, target movement %f %f) \n",
                    scanner_x, scanner_y, dx, dy ); 

                //reset the scanner position to center, to be ready to set the ROI again

                m_rCalibrationManager.setIndexForNextImageFromDisk(0); // for debug without camera
                m_rCalibrationManager.setScannerPosition(0.0, 0.0);
                auto oReferencePositionImage = m_rCalibrationManager.getImage();
                m_rCalibrationManager.drawText(10, 50, "Reference Image: Scanner 0.0\n", oColorReference);
                m_rCalibrationManager.renderImage(oReferencePositionImage);
                sleep(2);
                return ScannerDirectionEvaluation{false, dx, dy};
            }
            double deltaPix = std::sqrt(dx*dx + dy*dy);
            double delta_mm = std::sqrt(scanner_x * scanner_x + scanner_y*scanner_y);
            return ScannerDirectionEvaluation{true, dx, dy, deltaPix / delta_mm};
        };
        
        
        renderLastStepResult("Calibrate Angle: Reference 0,0");
        
        // if we move of half the gride size, it's likely that the grid will not be fully visible
        oCalibrationGraph.setUseGridRecognition(false);


        // slightly move the scanner along the x axis, so that the reference image is visible

        {
            auto oEvaluationResultX = evaluateScannerDirection(ScannerDirection::x, 0.25, "01_");
            if (!oEvaluationResultX.valid)
            {
                return false;
            }

            invertX = oEvaluationResultX.dx_pix > 0;
            slope = oEvaluationResultX.dy_pix / oEvaluationResultX.dx_pix;
            if (std::abs(slope) > 1e-6)
            {
                double angle = std::atan(slope) * 180 / M_PI;
                wmLog(eInfo, "Angle between camera and scanner direction : %f degrees \n",  angle);
            }
            pix_mm_x = oEvaluationResultX.pix_mm;

        }

        // slightly move the scanner along the y axis, so that the reference image is visible
        {            
            auto oEvaluationResultY = evaluateScannerDirection(ScannerDirection::y, 0.25, "02_");
            if (!oEvaluationResultY.valid)
            {
                return false;
            }
            invertY = oEvaluationResultY.dy_pix  < 0; // different y convention between image and coord3d      
            pix_mm_y = oEvaluationResultY.pix_mm;
        }
    
        oCalibrationGraph.setUseGridRecognition(useGridRecognitionParameter);


        m_rCalibrationManager.setIndexForNextImageFromDisk(3); // for debug without camera
    
    }
    else
    {
        
        wmLog(eWarning, "Using debug setting: SM_CalibrateAngle disabled \n");
    }
    ////////////////////////////////////////////////////////////////////////////
    // 4 update the calibration values
    ////////////////////////////////////////////////////////////////////////////

    {
        double mm_to_pix = 0.0;
        if (useGridRecognitionParameter)
        {
            double gridSide_mm = rCalibrationDataParams.getDouble("SM_EnclosingSquareSize_mm");   
            mm_to_pix = oCalibrationGraph.getRatio_pix_mm_from_Grid(gridSide_mm);
        }
        else
        {         
            double radius_mm = rCalibrationDataParams.getDouble("SM_CircleRadius_mm");
            mm_to_pix = oCalibrationGraph.getRatio_pix_mm_from_Circle(radius_mm);
        }
        if (mm_to_pix <= 0.0)
        {                
            wmLog(eWarning, "Invalid mm to pixel conversion (%f) \n", mm_to_pix);
            return false;
        }

        if (pix_mm_x == 0.0)
        {
            pix_mm_x = mm_to_pix;
        }
        if (pix_mm_y == 0.0)
        {
            pix_mm_y = mm_to_pix;
        }
        
        const auto oCameraRelatedParameters = m_rCalibrationManager.readCameraRelatedParameters();
        
        double estimatedBeta0 = oCameraRelatedParameters.m_oDpixX * mm_to_pix;
    
        //now  update the calibration
        auto & rCalibrationData = m_rCalibrationManager.getCalibrationData(oSensorID); 

        rCalibrationData.setKeyValue("sensorWidth", oCameraRelatedParameters.m_oWidth);
        rCalibrationData.setKeyValue("sensorHeight", oCameraRelatedParameters.m_oHeight);
        rCalibrationData.setKeyValue("DpixX", oCameraRelatedParameters.m_oDpixX);
        rCalibrationData.setKeyValue("DpixY", oCameraRelatedParameters.m_oDpixY);
        rCalibrationData.setKeyValue("beta0", estimatedBeta0);
        rCalibrationData.setKeyValue("InvertX", invertX);
        rCalibrationData.setKeyValue("InvertY", invertY);
        rCalibrationData.setKeyValue("SM_scanXToPixel", invertX ? - pix_mm_x : pix_mm_x);
        rCalibrationData.setKeyValue("SM_scanYToPixel", invertY ?  pix_mm_y : - pix_mm_y);
        rCalibrationData.setKeyValue("SM_slopePixel", slope);
        
        rCalibrationData.setKeyValue  ("SensorParametersChanged",false);
        rCalibrationData.load3DFieldFromParameters();

                        
        m_rCalibrationManager.sendCalibDataChangedSignal(oSensorID, true);
        wmLogTr(eInfo, "QnxMsg.Calib.CalibOK", "Calibration successfully completed!\n");
        
    }
    

    auto oScanMasterCalibrationData = ScanMasterCalibrationData::load(rCalibrationDataParams);
    
    
    m_rCalibrationManager.clearCanvas();
    AcquireScanfieldRoutine oScanfieldRoutine(
                        ScanFieldImageParameters::computeParameters(oScanMasterCalibrationData, oImageSize, oScanFieldGrid),
                        m_oDebugFolder,m_oScanFieldImageFolder);

    bool minimizeJump = rCalibrationDataParams.getBool("SM_CalibRoutineMinimizeJump");

    if (!m_oHasCamera && minimizeJump)
    {
        wmLog(eWarning, "Debug without camera: update scan strategy according to the debug images filenames\n");
        minimizeJump = false;
    }

    const auto scannerPositions = oScanFieldGrid.computeScannerPositions(minimizeJump);
    std::vector<std::pair<geo2d::DPoint, coordinates::CalibrationCameraCorrection>> cameraCorrections;
    cameraCorrections.reserve(scannerPositions.size());

    std::vector<double> scannerPositionX;
    std::vector<double> scannerPositionY;
    std::vector<double> ax, bx, cx, dx, ex, fx;
    std::vector<double> ay, by, cy, dy, ey, fy;

    for (int counter = 0, n = scannerPositions.size(); counter < n; counter++)
    {
        auto &rScannerPosition = scannerPositions[counter];
        m_rCalibrationManager.setScannerPosition(rScannerPosition.x, rScannerPosition.y); 
        std::string labelScannerPosition = std::to_string(rScannerPosition.x) + " " + std::to_string(rScannerPosition.y);

        bool validResult = false;
        coordinates::CalibrationCameraCorrection oCorrection;
        CalibrationGraphScanmaster::CircleResult result;

        std::tie(validResult, oCorrection, result) = oCalibrationGraph.evaluateAll();

        m_rCalibrationManager.drawText(10, 30, "Scanner " + labelScannerPosition + " " + std::to_string(counter+1) + "/" + std::to_string(n), Color::m_oOrangeDark);
        m_rCalibrationManager.renderCanvas();

        if (!validResult)
        {
            wmLog(eError, "Could not process image at %f %f, interrupt calibration \n", rScannerPosition.x, rScannerPosition.y); 
            return false;
        }

        auto oImage = m_rCalibrationManager.getCurrentImage();

        if (!oImage.isValid())
        {
            wmLog(eError, "Invalid image, interrupt calibration \n");
            return false;
        }

        const double subGridLength_mm = gridSide_mm / 2;
        const double pixelPerMmLocal = (result.getTopRightCorner().x - result.getTopLeftCorner().x) / gridSide_mm;
        const int templateSize = std::abs(gridSide_mm * pixelPerMmLocal / 10);
        const auto gridPosition = detectGridPosition(oImage.begin(), oImage.height(), oImage.width(), oImage.stride(), 0.10, std::abs(gridSide_mm * pixelPerMmLocal * 0.8), templateSize);
        auto gridCenter = findClosest(gridPosition, result.x, result.y);
        if (std::abs(gridCenter.first - result.x) > 5 ||
            std::abs(gridCenter.second - result.y) > 5)
        {
            wmLog(eWarning, "Image quality is low for grid center detection at scanner position %f %f\n", rScannerPosition.x, rScannerPosition.y);
            wmLog(eWarning, "Grid center: %f %f, TCP: %f %f, %f\n", gridCenter.first, gridCenter.second, result.x, result.y, subGridLength_mm * pixelPerMmLocal / 5);
            gridCenter.first = result.x;
            gridCenter.second = result.y;
        }
        const auto gridPositionRelative = subtractGridCenter(gridPosition, gridCenter.first, gridCenter.second);
        const auto gridCoordinateRelative = assignGridCoordinate(gridPositionRelative, subGridLength_mm, invertX, !invertY);
        const auto distortionCoefficient = optimizeDistortionCoefficient(gridPositionRelative, gridCoordinateRelative, 5, 500);
        scannerPositionX.emplace_back(rScannerPosition.x);
        scannerPositionY.emplace_back(rScannerPosition.y);
        ax.emplace_back(distortionCoefficient[0]);
        bx.emplace_back(distortionCoefficient[1]);
        cx.emplace_back(distortionCoefficient[2]);
        dx.emplace_back(distortionCoefficient[3]);
        ex.emplace_back(distortionCoefficient[4]);
        fx.emplace_back(distortionCoefficient[5]);
        ay.emplace_back(distortionCoefficient[6]);
        by.emplace_back(distortionCoefficient[7]);
        cy.emplace_back(distortionCoefficient[8]);
        dy.emplace_back(distortionCoefficient[9]);
        ey.emplace_back(distortionCoefficient[10]);
        fy.emplace_back(distortionCoefficient[11]);

        //display artificial grid
        const int gridLineCount = std::min(50.0, std::abs(oImage.width() / distortionCoefficient[0] / (subGridLength_mm / 2) * 1.2));
        std::vector<std::pair<double, double>> displayGrid;
        displayGrid.resize(gridLineCount * gridLineCount);
        for (int j = 0; j < gridLineCount; ++j)
        {
            for (int i = 0; i < gridLineCount; ++i)
            {
                const double u = (i - (gridLineCount / 2)) * subGridLength_mm / 2;
                const double v = (j - (gridLineCount / 2)) * subGridLength_mm / 2;
                const auto point = worldToPixel(u, v, distortionCoefficient);
                displayGrid[j * gridLineCount + i] = {point.first + gridCenter.first, point.second + gridCenter.second};
            }
        }
        for (int j = 0; j < gridLineCount; ++j)
        {
            for (int i = 1; i < gridLineCount; ++i)
            {
                const auto point0 = displayGrid[j * gridLineCount + i - 1];
                const auto point1 = displayGrid[j * gridLineCount + i];


                m_rCalibrationManager.drawLine(point0.first, point0.second, point1.first, point1.second, Color::Blue());
            }
        }
        for (int i = 0; i < gridLineCount; ++i)
        {
            for (int j = 1; j < gridLineCount; ++j)
            {
                const auto point0 = displayGrid[(j - 1) * gridLineCount + i];
                const auto point1 = displayGrid[j * gridLineCount + i];


                m_rCalibrationManager.drawLine(point0.first, point0.second, point1.first, point1.second, Color::Blue());
            }
        }

        for (std::size_t i = 0; i < gridPosition.size(); ++i)
        {
            m_rCalibrationManager.drawCross(gridPosition[i].first, gridPosition[i].second, 10, Color::m_oOrangeDark);
            std::ostringstream oss;
            oss << "(" << gridCoordinateRelative[i].first << ", " << gridCoordinateRelative[i].second << ")";
            m_rCalibrationManager.drawText(gridPosition[i].first, gridPosition[i].second, oss.str(), Color::m_oOrangeDark);
        }

        // use grid center for more accuracy
        oCorrection.m_oTCPXOffset = gridCenter.first - oCalibrationGraph.getReferencePosition().x;
        oCorrection.m_oTCPYOffset = gridCenter.second - oCalibrationGraph.getReferencePosition().y;

        // no TCP offset for scanner position 0,0
        if (std::abs(rScannerPosition.x) < 1e-6 && std::abs(rScannerPosition.y) < 1e-6)
        {
            oCorrection.m_oTCPXOffset = 0;
            oCorrection.m_oTCPYOffset = 0;
        }
        drawOffset(oCorrection.m_oTCPXOffset, oCorrection.m_oTCPYOffset);

        m_rCalibrationManager.renderCanvas();

        cameraCorrections.push_back({{rScannerPosition.x, rScannerPosition.y}, oCorrection});

        oScanfieldRoutine.processCurrentImage( oImage, rScannerPosition);
    }

    const auto gridDistortionDataDebugPath = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "") + "/" + "config/gridDistortionDataDebug.txt";

    std::ofstream gridDistortionDataDebugFile (gridDistortionDataDebugPath);

    if (gridDistortionDataDebugFile.is_open())
    {
        for(std::size_t count = 0; count < scannerPositionX.size(); count ++)
        {
            gridDistortionDataDebugFile << scannerPositionX[count] << " ";
            gridDistortionDataDebugFile << scannerPositionY[count] << " ";
            gridDistortionDataDebugFile << ax[count] << " ";
            gridDistortionDataDebugFile << bx[count] << " ";
            gridDistortionDataDebugFile << cx[count] << " ";
            gridDistortionDataDebugFile << dx[count] << " ";
            gridDistortionDataDebugFile << ex[count] << " ";
            gridDistortionDataDebugFile << fx[count] << " ";
            gridDistortionDataDebugFile << ay[count] << " ";
            gridDistortionDataDebugFile << by[count] << " ";
            gridDistortionDataDebugFile << cy[count] << " ";
            gridDistortionDataDebugFile << dy[count] << " ";
            gridDistortionDataDebugFile << ey[count] << " ";
            gridDistortionDataDebugFile << fy[count] << std::endl;
        }
        gridDistortionDataDebugFile.close();
    }
    
    auto & rCalibrationData =  m_rCalibrationManager.getCalibrationData(oSensorID);

    const auto kax = ka(scannerPositionX, scannerPositionY, ax);
    const auto kbx = kb(scannerPositionX, scannerPositionY, bx);
    const auto kcx = linearRegression(scannerPositionX, cx);
    const auto kdx = linearRegression(scannerPositionX, dx);
    const auto kex = linearRegression(scannerPositionY, ex);
    const auto kfx = constantRegression(fx);
    const auto kay = ka(scannerPositionX, scannerPositionY, ay);
    const auto kby = kb(scannerPositionX, scannerPositionY, by);
    const auto kcy = linearRegression(scannerPositionY, cy);
    const auto kdy = linearRegression(scannerPositionY, dy);
    const auto key = linearRegression(scannerPositionX, ey);
    const auto kfy = constantRegression(fy);

    if (rCalibrationData.isValueInRange("ScanfieldDistortion_kax1", kax.first) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kax2", kax.second) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kbx1", kbx.first) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kbx2", kbx.second) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kcx", kcx) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kdx", kdx) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kex", kex) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kfx", kfx) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kay1", kay.first) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kay2", kay.second) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kby1", kby.first) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kby2", kby.second) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kcy", kcy) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kdy", kdy) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_key", key) &&
        rCalibrationData.isValueInRange("ScanfieldDistortion_kfy", kfy))
    {
        rCalibrationData.setKeyValue("ScanfieldDistortion_kax1", kax.first, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kax2", kax.second, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kbx1", kbx.first, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kbx2", kbx.second, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kcx", kcx, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kdx", kdx, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kex", kex, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kfx", kfx, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kay1", kay.first, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kay2", kay.second, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kby1", kby.first, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kby2", kby.second, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kcy", kcy, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kdy", kdy, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_key", key, true);
        rCalibrationData.setKeyValue("ScanfieldDistortion_kfy", kfy, true);
    }
    else
    {
        wmLog(eWarning, "Scan field distortion calibration failed! Calibration values: \n");
        wmLog(eWarning, "kax1: %f, kax2: %f, kay1: %f, kay2: %f\n", kax.first, kax.second, kay.first, kay.second);
        wmLog(eWarning, "kbx1: %f, kbx2: %f, kby1: %f, kby2: %f\n", kbx.first, kbx.second, kby.first, kby.second);
        wmLog(eWarning, "kcx: %f, kcy: %f\n", kcx, kcy);
        wmLog(eWarning, "kdx: %f, kdy: %f\n", kdx, kdy);
        wmLog(eWarning, "kex: %f, key: %f\n", kex, key);
        wmLog(eWarning, "kfx: %f, kfy: %f\n", kfx, kfy);
    }

    //if we are here, the computation was valid
    wmLog(eDebug, "Export correction grid \n");
    auto oContainer = coordinates::CalibrationCameraCorrectionContainer( cameraCorrections );
    assert (oContainer.m_data.getNumberOfMeasurements() == scannerPositions.size());

    auto filename = rCalibrationData.getFilenamesConfiguration().getCameraCorrectionGridFilename();
    auto writeOk = coordinates::CalibrationCameraCorrectionContainer::write(oContainer, filename);
    if (!writeOk)
    {
        wmLog(eWarning, "Error writing to " + filename + "\n");
        return false;
    }
    rCalibrationData.setCalibrationCorrectionContainer (oContainer); //update the internal calibration offset

    //preview scan field image
    BImage oScanfieldImage = oScanfieldRoutine.exportScanFieldImage();
    Size2d oMaxPreviewSize(1280*0.75, 1024*0.75);

    int jump = std::max(oScanfieldImage.width() / oMaxPreviewSize.width, oScanfieldImage.height() / oMaxPreviewSize.height);

    Size2d oPreviewSize{oScanfieldImage.width() / jump, oScanfieldImage.height() / jump};
    BImage oScanFieldPreview(oPreviewSize);
    filter::downsampleImage(oScanFieldPreview, oScanfieldImage, jump, jump);

    m_rCalibrationManager.drawImage(0,0,oScanFieldPreview, "Preview Scanfield Image 1:" + std::to_string(jump)  , Color::Green());
    m_rCalibrationManager.renderCanvas();
    return true;
}

bool CalibrateScanField::acquireScanFieldImage() const
{

    //acquire the first image just to get the size
    auto oImageSize = m_rCalibrationManager.getImage().size();
    
    if (oImageSize.area() == 0)
    {
        wmLog(eError, "Invalid image, interrupt calibration \n"); 
        return false;
    }
    m_rCalibrationManager.setIndexForNextImageFromDisk(0);
    
    const int oSensorID = 0;
    const auto & rCalibrationDataParams = m_rCalibrationManager.getCalibrationData(oSensorID).getParameters();
    ScanFieldGridParameters oScanFieldGrid = loadScanFieldGridParameters();
    auto oScanMasterCalibrationData = ScanMasterCalibrationData::load(rCalibrationDataParams);
    
    
    m_rCalibrationManager.clearCanvas();
    AcquireScanfieldRoutine oScanfieldRoutine(
                        ScanFieldImageParameters::computeParameters(oScanMasterCalibrationData, oImageSize, oScanFieldGrid),
                        m_oDebugFolder, m_oScanFieldImageFolder);


    bool minimizeJump = rCalibrationDataParams.getBool("SM_CalibRoutineMinimizeJump");

    if (!m_oHasCamera && minimizeJump)
    {
        wmLog(eWarning, "Debug without camera: update scan strategy according to the debug images filenames\n");
        minimizeJump = false;
    }

    const auto scannerPositions = oScanFieldGrid.computeScannerPositions(minimizeJump );
    std::vector<std::pair<geo2d::DPoint, coordinates::CalibrationCameraCorrection>> cameraCorrections;
    cameraCorrections.reserve(scannerPositions.size());
    
    
    for (int counter = 0, n = scannerPositions.size(); counter < n; counter++)
    {
        auto &rScannerPosition = scannerPositions[counter];
        m_rCalibrationManager.setScannerPosition(rScannerPosition.x, rScannerPosition.y); 
        
        auto oImage = m_rCalibrationManager.getImage();
            
        m_rCalibrationManager.clearCanvas();
        m_rCalibrationManager.drawText(10, 10, std::to_string(rScannerPosition.x) + " " + std::to_string(rScannerPosition.y), Color::m_oOrangeDark);
        m_rCalibrationManager.drawText(200, 10, std::to_string(counter) + "/" + std::to_string(n), Color::m_oOrangeDark);
        m_rCalibrationManager.renderImage(oImage);

        if (!oImage.isValid())
        {
            wmLog(eError, "Invalid image, interrupt calibration \n"); 
            return false;
        }

        oScanfieldRoutine.processCurrentImage( oImage, rScannerPosition);

    }
    
    //export scan field image
    BImage oScanfieldImage = oScanfieldRoutine.exportScanFieldImage();
    Size2d oMaxPreviewSize{1280, 1024};

    int jump = std::max(oScanfieldImage.width() / oMaxPreviewSize.width, oScanfieldImage.height() / oMaxPreviewSize.height);

    Size2d oPreviewSize{oScanfieldImage.width() / jump, oScanfieldImage.height() / jump};
    BImage oScanFieldPreview(oPreviewSize);
    filter::downsampleImage(oScanFieldPreview, oScanfieldImage, jump, jump);

    m_rCalibrationManager.clearCanvas();
    m_rCalibrationManager.drawText(10,20, "Preview Scanfield Image 1:" + std::to_string(jump)  , Color::Green());
    //TODO set sampling in context if measuring dialog supports it
    m_rCalibrationManager.renderImage(oScanFieldPreview);


    return true;
}


bool CalibrateScanField::calibrateIDMSurface(bool updateSystemCalibration) const
{
    if (!m_rCalibrationManager.isOCTEnabled())
    {
        wmLog(eError, "OCT not enabled \n");
        return false;
    }
    const int oSensorID = 0;
    const auto & rCalibrationDataParams = m_rCalibrationManager.getCalibrationData(oSensorID).getParameters();
    bool extraDebug = rCalibrationDataParams.getInt("Debug") > 0 || !updateSystemCalibration; //in the verification case enable the debug automatically

    //set IDM adaptive exposure mode
    {
        bool adaptiveExposureMode = rCalibrationDataParams.getBool("IDM_AdaptiveExposureMode");
        int adaptiveExposureBasicValue = rCalibrationDataParams.getInt("IDM_AdaptiveExposureBasicValue");
        m_rCalibrationManager.setIDMKeyValue(interface::SmpKeyValue{new interface::TKeyValue<bool> ("Adaptive Exposure Mode On/Off", adaptiveExposureMode )});
        if (adaptiveExposureMode)
        {
            m_rCalibrationManager.setIDMKeyValue(interface::SmpKeyValue{new interface::TKeyValue<int> ("Adaptive Exposure Basic Value", adaptiveExposureBasicValue )});
        }
    }
    
    IDMAcquisition oIDMAcquisition(m_rCalibrationManager);
    oIDMAcquisition.setExtraDebug(extraDebug);
    oIDMAcquisition.setNumRepetitions(rCalibrationDataParams.getInt("SM_CalibRoutineRepetitions"));

    int trigger_ms = rCalibrationDataParams.getInt("SM_CalibRoutineTrigger_ms");
    
    ScanFieldGridParameters oScanFieldGrid = loadScanFieldGridParameters();
    bool minimizeJump = rCalibrationDataParams.getBool("SM_CalibRoutineMinimizeJump");
    IDMMeasurements oIDMMeasurements(oScanFieldGrid, minimizeJump);

    const std::size_t referenceArmTotal = interface::SystemConfiguration::instance().getBool("OCT_with_reference_arms", false) ? 4 : 1;
    std::vector<float> x(oIDMMeasurements.getNumberOfPositions());
    std::vector<float> y(oIDMMeasurements.getNumberOfPositions());
    std::vector<std::vector<float>> z(referenceArmTotal);
    
    m_rCalibrationManager.setIndexForNextImageFromDisk(0);

    for (std::size_t i = 0; i < referenceArmTotal; ++i)
    {
        z[i].resize(oIDMMeasurements.getNumberOfPositions());
        m_rCalibrationManager.clearCanvas();
        oIDMMeasurements.renderBackgroundImage(m_rCalibrationManager);
        m_rCalibrationManager.setOCTReferenceArm(i + 1);

        for (int index = 0, n = oIDMMeasurements.getNumberOfPositions(); index < n; index++)
        {
            const auto & rScannerPosition = oIDMMeasurements.getScannerPosition(index);
            m_rCalibrationManager.setScannerPosition(rScannerPosition.x, rScannerPosition.y);
            std::ostringstream labelTop;
            labelTop << rScannerPosition.x << " " << rScannerPosition.y;

            bool validResult = false;
            IDMAcquisition::RawIdmMeasurements oIDMRawMeasurement;
            std::tie(validResult, oIDMRawMeasurement) = oIDMAcquisition.evaluateIDMWeldingDepth(trigger_ms);

            if (!validResult)
            {
                wmLog(eError, "Could not process image %d/%d at %f %f, interrupt calibration \n",
                    index, n,
                    rScannerPosition.x, rScannerPosition.y);
                oIDMMeasurements.renderMeasurementPoint(rScannerPosition, m_rCalibrationManager, labelTop.str(), "ERROR");
            }

            const auto medianValue = IDMMeasurements::computeMedian(oIDMRawMeasurement);
            oIDMMeasurements.renderMeasurementPoint(rScannerPosition, m_rCalibrationManager,
                                                    labelTop.str(), std::to_string(int(medianValue)));

            if (i == 0)
            {
                oIDMMeasurements.addMeasurements(index, std::move(oIDMRawMeasurement));
            }

            x[index] = rScannerPosition.x;
            y[index] = rScannerPosition.y;
            z[i][index] = medianValue;
        }
    }

    const float ransacThreshold = 150; //um
    const float ransacRepetition = 1500;
    const auto model = precitec::coordinates::idmModel(x, y, z, ransacThreshold, ransacRepetition);
    wmLog(eInfo, "IDM Calibration Report");
    wmLog(eInfo, "Curvature X (k1): %f um/mm^2, Curvature Y (k2): %f um/mm^2", model.second[0], model.second[1]);
    wmLog(eInfo, "Tilt X (k3): %f um/mm, Tilt Y (k4): %f um/mm", model.second[2], model.second[3]);
    for (std::size_t i = 0; i < referenceArmTotal; ++i)
    {
        wmLog(eInfo, "Reference Position l0%u: %f um", i, model.first[i]);
    }
    const auto idmRawDataDebugPath = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "") + "/" + "config/idmRawDataDebug.txt";
    precitec::coordinates::saveIdmRawData(x, y, z, idmRawDataDebugPath);

    oIDMMeasurements.computeMedianValues();
    
        
    if (updateSystemCalibration)
    {
        auto & rCalibrationData =  m_rCalibrationManager.getCalibrationData(oSensorID);
        auto idmGridFilename =  rCalibrationData.getFilenamesConfiguration().getIDMCorrectionGridFilename();
        
        auto oContainer = oIDMMeasurements.computeCorrectionContainer();
        oIDMMeasurements.computeCalibratedValues(oContainer);
        
        rCalibrationData.setCalibrationIDMCorrectionContainer(oContainer);
        bool saved = oIDMMeasurements.saveIDMCorrectionFile(idmGridFilename);

        const std::vector<std::string> parameterKey = {"IDM_k1", "IDM_k2", "IDM_k3", "IDM_k4"};
        const std::vector<std::string> lengthKey = {"IDM_l01", "IDM_l02", "IDM_l03", "IDM_l04"};

        bool idmParameterIsValid = true;
        for (int i = 0; i < 4; ++i)
        {
            if (!rCalibrationData.isValueInRange(parameterKey.at(i), model.second.at(i)))
            {
                idmParameterIsValid = false;
                break;
            }
        }

        for (std::size_t i = 0; i < lengthKey.size() && i < z.size(); ++i)
        {
            if (!rCalibrationData.isValueInRange(lengthKey.at(i), model.first.at(i)))
            {
                idmParameterIsValid = false;
                break;
            }
        }

        if (idmParameterIsValid)
        {
            auto lengths = model.first;
            lengths.resize(lengthKey.size(), 0.0);
            for (std::size_t i = 0; i < lengthKey.size(); ++i)
            {
                rCalibrationData.setKeyValue(lengthKey.at(i), lengths.at(i), true);
            }
            for (std::size_t i = 0; i < parameterKey.size(); ++i)
            {
                rCalibrationData.setKeyValue(parameterKey.at(i), model.second.at(i), true);
            }
            rCalibrationData.syncXMLContent(); //save to xml file first before updating weld head
            m_rCalibrationManager.weldHeadReloadFiberSwitchCalibration();
            wmLog(eInfo, "IDM calibration values updated");
        }
        else
        {
            wmLog(eError, "invalid IDM calibration values, calibration failed");
        }

        if (!saved)
        {
            return false;
        }
    }

    oIDMMeasurements.computeCalibratedValues(m_rCalibrationManager.getCalibrationData(0));
    
    
    // render depth image
    if (m_calibrationType == interface::CalibType::eVerifyScanFieldIDM_Z)
    {
        //for the verification, the image contrast should enhance the positions where the Z measurement varies above a certain variability,
        //instead of stretching the contrast at the full range
        double rangeZ = 400.0;
        byte rangeGray = 200;
        oIDMMeasurements.computeDepthImageFromCalibratedValues(&m_rCalibrationManager, rangeZ, rangeGray);
    }
    else
    {        
        oIDMMeasurements.computeDepthImageFromRawValues(&m_rCalibrationManager);
    }
    
    if (extraDebug)
    {
        assert(m_oDebugFolder.back() == '/');;
        //compute depth image without rendering
        
        oIDMMeasurements.saveIDMCorrectionFile(m_oDebugFolder + "rawZ.csv"); 
        
        saveImage(oIDMMeasurements.computeDepthImageFromRawValues(), m_oDebugFolder + "ImageZRaw.bmp");
        saveImage(oIDMMeasurements.computeDepthImageFromCalibratedValues(), m_oDebugFolder + "ImageZCalibrated.bmp");
    }
    
    return true;
}

#define CHECK_ERROR(x) \
    do { \
        int err = (x); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

enum CalibrationFileFitStrategy
{
    UseFittedValue,
    UseBoundedValue,
    UseRawMeasurement,
};

static const std::string CalibrationFileFitStrategyString[] = {"UseFittedValue", "UseBoundedValue", "UseRawMeasurement"};

int generateScanlabCt5File(const std::vector<std::pair<double, double>>& targetPosition, const std::vector<std::pair<double, double>>& measuredPosition, const std::string& originalFileName, const std::string& newFileName, CalibrationFileFitStrategy strategy)
{
    CHECK_ERROR(slcl_activate(SLCL_ACTIVATE_PASSWORD));

    std::size_t fileHandle = 0;
    CHECK_ERROR(slcl_load_correction_table(&fileHandle, originalFileName.c_str(), nullptr));

    uint32_t calibrationFactor;
    CHECK_ERROR(slcl_get_current_calibration_factor(fileHandle, &calibrationFactor));

    const auto N = targetPosition.size();

    std::vector<int32_t> xImageBits(N);
    std::vector<int32_t> yImageBits(N);
    std::vector<int32_t> xAngleBits(N);
    std::vector<int32_t> yAngleBits(N);
    std::vector<double> xTargetsMm(N);
    std::vector<double> yTargetsMm(N);
    std::vector<double> xMeasurementsMm(N);
    std::vector<double> yMeasurementsMm(N);

    for (std::size_t i = 0; i < N; ++i)
    {
        xImageBits[i] = targetPosition[i].first * calibrationFactor;
        yImageBits[i] = targetPosition[i].second * calibrationFactor;
        xTargetsMm[i] = targetPosition[i].first;
        yTargetsMm[i] = targetPosition[i].second;
        xMeasurementsMm[i] = measuredPosition[i].first;
        yMeasurementsMm[i] = measuredPosition[i].second;
    }

    CHECK_ERROR(slcl_transform_points_2d(fileHandle, N, xImageBits.data(), yImageBits.data(), xAngleBits.data(), yAngleBits.data()));

    const auto polyOrder = 5;
    const auto tolerance = 50e-3;
    const auto iteration = 20000;
    auto xModelMm = polyFit2D({xAngleBits.begin(), xAngleBits.end()}, {yAngleBits.begin(), yAngleBits.end()}, xMeasurementsMm, polyOrder, tolerance * 2, iteration);
    auto yModelMm = polyFit2D({xAngleBits.begin(), xAngleBits.end()}, {yAngleBits.begin(), yAngleBits.end()}, yMeasurementsMm, polyOrder, tolerance * 2, iteration);

    auto xBoundedMm = xMeasurementsMm;
    auto yBoundedMm = yMeasurementsMm;

    double sumdx2 = 0;
    double sumdy2 = 0;

    for (std::size_t i = 0; i < N; ++i)
    {
        const auto dx = xMeasurementsMm[i] - xModelMm[i];
        const auto dy = yMeasurementsMm[i] - yModelMm[i];

        if (std::abs(dx) > tolerance)
        {
            xBoundedMm[i] = xModelMm[i];
        }

        if (std::abs(dy) > tolerance)
        {
            yBoundedMm[i] = yModelMm[i];
        }

        sumdx2 += dx * dx;
        sumdy2 += dy * dy;
    }

    const auto rmsX = std::sqrt(sumdx2 / N);
    const auto rmsY = std::sqrt(sumdy2 / N);

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    long long now_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time_t);
    int ms = now_since_epoch % 1000;
    std::ostringstream timeString;
    timeString << std::put_time(now_tm, "%Y-%m-%d_%H-%M-%S");

    std::string filePath;
    filePath = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) + "/logfiles/" : "") + "scanner_calibration_" + timeString.str() + ".log";

    std::fstream logFile;
    logFile.open(filePath, std::ios_base::out);

    logFile << "[" << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S.") << std::setfill('0') << std::setw(3) << ms << "]" << std::endl;
    logFile << "[Original Ct5 File: " << originalFileName << "]" << std::endl;
    logFile << "[New Ct5 File: " << newFileName << "]" << std::endl;
    logFile << "[Fit Strategy: " << CalibrationFileFitStrategyString[strategy] << "]" << std::endl;
    logFile << "[rmsX: " << rmsX << ", rmsY: " << rmsY << "]" << std::endl;

    logFile << std::left << std::setfill(' ') << std::setw(7) << "index"
        << std::right << std::setfill(' ') << std::setw(10) << "alpha" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "beta" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "sx" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "sy" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "x" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "y" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "xModel" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "yModel" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "x-xModel" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "y-yModel" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "xBounded" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "yBounded" << std::endl;

    logFile << std::fixed << std::setprecision(3);
    for (std::size_t i = 0; i < N; ++i)
    {
        logFile << std::left << std::setfill(' ') << std::setw(7) << std::to_string(i + 1)
                << std::right << std::setfill(' ') << std::setw(10) << xAngleBits[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yAngleBits[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << xTargetsMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yTargetsMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << xMeasurementsMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yMeasurementsMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << xModelMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yModelMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << xMeasurementsMm[i] - xModelMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yMeasurementsMm[i] - yModelMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << xBoundedMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yBoundedMm[i] << std::endl;
    }

    logFile.close();

    slcl_xy_calibration_settings calibrationSettings;
    calibrationSettings.XYCalibrationOptions = 0;
    calibrationSettings.NewCalibrationFactor = calibrationFactor;
    calibrationSettings.RestrictionScaling = 1.0;
    calibrationSettings.ToleranceUM = 5.0;

    slcl_xy_calibration_interpolation_results calibrationResults;

    const auto& xFinal = strategy == CalibrationFileFitStrategy::UseFittedValue ? xModelMm :
                         strategy == CalibrationFileFitStrategy::UseBoundedValue ? xBoundedMm :
                         xMeasurementsMm;

    const auto& yFinal = strategy == CalibrationFileFitStrategy::UseFittedValue ? yModelMm :
                         strategy == CalibrationFileFitStrategy::UseBoundedValue ? yBoundedMm :
                         yMeasurementsMm;

    CHECK_ERROR(slcl_xy_calibration_mm_targets(fileHandle, N, xTargetsMm.data(), yTargetsMm.data(), xFinal.data(), yFinal.data(), &calibrationSettings, &calibrationResults, newFileName.c_str()));

    CHECK_ERROR(slcl_delete_correction_table_handle(fileHandle));

    return 0;
}

void generateSmartMoveCorrectionFile(const std::vector<std::pair<double, double>>& targetPosition, const std::vector<std::pair<double, double>>& measuredPosition, const std::string& originalFileName, const std::string& newFileName, CalibrationFileFitStrategy strategy)
{
    // get focal length parameter
    std::ifstream file(originalFileName);
    if (!file.is_open())
    {
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    const std::string fileText = buffer.str();

    std::smatch match;
    std::regex_search(fileText, match, std::regex("FOCALLENGTH *= *([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"));

    if (match.size() != 2) // cannot find focal length parameter in calibration file
    {
        wmLog(eError, "[calibrateScanner] cannot find focal length parameter in calibration file!\n");
        return;
    }

    const double focalLength = std::stod(match[1].str());

    const auto N = targetPosition.size();

    std::vector<int32_t> xAngleBits(N);
    std::vector<int32_t> yAngleBits(N);
    std::vector<double> xTargetsMm(N);
    std::vector<double> yTargetsMm(N);
    std::vector<double> xMeasurementsMm(N);
    std::vector<double> yMeasurementsMm(N);

    for (std::size_t i = 0; i < N; ++i)
    {
        xTargetsMm[i] = targetPosition[i].first;
        yTargetsMm[i] = targetPosition[i].second;
        xMeasurementsMm[i] = measuredPosition[i].first;
        yMeasurementsMm[i] = measuredPosition[i].second;
    }

    const auto polyOrder = 5;
    const auto tolerance = 50e-3;
    const auto iteration = 20000;
    const auto xModelMm = polyFit2D({xTargetsMm.begin(), xTargetsMm.end()}, {yTargetsMm.begin(), yTargetsMm.end()}, xMeasurementsMm, polyOrder, tolerance * 2, iteration);
    const auto yModelMm = polyFit2D({xTargetsMm.begin(), xTargetsMm.end()}, {yTargetsMm.begin(), yTargetsMm.end()}, yMeasurementsMm, polyOrder, tolerance * 2, iteration);

    auto xBoundedMm = xMeasurementsMm;
    auto yBoundedMm = yMeasurementsMm;

    double sumdx2 = 0;
    double sumdy2 = 0;

    for (std::size_t i = 0; i < N; ++i)
    {
        const auto dx = xMeasurementsMm[i] - xModelMm[i];
        const auto dy = yMeasurementsMm[i] - yModelMm[i];

        if (std::abs(dx) > tolerance)
        {
            xBoundedMm[i] = xModelMm[i];
        }

        if (std::abs(dy) > tolerance)
        {
            yBoundedMm[i] = yModelMm[i];
        }

        sumdx2 += dx * dx;
        sumdy2 += dy * dy;
    }

    const auto rmsX = std::sqrt(sumdx2 / N);
    const auto rmsY = std::sqrt(sumdy2 / N);

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    long long now_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time_t);
    int ms = now_since_epoch % 1000;
    std::ostringstream timeString;
    timeString << std::put_time(now_tm, "%Y-%m-%d_%H-%M-%S");

    std::string filePath;
    filePath = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) + "/logfiles/" : "") + "scanner_calibration_" + timeString.str() + ".log";

    std::fstream logFile;
    logFile.open(filePath, std::ios_base::out);

    logFile << "[" << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S.") << std::setfill('0') << std::setw(3) << ms << "]" << std::endl;
    logFile << "[Original File: " << originalFileName << "]" << std::endl;
    logFile << "[New File: " << newFileName << "]" << std::endl;
    logFile << "[Fit Strategy: " << CalibrationFileFitStrategyString[strategy] << "]" << std::endl;
    logFile << "[rmsX: " << rmsX << ", rmsY: " << rmsY << "]" << std::endl;

    logFile << std::left << std::setfill(' ') << std::setw(7) << "index"
        << std::right << std::setfill(' ') << std::setw(10) << "sx" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "sy" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "x" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "y" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "xModel" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "yModel" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "x-xModel" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "y-yModel" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "xBounded" << " "
        << std::right << std::setfill(' ') << std::setw(10) << "yBounded" << std::endl;

    logFile << std::fixed << std::setprecision(3);
    for (std::size_t i = 0; i < N; ++i)
    {
        logFile << std::left << std::setfill(' ') << std::setw(7) << std::to_string(i + 1)
                << std::right << std::setfill(' ') << std::setw(10) << xTargetsMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yTargetsMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << xMeasurementsMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yMeasurementsMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << xModelMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yModelMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << xMeasurementsMm[i] - xModelMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yMeasurementsMm[i] - yModelMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << xBoundedMm[i] << " "
                << std::right << std::setfill(' ') << std::setw(10) << yBoundedMm[i] << std::endl;
    }

    logFile.close();

    const auto& xFinal = strategy == CalibrationFileFitStrategy::UseFittedValue ? xModelMm :
                         strategy == CalibrationFileFitStrategy::UseBoundedValue ? xBoundedMm :
                         xMeasurementsMm;

    const auto& yFinal = strategy == CalibrationFileFitStrategy::UseFittedValue ? yModelMm :
                         strategy == CalibrationFileFitStrategy::UseBoundedValue ? yBoundedMm :
                         yMeasurementsMm;

    // generate input file for SmartMove calibration tool
    const auto calibrationInputFileName = std::filesystem::path(newFileName).replace_extension(".txt");
    const auto calibrationName = std::filesystem::path(newFileName).stem().string();

    std::fstream calibrationInputFile;
    calibrationInputFile.open(calibrationInputFileName, std::ios_base::out);

    if (!calibrationInputFile.is_open())
    {
        return;
    }

    calibrationInputFile << "[General]\n"
                          << "CorrectionName = " << calibrationName << "\n"
                          << "FocalLength = " << focalLength << "\n"
                          << "[Data]\n";

    for (std::size_t i = 0; i < N; ++i)
    {
        calibrationInputFile << xTargetsMm[i] << "," << yTargetsMm[i] << "," << xFinal[i] << "," << yFinal[i] << std::endl;
    }

    calibrationInputFile.close();

    std::string command = "sfccalc-2d " + calibrationInputFileName.string() + " -o " + std::filesystem::path(newFileName).replace_extension("").string() + " -s " + originalFileName;

    wmLog(eInfo, "Running system command '%s'\n", command);

    const auto commandRet = std::system(command.c_str());

    if (commandRet != 0) // error
    {
        wmLog(eError, "Error generating SmartMove calibration file. Error Number: %d\n", commandRet);
    }

    return;
}

bool CalibrateScanField::calibrateScanner()
{
    const auto customCorrectionFileKey = interface::SystemConfiguration::keyToString(interface::SystemConfiguration::StringKey::ScanlabScanner_Correction_File);
    const auto customCorrectionFile = interface::SystemConfiguration::instance().getString(customCorrectionFileKey, "");

    const auto scanner2DController = static_cast<interface::ScannerModel>(interface::SystemConfiguration::instance().getInt(interface::SystemConfiguration::keyToString(interface::SystemConfiguration::IntKey::Scanner2DController), 0));
    if (scanner2DController != interface::ScannerModel::ScanlabScanner && scanner2DController != interface::ScannerModel::SmartMoveScanner)
    {
        wmLog(eError, "[calibrateScanner] Calibration procedure for scanner model not supported!\n");
        return false;
    }

    const auto lensType = static_cast<interface::LensType>(interface::SystemConfiguration::instance().getInt(interface::SystemConfiguration::keyToString(interface::SystemConfiguration::IntKey::ScanlabScanner_Lens_Type), 1));

    std::map<interface::ScannerModel, std::map<interface::LensType, std::string>> defaultCorrectionFile;
    defaultCorrectionFile[interface::ScannerModel::ScanlabScanner][interface::LensType::F_Theta_340] = "IntelliScanIII30_F_Theta_340.ct5";
    defaultCorrectionFile[interface::ScannerModel::ScanlabScanner][interface::LensType::F_Theta_460] = "IntelliScanIII30_F_Theta_460.ct5";
    defaultCorrectionFile[interface::ScannerModel::ScanlabScanner][interface::LensType::F_Theta_255] = "IntelliScanIII30_F_Theta_255.ct5";
    defaultCorrectionFile[interface::ScannerModel::SmartMoveScanner][interface::LensType::F_Theta_340] = "ME_F340.sbd";
    defaultCorrectionFile[interface::ScannerModel::SmartMoveScanner][interface::LensType::F_Theta_460] = "ME_F460.sbd";
    defaultCorrectionFile[interface::ScannerModel::SmartMoveScanner][interface::LensType::F_Theta_255] = "ME_F255.sbd";

    const auto currentCorrectionFile = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "") +
                                (customCorrectionFile != "" ? "/config/calib/" + customCorrectionFile :
                                 "/calib/" + defaultCorrectionFile[scanner2DController][lensType]);

    const std::string scannerModelString[] = {"ScanlabScanner", "SmartMoveScanner"};
    wmLog(eWarning, "[calibrateScanner] Scanner2DController: %s\n", scannerModelString[static_cast<int>(scanner2DController)]);

    wmLog(eWarning, "[calibrateScanner] Current correction File: \n" + currentCorrectionFile);

    if (!std::filesystem::exists(currentCorrectionFile) || !std::filesystem::is_regular_file(currentCorrectionFile))
    {
        wmLog(eError, "[calibrateScanner] The current correction file specified by ScanlabScanner_Correction_File/ScanlabScanner_Lens_Type/Scanner2DController is not found!\n");
        return false;
    }

    m_rCalibrationManager.setIndexForNextImageFromDisk(0);

    // 1. Determine orientation of scanner relative to camera
    // - move scanner to (0,0), (1,0) and (0,1). Take an image at each position
    m_rCalibrationManager.setScannerPosition(0.0, 0.0);
    BImage imageLast;
    BImage image;
    m_rCalibrationManager.getImage().copyPixelsTo(imageLast);

    const auto period = horizontalPeriod(imageLast.begin(), imageLast.height(), imageLast.width(), imageLast.stride());
    wmLog(eWarning, "[calibrateScanner] Detected Calibration Plate Size: %d px\n", period);

    m_rCalibrationManager.clearCanvas();
    m_rCalibrationManager.drawText(10, 10, "Scanner origin 0,0", Color::m_oOrangeDark);
    m_rCalibrationManager.renderImage(imageLast);

    m_rCalibrationManager.setScannerPosition(1.0, 0.0);
    image = m_rCalibrationManager.getImage();
    m_rCalibrationManager.clearCanvas();
    m_rCalibrationManager.renderImage(image);

    const auto dxShift = imageShift(imageLast.begin(), image.begin(), image.height(), image.width(), imageLast.stride(), image.stride());
    wmLog(eDebug, "[calibrateScanner] Scanner 1.0 mm, 0.0 mm, image shift: %d, %d\n", dxShift.first, dxShift.second);

    m_rCalibrationManager.setScannerPosition(0.0, 1.0);
    image = m_rCalibrationManager.getImage();
    m_rCalibrationManager.clearCanvas();
    m_rCalibrationManager.renderImage(image);

    const auto dyShift = imageShift(imageLast.begin(), image.begin(), image.height(), image.width(), imageLast.stride(), image.stride());
    wmLog(eDebug, "[calibrateScanner] Scanner 0.0 mm, 1.0 mm, image shift: %d, %d\n", dyShift.first, dyShift.second);

    enum class AxisDirection
    {
        Left = 0, Right = 1, Top = 2, Bottom = 3
    };

    std::string axisDirectionMap[4] = {"Left", "Right", "Top", "Bottom"};

    const AxisDirection xDirection = std::abs(dxShift.first) > std::abs(dxShift.second) ?
                               (dxShift.first > 0 ? AxisDirection::Left : AxisDirection::Right) :
                               (dxShift.second > 0 ? AxisDirection::Top : AxisDirection::Bottom);

    const AxisDirection yDirection = std::abs(dyShift.first) > std::abs(dyShift.second) ?
                               (dyShift.first > 0 ? AxisDirection::Left : AxisDirection::Right) :
                               (dyShift .second > 0 ? AxisDirection::Top : AxisDirection::Bottom);

    wmLog(eWarning, "xDirection: %s, yDirection: %s", axisDirectionMap[(int)xDirection], axisDirectionMap[(int)yDirection]);

    const auto & calibrationParameters = m_rCalibrationManager.getCalibrationData(0).getParameters();
    const auto xMin = calibrationParameters.getDouble("SM_X_min");
    const auto xMax = calibrationParameters.getDouble("SM_X_max");
    const auto yMin = calibrationParameters.getDouble("SM_Y_min");
    const auto yMax = calibrationParameters.getDouble("SM_Y_max");
    const auto periodMm = 10;

    std::vector<std::pair<double, double>> scannerPosition;
    // calculate scanner positions
    for (auto y = std::ceil(yMin / periodMm) * periodMm; y <= yMax; y = std::floor(y + periodMm))
    {
        for (auto x = std::ceil(xMin / periodMm) * periodMm; x <= xMax; x = std::floor(x + periodMm))
        {
            scannerPosition.emplace_back(x, y);
        }
    }

    std::vector<std::vector<std::vector<std::pair<double, double>>>> squareResult;
    for (const auto& s : scannerPosition)
    {
        m_rCalibrationManager.setScannerPosition(s.first, s.second);
        image = m_rCalibrationManager.getImage();
        const auto result =  findAnalyseSquaresLoG(image.begin(), image.height(), image.width(), image.stride(), period);

        if (result.empty())
        {
            wmLog(eError, "[calibrateScanner] no chessboard detected! calibration routine terminated!\n");
            return false;
        }

        const auto squaresDetected = result.size();
        int spotsDetected = 0;

        m_rCalibrationManager.clearCanvas();
        for (const auto& square : result)
        {
            m_rCalibrationManager.drawCross(square[0].first + 0.5, square[0].second + 0.5, 10, Color::Green());

            for (std::size_t i = 1; i < square.size(); i = i + 2)
            {
                m_rCalibrationManager.drawCross(square[i].first + 0.5, square[i].second + 0.5, 10, Color::Red());
            }

            spotsDetected += (square.size() - 1) / 2;
        }
        m_rCalibrationManager.renderImage(image);

        squareResult.emplace_back(result);

        wmLog(eInfo, "Squares Detected: %d, Spots Detected: %d\n", squaresDetected, spotsDetected);
    }

    const auto imageHeight = image.height();
    const auto imageWidth = image.width();
    auto spotPosition = postprocessSquareResult(squareResult, scannerPosition, imageWidth / 2, imageHeight / 2);

    // calculate the absolute spot position by adding the scanner position with the offset.
    for (std::size_t i = 0; i < scannerPosition.size(); ++i)
    {
        auto spot = scannerPosition[i];
        const auto& offset = spotPosition[i];

        spot.first = xDirection == AxisDirection::Left ? spot.first - offset.first :
                     xDirection == AxisDirection::Right ? spot.first + offset.first :
                     xDirection == AxisDirection::Top ? spot.first  - offset.second :
                     spot.first + offset.second;
        spot.second = yDirection == AxisDirection::Top ? spot.second - offset.second :
                      yDirection == AxisDirection::Bottom ? spot.second + offset.second :
                      yDirection == AxisDirection::Left ? spot.second - offset.first :
                      spot.second + offset.first;

        spotPosition[i] = spot;
    }

    for (std::size_t i = 0; i < spotPosition.size(); ++i)
    {
        wmLog(eInfo, "sx: %f, sy: %f, x: %f, y: %f\n", scannerPosition[i].first, scannerPosition[i].second, spotPosition[i].first, spotPosition[i].second);
    }

    const std::filesystem::path filePath = currentCorrectionFile;
    const auto fileName = filePath.filename().string();
    const std::string baseFileName = filePath.stem().string();
    const std::string fileExtension = filePath.extension().string();

    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);

    std::stringstream timeStampStream;
    timeStampStream << std::put_time(std::localtime(&nowTime), "%y%m%d_%H%M%S");
    const std::string timeStamp = timeStampStream.str();

    // if old file has a name that looks like "FileName_YYYYMMDD_HHMMSS" or ""FileName_YYMMDD_HHMMSS"", remove the old timestamp and add a new timestamp
    const auto newFileName = std::regex_replace(baseFileName, std::regex("_\\d{8}_\\d{6}$|_\\d{6}_\\d{6}$"), "") + "_" + timeStamp + fileExtension;
    const auto newCorrectionFile = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "") +
                                "/config/calib/" + newFileName;

    switch (scanner2DController)
    {
        case interface::ScannerModel::ScanlabScanner:
        {
            int error = generateScanlabCt5File(scannerPosition, spotPosition, currentCorrectionFile, newCorrectionFile, CalibrationFileFitStrategy::UseFittedValue);

            if (error != 0)
            {
                std::string slclErrorCode[] =
                {
                    "NO_ERROR",
                    "ACTIVATION_CODE_INVALID",
                    "LIB_ACCESS_DENIED",
                    "WRONG_FILE_EXTENSION",
                    "COULD_NOT_OPEN_CORR_FILE",
                    "WRONG_FILE_SIZE",
                    "CHECKSUM_INVALID",
                    "BAD_HANDLE",
                    "WRONG_TABLETYPE",
                    "TABLE_NOT_3D",
                    "CALCULATION_FAILED",
                    "MISSING_README_PARAMS",
                    "INSUFFICIENT_MEMORY",
                    "INTERPOLATION_FAILED",
                    "SPLINE_INVERSION_FAILED",
                    "BAD_DIRECTION_VECTOR",
                    "RADIUS_TOO_SMALL",
                    "FUNCTION_CALL_NOT_ALLOWED",
                    "ANGLE_OUT_OF_BOUNDS"
                };

                wmLog(eError, "[calibrateScanner] Calibration Failed: Scanlab Calibration Library returned with error: %s\n", slclErrorCode[error]);
                return false;
            }

            // Temporary fix in case the x and y are swapped
            {
                const auto newFileNameXYSwapped = std::regex_replace(baseFileName, std::regex("_\\d{8}_\\d{6}$"), "") + "_XYSwapped(SmartMove)_" + timeStamp + fileExtension;
                const auto newCt5FileXYSwapped = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "") +
                                            "/config/calib/" + newFileNameXYSwapped;
                std::for_each(scannerPosition.begin(), scannerPosition.end(), [](auto& pair) {std::swap(pair.first, pair.second);});
                std::for_each(spotPosition.begin(), spotPosition.end(), [](auto& pair) {std::swap(pair.first, pair.second);});
                usleep(1000000); // prevent same log file timestamp
                generateScanlabCt5File(scannerPosition, spotPosition, currentCorrectionFile, newCt5FileXYSwapped, CalibrationFileFitStrategy::UseFittedValue);
            }
            break;
        }
        case interface::ScannerModel::SmartMoveScanner:
        {
            // Swap X and Y first!
            std::for_each(scannerPosition.begin(), scannerPosition.end(), [](auto& pair) {std::swap(pair.first, pair.second);});
            std::for_each(spotPosition.begin(), spotPosition.end(), [](auto& pair) {std::swap(pair.first, pair.second);});

            generateSmartMoveCorrectionFile(scannerPosition, spotPosition, currentCorrectionFile, newCorrectionFile, CalibrationFileFitStrategy::UseFittedValue);
            break;
        }
        default:
        {
            break;
        }
    }

    if (!std::filesystem::exists(newCorrectionFile) || !std::filesystem::is_regular_file(newCorrectionFile))
    {
        wmLog(eError, "[calibrateScanner] Calibration Failed: No correction file was generated!\n");
        return false;
    }

    wmLog(eWarning, "[calibrateScanner] New Correction File saved to " + newCorrectionFile + "\n");
    wmLog(eWarning, "[calibrateScanner] Please update the system parameter '%s' manually and restart the system!\n", customCorrectionFileKey);

    return true;
}

bool CalibrateScanField::calibrateCamera()
{
    m_rCalibrationManager.setIndexForNextImageFromDisk(0);

    // 1. Determine orientation of scanner relative to camera
    // - move scanner to (0,0), (1,0) and (0,1). Take an image at each position
    m_rCalibrationManager.setScannerPosition(0.0, 0.0);
    BImage imageLast;
    BImage image;
    m_rCalibrationManager.getImage().copyPixelsTo(imageLast);

    const auto period = horizontalPeriod(imageLast.begin(), imageLast.height(), imageLast.width(), imageLast.stride());
    wmLog(eWarning, "[calibrateCamera] Detected Calibration Plate Size: %d px\n", period);

    m_rCalibrationManager.clearCanvas();
    m_rCalibrationManager.drawText(10, 10, "Scanner origin 0,0", Color::m_oOrangeDark);
    m_rCalibrationManager.renderImage(imageLast);

    m_rCalibrationManager.setScannerPosition(1.0, 0.0);
    image = m_rCalibrationManager.getImage();
    m_rCalibrationManager.clearCanvas();
    m_rCalibrationManager.renderImage(image);

    const auto dxShift = imageShift(imageLast.begin(), image.begin(), image.height(), image.width(), imageLast.stride(), image.stride());
    wmLog(eDebug, "[calibrateCamera] Scanner 1.0 mm, 0.0 mm, image shift: %d, %d\n", dxShift.first, dxShift.second);

    m_rCalibrationManager.setScannerPosition(0.0, 1.0);
    image = m_rCalibrationManager.getImage();
    m_rCalibrationManager.clearCanvas();
    m_rCalibrationManager.renderImage(image);

    const auto dyShift = imageShift(imageLast.begin(), image.begin(), image.height(), image.width(), imageLast.stride(), image.stride());
    wmLog(eDebug, "[calibrateCamera] Scanner 0.0 mm, 1.0 mm, image shift: %d, %d\n", dyShift.first, dyShift.second);

    enum class AxisDirection
    {
        Left = 0, Right = 1, Top = 2, Bottom = 3
    };

    std::string axisDirectionMap[4] = {"Left", "Right", "Top", "Bottom"};

    const AxisDirection xDirection = std::abs(dxShift.first) > std::abs(dxShift.second) ?
                               (dxShift.first > 0 ? AxisDirection::Left : AxisDirection::Right) :
                               (dxShift.second > 0 ? AxisDirection::Top : AxisDirection::Bottom);

    const AxisDirection yDirection = std::abs(dyShift.first) > std::abs(dyShift.second) ?
                               (dyShift.first > 0 ? AxisDirection::Left : AxisDirection::Right) :
                               (dyShift .second > 0 ? AxisDirection::Top : AxisDirection::Bottom);

    wmLog(eWarning, "xDirection: %s, yDirection: %s", axisDirectionMap[(int)xDirection], axisDirectionMap[(int)yDirection]);

    const auto& calibrationParameters = m_rCalibrationManager.getCalibrationData(0).getParameters();
    const auto xMin = calibrationParameters.getDouble("SM_X_min");
    const auto xMax = calibrationParameters.getDouble("SM_X_max");
    const auto yMin = calibrationParameters.getDouble("SM_Y_min");
    const auto yMax = calibrationParameters.getDouble("SM_Y_max");
    const auto periodMm = 10;

    std::vector<std::pair<double, double>> scannerPosition;
    // calculate scanner positions
    for (auto y = std::ceil(yMin / periodMm) * periodMm; y <= yMax; y = std::floor(y + periodMm))
    {
        for (auto x = std::ceil(xMin / periodMm) * periodMm; x <= xMax; x = std::floor(x + periodMm))
        {
            scannerPosition.emplace_back(x, y);
        }
    }

    std::vector<std::vector<Corner>> scanfieldResult;
    for (const auto& s : scannerPosition)
    {
        m_rCalibrationManager.setScannerPosition(s.first, s.second);
        image = m_rCalibrationManager.getImage();
        const auto imageResult = findChessBoardCorners(image.begin(), image.height(), image.width(), image.stride(), period);

        scanfieldResult.emplace_back(imageResult);

        m_rCalibrationManager.clearCanvas();
        for (const auto& corner : imageResult)
        {
            m_rCalibrationManager.drawCross(corner.x + 0.5, corner.y + 0.5, 10, Color::Green());
        }
        m_rCalibrationManager.renderImage(image);
    }

    const auto calibrationResult = postprocess(scannerPosition, scanfieldResult, period, image.width() * 0.5, image.height() * 0.5, xDirection == AxisDirection::Right ? 1 : -1, yDirection == AxisDirection::Bottom ? 1 : -1);

    auto& calibrationData = m_rCalibrationManager.getCalibrationData(0);
    const auto pixelSizeMm = calibrationParameters.getDouble("DpixX");
    const double squareSizeMm = 5.0;
    const double pixelToMm = period / (2 * squareSizeMm);

    //Linear model
    calibrationData.setKeyValue("beta0", pixelSizeMm * period / (2 * squareSizeMm));
    calibrationData.setKeyValue("InvertX", xDirection == AxisDirection::Left);
    calibrationData.setKeyValue("InvertY", yDirection == AxisDirection::Bottom);
    calibrationData.setKeyValue("SM_scanXToPixel", xDirection == AxisDirection::Left ? -pixelToMm : pixelToMm);
    calibrationData.setKeyValue("SM_scanYToPixel", yDirection == AxisDirection::Bottom ?  pixelToMm : -pixelToMm);

    //Distortion Model
    const auto& modelx = calibrationResult.modelx;
    const auto& modely = calibrationResult.modely;
    if (calibrationData.isValueInRange("ScanfieldDistortion_kax1", modelx.at(0)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kax2", modelx.at(1)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kbx1", modelx.at(2)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kbx2", modelx.at(3)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kcx", modelx.at(4)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kdx", modelx.at(5)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kex", modelx.at(6)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kfx", modelx.at(7)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kay1", modely.at(0)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kay2", modely.at(1)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kby1", modely.at(2)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kby2", modely.at(3)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kcy", modely.at(4)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kdy", modely.at(5)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_key", modely.at(6)) &&
        calibrationData.isValueInRange("ScanfieldDistortion_kfy", modely.at(7)))
    {
        calibrationData.setKeyValue("ScanfieldDistortion_kax1", modelx.at(0), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kax2", modelx.at(1), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kbx1", modelx.at(2), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kbx2", modelx.at(3), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kcx", modelx.at(4), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kdx", modelx.at(5), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kex", modelx.at(6), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kfx", modelx.at(7), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kay1", modely.at(0), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kay2", modely.at(1), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kby1", modely.at(2), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kby2", modely.at(3), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kcy", modely.at(4), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kdy", modely.at(5), true);
        calibrationData.setKeyValue("ScanfieldDistortion_key", modely.at(6), true);
        calibrationData.setKeyValue("ScanfieldDistortion_kfy", modely.at(7), true);

        wmLog(eWarning, "[calibrateCamera] changed 'ScanfieldDistortion' parameters\n");
    }
    else
    {
        wmLog(eError, "Scan field distortion calibration failed!\n");
    }

    // TCP
    const auto tcp0x = calibrationResult.tcp0x;
    const auto tcp0y = calibrationResult.tcp0y;

    calibrationData.setKeyValue("xtcp", tcp0x);
    calibrationData.setKeyValue("ytcp", tcp0y);

    wmLog(eWarning, "[calibrateCamera] changed calibration parameter 'xtcp' to: %d px\n", tcp0x);
    wmLog(eWarning, "[calibrateCamera] changed calibration parameter 'ytcp' to: %d px\n", tcp0y);

    // TCP offset table
    const std::string filePath = calibrationData.getFilenamesConfiguration().getCameraCorrectionGridFilename();

    std::fstream offsetTable;
    offsetTable.open(filePath, std::ios_base::out);

    if (!offsetTable.is_open() || offsetTable.fail())
    {
        wmLog(eError, "[calibrateCamera] Calibration failed: cannot open '%s'\n", filePath);
        return false;
    }

    offsetTable << "Scanner X; Scanner Y; TCP x; TCP y;" << std::endl;

    for (std::size_t i = 0; i < scannerPosition.size(); ++i)
    {
        offsetTable << scannerPosition[i].first << ";"
                << scannerPosition[i].second << ";"
                << calibrationResult.tcpx[i] - tcp0x << ";"
                << calibrationResult.tcpy[i] - tcp0y << ";"
                << std::endl;
    }

    offsetTable.close();

    // reload
    calibrationData.setCalibrationCorrectionContainer(coordinates::CalibrationCameraCorrectionContainer::load(filePath));

    wmLog(eWarning, "[calibrateCamera] updated TCP offset table '%s'\n", filePath);

    return true;
}

bool CalibrateScanField::saveImage(const image::BImage & rImage, std::string filename)
{
    fileio::Bitmap oBitmap(filename, rImage.width(), rImage.height(),false);
    return oBitmap.isValid() & oBitmap.save(rImage.data());
}


bool CalibrateScanField::saveDebugImage(std::string label) const
{
    if (m_oDebugFolder.empty())
    {
        return false;
    }
    return saveImage(m_rCalibrationManager.getCurrentImage(), m_oDebugFolder+"/"+label+".bmp");
}

math::CalibrationCornerGrid CalibrateScanField::chessboardRecognition(const image::BImage & rImage, CalibrationManager * pCalibrationManager) const
{
    wmLog(eDebug, "CalibMgr ChessboardRecognition \n");
    precitec::calibration_algorithm::ChessboardRecognitionAlgorithm oChessboardRecognition(rImage);
    const auto oCornerColor = Color::Red();
    const auto oEdgeColor = Color::Yellow();
    auto & rCorners = oChessboardRecognition.getRecognizedCorners();
    wmLog(eInfo, "Found %d corners \n", rCorners.size() );
    if (pCalibrationManager)
    {
        for (auto && rCorner : rCorners)
        {
            pCalibrationManager->drawPixel(rCorner.x, rCorner.y, oCornerColor);
        }
    }
    if (!oChessboardRecognition.isValid())
    {
        wmLog(eDebug, "No valid grid found \n");
        return math::CalibrationCornerGrid{{0, 0}};
    }
    const auto & rCornerGrid =  oChessboardRecognition.getCornerGrid();
    
    if (pCalibrationManager)
    {
        const auto paintLineWidth = rImage.width();
        double x1,y1,x2,y2;
        for (auto && rFittedLine :  rCornerGrid.getAllLines())
        {
            rFittedLine.get2Points(x1,y1,x2,y2,paintLineWidth);
            pCalibrationManager->drawLine(x1,y1,x2,y2,oEdgeColor);
        }
    }
    return rCornerGrid;

}

bool CalibrateScanField::setCalibrationType(int p_calibType)
{    
    switch (p_calibType)
    {
        case interface::CalibType::eAcquireScanFieldImage:
            m_calibrationType = interface::CalibType::eAcquireScanFieldImage;
            return true;
        case interface::CalibType::eCalibrateScanFieldTarget:
            m_calibrationType = interface::CalibType::eCalibrateScanFieldTarget;
            return true;
        case interface::CalibType::eCalibrateScanFieldIDM_Z:
            m_calibrationType = interface::CalibType::eCalibrateScanFieldIDM_Z;
            return true;
        case interface::CalibType::eVerifyScanFieldIDM_Z:
            m_calibrationType = interface::CalibType::eVerifyScanFieldIDM_Z;
            return true;
        case interface::CalibType::eScannerCalibrationMeasure:
            m_calibrationType = interface::CalibType::eScannerCalibrationMeasure;
            return true;
        case interface::CalibType::eScanmasterCameraCalibration:
            m_calibrationType = interface::CalibType::eScanmasterCameraCalibration;
            return true;
        default:
            assert(false);
            //set safe calibration type, but we should not be here at all
            m_calibrationType = interface::CalibType::eAcquireScanFieldImage;
            return false;
    }
    
}

double CalibrateScanField::rmsErrorOnChessboard_pix(const math::CalibrationCornerGrid& rCornerGrid, double expectedSide_pix)
{
    auto oSumSquareErrors = 0;
    auto oAllSegments = rCornerGrid.getAllSegmentsScreen(); //by construction they are unique
    if (oAllSegments.size() == 0)
    {
        return 0.0;
    }
    
   
    for (auto & rSegment: oAllSegments)
    {
        auto & rPointA = rSegment[0];
        auto & rPointB = rSegment[1];
        
        static_assert(std::is_same<decltype(rPointA.ScreenX), double>::value, "the current implementation assumes corner positions are exact");        
        
        double dx = rPointB.ScreenX - rPointA.ScreenX;
        double dy = rPointB.ScreenY - rPointA.ScreenY;
        auto error =  std::sqrt(dx*dx + dy*dy) - expectedSide_pix;
        oSumSquareErrors += (error*error);        
    }
    return std::sqrt(oSumSquareErrors/oAllSegments.size());
}

BImage IDMMeasurements::computeDepthImageWithInterpolation(
        const std::vector<ScanFieldGridParameters::ScannerPosition> & rScannerPositions, const std::vector<double> & zValues) const
{
    assert(rScannerPositions.size() == zValues.size());

    auto bounds = std::minmax_element(zValues.begin(), zValues.end());
    double globalMinZ = *bounds.first;
    double globalMaxZ = *bounds.second;

    double Zrange = (globalMaxZ - globalMinZ);
    if (Zrange < 1e-6)
    {
        Zrange = 1;
    }

    BImage image(m_imageSize);

    auto oZContainer = [&zValues, &rScannerPositions] ()
        {
            //FIXME use an appropriate structure
            std::vector<std::pair<geo2d::DPoint, coordinates::CalibrationIDMCorrection>> pseudoCorrections;
            pseudoCorrections.reserve(zValues.size());
            for (int i = 0, n = rScannerPositions.size(); i < n; i++)
            {
                auto & rScannerPosition = rScannerPositions[i];
                double z = zValues[i];
                pseudoCorrections.push_back({
                    geo2d::DPoint{rScannerPosition.x, rScannerPosition.y},
                    coordinates::CalibrationIDMCorrection{static_cast<int>(z)}});
            }
            return coordinates::CalibrationIDMCorrectionContainer( pseudoCorrections );
        }();

    for (int row = 0; row < m_imageSize.height; row++)
    {
        double y =  yDepthImageToScanner(row);
        for (int col = 0; col < m_imageSize.width; col ++)
        {
            double x  =  xDepthImageToScanner(col);
            auto z = oZContainer.getDelta(x,y);
            image[row][col] = (z - globalMinZ) * 255 / Zrange;
        }
    }
    return image;
}



BImage IDMMeasurements::computeDepthImageWithoutInterpolation(
        const std::vector<ScanFieldGridParameters::ScannerPosition> & rScannerPositions, const std::vector<double> & zValues,
        byte rangeGray, double rangeZ) const
{
    assert(rScannerPositions.size() == zValues.size());
    assert((int)(rScannerPositions.size()) == (m_oScanFieldGridParameters.numCols() +1) *( m_oScanFieldGridParameters.numRows()+1));

    BImage image(m_imageSize);
    auto dx = m_imageSize.width / double( m_oScanFieldGridParameters.numCols());
    auto dy = m_imageSize.height / double( m_oScanFieldGridParameters.numRows());
    if (zValues.empty())
    {
        return image;
    }
    auto meanZ = std::accumulate(zValues.begin(), zValues.end(), 0.0) / zValues.size();

    if (std::isnan(rangeZ))
    {
        auto bounds = std::minmax_element(zValues.begin(), zValues.end());
        double globalMinZ = *bounds.first;
        double globalMaxZ = *bounds.second;
        rangeZ = 2 * std::max(globalMaxZ - meanZ, meanZ - globalMinZ);
    }
    if (rangeZ < 1e-6)
    {
        rangeZ = 1;
    }
    rangeZ = std::abs(rangeZ);
    rangeGray = std::max<byte>(std::min<byte>(rangeGray, 255),1);

    const double halfZRange = rangeZ / 2.0;
    const double minZInRange = meanZ - halfZRange;
    const double maxZInRange = meanZ + halfZRange;

    auto zToGrayLevel = [&minZInRange, &maxZInRange, &meanZ, &rangeGray, &rangeZ](double z)
        {
            if (z < minZInRange)
            {
                return 0;
            }
            if (z > maxZInRange)
            {
                return 255;
            }
            return byte((z-meanZ) * rangeGray/rangeZ) + 125;
        };

    Size2d tileSize(dx,dy);

    for (int i = 0, n = zValues.size(); i < n; i++)
    {
        auto & rScannerPosition = rScannerPositions[i];
        auto & z = zValues[i];
        auto X = xScannerToDepthImage(rScannerPosition.x);
        auto Y = yScannerToDepthImage(rScannerPosition.y);
        geo2d::Rect tile(geo2d::Point(std::round(X-dx/2),std::round(Y-dy/2)), tileSize);
        BImage roi (image, geo2d::intersect(tile, m_imageSize), true);
        auto intensity = zToGrayLevel(z);
        roi.fill(intensity); //roi is a shallow copy: also the original image will be updated
    }
    return image;
}


BImage IDMMeasurements::computeDepthImageFromRawValues(CalibrationManager * pCalibrationManager) const
{
    assert(m_calibratedValues.size() == m_medianValues.size());
    
    
    BImage imageZ = computeDepthImageWithInterpolation(m_scannerPositions, m_medianValues);

    if (pCalibrationManager != nullptr)
    {
        auto oImageNotInterpolated  = computeDepthImageWithoutInterpolation(m_scannerPositions, m_medianValues);
        pCalibrationManager->drawImage(0,0, oImageNotInterpolated, "Raw IDM Measurements", Color::Green() );
        pCalibrationManager->renderImage( imageZ);
    }
    return imageZ;
}

BImage IDMMeasurements::computeDepthImageFromCalibratedValues(CalibrationManager * pCalibrationManager, byte rangeGray, double rangeZ) const
{

    assert(m_calibratedValues.size() == m_rawMeasurements.size());
    
    auto bounds = std::minmax_element(m_calibratedValues.begin(), m_calibratedValues.end());
    double globalMinZ = *bounds.first;
    double globalMaxZ = *bounds.second;

    BImage imageZ = computeDepthImageWithoutInterpolation(m_scannerPositions, m_calibratedValues, rangeGray, rangeZ);

    if (pCalibrationManager != nullptr)
    {
        pCalibrationManager->drawText(0, 20,  "minimum Z " + std::to_string(globalMinZ), Color::m_oOrangeDark);
        pCalibrationManager->drawText(0, 50,  "maximum Z " + std::to_string(globalMaxZ), Color::m_oOrangeDark);
        
        pCalibrationManager->drawText(0, m_imageSize.height/2,  "min X" + std::to_string(m_oScanFieldGridParameters.xMin), Color::m_oOrangeDark);
        pCalibrationManager->drawText(m_imageSize.width - 200, m_imageSize.height/2, "max X" + std::to_string(m_oScanFieldGridParameters.xMax), Color::m_oOrangeDark);
        pCalibrationManager->drawText(m_imageSize.width/2 - 200, 10,  "min Y" + std::to_string(m_oScanFieldGridParameters.yMin), Color::m_oOrangeDark);
        pCalibrationManager->drawText(m_imageSize.width/2 - 200, m_imageSize.height -50, "max Y" + std::to_string(m_oScanFieldGridParameters.yMax), Color::m_oOrangeDark);
        
        pCalibrationManager->renderImage( imageZ );
    }
    return imageZ;
}

void IDMMeasurements::renderBackgroundImage(CalibrationManager& rCalibrationManager) const
{
    //rCalibrationManager.clearCanvas();
    rCalibrationManager.renderImage(m_oBackgroundImage);
}

IDMMeasurements::IDMMeasurements(const ScanFieldGridParameters & oScanFieldGrid, bool minimizeJump):
m_oScanFieldGridParameters(oScanFieldGrid),
m_scannerPositions{oScanFieldGrid.computeScannerPositions(minimizeJump)}
{
    m_imageSize = Size2d{1024,1024};
    
    double yRange = oScanFieldGrid.yRange();
    double xRange = oScanFieldGrid.xRange();
    
    if (xRange < yRange)
    {
        m_imageSize.width = 512 * xRange / yRange;
    }
    if (yRange < xRange)
    {
        m_imageSize.height = 512 * yRange / xRange;
    }
    m_oBackgroundImage = BImage{m_imageSize};
}


void IDMMeasurements::renderMeasurementPoint(ScanFieldGridParameters::ScannerPosition rScannerPosition, CalibrationManager& rCalibrationManager,
                                             std::string labelTop, std::string labelBottom ) const
{
    double X =  xScannerToDepthImage(rScannerPosition.x);
    double Y =  yScannerToDepthImage(rScannerPosition.y);
    rCalibrationManager.drawCross(X, Y, 5, Color::m_oScarletRed);
    if (!labelTop.empty())
    {
        rCalibrationManager.drawText(X, Y - 20, labelTop, Color::m_oScarletRed);
    }
    if (!labelBottom.empty())
    {
        rCalibrationManager.drawText(X, Y, labelBottom, Color::m_oScarletRed);
    }
    rCalibrationManager.renderImage(m_oBackgroundImage);

}


void IDMMeasurements::computeCalibratedValues(const math::CalibrationData & rCalibrationData)
{
    assert(m_medianValues.size() == m_rawMeasurements.size());
    
    m_calibratedValues.clear();
    m_calibratedValues.reserve(m_rawMeasurements.size());
    if (!rCalibrationData.hasIDMCorrectionGrid())
    {
        wmLog(eWarning,"Calibrated IDM values requested, but no calibration available \n");
    }
    for (int i = 0, n = m_rawMeasurements.size(); i < n; i++)
    {
        auto & rScannerPosition = m_scannerPositions[i];
        double medianValue = m_medianValues[i];
        m_calibratedValues.push_back(rCalibrationData.getCalibratedIDMWeldingDepth(rScannerPosition.x, rScannerPosition.y, medianValue));
    }
}

void IDMMeasurements::computeCalibratedValues(coordinates::CalibrationIDMCorrectionContainer & rCalibrationIDMCorrectionContainer)
{
    assert(m_medianValues.size() == m_rawMeasurements.size());
    
    m_calibratedValues.clear();
    m_calibratedValues.reserve(m_rawMeasurements.size());
    
    for (int i = 0, n = m_rawMeasurements.size(); i < n; i++)
    {
        auto & rScannerPosition = m_scannerPositions[i];
        double medianValue = m_medianValues[i];
        m_calibratedValues.push_back(rCalibrationIDMCorrectionContainer.get(rScannerPosition.x, rScannerPosition.y, medianValue));
    }
}


coordinates::CalibrationIDMCorrectionContainer IDMMeasurements::computeCorrectionContainer()
{
    assert(m_medianValues.size() == m_scannerPositions.size());
    
    m_offsets.clear();
    m_offsets.reserve(m_medianValues.size());
    auto cmp =
        [](const ScanFieldGridParameters::ScannerPosition & a,
           const ScanFieldGridParameters::ScannerPosition & b)
    {
        auto d_a = std::abs(a.x) +  std::abs(a.y);
        auto d_b = std::abs(b.x) +  std::abs(b.y);
        return d_a < d_b;
    };

    auto itReference = std::min_element(m_scannerPositions.begin(), m_scannerPositions.end(),cmp );
    auto indexReference = std::distance(m_scannerPositions.begin(), itReference);
    double refZ = computeMedian(m_rawMeasurements[indexReference]);

    for (int i = 0, n = m_rawMeasurements.size(); i < n; i++)
    {
        auto & rScannerPosition = m_scannerPositions[i];
        double medianValue = m_medianValues[i];
        m_offsets.push_back({ geo2d::DPoint{rScannerPosition.x, rScannerPosition.y},
            coordinates::CalibrationIDMCorrection{int(refZ - medianValue) }});
    }
    auto oContainer = coordinates::CalibrationIDMCorrectionContainer(m_offsets);

    assert(oContainer.m_data.getNumberOfMeasurements() > 0);
    assert(oContainer.m_data.getNumberOfMeasurements() == m_rawMeasurements.size());
    return oContainer;
}

void IDMMeasurements::computeMedianValues()
{
    assert(m_rawMeasurements.size() == m_scannerPositions.size());

    m_medianValues.resize(m_rawMeasurements.size());
    std::transform(m_rawMeasurements.begin(), m_rawMeasurements.end(), m_medianValues.begin(), IDMMeasurements::computeMedian);

    
    assert(m_medianValues.size() == m_scannerPositions.size());
}

bool IDMMeasurements::saveIDMCorrectionFile(std::string filename) const
{
    
    wmLog(eDebug, "Save IDM Z grid to " + filename +"\n");
    
    std::ofstream oOut(filename);
    if (!oOut.good())
    {
        wmLog(eWarning, "File" + filename + "cannot be written\n");
        return false;
    }
    const char separator = ';' ;

    oOut << "Scanner X " << separator
         << "Scanner Y " << separator
         << "median value" << separator
         << "offset " << separator
         << "calibrated value " << separator
         << "raw values " << separator
         << "\n";

    bool hasOffset = (m_offsets.size() == m_rawMeasurements.size());
    bool hasCalibratedValues = (m_calibratedValues.size() == m_rawMeasurements.size());
    
    for (int i = 0, n = m_rawMeasurements.size(); i < n; i++)
    {
        auto & rScannerPosition = m_scannerPositions[i];

        oOut << rScannerPosition.x << separator  << rScannerPosition.y << separator
             << m_medianValues[i] << separator
             << int(hasOffset ? m_offsets[i].second.m_oDelta : 0) << separator
             << double(hasCalibratedValues? m_calibratedValues[i] : -999.0) << separator;

        for (auto value : m_rawMeasurements[i])
        {
            oOut << value << separator;
        }
        oOut << "\n";
    }
    return true;

}

void IDMMeasurements::addMeasurements(int index, std::vector<double> oIDMRawMeasurement)
{
    assert( (int) (m_rawMeasurements.size()) == index);
    m_rawMeasurements.emplace_back(std::move(oIDMRawMeasurement));
}

int IDMMeasurements::getNumberOfPositions() const
{
    return m_scannerPositions.size();
}

const ScanFieldGridParameters::ScannerPosition & IDMMeasurements::getScannerPosition(int index) const
{
    assert(index < (int)m_scannerPositions.size());
    return m_scannerPositions[index] ;
}

double IDMMeasurements::xDepthImageToScanner(int col) const
{
    return m_oScanFieldGridParameters.xMin + (m_oScanFieldGridParameters.xMax - m_oScanFieldGridParameters.xMin) * col / m_imageSize.width;
}

double IDMMeasurements::yDepthImageToScanner(int row) const
{
    return m_oScanFieldGridParameters.yMin + (m_oScanFieldGridParameters.yMax - m_oScanFieldGridParameters.yMin) * row / m_imageSize.height;
}

double IDMMeasurements::xScannerToDepthImage(double x) const
{
    double xRange = m_oScanFieldGridParameters.xRange();
    return xRange == 0 ? 0: m_imageSize.width * (x - m_oScanFieldGridParameters.xMin) / xRange;
}

double IDMMeasurements::yScannerToDepthImage(double y) const
{
    double yRange = m_oScanFieldGridParameters.yRange();
    return yRange == 0 ? 0: m_imageSize.height * (y - m_oScanFieldGridParameters.yMin) / yRange;
}

/*static*/ double IDMMeasurements::computeMedian(IDMAcquisition::RawIdmMeasurements values)
{
    std::sort(values.begin(), values.end());
    return values[values.size()/2];
};



}
}
