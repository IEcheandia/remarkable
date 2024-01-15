#ifndef CALIBRATESCANFIELD_H
#define CALIBRATESCANFIELD_H

#include <calibration/calibrationProcedure.h>
#include <message/calibration.interface.h>
#include "geo/point.h"
#include "image/image.h"

#include "util/ScanFieldImageParameters.h"

// includes specific for CalibrationGraphCircleDetection
#include <util/CalibrationCorrectionContainer.h> //for CalibrationCorrection
#include "calibrationGraph.h"

namespace precitec
{
namespace math
{
    class CalibrationCornerGrid;
    class CalibrationParamMap;
}
namespace calibration
{
class ScanFieldImageCalculator;


class CalibrateScanField : public CalibrationProcedure
{
    public:

        CalibrateScanField(CalibrationManager & p_rCalibrationManager);
        ~CalibrateScanField();
        virtual bool calibrate();
        bool setCalibrationType(int p_calibType);
private:
             
        bool prepareExportFolder(); 

        math::CalibrationCornerGrid chessboardRecognition(const image::BImage & rImage, CalibrationManager * pCalibrationManager) const;
        static double rmsErrorOnChessboard_pix(const math::CalibrationCornerGrid & rCornerGrid, double expectedSide_pix);
        
        interface::CalibType m_calibrationType = interface::CalibType::eAcquireScanFieldImage;
        
        ScanFieldGridParameters loadScanFieldGridParameters() const;
        bool calibrateCameraTarget() const;
        bool acquireScanFieldImage() const;
        bool calibrateIDMSurface(bool updateSystemCalibration) const;
        bool calibrateScanner();
        bool calibrateCamera();
                                      
        static bool saveImage(const image::BImage & rImage, std::string filename);
        bool saveDebugImage(std::string label) const;
    
        std::string m_oDebugFolder;
        std::string m_oScanFieldImageFolder;
        bool m_oHasCamera;
        
        
}; 

class CalibrationGraphScanmaster
{
public:

    typedef std::array<geo2d::DPoint, 4>  EnclosingSquare;
    struct CircleResult
    {
        double x = 0.0;
        double y = 0.0;
        double r = 0.0;
        EnclosingSquare square;
        double p;
        geo2d::DPoint getTopLeftCorner() const
        {
            return square[0];
        }
        geo2d::DPoint getTopRightCorner() const
        {
            return square[1];
        }
        geo2d::DPoint getBottomLeftCorner() const
        {
            return square[2];
        }
        geo2d::DPoint getBottomRightCorner() const
        {
            return square[3];
        }
    };

    
    CalibrationGraphScanmaster(CalibrationManager & rCalibrationManager);
    bool getUseGridRecognition() const { return m_useGridRecognition;}
    void setUseGridRecognition(bool value);
    bool getExtraDebug() const { return m_extraDebug;}
    void setExtraDebug(bool value)
    {
        m_extraDebug = value;
    };
    int getNumRepetions() const {return m_numRepetitions;}
    void setNumRepetitions(int value)
    {
        m_numRepetitions = value;
    }
    
    bool init(double threshold, std::array<int,4> searchROI, int borderGridSearch, int sideGridSearch, int expectedRadius, int numRepetitions);
    std::pair<bool, coordinates::CalibrationCameraCorrection>  evaluateTranslation();

    std::pair<bool, coordinates::CalibrationCameraCorrection> evaluateCircleFit();

    std::tuple<bool, coordinates::CalibrationCameraCorrection, CircleResult> evaluateAll();
    
    double getRatio_pix_mm_from_Grid(double gridSide_mm) const; 
    double getRatio_pix_mm_from_Circle(double circleRadius_mm) const; 
    geo2d::DPoint getReferencePosition() const;
    
    private:
    // utility function to reload the images from disk (for development, without camera)
    void resetIndexImageFromDisk();
    
    /*
     * Methods for graph manipulation
     */
    bool setRoi(unsigned int x, unsigned int y, unsigned int w, unsigned int h);
    bool setSearchRadius(int radiusMin, int radiusMax, int step);
    bool setSearchOutsideROI(bool value);
    bool setGridSearch(int border, int roiShortSide);

    void setRoiAroundCircle(CircleResult p_circle, int border);
    
    /*
     * Methods for graph evaluation
     */
    CircleResult fitCircle();
    
    CalibrationGraph m_oCircleDetectionGraph;    
    CircleResult m_oReferenceCircle;
    double m_treshold = 1.0;
    int m_tol_pix = 0.0; //max distance in pixel between the center of the detected circle and the enclosing square
    int m_numRepetitions = 1;
    bool m_useGridRecognition = true;
    
    //defined in graph
    enum GraphResult
    {   id_x = 28,
        id_y = 29,
        id_r = 26,
        id_x1 = 585,
        id_y1 = 586,
        id_x2 = 587,
        id_y2 = 588,
        id_x3 = 589,
        id_y3 = 590,
        id_x4 = 591,
        id_y4 = 592,
        COUNT = 11
    };
    struct GraphFilterInstancesUUID
    {
        const std::string m_searchROI = "63c18992-0dae-4519-9e1c-91692272e39e";
        const std::string m_border = "e4ad7f63-b7a0-4281-bb08-1ac894d5cbb9";
        const std::string m_roiShortSide = "fcd9a393-2d69-40e6-aef4-68448cda64dc";
        const std::string m_circleDetection = "bb8dfbe2-36e2-4066-a086-7adce3cfc2cb";
        const std::string m_radiusStart = "6c84e847-9384-44a2-9eff-751e210f4021";
        const std::string m_radiusEnd  = "2677efbb-515d-48df-b8f2-5dec96c343b6";
    };
    GraphFilterInstancesUUID m_GraphFilterInstancesUUID;
    
    bool m_hasCamera = true;
    bool m_extraDebug = false;
    
};

class IDMAcquisition
{
public:
    IDMAcquisition(CalibrationManager & rCalibrationManager)
    : m_rCalibrationManager(rCalibrationManager)
    {
    }
    typedef std::vector<double> RawIdmMeasurements;
    std::pair<bool, RawIdmMeasurements> evaluateIDMWeldingDepth(int trigger_ms);
    
    bool getExtraDebug() const { return m_extraDebug;}
    void setExtraDebug(bool value)
    {
        m_extraDebug = value;
    };
    int getNumRepetions() const {return m_numRepetitions;}
    void setNumRepetitions(int value)
    {
        m_numRepetitions = value;
    }
    
private:    
    bool m_extraDebug = false;
    int m_numRepetitions = 1;
    CalibrationManager & m_rCalibrationManager;
};
    
    
} //end namespace
}
#endif
