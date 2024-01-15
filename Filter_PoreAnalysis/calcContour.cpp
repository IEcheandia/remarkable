#include "calcContour.h"

#include "direction.h"

namespace precitec
{
namespace filter
{
bool getNextContourPointAndDirection(const image::BImage& p_rImageIn, Dir &p_rDir, geo2d::Point &p_rCurrentPos) {
    Dir dStart ( p_rDir );
    geo2d::Point oNeighboorPos;
    int oImgVal ( 0 );

    do
    {
        // naechsten Nachbarpunkt besorgen
        oNeighboorPos = getNeighborFrom(p_rCurrentPos, ++p_rDir);
        // isolierter Pixel
        if (p_rDir == dStart)
        {
            return false;
        }

        const geo2d::Size oImgSize ( p_rImageIn.size() );
        if (oNeighboorPos.x >= 0 && oNeighboorPos.y >= 0 && oNeighboorPos.x < oImgSize.width && oNeighboorPos.y < oImgSize.height)
        {
            oImgVal = p_rImageIn[oNeighboorPos.y][oNeighboorPos.x];
        }
        else
        {
            oImgVal = 0; // happens eg on start pos (0, 0). Then assume a zero (false) value.
        }
    }
    while(oImgVal == 0 );

    p_rCurrentPos = oNeighboorPos;

    return true;
}

void calcContour (const image::BImage& p_rImageIn, geo2d::Blobarray& p_rBlobsOut, geo2d::Doublearray & oValX, geo2d::Doublearray & oValY, std::vector<geo2d::AnnotatedDPointarray> & oValPointLists,
    const std::string& filterName
)
{
    const geo2d::Range			oValidImgRangeX		( 0, p_rImageIn.size().width - 1 );
    const geo2d::Range			oValidImgRangeY		( 0,  p_rImageIn.size().height - 1 );
    std::vector<geo2d::Blob>&	rBlobVector			( p_rBlobsOut.getData() );

    auto oBlobRankIt ( std::begin(p_rBlobsOut.getRank()));
    bool isFirst = true;

    for(auto oBlobIt = std::begin(rBlobVector); oBlobIt != std::end(rBlobVector); ++oBlobIt, ++oBlobRankIt)
    {
        const geo2d::Point  oBoundinxBoxStart   ( oBlobIt->xmin, oBlobIt->ymin );
        const geo2d::Point  oBoundinxBoxEnd     ( oBlobIt->xmax + 1, oBlobIt->ymax + 1); // +1 because Rect CTOR substracts 1
        const geo2d::Rect   oBoundinxBox        ( oBoundinxBoxStart, oBoundinxBoxEnd );
        geo2d::AnnotatedDPointarray oValPoint;

        if (oBoundinxBox.isEmpty() == true) {
            //wmLog(eDebug, "Filter '%s': Empty pore discarded. Size of bounding box: (%iX%i). Bad rank set.\n", m_oFilterName.c_str(),  oBoundinxBox.width(), oBoundinxBox.height() );
            *oBlobRankIt = eRankMin;
            continue;
        }

        std::vector<geo2d::Point>& rContour (oBlobIt->m_oContour);
        rContour.reserve(64);

        const geo2d::Point oStartPos     ( oBlobIt->startx, oBlobIt->starty );
        geo2d::Point       oCurrentPos   ( oStartPos );
        geo2d::Point       oNeighboorPos ( 0, 0 );
        Dir                oCurrentDir   ( Dir::SW );  // Startpunkt wird von Suedwesten angesteuert!
        const Dir          oStartDir     ( oCurrentDir );

        poco_assert_dbg(p_rImageIn[oStartPos.y][oStartPos.x] != 0);

        if (getNextContourPointAndDirection(p_rImageIn, oCurrentDir, oCurrentPos) == false)
        {
            // isolierter Pixel (wird nicht in die Konturliste geschrieben!!!)
            //wmLog(eDebug, "Filter '%s': One pixel blob discarded.\n", m_oFilterName.c_str() );
            *oBlobRankIt = eRankMin;
            continue;
        }

        // zuletzt gefundener "Nicht-Konturpunkt" besorgen
        --oCurrentDir;
        oNeighboorPos = getNeighborFrom(oStartPos, oCurrentDir);
        if (oValidImgRangeX.contains(oNeighboorPos.x) && oValidImgRangeY.contains(oNeighboorPos.y) && p_rImageIn[oNeighboorPos.y][oNeighboorPos.x] != 0)
        {
            wmLog(eDebug, "Filter '%s': Last non-contour neighboor is true.\n", filterName.c_str() );
        }

        // Richtung zwischen 2. Konturpunkt und zuletzt gefundenen "Nicht-Konturpunkt"
        oCurrentDir = getDir(oCurrentPos, oNeighboorPos);
        ++oCurrentDir;  // beim naechsten Punkt weitermachen


        // Kontur entlanglaufen, bis StartPunkt und StartRichtung wieder erreicht wird
        // Abbruchkriterium: Jacob's stopping
        const unsigned int oNbMaxContourPoints ( (p_rImageIn.size().width + p_rImageIn.size().height) * 2 );
        unsigned int       oNbRuns             ( 0 );
        while (oCurrentPos != oStartPos || oCurrentDir != oStartDir)
        {
            if (oNbRuns >= oNbMaxContourPoints*8) // 8 neighborhood
            {
                wmLog(eDebug, "Filter '%s': Too many runs (%i), could not find contour, abort. Bad rank set.\n", filterName.c_str(), oNbRuns);
                *oBlobRankIt = eRankMin;

                break;
            }

            // naechsten Nachbarpunkt besorgen
            oNeighboorPos = getNeighborFrom(oCurrentPos, oCurrentDir);

            bool oIsOnBlob ( false );
            if (oValidImgRangeX.contains(oNeighboorPos.x) && oValidImgRangeY.contains(oNeighboorPos.y) && p_rImageIn[oNeighboorPos.y][oNeighboorPos.x] == 255)
            {
                oIsOnBlob = true;
            }

            if (oIsOnBlob == true)
            {
                const geo2d::Point oCurrentPosOld ( oCurrentPos ); // letzten Konturpunkt merken
                oCurrentPos = oNeighboorPos;                       // neuen Konturpunkt setzen

                rContour.push_back(oCurrentPos);                   // Direction in Liste eintragen
                geo2d::DPoint point;
                point.x = oCurrentPos.x;
                point.y = oCurrentPos.y;
                oValPoint.getData().push_back(point);
                oValPoint.getRank().push_back(255);
                if (isFirst)
                {
                    oValX.getData().push_back(oCurrentPos.x);
                    oValY.getData().push_back(oCurrentPos.y);
                    oValX.getRank().push_back(255);
                    oValY.getRank().push_back(255);
                }

                if (rContour.size() >= oNbMaxContourPoints)
                {
                    wmLog(eDebug, "Filter '%s': More than (%i) contour points found, abort. Bad rank set.\n", filterName.c_str(), oNbMaxContourPoints );
                    *oBlobRankIt = eRankMin;
                    break;
                }

                // vom zuletzt gefundenen Nicht-Konturpunkt weitermachen
                oNeighboorPos = getNeighborFrom(oCurrentPosOld, --oCurrentDir);
                oCurrentDir   = getDir(oCurrentPos,oNeighboorPos);
            }

            ++oCurrentDir;
            ++oNbRuns;
        }
        oValPointLists.push_back(oValPoint);
        isFirst = false;
    }
}
}
}
