/**
*   @file
*   @copyright  Precitec Vision GmbH & Co. KG
*   @author     GUR
*   @date       2022
*   @brief      Filter 'SurfaceClassifier2Rows', copied from filter 'SurfaceClassifier'.
*               Gets a SurfaceInfo structure with two (overlying) rows of tiles and checks the given limits.
*/

#ifndef SURFACECLASSIFIER2ROWS_H_
#define SURFACECLASSIFIER2ROWS_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "common/geoContext.h"
#include "filter/parameterEnums.h"
#include <geo/geo.h>

// std lib includes
#include <functional>

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"


namespace precitec
{
using namespace image;

namespace filter
{


struct SurfacePoint2Rows
{
    SurfacePoint2Rows();
    SurfacePoint2Rows(int x, int y, Color color);
    int x;
    int y;
    Color color;
};

struct SurfaceLine2Rows
{
    SurfaceLine2Rows();
    SurfaceLine2Rows(int x1, int y1, int x2, int y2, Color color);
    int x1;
    int y1;
    int x2;
    int y2;
    Color color;
};

struct SurfaceRectangle2Rows
{
    SurfaceRectangle2Rows();
    SurfaceRectangle2Rows(int x, int y, int width, int height, Color color);
    int x;
    int y;
    int width;
    int height;
    Color color;
};

class SurfaceOverlay2Rows
{
public:
    SurfaceOverlay2Rows();

    void reset();

    const std::vector<SurfacePoint2Rows>     &getPointContainer() const;
    const std::vector<SurfaceLine2Rows>      &getLineContainer() const;
    const std::vector<SurfaceRectangle2Rows> &getRectangleContainer() const;

    void addPoint(int x, int y, Color color);
    void addLine(int x1, int y1, int x2, int y2, Color color);
    void addRectangle(int x, int y, int width, int height, Color color);

private:
    std::vector<SurfacePoint2Rows>     _pointContainer;
    std::vector<SurfaceLine2Rows>      _lineContainer;
    std::vector<SurfaceRectangle2Rows> _rectangleContainer;
};

enum SurfaceParameterDisplay
{
    eSurfacePNone = 0,      ///< Nothing to display
    eSurfacePTileNo,        ///< Number of the tile
    eSurfacePMean,          ///< Mean value (grey level)
    eSurfacePRelBrightness, ///< relative brightness (in percent to the mean value)
    eSurfacePVariation,     ///< Variation of intensity values
    eSurfacePMinMaxDist,    ///< Min-to-Max distance
    eSurfacePSurface,       ///< Surface checks
    eSurfacePSurfaceX,      ///< Surface check in X direction (horizontal)
    eSurfacePSurfaceY,      ///< Surface check in Y direction (vertical)
    eSurfacePTexture,       ///< Fine structure of surface
    eSurfacePStructure      ///< Structure of surface
};


class SurfaceInfoLine2Rows
{
public:
    SurfaceInfoLine2Rows();
    SurfaceInfoLine2Rows(SurfaceParameterDisplay eValue, int value, Color color);
    std::string getLine() const;

    SurfaceParameterDisplay _number;
    int                     _value;
    Color                   _color;
};


/**
* @brief Uses an image with 2 rows of tiles. One row is 'inside' the other (horizontally). For each tile certain texture features are calculated.
*/
class FILTER_API SurfaceClassifier2Rows : public fliplib::TransformFilter
{
public:

    /**
    * @brief CTor.
    */
    SurfaceClassifier2Rows();

    /**
    * @brief Set filter parameters.
    */
    void setParameter();

    /**
    * @brief Paint overlay output.
    */
    void paint();


    // Declare constants

    static const std::string m_oFilterName;                 ///< Filter name.
    static const std::string m_oPipeOutSizeConnected1Name;  ///< Pipe name: out-pipe.
    static const std::string m_oPipeOutSizeConnected2Name;  ///< Pipe name: out-pipe.


protected:

    /**
    * @brief In-pipe registration.
    * @param p_rPipe Reference to pipe that is getting connected to the filter.
    * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
    */
    bool subscribe(fliplib::BasePipe &p_rPipe, int p_oGroup);

    /**
    * @brief Processing routine.
    * @param p_pSender pointer to
    * @param p_rEvent
    */
    void proceed(const void *p_pSender, fliplib::PipeEventArgs &p_rEvent);

private:

    const fliplib::SynchronePipe< interface::GeoSurfaceInfoarray > *m_pPipeInSurfaceInfo;       ///< in-pipe
    fliplib::SynchronePipe< interface::GeoDoublearray >            m_oPipeOutSizeConnected1;    ///< out-pipe.
    fliplib::SynchronePipe< interface::GeoDoublearray >            m_oPipeOutSizeConnected2;    ///< out-pipe.

    geo2d::SurfaceInfo  m_oInInfo;
    interface::SmpTrafo m_oSpTrafo;

    int                 m_oDisplay;

    double              m_oMinMean;
    double              m_oMaxMean;
    double              m_oMinRelInt;
    double              m_oMaxRelInt;
    double              m_oMinVariation;
    double              m_oMaxVariation;
    double              m_oMinMinMaxDistance;
    double              m_oMaxMinMaxDistance;
    double              m_oMinSurface;
    double              m_oMaxSurface;
    double              m_oMinSurfaceX;
    double              m_oMaxSurfaceX;
    double              m_oMinSurfaceY;
    double              m_oMaxSurfaceY;
    double              m_oMinTexture;
    double              m_oMaxTexture;
    double              m_oMinStructure;
    double              m_oMaxStructure;

    double              m_oMinMean2;
    double              m_oMaxMean2;
    double              m_oMinRelInt2;
    double              m_oMaxRelInt2;
    double              m_oMinVariation2;
    double              m_oMaxVariation2;
    double              m_oMinMinMaxDistance2;
    double              m_oMaxMinMaxDistance2;
    double              m_oMinSurface2;
    double              m_oMaxSurface2;
    double              m_oMinSurfaceX2;
    double              m_oMaxSurfaceX2;
    double              m_oMinSurfaceY2;
    double              m_oMaxSurfaceY2;
    double              m_oMinTexture2;
    double              m_oMaxTexture2;
    double              m_oMinStructure2;
    double              m_oMaxStructure2;

    precitec::geo2d::TileContainer m_tileContainer;

    // Funktionen zur Analyse des TileContainers
    void tagBlobs(precitec::geo2d::TileContainer &tileContainer);
    int  getMaxBlobNumber(precitec::geo2d::TileContainer &tileContainer, const int row);
    void findBiggestBlobInRows(precitec::geo2d::TileContainer &tileContainer, int &blobHeight1, int &blobHeight2);

    bool m_hasPainting;

    std::vector<std::vector<SurfaceInfoLine2Rows>> m_allInfoLines;

    std::string         m_oOverlayString;
};


} // namespace filter
} // namespace precitec

#endif /* SURFACECLASSIFIER2ROWS_H_ */
