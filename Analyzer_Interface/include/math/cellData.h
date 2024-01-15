#ifndef CELLDATA_H
#define CELLDATA_H
#include "util/camGridData.h"
#include "math/3D/projectiveMathStructures.h"

namespace precitec
{
namespace math
{

//same order as Calibration3DCoords::cellTo3D
enum eCornerPosition {
        eTopLeft = 0,
        eTopRight,
        eBotLeft,
        eBotRight,
        eNumCornersInCell
};
//equivalent representation of the values of sCellData as a vector
typedef std::array< geo2d::coordScreenToPlaneDouble, eCornerPosition::eNumCornersInCell > tCellDataArray;
    
enum eEdgePosition {
        eLeft = 0,
        eTop,
        eRight,
        eBottom
};



class CalibrationCornerGrid;

//struct that holds a reference to the position of 4 points forming a cell in a tGridMap2D3D
struct sCellData  
{
#ifndef NDEBUG
    //forward declaration
    friend class precitec::math::CalibrationCornerGrid; //only for assertions in CalibrationCornerGrid::findCell
#endif
public:

    typedef std::array<geo2d::coordScreenToPlaneDouble ,2> tSegment;
    enum class CellMode {fixedScreenCoordinates, fixedChessboardCoordinates, closestPlaneCoordinates, closestScreenCoordinates};
    
    sCellData();
    
	sCellData (const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & rLineTop, const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & rLineBot, 
              const size_t & p_oXIdxTopLeft,const size_t & p_oXIdxTopRight,const size_t & p_oXIdxBotLeft,const size_t & p_oXIdxBotRight);
    
	sCellData (const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & rLineTop, const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & rLineBot, 
              const size_t & p_oXIdxTopLeft, CellMode pCellMode);

private:

	static const std::vector< precitec::geo2d::coordScreenToPlaneDouble > m_emptyline ;
	std::size_t m_oXIdxTopRight;
	std::size_t m_oXIdxBotRight;
	std::size_t m_oXIdxTopLeft;
	std::size_t m_oXIdxBotLeft;
    const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & m_rLineTop;
    const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & m_rLineBottom;
	void resetLineIndexes(); // invalidates struct, does not clear lineTop and lineBot iteration
public:
	bool isValid() const;
	const geo2d::coordScreenToPlaneDouble & getCorner(const int pos) const;
	tSegment getSegment(eEdgePosition position) const;

	void findMaximumPixelRange(double & xMin, double & xMax, double & yMin, double & yMax) const;
        
    std::pair<bool,tCellDataArray>  computeRealWorldCoordinatesFromCell(RealWorldTransform p_transform) const;
    
    

};

}
}

#endif
