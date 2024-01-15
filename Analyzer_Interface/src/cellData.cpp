#include "cellData.h"

namespace precitec
{
namespace math
{

const std::vector< precitec::geo2d::coordScreenToPlaneDouble > sCellData::m_emptyline = std::vector< precitec::geo2d::coordScreenToPlaneDouble > (0) ;

sCellData::sCellData()
: m_rLineTop(m_emptyline), m_rLineBottom(m_emptyline)
{
    resetLineIndexes();
}


sCellData::sCellData (const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & rLineTop, const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & rLineBot, 
              const size_t & p_oXIdxTopLeft,const size_t & p_oXIdxTopRight,const size_t & p_oXIdxBotLeft,const size_t & p_oXIdxBotRight)
:m_oXIdxTopRight(p_oXIdxTopRight),
m_oXIdxBotRight(p_oXIdxBotRight),
m_oXIdxTopLeft(p_oXIdxTopLeft),
m_oXIdxBotLeft(p_oXIdxBotLeft),
m_rLineTop(rLineTop), m_rLineBottom(rLineBot)
{}

sCellData::sCellData (const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & rLineTop, 
                     const std::vector< precitec::geo2d::coordScreenToPlaneDouble > & rLineBot, 
                     const size_t & p_oXIdxTopLeft, CellMode pCellMode)
: 
m_oXIdxTopRight(0), m_oXIdxBotRight(0), m_oXIdxTopLeft(0), m_oXIdxBotLeft(0),
m_rLineTop(rLineTop),
m_rLineBottom(rLineBot)
{
    
	const auto lineTopLastIndex = m_rLineTop.size() - 1;
	const auto lineBottomLastIndex = m_rLineBottom.size() - 1;
    
    if ( p_oXIdxTopLeft >= lineTopLastIndex)
	{
        assert(!isValid());
		return;
	}
	m_oXIdxTopLeft = p_oXIdxTopLeft;
    
    auto & oCTopLeft = getCorner(eTopLeft);
	
    switch(pCellMode)
    {
        case CellMode::fixedScreenCoordinates:            
            m_oXIdxBotLeft = m_oXIdxTopLeft;            
            break;
        case CellMode::closestScreenCoordinates:
            while ( m_oXIdxBotLeft < lineBottomLastIndex && getCorner(eBotLeft).ScreenX < oCTopLeft.ScreenX )
            {
                m_oXIdxBotLeft++;
            }            
            if (m_oXIdxBotLeft > 0 && 
                (oCTopLeft.ScreenX  - m_rLineBottom[m_oXIdxBotLeft-1].ScreenX) < (m_rLineBottom[m_oXIdxBotLeft].ScreenX - oCTopLeft.ScreenX) )
            {
                m_oXIdxBotLeft = m_oXIdxBotLeft-1;
            }
            break;
                                
        case CellMode::closestPlaneCoordinates:
            while ( m_oXIdxBotLeft < lineBottomLastIndex && getCorner(eBotLeft).RealX < oCTopLeft.RealX )
            {
                m_oXIdxBotLeft++;
            }            
            if (m_oXIdxBotLeft > 0 && 
                (oCTopLeft.RealX  - m_rLineBottom[m_oXIdxBotLeft-1].RealX) < (m_rLineBottom[m_oXIdxBotLeft].RealX - oCTopLeft.RealX) )
            {
                m_oXIdxBotLeft = m_oXIdxBotLeft-1;
            }
            break;
            
        case CellMode::fixedChessboardCoordinates:        
            while ( m_oXIdxBotLeft < lineBottomLastIndex && getCorner(eBotLeft).RealX != oCTopLeft.RealX  )
            {
                m_oXIdxBotLeft++;
            }            
            break;
    }
    

    if (m_oXIdxTopLeft >=  lineTopLastIndex  || m_oXIdxBotLeft >= lineBottomLastIndex)
    {
        
        m_oXIdxTopRight =  (m_oXIdxTopLeft <  lineTopLastIndex ) ? m_oXIdxTopLeft + 1 : m_oXIdxTopLeft;
        m_oXIdxBotRight =  ( m_oXIdxBotLeft < lineBottomLastIndex) ? m_oXIdxBotLeft +1 : m_oXIdxBotLeft;
        assert(!isValid());
        return;
    }   
    
    m_oXIdxTopRight =  m_oXIdxTopLeft + 1;
    m_oXIdxBotRight =  m_oXIdxBotLeft +1;

    
    
    //validate the indexes  
    
    const double referenceX = oCTopLeft.RealX;
    const double referenceY = oCTopLeft.RealY;
    switch(pCellMode)
    {
        case CellMode::fixedScreenCoordinates:            
            if (getCorner(eTopRight).ScreenY != oCTopLeft.ScreenY
                || getCorner(eBotLeft).ScreenX != oCTopLeft.ScreenX
                || getCorner(eBotRight).ScreenY != getCorner(eBotLeft).ScreenY 
                || getCorner(eBotRight).ScreenX != getCorner(eTopRight).ScreenX)
            {
                resetLineIndexes(); 
                assert(!isValid());
                return;
            }            
            break;
            
            
        case CellMode::fixedChessboardCoordinates:        
            if ( trunc(referenceX) != referenceX || trunc(referenceY) != referenceY // Cells should represent integer chessboard corners
                || getCorner(eBotLeft).RealX != referenceX 
                ||  std::ceil(oCTopLeft.RealX) != referenceX || std::ceil(oCTopLeft.RealY) != referenceY
                ||   getCorner(eBotLeft).RealY != (referenceY + 1) 
                ||  getCorner(eTopRight).RealX != (referenceX + 1) 
            )
            {
                resetLineIndexes();
                assert(!isValid());
                return;
            }
            break;
            
        case CellMode::closestScreenCoordinates:
        case CellMode::closestPlaneCoordinates:
            break;
    }

    assert(isValid());
};

void sCellData::resetLineIndexes() // invalidates struct, does not clear lineTop and lineBot vectors
{
	m_oXIdxTopRight = 0; m_oXIdxBotRight = 0;
	m_oXIdxTopLeft = 0; m_oXIdxBotLeft = 0;
};



bool sCellData::isValid() const
{
	return ((m_oXIdxTopRight != m_oXIdxTopLeft) && (m_oXIdxBotRight != m_oXIdxBotLeft));
};



const geo2d::coordScreenToPlaneDouble & sCellData::getCorner(const int pos) const
{
	switch ( pos )
	{
		case eTopLeft:
			return  m_rLineTop[m_oXIdxTopLeft];
		case eTopRight:
			return m_rLineTop[m_oXIdxTopRight];
		case eBotLeft:
			return m_rLineBottom[m_oXIdxBotLeft];
		default:
		case eBotRight:
			return m_rLineBottom[m_oXIdxBotRight];
	}
}

sCellData::tSegment sCellData::getSegment(eEdgePosition pos) const
{

    switch (pos)
    {
        case eEdgePosition::eTop:
            return {{getCorner(eCornerPosition::eTopLeft),getCorner(eCornerPosition::eTopRight)}};
        case eEdgePosition::eLeft:
            return {{ getCorner(eCornerPosition::eTopLeft),getCorner(eCornerPosition::eBotLeft)}};
        case eEdgePosition::eBottom:
            return {{ getCorner(eCornerPosition::eBotLeft), getCorner(eCornerPosition::eBotRight)}};
        case eEdgePosition::eRight:
            return {{ getCorner(eCornerPosition::eTopRight), getCorner(eCornerPosition::eBotRight)}};
    }
    assert(false && "sCellData::getSegment: unknown argument");
    return tSegment();
}

void sCellData::findMaximumPixelRange(double & xMin, double & xMax, double & yMin, double & yMax) const
{
	auto & oFirstCorner = getCorner(eCornerPosition::eTopLeft);
	xMin = oFirstCorner.ScreenX;
	xMax = oFirstCorner.ScreenX;
	yMin = oFirstCorner.ScreenY;
	yMax = oFirstCorner.ScreenY;
	for ( int i = 1; i < eCornerPosition::eNumCornersInCell; i++ )
	{
		auto & oCorner = getCorner(eCornerPosition(i));
		xMin = oCorner.ScreenX < xMin ? oCorner.ScreenX : xMin;
		xMax = oCorner.ScreenX > xMax ? oCorner.ScreenX : xMax;
		yMin = oCorner.ScreenY < yMin ? oCorner.ScreenY : yMin;
		yMax = oCorner.ScreenY > yMax ? oCorner.ScreenY : yMax;
	}
}




std::pair< bool, precitec::math::tCellDataArray > sCellData::computeRealWorldCoordinatesFromCell(RealWorldTransform p_transform) const
{
    tCellDataArray rRealWorldCoordinates;

    if ( !isValid() )
    {
        return {false,rRealWorldCoordinates};
    }

    for ( int i = 0; i < eCornerPosition::eNumCornersInCell; ++i )
    {
        rRealWorldCoordinates[i] = getCorner(i);
        // convert grid coordinates to real world coordinates
        rRealWorldCoordinates[i].convertToRealWorldCoords(p_transform);
    }

    return {true, rRealWorldCoordinates};
}

}
}
