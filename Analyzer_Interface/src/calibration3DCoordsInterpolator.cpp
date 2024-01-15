#include "calibration3DCoordsInterpolator.h"
#include "calibrationCornerGrid.h"


//helper functions private to this class
namespace {

	using precitec::geo2d::eCoordDimension;
	using precitec::geo2d::coordScreenToPlaneDouble;
	using precitec::math::eCornerPosition;
	using precitec::math::sCellData;
    using precitec::math::tCellDataArray;

	enum Rectify {
		eDontRectify, eRectify
	};

	enum ExtensionDirection {
		eTowardStart = 0,
		eTowardEnd = 1,
	};


	//conventions on the order of corners in a cell
	typedef std::array<eCornerPosition, 2> tCornersPositionInEdge;
	const tCornersPositionInEdge posLeftEdge{{eCornerPosition::eTopLeft, eCornerPosition::eBotLeft}};
	const tCornersPositionInEdge posRightEdge{{eCornerPosition::eTopRight, eCornerPosition::eBotRight}};
	const tCornersPositionInEdge posTopEdge{{eCornerPosition::eTopLeft, eCornerPosition::eTopRight}};
	const tCornersPositionInEdge posBottomEdge{{eCornerPosition::eBotLeft, eCornerPosition::eBotRight}};
	const std::array<tCornersPositionInEdge, 2> VerticalEdges{{posLeftEdge, posRightEdge}};
	const std::array<tCornersPositionInEdge, 2> HorizontalEdges{{posTopEdge, posBottomEdge}};

//Linear Interpolation between two instances of coordScreenToPlaneDouble
class SegmentInterpolator
{
public:
	const coordScreenToPlaneDouble m_Start;
	const coordScreenToPlaneDouble m_Delta;
	SegmentInterpolator(const coordScreenToPlaneDouble & p_Start, const coordScreenToPlaneDouble & p_End) :
		m_Start(p_Start), m_Delta(p_End - p_Start)
	{
		assert(interpolate(0) == p_Start);
		assert(interpolate(1) == p_End);
		assert(interpolate(0.5) == (p_Start + p_End) * 0.5);
		assert(interpolate(-1) == (p_Start * 2 - p_End));
		assert(interpolate(2) == (p_End * 2 - p_Start));
		if ( m_Delta.ScreenX != 0 )
		{
			assert(std::abs(interpolate(getT(0, eCoordDimension::eScreenX)).ScreenX - 0) < 1e-12);
		}
	};
	SegmentInterpolator(const tCellDataArray & p_Cell, const std::array<eCornerPosition, 2> & p_Indexes)
			:SegmentInterpolator(p_Cell[p_Indexes[0]], p_Cell[p_Indexes[1]])
		{};
	coordScreenToPlaneDouble interpolate(double t) const
	{
		return m_Start + m_Delta*t;
	}

	//returns 0 for division by 0
	double getT(double val, eCoordDimension p_dim) const
	{
		return  m_Delta.get(p_dim) == 0 ? 0 : (val - m_Start.get(p_dim)) / m_Delta.get(p_dim);
	}

	coordScreenToPlaneDouble getInterpolation(double val, eCoordDimension p_dim) const
	{
		return interpolate(getT(val, p_dim));
	}

};


	//Extend a cell, extrapolating its corners along oInterpolationDirection toward pExtensionDirection
	//Example: a cell can be extended horizontally such as the left corners have ScreenX = 0
	tCellDataArray extendCell(const tCellDataArray & pCell,
		double pTargetCoordinate, ExtensionDirection pExtensionDirection, eCoordDimension oInterpolationDirection)
{
		// the other cases are not implemented
		assert(oInterpolationDirection == eCoordDimension::eScreenX || oInterpolationDirection == eCoordDimension::eScreenY); 
#ifdef extradebugcalib
		//in this implementation the method is called only when the targetcoordinate is outside the cell
		if ( oInterpolationDirection == eCoordDimension::eScreenX )
		{
			for ( auto edges : HorizontalEdges )
			{
				if ( pExtensionDirection == eTowardStart )
				{
					assert(pCell[edges[0]].ScreenX >= pTargetCoordinate);
				}
				else
				{
					assert(pCell[edges[1]].ScreenX <= pTargetCoordinate);
				}
			}
		}
		else
		{
			for ( auto edges : VerticalEdges )
			{
				if ( pExtensionDirection == eTowardStart )
				{
					assert(pCell[edges[0]].ScreenY >= pTargetCoordinate);
				}
				else
				{
					assert(pCell[edges[1]].ScreenY <= pTargetCoordinate);
				}
			}
		}
#endif

		tCellDataArray oResult;
		for ( int countEdges = 0; countEdges < 2; ++countEdges )
		{
			tCornersPositionInEdge oIndexToInterpolate;
			
			if ( oInterpolationDirection == eCoordDimension::eScreenX )
			{
				//we will interpolate first along the top edge, then along the bottom edge
				oIndexToInterpolate = HorizontalEdges[countEdges];  
			}
			else
			{
				//we will interpolate first along the left edge, then along the right edge
				oIndexToInterpolate = VerticalEdges[countEdges];  
			}


			SegmentInterpolator oEdge(pCell, oIndexToInterpolate);

			//where should the computed coordinate go in the output cell?
			//e.g first iteration, horizontal direction (top edge), towardEnd: we are computing the topRightCorner
			//e.g first iteration, vertical direction (left edge), towardStart: we are computing the topLeftCorner
			auto oOutputIndexExtrapolated = oIndexToInterpolate[pExtensionDirection];
			oResult[oOutputIndexExtrapolated] = oEdge.getInterpolation(pTargetCoordinate, oInterpolationDirection);

			//where should the other coordinate of the edge go?
			//e.g topEdge, we computed the topRightCorner, now we need to fill the topLeftCorner
			auto oOutputIndexKnownCoordinate = oIndexToInterpolate[!pExtensionDirection];
			oResult[oOutputIndexKnownCoordinate] = oEdge.interpolate(0.5); //take the midpoint of the edge

#ifdef extradebugcalib
			if ( countEdges == 1 )
			{
				if ( oInterpolationDirection == eCoordDimension::eScreenX )
				{
					assert(oResult[eCornerPosition::eTopLeft].RealX < oResult[eCornerPosition::eTopRight].RealX);
					assert(oResult[eCornerPosition::eBotLeft].RealX < oResult[eCornerPosition::eBotRight].RealX);
				}
				else
				{
					//the sign of RealY has a different convention in CornerGrid and 3DCoords
					assert(
						(pCell[eCornerPosition::eTopLeft].RealY < pCell[eCornerPosition::eBotLeft].RealY) ==
						(oResult[eCornerPosition::eTopLeft].RealY < oResult[eCornerPosition::eBotLeft].RealY));

					assert(
						(pCell[eCornerPosition::eTopRight].RealY < pCell[eCornerPosition::eBotRight].RealY) ==
						(oResult[eCornerPosition::eTopRight].RealY < oResult[eCornerPosition::eBotRight].RealY));
				}
			}
#endif
		}

#ifdef extradebugcalib
		assert(oResult[eCornerPosition::eTopLeft].ScreenX < oResult[eCornerPosition::eTopRight].ScreenX);
		assert(oResult[eCornerPosition::eBotLeft].ScreenX < oResult[eCornerPosition::eBotRight].ScreenX);
		assert(oResult[eCornerPosition::eTopLeft].ScreenY < oResult[eCornerPosition::eBotLeft].ScreenY);
		assert(oResult[eCornerPosition::eTopRight].ScreenY < oResult[eCornerPosition::eBotRight].ScreenY);
#endif
	return oResult;
}


bool computeRealWorldCoordinatesFromCell(precitec::math::tCellDataArray & rRealWorldCoordinates,
	const sCellData & oCell, Rectify pRectify, precitec::math::RealWorldTransform p_transform)
{
	if ( !oCell.isValid() )
	{
		return false;
	}
	if ( pRectify == eDontRectify )
	{
		for ( int i = 0; i < eCornerPosition::eNumCornersInCell; ++i )
		{
			rRealWorldCoordinates[i] = oCell.getCorner(i);
		}
	}
	else
	{
		double xMin, xMax, yMin, yMax;
        oCell.findMaximumPixelRange(xMin, xMax, yMin, yMax);
		
		xMin = std::floor(xMin);
		xMax = std::floor(xMax);
		yMin = std::floor(yMin);
		yMax = std::floor(yMax);
		//interpolate vertically
		for ( auto & oEdgePosition : VerticalEdges )
		{
			SegmentInterpolator oEdge(oCell.getCorner(oEdgePosition[0]), oCell.getCorner(oEdgePosition[1]));
			if ( oEdge.m_Delta.ScreenY == 0 )
			{
				return false;
			}
			rRealWorldCoordinates[oEdgePosition[0]] = oEdge.getInterpolation(yMin, eCoordDimension::eScreenY);  //topLeft or topRight
			rRealWorldCoordinates[oEdgePosition[1]] = oEdge.getInterpolation(yMax, eCoordDimension::eScreenY);  // botLeft or botRight
		}
		//continue interpolating horizontally
		for ( auto & oEdgePosition : HorizontalEdges )
		{
			SegmentInterpolator oEdge(rRealWorldCoordinates, oEdgePosition);
			if ( oEdge.m_Delta.ScreenX == 0 )
			{
				return false;
			}
			rRealWorldCoordinates[oEdgePosition[0]] = oEdge.getInterpolation(xMin, eCoordDimension::eScreenX); //topLeft or botLeft
			rRealWorldCoordinates[oEdgePosition[1]] = oEdge.getInterpolation(xMax, eCoordDimension::eScreenX); //topRight or botRight
		}
		
        #ifndef NDEBUG
        { 
            auto oExpectedRealWorldCoordinates = rRealWorldCoordinates;
            for ( int i = 0; i < eCornerPosition::eNumCornersInCell; i++ )
            {
                oExpectedRealWorldCoordinates[i].convertToRealWorldCoords(p_transform);
            }
           
            
            auto fTest = [&oCell, p_transform, xMin, xMax, yMin, yMax] (const tCellDataArray & rExpectedRealWorldCoordinates)
            {
                //auto oChessBoardCoordinates = oCell.getChessboardCoordinatesInCell();
                auto oRealWorldCoordinatesNotRectified = oCell.computeRealWorldCoordinatesFromCell(p_transform).second; 
                //interpolate vertically
                for ( auto & oEdgePosition : VerticalEdges )
                {
                    SegmentInterpolator oInterpolatingEdge(oRealWorldCoordinatesNotRectified[oEdgePosition[0]], oRealWorldCoordinatesNotRectified[oEdgePosition[1]]);
                    oRealWorldCoordinatesNotRectified[oEdgePosition[0]] = oInterpolatingEdge.getInterpolation(yMin, eCoordDimension::eScreenY);  //topLeft or topRight
                    oRealWorldCoordinatesNotRectified[oEdgePosition[1]] = oInterpolatingEdge.getInterpolation(yMax, eCoordDimension::eScreenY);  // botLeft or botRight
                }
                //continue interpolating horizontally
                for ( auto & oEdgePosition : HorizontalEdges )
                {
                    SegmentInterpolator oInterpolatingEdge(oRealWorldCoordinatesNotRectified, oEdgePosition);
                
                    oRealWorldCoordinatesNotRectified[oEdgePosition[0]] = oInterpolatingEdge.getInterpolation(xMin, eCoordDimension::eScreenX); //topLeft or botLeft
                    oRealWorldCoordinatesNotRectified[oEdgePosition[1]] = oInterpolatingEdge.getInterpolation(xMax, eCoordDimension::eScreenX); //topRight or botRight
                }
                for (int i =0; i < 4; i++)
                {
                    assert(precitec::math::isClose(oRealWorldCoordinatesNotRectified[i].ScreenX, rExpectedRealWorldCoordinates[i].ScreenX, 1e-10));
                    assert(precitec::math::isClose(oRealWorldCoordinatesNotRectified[i].ScreenY, rExpectedRealWorldCoordinates[i].ScreenY, 1e-10));

                    assert(precitec::math::isClose(oRealWorldCoordinatesNotRectified[i].RealX, rExpectedRealWorldCoordinates[i].RealX, 1e-10));
                    assert(precitec::math::isClose(oRealWorldCoordinatesNotRectified[i].RealY, rExpectedRealWorldCoordinates[i].RealY, 1e-10));
                }
                return true;
            };
            fTest(oExpectedRealWorldCoordinates);
            
        }
#endif
		
        #ifndef NDEBUG
        { 
            auto oExpectedRealWorldCoordinates = rRealWorldCoordinates;
            for ( int i = 0; i < eCornerPosition::eNumCornersInCell; i++ )
            {
                oExpectedRealWorldCoordinates[i].convertToRealWorldCoords(p_transform);
            }
            auto fTest = [&oCell, p_transform, xMin, xMax, yMin, yMax] (const tCellDataArray & rExpectedRealWorldCoordinates)
            {
                //auto oChessBoardCoordinates = oCell.getChessboardCoordinatesInCell();
                auto oRealWorldCoordinatesNotRectified = oCell.computeRealWorldCoordinatesFromCell(p_transform).second; 
                //interpolate vertically
                for ( auto & oEdgePosition : VerticalEdges )
                {
                    SegmentInterpolator oInterpolatingEdge(oRealWorldCoordinatesNotRectified[oEdgePosition[0]], oRealWorldCoordinatesNotRectified[oEdgePosition[1]]);
                    oRealWorldCoordinatesNotRectified[oEdgePosition[0]] = oInterpolatingEdge.getInterpolation(yMin, eCoordDimension::eScreenY);  //topLeft or topRight
                    oRealWorldCoordinatesNotRectified[oEdgePosition[1]] = oInterpolatingEdge.getInterpolation(yMax, eCoordDimension::eScreenY);  // botLeft or botRight
                }
                //continue interpolating horizontally
                for ( auto & oEdgePosition : HorizontalEdges )
                {
                    SegmentInterpolator oInterpolatingEdge(oRealWorldCoordinatesNotRectified, oEdgePosition);
                
                    oRealWorldCoordinatesNotRectified[oEdgePosition[0]] = oInterpolatingEdge.getInterpolation(xMin, eCoordDimension::eScreenX); //topLeft or botLeft
                    oRealWorldCoordinatesNotRectified[oEdgePosition[1]] = oInterpolatingEdge.getInterpolation(xMax, eCoordDimension::eScreenX); //topRight or botRight
                }
                for (int i =0; i < 4; i++)
                {
                    assert(precitec::math::isClose(oRealWorldCoordinatesNotRectified[i].ScreenX, rExpectedRealWorldCoordinates[i].ScreenX, 1e-10));
                    assert(precitec::math::isClose(oRealWorldCoordinatesNotRectified[i].ScreenY, rExpectedRealWorldCoordinates[i].ScreenY, 1e-10));

                    assert(precitec::math::isClose(oRealWorldCoordinatesNotRectified[i].RealX, rExpectedRealWorldCoordinates[i].RealX, 1e-10));
                    assert(precitec::math::isClose(oRealWorldCoordinatesNotRectified[i].RealY, rExpectedRealWorldCoordinates[i].RealY, 1e-10));
                }
                return true;
            };
            fTest(oExpectedRealWorldCoordinates);
            
        }
#endif
		
	}
	
	// convert grid coordinates to real world coordinates				
	for ( int i = 0; i < eCornerPosition::eNumCornersInCell; i++ )
	{
		rRealWorldCoordinates[i].convertToRealWorldCoords(p_transform);
	}
	return true;
}

}

namespace precitec {
namespace math {

using geo2d::eCoordDimension;
using geo2d::coordScreenToPlaneDouble;

bool Calibration3DCoordsInterpolator::computeMissingGridPoints()
{
	/* Area above first and below last grid line is invalidated as we can not deliver reasonable values here */


	bool oOK(true);

	auto oFirstValidY = mHeight;
	for ( unsigned int oY = 0; oY < mHeight; oY++ )
	{
		int oBentCells(0);

		const unsigned int oLineIdx(mWidth*oY);
		int oStart(0), oEnd(mWidth - 1);
		{//find  boundaries
			while ( (oStart <= oEnd) && (m_oTimesComputed[oLineIdx + oStart] == 0) )
			{
#ifndef NDEBUG
                float coordx, coordy;
				assert(!m_o3DCoords.getCoordinates( coordx, coordy, oStart, oY) );
#endif
				oStart++;
			}
			//oStart now is the first position in line oY having a valid coordinate
			//if the line is empty, oStart = mWidth
			if ( oStart <= oEnd )
			{
				if ( oFirstValidY > oY )
				{
						oFirstValidY = oY;
				}
			}
			while ( oEnd >= oStart && (m_oTimesComputed[oLineIdx + oEnd] == 0) )
			{
#ifndef NDEBUG
                float coordx, coordy;
				assert(!m_o3DCoords.getCoordinates( coordx, coordy, oEnd, oY) && "valid coordinate, but times computed  == 0");
#endif
                oEnd--;
			}
		}
		assert(oStart <= (oEnd + 1)); //empty line: oStart = 1024, oEnd = 1023

		//if this row is along the border, the error conditions are more relaxed
		//unfortunately at this point I don't know which is the lastValidY, I assume it's the last line of the image (because of extrapolation in previous steps)
		bool isBorderRow = oY <= oFirstValidY || oY >= (mHeight - 1); 

		for ( auto oX = oStart; oX <= oEnd; ++oX )
		{
			if ( getTC(oX, oY) == 0 )
			{
				bool ok = getValueByNeighbours(oX, oY);
				assert(ok == (getTC(oX, oY) == 1));
				(void) ok;
			}

			//the points at the border sometimes are impossible to compute, but it's not an error
			bool isBorderColumn = oX <= oStart || oX >= oEnd;
			if ( !isBorderRow && !isBorderColumn && getTC(oX, oY) == 0 )
			{
				oBentCells++;
				oOK = false;
				assert(false);
			}
		}
		if ( oBentCells > 0 )
		{
			//"QnxMsg.Calib.BadPoint"
			wmLog(eError, "Line %d (x from %d to %d) Cannot map %d coordinates to real world! Grid bends too much?\n", oY, oStart, oEnd, oBentCells);
			assert(false);
		}
	}

	return oOK;
}




bool Calibration3DCoordsInterpolator::normalize3DData()
{

	for ( unsigned int j = 0; j < mHeight; ++j )
	{
        unsigned int idx = mWidth*j;
		for ( unsigned int i = 0; i < mWidth; ++i)
		{
			if ( m_oTimesComputed[idx] > 1 )
			{
				m_o3DCoords.X(i,j) /= m_oTimesComputed[idx];
				m_o3DCoords.Y(i,j)  /= m_oTimesComputed[idx];
				m_oTimesComputed[idx] = 1;
				assert(getTC(i,j) == 1);
			}
			++idx;
		}
	}

	return true;
}

bool Calibration3DCoordsInterpolator::allCellsTo3D(const CornerGridMap & pCornerGrid, bool pExtrapolate, bool pRectify )
{
	std::cout << "CalibrationGrid: All cells to 3d " << std::endl;

	auto & rGrid = pCornerGrid.getGrid2D3D();
    auto & rTransform = pCornerGrid.getTransform();

	if ( rGrid.size() < 2 ) // we need at least two lines!
	{
		std::cout << "CalibrationGrid:allCellsTo3D camData empty" << std::endl;
		return false;
	}

	const auto rectifyParameter = pRectify ? eRectify : eDontRectify;
	bool oOK(true);
	

	auto oItTopLine = rGrid.begin();
	auto oItBotLine = next(oItTopLine);

	//store values for extrapolation
	bool isUpperMostLine = true;

	std::vector<tCellDataArray> oLastRowsOfRealWorldCells;
    
    //workaround to recognize whether this is a CalibrationCornerGrid or UniformGrid
    const auto oCellMode = [&] () {
            assert(oItBotLine != rGrid.end() && "at least two lines needed, input was not checked");
            const auto & rLineTop = oItTopLine->second;
            const auto & rLineBot = oItBotLine->second;
            if ( rLineTop.size() >=2 &&  rLineBot.size() >= 2 )
            {
                for (auto cellmode : {sCellData::CellMode::fixedChessboardCoordinates, sCellData::CellMode::fixedScreenCoordinates, sCellData::CellMode::closestPlaneCoordinates, sCellData::CellMode::closestScreenCoordinates })
                {
                    sCellData oCell(oItTopLine->second, oItBotLine->second, 0, cellmode);
                    if (oCell.isValid())
                    {
                        return cellmode;
                    }   
                }
            }
            return sCellData::CellMode::fixedChessboardCoordinates; //just return a default value to exit
        }();
        
	while ( oItBotLine != rGrid.end() && oOK )
	{
		const auto & rLineTop = oItTopLine->second;
		const auto & rLineBot = oItBotLine->second;

		if ( (rLineTop.size() < 2) || (rLineBot.size() < 2) )
		{
			oOK = false;
			wmLogTr(eError, "QnxMsg.Calib.TwoCorners", "Bad calibration grid. Each line needs at least two corners.\n");
			break;
		}

		bool isFirstCell = true;
		tCellDataArray oRealWorldCoordinates;
		tCellDataArray oRealWorldCoordinatesFirstCell;
		oLastRowsOfRealWorldCells.clear();
		oLastRowsOfRealWorldCells.reserve(rLineTop.size() + 1);

        
		//iterate checkerboard squares (from itTopLeft)
		for ( int oIdxTopLeft = 0, oIdxTopLeftMax =rLineTop.size() - 1; oIdxTopLeft < oIdxTopLeftMax; oIdxTopLeft++ )
		{
            sCellData oLastCell(oItTopLine->second, oItBotLine->second, oIdxTopLeft, oCellMode);
			bool valid = oLastCell.isValid();
            if(!valid)
            {
                continue;
            }

			// for extrapolation, rectified should be enabled for easier interpolation
			// the rectified cells will overlap, but that's already taken care by normalize cells
			valid = computeRealWorldCoordinatesFromCell(oRealWorldCoordinates, oLastCell, rectifyParameter, rTransform);
			if ( !valid )
			{
				continue;
			}
#ifndef NDEBUG
            if (rectifyParameter == eRectify)
            {
                assert(isClose(oRealWorldCoordinates[eTopLeft].ScreenX, oRealWorldCoordinates[eBotLeft].ScreenX, 1e-12));
                assert(isClose(oRealWorldCoordinates[eTopRight].ScreenX, oRealWorldCoordinates[eBotRight].ScreenX, 1e-12));
                assert(isClose(oRealWorldCoordinates[eTopLeft].ScreenY, oRealWorldCoordinates[eTopRight].ScreenY, 1e-12));
                assert(isClose(oRealWorldCoordinates[eBotLeft].ScreenY, oRealWorldCoordinates[eBotRight].ScreenY, 1e-12));
            }
#endif
			// and find mapping 2D to 3D for each cell			
			oOK &= cellTo3D(oRealWorldCoordinates);
            assert(oRealWorldCoordinates[0].RealY != oRealWorldCoordinates[2].RealY);
			if ( pExtrapolate )
			{
				oLastRowsOfRealWorldCells.push_back(oRealWorldCoordinates);
				//update firstcell on first iteration
				if ( isFirstCell )
				{
					oRealWorldCoordinatesFirstCell = oRealWorldCoordinates;
					isFirstCell = false;
				}
			}
			
		}; //end loop x

		assert(!pExtrapolate || oRealWorldCoordinatesFirstCell[0].RealY != oRealWorldCoordinatesFirstCell[2].RealY);
		
		if ( pExtrapolate )
		{
			//extrapolate on left (oRealWorldCoordinatesFirstCell)
			{
				tCellDataArray oNewCell = extendCell(oRealWorldCoordinatesFirstCell, 0, eTowardStart, eCoordDimension::eScreenX);
				oOK &= cellTo3D(oNewCell);
				oLastRowsOfRealWorldCells.push_back(oNewCell);
			}
			//extrapolate on right (last value of oRealWorldCoordinates)
			{
				tCellDataArray oNewCell = extendCell(oRealWorldCoordinates, mWidth, eTowardEnd, eCoordDimension::eScreenX);
				oOK &= cellTo3D(oNewCell);
				oLastRowsOfRealWorldCells.push_back(oNewCell);
			}

			//extrapolate top line if necessary
			if ( isUpperMostLine )
			{
				for ( auto & oCell : oLastRowsOfRealWorldCells )
				{
					tCellDataArray oNewCell = extendCell(oCell, 0, eTowardStart, eCoordDimension::eScreenY);
					oOK &= cellTo3D(oNewCell);
				}
				isUpperMostLine = false;
			}
		}

		//increment loop var
		++oItTopLine;
		++oItBotLine;

	} //end loop y

	if ( pExtrapolate )
	{
		//finally, extrapolate bottom line
		for ( auto & oCell : oLastRowsOfRealWorldCells )
		{
			tCellDataArray oNewCell = extendCell(oCell, mHeight, eTowardEnd, eCoordDimension::eScreenY);
			oOK &= cellTo3D(oNewCell);
		}
	}
	
	if ( oOK )
	{
		oOK = normalize3DData() && computeMissingGridPoints();
		//printGrid(SensorId(p_oSensorID), eScheimpflug, std::cout);
		if ( oOK )
		{
			return true;
		}
	}

	wmLogTr(eError, "QnxMsg.Calib.BadGrid", "Cannot determine 3D data.\n");
	return false;
}



geo2d::coordScreenToPlaneDouble Calibration3DCoordsInterpolator::cellTo3D(const tCellDataArray & rChessBoardCoordinates, double screenX, double screenY)
{
 
    const SegmentInterpolator oLeftEdge(rChessBoardCoordinates[eTopLeft], rChessBoardCoordinates[eBotLeft]);
	const SegmentInterpolator oRightEdge(rChessBoardCoordinates[eTopRight], rChessBoardCoordinates[eBotRight]);
    
    const coordScreenToPlaneDouble oLeft = oLeftEdge.getInterpolation(screenY, eCoordDimension::eScreenY);
    const coordScreenToPlaneDouble oRight = oRightEdge.getInterpolation(screenY, eCoordDimension::eScreenY);
    const SegmentInterpolator oChordHorizontal(oLeft, oRight);
    auto oCoordinatesHorizontal =  oChordHorizontal.getInterpolation(screenX, eCoordDimension::eScreenX);
    assert(oCoordinatesHorizontal.ScreenX == screenX);
    assert(oCoordinatesHorizontal.ScreenY == screenY);
    
    const SegmentInterpolator oTopEdge(rChessBoardCoordinates[eTopLeft], rChessBoardCoordinates[eTopRight]);
	const SegmentInterpolator oBottomEdge(rChessBoardCoordinates[eBotLeft], rChessBoardCoordinates[eBotRight]);
    
    const coordScreenToPlaneDouble oTop = oTopEdge.getInterpolation(screenX, eCoordDimension::eScreenX);
    const coordScreenToPlaneDouble oBottom = oBottomEdge.getInterpolation(screenX, eCoordDimension::eScreenX);
    const SegmentInterpolator oChordVertical(oTop, oBottom);
    auto oCoordinatesVertical =  oChordVertical.getInterpolation(screenY, eCoordDimension::eScreenY);
    assert(oCoordinatesVertical.ScreenX == screenX);
    assert(oCoordinatesVertical.ScreenY == screenY);
    
    return {screenX, screenY, (oCoordinatesHorizontal.RealX + oCoordinatesVertical.RealX)/2.0, (oCoordinatesHorizontal.RealY + oCoordinatesVertical.RealY)/2.0};
    


}


//writes m_o3DCoords, m_oTimesComputed
bool Calibration3DCoordsInterpolator::cellTo3D(const tCellDataArray & rCellDataArray)
{
	// inits

	auto isValidIndex = [this] (const int & x, const int & y)
	{
		return x >= 0 && x < (int) mWidth && y >= 0 && y < (int) mHeight;
	};

	auto clipX = [this] (double x)
	{
		return clip<int>(std::floor(x), 0, mWidth - 1);
	};

	auto clipY = [this] (double x)
	{
		return clip<int>(std::floor(x), 0, mHeight - 1);
	};


	std::ostringstream oInfo;
#ifndef NDEBUG
	oInfo << "Interpolating cell ";
	for ( auto && oCorner : rCellDataArray)
	{
		oInfo << oCorner;
	}
	oInfo << "\n";
#endif

    auto & p_rTopLeft = rCellDataArray[eTopLeft];
    auto & p_rTopRight = rCellDataArray[eTopRight];
    auto & p_rBotLeft = rCellDataArray[eBotLeft];
    auto & p_rBotRight = rCellDataArray[eBotRight];
    
	int oLengthYLeft(std::abs(p_rBotLeft.ScreenY - p_rTopLeft.ScreenY));
	int oLengthYRight(std::abs(p_rBotRight.ScreenY - p_rTopRight.ScreenY));


	if ( ((p_rBotRight.ScreenX - p_rBotLeft.ScreenX) <= 0) || ((p_rTopRight.ScreenX - p_rTopLeft.ScreenX) <= 0) )
	{
		//"QnxMsg.Calib.BadDist"
		wmLog(eError,  "Two corners ore overlapping! Origin cannot be determined.\n");
		return false;
	}

	if ( ((oLengthYLeft == 0) && (oLengthYRight != 0)) || ((oLengthYLeft != 0) && (oLengthYRight == 0)) )
	{
		wmLogTr(eError, "QnxMsg.Calib.GridIncomplete", "Scheimpflug grid incomplete.\n");
		return false;
	}

	// number of loop runs equals largest pixel difference
	double oMaxDelta = std::max({oLengthYLeft, oLengthYRight});
	if ( std::abs(oMaxDelta) < math::eps )
	{
		wmLogTr(eError, "QnxMsg.Calib.GridNarrow", "Scheimpflug grid too narrow in %s-direction.\n", "y");
		return false;
	}

	// start computation

	//Divide the cell in numSubdivisions line segments, whose edges are equidistant on the vertical sides of the trapezoid defined by p_rTopLeft, p_rTopRight, p_rBotLeft, p_rBotRight
	const double oResolutionY = 1; //pixel
	const double oResolutionX = 1; //pixel 
	const double numSubdivisions = oMaxDelta / oResolutionY;

	const SegmentInterpolator LeftEdge(p_rTopLeft, p_rBotLeft);
	const SegmentInterpolator RightEdge(p_rTopRight, p_rBotRight);

	try
	{
		for ( int iH = 0; iH <= numSubdivisions; iH++ )
		{
			//find the extremes of the "chord" used for interpolation
			const coordScreenToPlaneDouble oLeft = LeftEdge.interpolate(iH / numSubdivisions);
			const coordScreenToPlaneDouble oRight = RightEdge.interpolate(iH / numSubdivisions);

			const SegmentInterpolator oChord(oLeft, oRight);

			const double oXStart = clipX(oLeft.ScreenX);
			const double oXEnd = clipX(oRight.ScreenX) + 1;
			//const double oXDelta = oRight.ScreenX - oLeft.ScreenX;

			assert(oXEnd > oXStart);

#ifdef extradebugcalib		
			oInfo << iH << "/" << int(numSubdivisions) << " interp. line " << oLeft << "  " << oRight << " with x along" << oXStart << "  " << oXEnd << "\n";
#endif 
			std::ostringstream oExtraInfo;
			bool oPrintExtraInfo = false;

			//compute the x-values along the line
			for ( double oX = oXStart; oX <= oXEnd; oX += oResolutionX )
			{
				//parametric position (percentage in the segment length)
				double t = oChord.getT(oX, eCoordDimension::eScreenX);

				coordScreenToPlaneDouble oCurrentPoint = oChord.interpolate(t);

				int oXPos = std::round(oCurrentPoint.ScreenX);
				int oYPos = std::round(oCurrentPoint.ScreenY);
				if ( oXPos != std::floor(oX) )
				{
					//for example oX=0, oCurrentPoint.ScreenY = -1e-15
					oExtraInfo << "Adjust rounding error in interpolation " << oCurrentPoint << "\n";
					t = oChord.getT(oX + 0.1, eCoordDimension::eScreenX);
					oCurrentPoint = oChord.interpolate(t);
					oXPos = std::round(oCurrentPoint.ScreenX);
					oYPos = std::round(oCurrentPoint.ScreenY);
					assert(oXPos == std::floor(oX));
				}

				if ( (t<0.02 || t> 0.98) )
				{
					oExtraInfo << "Computed coord [" << oXPos << " " << oYPos << "] " << " from " << t << " " << oCurrentPoint;
					oPrintExtraInfo = true;
				}

				if ( isValidIndex(oXPos, oYPos) )
				{
					//add coordinate, normalize later
                    m_o3DCoords.X(oXPos, oYPos) += oCurrentPoint.RealX;
                    m_o3DCoords.Y(oXPos, oYPos) += oCurrentPoint.RealY;
					m_oTimesComputed[mWidth * oYPos + oXPos] += 1;

#ifdef extradebugcalib
					if ( oPrintExtraInfo )
					{
						oExtraInfo << "- assigned " << getTC(oXPos, oYPos) << " times";
					}
					if ( isValidIndex(oXPos - 1, oYPos - 1) && isValidIndex(oXPos + 1, oYPos + 1) )
					{
						const auto & rTCLeft = getTC(oXPos - 1, oYPos);
						const auto & rTCRight = getTC(oXPos + 1, oYPos);

						if ( rTCLeft > 0 && rTCRight > 0 )
						{
							const auto xl = m_o3DCoords.X(oXPos-1, oYPos) / double(rTCLeft);
							const auto xr = m_o3DCoords.X(oXPos+1, oYPos) / double(rTCRight);
							const auto oXReal = oCurrentPoint.RealX;
							if ( (oXReal > xr || oXReal < xl) && false )
							{
								std::cout << oExtraInfo.str();
								std::cout << oInfo.str();
								oInfo.str("");
								oInfo.clear();

								std::cout << "Discontinuity along x " << " [ " << oXPos << " " << oYPos << "] " << oXReal << ":"
									<< xl << " (" << rTCLeft << ") " << xr << " (" << rTCRight << ") " << std::endl;
								assert(false);
							}
						}

						const auto & rTCUp = getTC(oXPos, oYPos - 1);
						const auto & rTCDown = getTC(oXPos, oYPos + 1);

						if ( rTCUp > 0 && rTCDown > 0 )
						{
							const auto yu = m_o3DCoords.Y(oXPos, oYPos-1)/ double(rTCUp);
							const auto yd = m_o3DCoords.Y(oXPos, oYPos+1) / double(rTCDown);
							const auto oYReal = oCurrentPoint.RealY;
							if ( (oYReal > yu || oYReal < yd) && false )
							{
								std::cout << oExtraInfo.str();

								std::cout << oInfo.str();
								oInfo.str("");
								oInfo.clear();
								std::cout << "Discontinuity along y " << " [ " << oXPos << " " << oYPos << "] " << oYReal
									<< ":" << yu << " (" << rTCUp << ") " << yd << " (" << rTCDown << ") " << std::endl;
								assert(false);
							}
						}
					}
#endif

				}

				else
				{
					if ( oPrintExtraInfo )
					{
						oExtraInfo << " out of range ";
					}
				}
				if ( oPrintExtraInfo )
				{
					oExtraInfo << "\n";
				}

			}// end x loop

			//compute the extremes again (If I am unlucky with x resolution)
			for ( auto oCorner : {oLeft, oRight} )
			{
				double oX = clipX(oCorner.ScreenX);
				double oY = clipY(oCorner.ScreenY);
				if ( getTC(oX, oY) < 1 )
				{
					auto oPoint = oChord.getInterpolation(oX, eCoordDimension::eScreenX);
					oInfo << "Computed missing extreme ";
					if ( oX != std::floor(oPoint.ScreenX) || oY != std::floor(oPoint.ScreenY) )
					{
						oInfo << "warning: possible rounding error, saving to extreme [ " << oX << "  " << oY << "]";
					}
					oInfo << "\n";
					//add coordinate, normalize later
                    m_o3DCoords.X(oX, oY) += oPoint.RealX;
                    m_o3DCoords.Y(oX, oY) += oPoint.RealY;
					m_oTimesComputed[mWidth * oY + oX] += 1;
				}
			}
			oExtraInfo.str("");
		} // end y loop
	}
	catch ( std::exception &re )
	{
		std::cout << "EXC cell to 3D " << re.what() << std::endl;
		return false;
	}

	bool error = false;
	for ( auto && pt : {p_rTopLeft, p_rTopRight, p_rBotLeft, p_rBotRight} )
	{
		auto oXPos = clipX(pt.ScreenX);
		auto oYPos = clipY(pt.ScreenY);
		if ( getTC(oXPos, oYPos) < 1 )
		{
			error = true;
			std::cout << " Vertex not computed " << oXPos << " " << oYPos << std::endl;
		}
	}
	if ( error )
	{
		std::cout << oInfo.str();
		oInfo.str("");
		oInfo.clear();
	}
	assert(!error  && "Vertex not computed");
	return true;
}

//no range-checking!
unsigned int Calibration3DCoordsInterpolator::getTC(unsigned int x, unsigned int y) const
{
	assert(m_oTimesComputed.size());
	assert(y < mHeight);
	assert(x < mWidth);
	return m_oTimesComputed[mWidth*y + x];
}


bool Calibration3DCoordsInterpolator::getValueByNeighbours(const unsigned int p_oX, const unsigned int p_oY)
{

	//reset current value
    m_o3DCoords.X(p_oX, p_oY) = 0;
    m_o3DCoords.Y(p_oX, p_oY) = 0;
	m_oTimesComputed[mWidth*p_oY + p_oX] = 0;


	/*
	* We need at least either known coordinates to both left and right or (better and) above and below for interpolating unknown points.
	* If, for instance, we only knew the one to the left and above, at least one of the coordinates is interpolated in the wrong direction.
	* By example: Given            2,2
	*                       1,3    x,y   (3,3)
	*                             (2,4)
	* Consider the values in brackets were not known. Interpolating by left and top neighbour leads to (x,y)=(1.5,2.5). For not linear distances (such as a Scheimpflug grid),
	* it might even be something like (0.9, 2.5), which clearly is a fairly bad approximation.
	*/
	for ( eCoordDimension oSearchDirection : { eCoordDimension::eScreenY, eCoordDimension::eScreenX} )
	{
		int oXA = p_oX;
		int oXB = p_oX;
		int oYA = p_oY;
		int oYB = p_oY;

		if ( oSearchDirection == eCoordDimension::eScreenX ) //tests left and right neighbours
		{
			oXA++;
			while ( (oXA >= 0) && (oXA < ( (int)(mWidth) - 1)) && getTC(oXA, oYA) == 0 )
			{
				oXA++;
			}
			oXB--;
			while ( (oXB > 0) && (oXB < (int)(mWidth)) && getTC(oXB, oYB) == 0 )
			{
				oXB--;
			}
		}
		else // tests top and bottom neighbours
		{
			assert(oSearchDirection == eCoordDimension::eScreenY);
			oYA++;
			while ( (oYA >= 0) && (oYA < ((int)(mHeight) - 1)) && getTC(oXA, oYA) == 0 )
			{
				oYA++;
			}
			oYB--;
			while ( (oYB > 0) && (oYB < (int)(mHeight)) && getTC(oXB, oYB) == 0 )
			{
				oYB--;
			}

		}

		if ( oXA < 0 || oXA >= (int) mWidth || oXB < 0 || oXB >= (int) mWidth ||
			oYA < 0 || oYA >= (int) mHeight || oYB < 0 || oYB >= (int) mHeight )
		{
			return false;
		}

		auto oNumTC_A = getTC(oXA, oYA);
		auto oNumTC_B = getTC(oXB, oYB);

		if ( oNumTC_A > 0 && oNumTC_B > 0 ) // if available, use left, right and bot, top neighbours for approximation
		{

			SegmentInterpolator oSegment(
				coordScreenToPlaneDouble(oXA, oYA, m_o3DCoords.X(oXA, oYA) / oNumTC_A, m_o3DCoords.Y(oXA, oYA)  / oNumTC_A),
				coordScreenToPlaneDouble(oXB, oYB, m_o3DCoords.X(oXB, oYB)  / oNumTC_B, m_o3DCoords.Y(oXB, oYB)  / oNumTC_B));

			double t = oSearchDirection == eCoordDimension::eScreenY ?
				oSegment.getT(p_oY, eCoordDimension::eScreenY) : oSegment.getT(p_oX, eCoordDimension::eScreenX);

			auto oInterpolated = oSegment.interpolate(t);
#ifdef extradebugcalib
			if ( (std::abs(oSegment.m_Delta.ScreenX) > 2 || std::abs(oSegment.m_Delta.ScreenY > 2)) && p_oX < 75 )
			{
				std::cout << "Computing " << p_oX << "  " << p_oY << " from distant neighbours: " << oSegment.m_Start << " " << oSegment.m_Start + oSegment.m_Delta
					<< " = " << oInterpolated << std::endl;
			}
#endif
            m_o3DCoords.X(p_oX, p_oY) += oInterpolated.RealX;
            m_o3DCoords.Y(p_oX, p_oY) += oInterpolated.RealY;
			m_oTimesComputed[mWidth*p_oY + p_oX] += 1;
		}
	}

	int numTimesComputed = getTC(p_oX, p_oY);
	if ( numTimesComputed == 0 )
	{
		std::cout << "Could not interpolate missing grid point " << p_oX << " " << p_oY;
		return false;
	}

		//normalize on the spot
		m_o3DCoords.X(p_oX, p_oY) /= numTimesComputed;
   		m_o3DCoords.Y(p_oX, p_oY) /= numTimesComputed;
		m_oTimesComputed[mWidth*p_oY + p_oX] = 1;
	return true;


}


void Calibration3DCoordsInterpolator::resetGridCellData(const int & pWidth, const int & pHeight)
{
	if ( pWidth <= 0 || pHeight <= 0 )
	{
		mWidth = 0;
		mHeight = 0;
	}
	else
	{
		mWidth = pWidth;
		mHeight = pHeight;
	}

	m_oTimesComputed.resize(mWidth*mHeight);
	m_oTimesComputed.assign(mWidth*mHeight, 0);

    m_o3DCoords.resetGridCellData(pWidth, pHeight, SensorModel::eCalibrationGridOnLaserPlane );
    
#ifndef NDEBUG
    float coordx, coordy;
	assert(!m_o3DCoords.getCoordinates( coordx, coordy, mWidth - 1, mHeight - 1) );
    assert(m_o3DCoords.getSensorSize().area() == (int) (m_oTimesComputed.size()));
#endif
}




bool Calibration3DCoordsInterpolator::applyTranslationToSetOrigin(CalibrationCornerGrid& pCornerGridToUpdate, double xScreenOrigin, double yScreenOrigin, double expected_xReal, double expected_yReal)
{

    auto oInitialTransform = pCornerGridToUpdate.getTransform();

    pCornerGridToUpdate.setTransform({});
    auto oCellDataInfo = pCornerGridToUpdate.findCellData( xScreenOrigin, yScreenOrigin);
    if (!std::get<0>(oCellDataInfo))
    {
        return false;
    }

    auto oOrigin_mm = Calibration3DCoordsInterpolator::cellTo3D(std::get<1>(oCellDataInfo), xScreenOrigin, yScreenOrigin);

    oInitialTransform.m_oTx -= oOrigin_mm.RealX;
    oInitialTransform.m_oTy -= oOrigin_mm.RealY;
    pCornerGridToUpdate.setTransform(oInitialTransform);
#ifndef NDEBUG
    {
        auto oNewCellDataInfo = pCornerGridToUpdate.findCellData( xScreenOrigin, yScreenOrigin);
        assert(std::get<0>(oNewCellDataInfo));
        auto oNewOrigin_mm = Calibration3DCoordsInterpolator::cellTo3D(std::get<1>(oNewCellDataInfo), xScreenOrigin, yScreenOrigin);
        assert(isClose( oNewOrigin_mm.RealX, 0.0, 1e-6));
        assert(isClose( oNewOrigin_mm.RealY, 0.0, 1e-6));

    }
#endif
    pCornerGridToUpdate.applyTransform();

    auto oTransformChessboardToScanner_mm = pCornerGridToUpdate.getTransform();
    assert(oTransformChessboardToScanner_mm.hasOnlyScale());

    oTransformChessboardToScanner_mm.m_oTx += expected_xReal;
    oTransformChessboardToScanner_mm.m_oTy += expected_yReal;
    
    pCornerGridToUpdate.setTransform(oTransformChessboardToScanner_mm);
#ifndef NDEBUG
    {
        auto oNewCellDataInfo = pCornerGridToUpdate.findCellData( xScreenOrigin, yScreenOrigin);
        assert(std::get<0>(oNewCellDataInfo));
        auto oNewOrigin_mm = Calibration3DCoordsInterpolator::cellTo3D(std::get<1>(oNewCellDataInfo), xScreenOrigin, yScreenOrigin);
        assert(isClose( oNewOrigin_mm.RealX, expected_xReal, 1e-6));
        assert(isClose( oNewOrigin_mm.RealY, expected_yReal, 1e-6));

    }
#endif
    pCornerGridToUpdate.applyTransform();

#ifndef NDEBUG
    {
        auto oNewCellDataInfo = pCornerGridToUpdate.findCellData( xScreenOrigin, yScreenOrigin);
        assert(std::get<0>(oNewCellDataInfo));
        auto oNewOrigin_mm = Calibration3DCoordsInterpolator::cellTo3D(std::get<1>(oNewCellDataInfo), xScreenOrigin, yScreenOrigin);
        assert(isClose( oNewOrigin_mm.RealX, expected_xReal, 1e-6));
        assert(isClose( oNewOrigin_mm.RealY, expected_yReal, 1e-6));
    }
#endif


    return true;


}

}
}
