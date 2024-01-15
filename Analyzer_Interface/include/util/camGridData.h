#ifndef CAMGRIDDATA_H
#define CAMGRIDDATA_H

#include <string>
#include <vector>
#include <map>
#include <tuple> // for tuple
#include <utility> // for pair
#include "math/calibrationCommon.h"
#include "geo/coordinate.h"

#include <Analyzer_Interface.h>

namespace precitec
{
namespace system
{

typedef std::vector<geo2d::coordScreenToPlaneDouble> t_coordScreenToPlaneLine;
typedef std::vector<geo2d::coord2D> tGridMapLine2D;
typedef std::map<int, tGridMapLine2D > tGridMap;           ///< Map yCoordAvg -> (x, y) that represents a points in a tube (y avg is the avg y of all ys of the line)
typedef std::map<int, t_coordScreenToPlaneLine> tGridMap2D3D;   ///< Extended version of tGridMap, including the sensor/real world data in addition.
typedef std::map<int, std::pair<int, int> > tLineBoundaries;     ///< Each line of Scheimpflug sensor has information about which area has valid 3D coordinates. Simple Start-End screen coordinates are stored in this map of tuples.


class ANALYZER_INTERFACE_API CamGridData
{
    
public:
    enum class ScheimpflugCameraModel
    {
        AA100 = 0,
        AA150,
        AA200,
        F64
    };
    static const int scheimpflugCameraModelCount = 3;

    static double scheimpflugCameraPatternSize(ScheimpflugCameraModel  model)
    {
        switch(model)
        {
            case ScheimpflugCameraModel::AA100: return 0.60;
            case ScheimpflugCameraModel::AA150: return 1.35;
            case ScheimpflugCameraModel::AA200: return 2.20;
            case ScheimpflugCameraModel::F64: return 0.60;
        }
        return 0;
    }
    //optics angle and scheimpflugAngle in degrees
    static std::array<double,2> scheimpflugCameraAngles(ScheimpflugCameraModel model)
    {
        switch(model)
        {
            case ScheimpflugCameraModel::AA100: return {8.6, 37.0} ;
            case ScheimpflugCameraModel::AA150: return {8.6, 29.0};
            case ScheimpflugCameraModel::AA200: return {8.6, 23.0};
            case ScheimpflugCameraModel::F64: return {0.0, 0.0};
        }
        return {0,0};
    }


    CamGridData();

    // camera flash memory
    static const unsigned int mBytesHeaderSize;
    static const unsigned int mBytesMaxAddress;
    static bool checkBytesHeader(const std::vector<std::uint8_t> & rBytes, std::uint16_t & rEndAddress, std::uint32_t& rChecksum);
    
    bool isCompatibleWithHeader(const std::vector<std::uint8_t> & rBytes) const;
    void saveToBytes(std::vector<std::uint8_t> & rBytes) const;
    bool loadFromBytes(const std::vector<std::uint8_t> & rBytes,std::uint32_t& rChecksum, bool verifyChecksum);
    bool saveToBytes(const std::string filename) const;
    bool loadFromBytes(const std::string filename, std::uint32_t& rChecksum, bool verifyChecksum);

    // getter
    double gridDelta() const   { return m_oGridDelta; }    ///< Returns checker board side length [mm] (rectangular pattern assumed!). Default: 1.35
    double triangulationAngle_rad() const {return m_oAngleRad; } ///< Returns triangulation angle.
    double triangulationAngle_deg() const { return m_oAngleRad * 180.0 / M_PI; } ///< Returns triangulation angle.
    int sensorWidth() const  { return m_oSensorWidth; }  ///< Returns sensor width. Default: 1024
    int sensorHeight() const   { return m_oSensorHeight; } ///< Returns sensor height. Default: 1024
    int originX()  const      { return m_oSensorWidth/2; }        ///< Returns sensor origin x. Default: width/2 per default
    int originY()  const      { return m_oSensorHeight/2; }        ///< Returns sensor origin y. Default: height/2
    std::string serial() const { return m_oSerial; }       ///< Returns camera serial. Default: empty string
    const tGridMap& gridRef() const  { return m_oGrid; }         ///< Returns reference to grid map
	double correctionFactor() const { return m_oCorrectionFactor; }


    // setter
    void setGridMap(const tGridMap &p_rGrid);               ///< Sets grid map by reference (no copy action performed!).
    void setSerial(const std::string p_oSerial);            ///< Set camera serial.
    void setSensorSize(const unsigned int p_oWidth, const unsigned int p_oHeight);  ///< Sets sensor dimensions.
    void setSensorWidth(const unsigned int p_oWidth);  ///< Sets sensor width.
    void setSensorHeight(const unsigned int p_oHeight); ///< Sets sensor height.
    void setTriangulationAngle_rad(double p_oAngle);
    bool hasTriangulation();                                ///< If setTriangulationAngle has been called at least once, returns true, false otherwise.
    void setGridDelta(const double p_oGridDelta);           ///< Sets checker board side length.
    void setDefaults();

    // internal state
    void show(std::ostream & out = std::cout) const;
    void parseGridSize(unsigned int & nPoints, unsigned int & nRows, unsigned int & nColsMin, unsigned int & nColsMax) const;
    bool hasData() const;

    //IO
    static const char mFillChar;
    static const char mSeparator;
    static const char mWhitespace[7];
    
    
    
    void gridToCsv(const tGridMap &p_rMap, std::ostream & out = std::cout) const; 
    std::string saveToCSV(const std::string & pFilename) const;
    std::string loadFromCSV(const std::string & pFilename);

private:

    // methods
    
    static std::uint32_t computeChecksum(const std::vector<std::uint8_t> & rBytes);
    bool gridFromCSV(tGridMap &p_rMap, std::istream & p_In) const;


    // variables
    double m_oGridDelta; ///< Rectangular checker board's side length in mm.
    double m_oAngleRad; ///< Triangulation angle in rad, [-pi, pi].

    std::int16_t m_oSensorWidth; ///< Sensor width.
    std::int16_t m_oSensorHeight; ///< Sensor height.

    // internal validity flag
    bool m_oHasTriangAngle;    ///< True if triangulation angle has been set at least once. False after setDefault().
    /// optional rescaling for correcting corner position (for example, after comparison with laser line). This value is just to be copied in calibGrid, no computation will be performed here
    /// not supported in bitmap format
    double m_oCorrectionFactor;  

    std::string m_oSerial;          ///< Camera serial.
    tGridMap m_oGrid;               ///< The grid from the QT wmCalibration programm.
};

} // namespace system
} // namespace precitec

#endif // CALIBDATA_H
