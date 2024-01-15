#include "calibrationCornerGrid.h"
#include "math/mathCommon.h" //isInRange

namespace precitec
{
namespace math
{

using geo2d::eCoordDimension;
using geo2d::coord2D;
using geo2d::coordScreenToPlaneDouble;
using system::tGridMap;
using system::t_coordScreenToPlaneLine;


void CornerGridMap::reset(geo2d::Size p_oValidArea)
{    
    m_oValidArea = p_oValidArea;
	m_oGrid.clear();
	m_oLineBoundaries.clear();
	m_oCornerCorrectionTransforms = RealWorldTransform{};
}

//does not check if row already exists
void CornerGridMap::addRow ( int yAverage, int yMin, int yMax, system::t_coordScreenToPlaneLine coordinates)
{
    m_oLineBoundaries[yAverage] = {yMin, yMax};
    m_oGrid[yAverage] = std::move(coordinates);
}



void CornerGridMap::setTransform(RealWorldTransform p_Transform)
{
    m_oCornerCorrectionTransforms = p_Transform;
}

void CornerGridMap::applyTransform()
{

    for (auto && rGridRow : m_oGrid)
    {
        for (auto && rCoordinate : rGridRow.second)
        {
            rCoordinate.convertToRealWorldCoords(m_oCornerCorrectionTransforms);
            
        }
    }
    m_oCornerCorrectionTransforms = RealWorldTransform{};
}


bool CornerGridMap::isEmpty() const {
    return m_oGrid.size() == 0;
}


CalibrationCornerGrid::CalibrationCornerGrid(geo2d::Size p_oValidArea)
{
    reset(p_oValidArea); 
}


void CalibrationCornerGrid::reset(geo2d::Size p_oValidArea)
{
    CornerGridMap::reset(p_oValidArea);
	m_oHorizontalLines.clear();
	m_oVerticalLines.clear();
}

const system::tGridMap2D3D& CornerGridMap::getGrid2D3D() const
{
	return m_oGrid;
}

system::tGridMap CornerGridMap::getGrid2D() const
{
	tGridMap orderedGrid;
	for( auto & entry: m_oGrid)
	{
		std::vector<coord2D> oLine;
		for (auto & coord: entry.second)
		{
			oLine.push_back(coord2D(coord.ScreenX, coord.ScreenY) );
		}
		orderedGrid[entry.first] = oLine;
	}
	return orderedGrid;
}

const RealWorldTransform& CornerGridMap::getTransform() const
{
	return m_oCornerCorrectionTransforms;
}

const system::tLineBoundaries& CornerGridMap::getLineBoundaries() const
{
    return m_oLineBoundaries;
}

void CornerGridMap::printGrid(std::ostream & out) const
{

	out << "Grid Screen Coordinates: " << "\n";
	out << std::setprecision(2);
	//out << std::setw(8);

	for ( auto && oLine : m_oGrid )
	{
		for ( auto && oPoint : oLine.second )
		{
			out << oPoint << ";  ";
		}
		out << '\n';
	}
	out << "\n";

	out << "Grid X [mm] Coordinates: " << "\n";
	for ( auto && oLine : m_oGrid )
	{
		for ( auto && oPoint : oLine.second )
		{
			out << std::setw(10) << oPoint.RealX << " ;";
		}
		out << "\n";
	}
	out << "\n";

	out << "Grid Y [mm] Coordinates: " << "\n";
	for ( auto && oLine : m_oGrid )
	{
		for ( auto && oPoint : oLine.second )
		{
			out << std::setw(10) << oPoint.RealY << " ; ";
		}
		out << "\n";
	}
	out << "\n";

}

bool CalibrationCornerGrid::computeCornerData(CalibrationCornerGrid &rGrid, const system::CamGridData & p_CamGridData, bool linearize)
{
	const tGridMap & rCamGrid = p_CamGridData.gridRef();
	const int oMaxXDelta = 20;
	return computeCornerData(rGrid, rCamGrid, oMaxXDelta, linearize);
}

bool CalibrationCornerGrid::computeCornerData(CalibrationCornerGrid &rGrid, const tGridMap & rGridMap, const int oMaxXDelta, bool linearize)
{
	/*
	* General Note: a "tube"is basically the y-coord extent of a "horizontal" line of corners, that is it is the region from min y to max y coord.
	*/

	//strict mode: exit if rows length < 2 or if there are incomplete cells
	bool bEnforceValidGrid = true;
	bool bMergeLines = true;

	bool oRes(true);


	//assumption: points are ordered 
	//here the recognition critera is simple, it's only the x diff with the prev line

	//final "padded" size
	int numRows = rGridMap.size();
	int numCols = numRows > 0 ? rGridMap.begin()->second.size() : 0;

	rGrid.m_oNumberOfChessboardRows = numRows;

	std::vector<int> oLastXValues;
	oLastXValues.reserve(2 * numCols);

	std::vector<int> oAvgYValues(numRows, -1);

	//chessboard coordinate (index of squares) .. will be saved as real coordinate, then rescaled with the square width value
	unsigned int indexRow = 0;

	for ( auto itCurLine = rGridMap.begin();
		itCurLine != rGridMap.end() && (!bEnforceValidGrid || oRes);
		++itCurLine ) // traverse lines, copy data from one data structure to another and initialize the latter
	{
		const auto &rAvgY(itCurLine->first);
		const auto &rCorners(itCurLine->second);
		oAvgYValues[indexRow] = rAvgY;

		assert(indexRow < rGridMap.size());
		assert(rGrid.m_oGrid.size() == indexRow); 

		int o_yMin = rAvgY + 1;
		int o_yMax = rAvgY - 1;

		if ( bEnforceValidGrid && !bMergeLines && rCorners.size() < 2 )
		{
			oRes = false;
			std::cout << "Line at  " << rAvgY << " is too short ";
			for ( auto & pt : rCorners )
			{
				std::cout << pt << "  ";
			}
			std::cout << std::endl;
			break;
		}
		//iterate horizontally						
		t_coordScreenToPlaneLine oMagLine(rCorners.size()); // local vector for holding a line, that is adjacent corners=coordinates
		int oMagLineIndex = 0;

		//for checking validity only
		auto x_old = rCorners.size() > 0 ? (rCorners.begin()->ScreenX - oMaxXDelta - 1) : 0;

		unsigned int indexColExpected = 0;
		for ( auto itCorners = rCorners.begin();
			itCorners != rCorners.end() && (!bEnforceValidGrid || oRes);
			++itCorners, ++oMagLineIndex )
		{
			assert(oMagLineIndex == itCorners - rCorners.begin());
			const auto & x = itCorners->ScreenX;
			const auto & y = itCorners->ScreenY;

			//update ymin, ymax
			o_yMin = y < o_yMin ? y : o_yMin;
			o_yMax = y > o_yMax ? y : o_yMax;

			unsigned int indexCol = 0;
			//check assumptions
			if ( x <= x_old )
			{
				oRes = false;
				std::cout << " Assumption violated: x coordinates are not sorted " << std::endl;
				break;
			}
			if ( x <= (x_old + oMaxXDelta) )
			{
				oRes = false;
				std::cout << " Assumption violated: x coordinates are too close " << std::endl;
				break;
			}

			//adjust position on xvalues
			//advance right

			while ( indexCol < oLastXValues.size() && x >(oLastXValues[indexCol] + oMaxXDelta) )
			{
				indexCol++;
			}
			if ( bEnforceValidGrid && indexColExpected != 0 && (indexCol > indexColExpected) )
			{
				std::cout << "Not contiguos cells: x value " << oLastXValues[indexColExpected] << " missing at line " << y
					<< " between coordinates " << x_old << " " << x << std::endl;
				oRes = false;
				break;
			}

			if ( indexCol == oLastXValues.size() )
			{
				assert(oLastXValues.begin() + indexCol == oLastXValues.end());

				//I just need to append the x values, no need to update the previous entries
				for ( auto it = itCorners; it != rCorners.end(); ++it )
				{
					oLastXValues.push_back(it->ScreenX);
				}
				assert(oLastXValues[indexCol] == x); //by definition
				assert(indexCol < oLastXValues.size());
				numCols = oLastXValues.size();
			}

			//check if the alignment condition is satisfied
			auto oExpectedXMin = oLastXValues[indexCol] - oMaxXDelta;

			if ( x < oExpectedXMin )
			{
				//update x reference with this point (and the immediate neighbours), if necessary
				//advance on the current line until I find the expected x neighbor

				//here I need to insert the column between already present columns

				if ( bEnforceValidGrid && indexCol > 0 )
				{
					oRes = false;
					std::cout << "Not contiguos cells: x value " << x << " missing ("
						<< oLastXValues[indexCol - 1] << " " << oLastXValues[indexCol]
						<< " tol " << oMaxXDelta << " ) " << std::endl;
					break;
				}

				int offset = 0;
				for ( auto it = itCorners;
					it != rCorners.end() && (it->ScreenX < oExpectedXMin);
					++it )
				{
					oLastXValues.insert(oLastXValues.begin() + indexCol + offset, it->ScreenX);
					offset++;
				}

				assert(offset == int(oLastXValues.size()) - numCols);
				assert(offset > 0);
				numCols = oLastXValues.size();
				//upgrade all the previous entries, hopefully I dont' need to do it anymore in this line
				for ( auto && rStoredMagLine : rGrid.m_oGrid )
				{
					for ( auto && rStoredCoord : rStoredMagLine.second )
					{
						auto & rOldIndexCol = rStoredCoord.RealX;
						if ( rOldIndexCol >= indexCol )
						{
							rOldIndexCol += offset;
						}
					}
				}
			}

			assert(indexCol < oLastXValues.size());
			assert(std::abs(x - oLastXValues[indexCol]) <= oMaxXDelta);
			assert(itCorners != rCorners.end());
			oMagLine[oMagLineIndex] = coordScreenToPlaneDouble(x, y, indexCol, indexRow);


			oLastXValues[indexCol] = x;
			indexColExpected = indexCol + 1; //to check missign points in next loop
			x_old = x;
		}//end horizontal iteration

		int oUpdateIndex = -1;
		if ( bMergeLines )
		{
			double yTol = 10;
			std::vector<int> CandidateLines;
			for ( auto it = rGrid.m_oLineBoundaries.begin(); it != rGrid.m_oLineBoundaries.end(); it++ )
			{
				auto&  rBoundMin = it->second.first;
				auto&  rBoundMax = it->second.second;
				if ( std::abs(o_yMin - rBoundMax) < yTol || std::abs(o_yMax - rBoundMin) < yTol )
				{
					CandidateLines.push_back(it->first);
				}
			}
			if ( CandidateLines.size() > 0 )
			{
				oUpdateIndex = CandidateLines[0];
			}
			if ( CandidateLines.size() > 1 )
			{
				std::cout << " not supported " << std::endl;
			}
		};
		//if the iteration was interrupted (for example because of missing points) I need to delete the elements not computed
		assert(!oRes || oMagLineIndex == int(rCorners.size()));
		oMagLine.erase(oMagLine.begin() + oMagLineIndex, oMagLine.end());

		if ( !(rAvgY + 1 >= o_yMin && rAvgY - 1 <= o_yMax) )
		{
			std::cout << " Wrong input data: AvgY " << rAvgY << "  " << o_yMin << "  " << o_yMax << std::endl;
			oRes = false;
			assert(false);
		}
		if ( oUpdateIndex < 0 )
		{
            // for each tube (area bounding a "horiz." line) set max and min y coordinate
            // and collect the tubes point
            rGrid.addRow(rAvgY, o_yMin, o_yMax, oMagLine);                        
			indexRow++;
		}
		else
		{
			//merging to line at oUpdateIndex
			rGrid.m_oNumberOfChessboardRows--;
			auto itBounds = rGrid.m_oLineBoundaries.find(oUpdateIndex);
			auto itGrid = rGrid.m_oGrid.find(oUpdateIndex);

			auto & rBoundElements = itBounds->second;

			if ( rBoundElements.first > o_yMin )
			{
				rBoundElements.first = o_yMin;
			}
			if ( rBoundElements.second > o_yMax )
			{
				rBoundElements.second = o_yMax;
			}

			auto & rMagLineToUpdate = itGrid->second;
			unsigned int newIndexRow = static_cast<int> (rMagLineToUpdate[0].RealY); 
			assert(int(newIndexRow) == std::floor( rMagLineToUpdate[0].RealY)) ;
			
			for ( auto & pt : oMagLine )
			{
				unsigned int newIndexCol = 0;
				while ( newIndexCol < oLastXValues.size() && oLastXValues[newIndexCol] + oMaxXDelta < pt.ScreenX )
				{
					newIndexCol++;
				}
				{  //check that value has been updated in previous step
					assert(newIndexCol < oLastXValues.size());
					assert(oLastXValues[newIndexCol] == pt.ScreenX);

				};

				//look for position in rMagLine
				auto itStoredCorner = rMagLineToUpdate.begin();
				while ( itStoredCorner != rMagLineToUpdate.end() && itStoredCorner->ScreenX < pt.ScreenX )
				{
					++itStoredCorner;
				}
				if ( itStoredCorner != rMagLineToUpdate.end() )
				{

					if(itStoredCorner->RealX <= newIndexCol)
					{
						oRes = false;
						break;
					}
				}
				else
				{
					if( prev(itStoredCorner)->RealX >= newIndexCol)
					{
						oRes = false;
						break;
					}
				}
				rMagLineToUpdate.insert(itStoredCorner, coordScreenToPlaneDouble(pt.ScreenX, pt.ScreenY, newIndexCol, newIndexRow));
			}

		} //end merge at updateIndex
	}// end curLine
	
	if (!oRes )
	{
		rGrid.m_oNumberOfChessboardRows = rGrid.m_oGrid.size();
		//invalid grid, no further processing
		return false;
	}

	//check if there are remaining points that could not be merged in the last operation
	if (!rGrid.m_oGrid.empty())
	{        
        auto itLastLine= rGrid.m_oGrid.rbegin();

        if (itLastLine->second.size() < 2)
        {
            std::cout << "Trimming last point " << itLastLine->first << std::endl;
            rGrid.m_oNumberOfChessboardRows--;
            rGrid.m_oLineBoundaries.erase(itLastLine->first);
            rGrid.m_oGrid.erase(itLastLine->first);
        }
    
    }
	
	rGrid.m_oNumberOfChessboardColumns = numCols;
	assert(rGrid.m_oGrid.size() == rGrid.m_oNumberOfChessboardRows);
	assert(int(oLastXValues.size()) == numCols);
	assert(int(oAvgYValues.size()) == numRows);

	rGrid.fitLines();
	if ( linearize )
	{
		std::cout << "Linearize ... \n";
		auto oCorrectedPoints = rGrid.alignPoints(1);
        //check the distance between the raw point and the corrected point
		for ( auto & oPoint : oCorrectedPoints )
		{
			double x, y;
			bool valid = rGrid.getLinearizedCornerPosition(oPoint.RealX, oPoint.RealY, x, y);
			if ( valid )
			{
				std::cout << " Adjusted [" << oPoint.ScreenX << " " << oPoint.ScreenY << "] to [" << x << "  " << y << "]\n";
			}
			else
			{
				std::cout << "Invalid position for point " << oPoint << std::endl;
			}
		}
	}

	//set y positive at top of the image, negative below
	rGrid.m_oCornerCorrectionTransforms.m_invertY = true; 

	return oRes;

}


bool CalibrationCornerGrid::fitLines()
{
	assert(m_oNumberOfChessboardRows == m_oGrid.size());
	if ( m_oNumberOfChessboardRows == 0 )
		return false;
	typedef std::vector<double> coordsToFit;
	std::vector< coordsToFit > oColumns_X(m_oNumberOfChessboardColumns);
	std::vector< coordsToFit > oColumns_Y(m_oNumberOfChessboardColumns);

	m_oHorizontalLines.resize(m_oNumberOfChessboardRows);
	m_oVerticalLines.resize(m_oNumberOfChessboardColumns);

	//iterate on row (compute horizontal lines and fill oColumns)
	const auto & rGrid = m_oGrid;
	int oCurrentRow = 0;
	for ( auto & oCornerRow : rGrid )
	{
		auto oRowSize = oCornerRow.second.size();
		std::vector<double> oXValues, oYValues;
		oXValues.reserve(oRowSize);
		oYValues.reserve(oRowSize);
		if ( oRowSize == 0 )
		{
			continue;
		}
		assert(oCornerRow.second.front().RealX + oRowSize <= m_oNumberOfChessboardColumns);
		
		for ( auto & oCorner : oCornerRow.second )
		{
			const unsigned int i = oCorner.RealX;
#ifndef NDEBUG            
			const int j = oCorner.RealY;

			assert(j == oCurrentRow);

			assert(oCorner.RealX == i);
			assert(oCorner.RealY == j);
#endif
			const double oX = static_cast<double> (oCorner.ScreenX);
			const double oY = static_cast<double> (oCorner.ScreenY);

			oXValues.push_back(oX);
			oYValues.push_back(oY);
			
			assert(i < m_oNumberOfChessboardColumns);
			oColumns_X[i].push_back(oX);
			oColumns_Y[i].push_back(oY);
		}

		assert(oXValues.size() == oYValues.size());
		assert(oXValues.size() == oCornerRow.second.size());
		m_oHorizontalLines[oCurrentRow] = LineEquation(oXValues, oYValues, true);
		const auto & rCurLine = m_oHorizontalLines[oCurrentRow];

		for ( auto & oCorner : oCornerRow.second )
		{
			const double x = static_cast<double> (oCorner.ScreenX);
			const double y = static_cast<double> (oCorner.ScreenY);
			double y1 = rCurLine.getY(x);
			double r = rCurLine.distance(x, y);
			if ( r > 1 || std::abs(y1 - y) > 2 )
			{
				std::cout << "Distortion at point " << x << " " << y << " " << " y should be " << y1 << std::endl;
			}
		}

		++oCurrentRow;
	}

	//iterate along columns
	m_oVerticalLines.resize(m_oNumberOfChessboardColumns);
	for ( unsigned int oCurrentColumn = 0; oCurrentColumn < m_oNumberOfChessboardColumns; ++oCurrentColumn )
	{
		auto & rXValues = oColumns_X[oCurrentColumn];
		auto & rYValues = oColumns_Y[oCurrentColumn];
		assert(rXValues.size() == rYValues.size());
		//swap x and y
		m_oVerticalLines[oCurrentColumn] = LineEquation(rXValues, rYValues, false);
		const auto & rCurLine = m_oVerticalLines[oCurrentColumn];

		for (unsigned int i = 0; i < rXValues.size(); i++ )
		{
			double & x = rXValues[i];
			double & y = rYValues[i];
			double x1 = rCurLine.getX(y);
			double r = rCurLine.distance(x, y);
			if ( r > 1 || std::abs(x1 - x) > 2 )
			{
				std::cout << "Distortion at point " << x << " " << y << " " << " x should be " << x1 << std::endl;
			}
		}
	}

    if ( m_oPrintLines )
    {
        std::cout << "\n";
        std::cout << "Horizontal lines " << "\n";

        int counter = 0;
        for ( auto & oLine : m_oHorizontalLines )
        {
            counter ++;
            if ( oLine.isValid() )
            {
                std::cout << counter << ") Inclination " << oLine.getInclinationDegrees() << "degrees,  passing through  (0," << oLine.getY(0) << ")\n";
            }
            else
            {
                std::cout << counter << ") Could not fit horizontal line" << std::endl;
            }
        }
        std::cout << "Vertical lines " << "\n";
        counter = 0;
        for ( auto & oLine : m_oVerticalLines )
        {
            counter ++;
            if (oLine.isValid())
            {
                std::cout << counter << ") Inclination " << oLine.getInclinationDegrees() << "degrees,  passing through  (" << oLine.getX(0) << ", 0)\n";
            }
            else
            {
                std::cout << counter << ") Could not fit vertical line" << std::endl;
            }
        }
    }

	return true;
}


t_coordScreenToPlaneLine CalibrationCornerGrid::alignPoints(double threshold)
{
	t_coordScreenToPlaneLine result;
	result.reserve(m_oNumberOfChessboardRows*m_oNumberOfChessboardColumns / 2);
	//now compute error
	for ( auto & oCornerRow : m_oGrid )
	{
		for ( auto & oCorner : oCornerRow.second )
		{
			const unsigned int i = oCorner.RealX;
			const unsigned int j = oCorner.RealY;

			auto & rX = oCorner.ScreenX;
			auto & rY = oCorner.ScreenY;
			
			//compute intersection
			double x, y;
			bool hasIntersection = getLinearizedCornerPosition(i,j,x,y);
			if ( hasIntersection == 0 )
			{
				std::cout << "Error in intersection at position " << i << " " << j 
					<< " lines " << m_oHorizontalLines[j].getInclinationDegrees() << m_oVerticalLines[i].getInclinationDegrees()
					<< std::endl; //TODO
				result.push_back(coordScreenToPlaneDouble(oCorner));
				continue;
			}
			
			if ( x <0 || x >= m_oValidArea.width || y < 0 || y >= m_oValidArea.height )
			{
				std::cout << "Intersection outside of image area " << x << " " << y <<  std::endl; //TODO
				result.push_back(coordScreenToPlaneDouble(oCorner));
				continue;
			}
			if ( std::abs(x - rX) > threshold || std::abs(y - rY) > threshold )
			{
				result.push_back(coordScreenToPlaneDouble(oCorner));
			}

			//finally update the values
			rX = x;
			rY = y;

			assert(oCorner.ScreenX == x);
			assert(oCorner.ScreenY == y);
			
		}
	}

	return result;
}


bool CalibrationCornerGrid::getLinearizedCornerPosition(const int ChessboardX, const int ChessboardY, double & ScreenX, double & ScreenY) const
{

	auto & rHorizontalLine = getHorizontalLineEquation(ChessboardY);
	auto & rVerticalLine = getVerticalLineEquation(ChessboardX);

	bool hasIntersection = rHorizontalLine.intersect(ScreenX, ScreenY, rVerticalLine);
	if ( !hasIntersection )
	{
		return false;
	}
	assert(rHorizontalLine.isValid());
	assert(rVerticalLine.isValid());
	assert(std::abs(rHorizontalLine.distance(ScreenX, ScreenY)) < 1e-5);
	assert(std::abs(rVerticalLine.distance(ScreenX, ScreenY)) < 1e-5);
	return true;

}

const LineEquation & CalibrationCornerGrid::getHorizontalLineEquation(unsigned int ChessboardY) const
{
    if ( ChessboardY > m_oNumberOfChessboardRows )
    {
        return m_oInvalidLine;
    }
    return  m_oHorizontalLines[ChessboardY];
}


const LineEquation & CalibrationCornerGrid::getVerticalLineEquation(unsigned int ChessboardX) const
{
    if ( ChessboardX > m_oNumberOfChessboardColumns)
    {
        return m_oInvalidLine;
    }
    return  m_oVerticalLines[ChessboardX];
}

std::vector< geo2d::coord2DScreen> CornerGridMap::getAllNodesScreen() const
{
    std::vector< geo2d::coord2DScreen> result;
    for( auto & entry: m_oGrid)
    {
       for (auto & coord: entry.second)
       {
           result.push_back(coord.getScreenCoords());
       }
    }
    return result;
}

//compare with Calibratio3DCoordsInterpolator::allCellsTo3D
std::vector< std::array<geo2d::coord2DScreen ,2>> CornerGridMap::getAllSegmentsScreen() const
{
    using precitec::math::eEdgePosition;
    typedef precitec::math::sCellData::tSegment cellDataSegment;
    typedef std::array<geo2d::coord2DScreen,2> screenCoordSegment;


    std::vector<screenCoordSegment> result;

    auto fCellDataSegmentToScreenCoordSegment = [] (const cellDataSegment& rSegment ) -> screenCoordSegment
    {
        return screenCoordSegment({   rSegment[0].getScreenCoords(),
                                      rSegment[1].getScreenCoords()
                                  });

    };

    if ( m_oGrid.size() < 2)
    {
        return result;
    }

    const auto & rGrid = m_oGrid;

    for (auto oItTopLine = rGrid.cbegin(), oItBotLine = next(oItTopLine), oItEnd = rGrid.end(), oItLastLine = prev(oItEnd);
         oItBotLine != oItEnd;
         ++oItTopLine, ++ oItBotLine
         )
    {
        bool isLastLine = oItBotLine == oItLastLine;
        sCellData::tSegment oLastRightSegment; //continously overwritten during row iteration
        
        //iterate checkerboard squares (from itTopLeft)
        for ( int oIdxTopLeft = 0, oIdxTopLeftMax=oItTopLine->second.size() - 1;
              oIdxTopLeft < oIdxTopLeftMax;
              ++oIdxTopLeft )
        {            
            sCellData oCell (oItTopLine->second, oItBotLine->second, oIdxTopLeft, sCellData::CellMode::fixedChessboardCoordinates);
            if (!oCell.isValid())
            {
                continue;
            }
            result.push_back(fCellDataSegmentToScreenCoordSegment( oCell.getSegment(eEdgePosition::eLeft)));
            result.push_back(fCellDataSegmentToScreenCoordSegment( oCell.getSegment(eEdgePosition::eTop)));
            
            if (isLastLine)
            {
                result.push_back(fCellDataSegmentToScreenCoordSegment( oCell.getSegment(eEdgePosition::eBottom)));
            }
            
            oLastRightSegment = oCell.getSegment(eEdgePosition::eRight);
            
        } //end row iteration
        //save right edge of the last cell
        result.push_back(fCellDataSegmentToScreenCoordSegment(oLastRightSegment));
    }//end grid iteration

    return result;
}

std::vector<LineEquation> CalibrationCornerGrid::getAllLines() const
{
    std::vector<LineEquation> result;
    for (const auto & line : m_oHorizontalLines)
    {
        result.push_back(line);
    }
    for (const auto & line : m_oVerticalLines)
    {
        result.push_back(line);
    }
    return result;
}

sCellData CalibrationCornerGrid::findCell(double p_oX, double p_oY) const
{
	// necessary conditions for this function to work correctly: adjacent lines must have a minimum distance of 2 regarding their max and min y coordinates.
	// returns false, if point is "out of bounds", that is not within the grid.


	if ( m_oGrid.size() < 2 ) // at least two lines/rows necessary, othwerwise there is no grid at all, not even a single cell...
	{
		return {};
	}


	// inits
	auto oItOne = m_oGrid.begin(); auto oItTwo = m_oGrid.begin();
	Vec3D oPoint(static_cast<double>(p_oX), static_cast<double>(p_oY), 0.0);

	/* search lines that sandwich the sought y coord when cosidering highest upper and lowest lower point. Graphically (starts are grid points)

	-------*-------
	*         *

	*     *
	--------------*


	If the point in question is above or below the grid, both iterators will be identical, pointing at the map's begin() or end()-1 position, respectively.
	*/
	auto oMin = m_oLineBoundaries.at(oItOne->first).second;  // get maximum. m_oLineBoundaries[p_oSensorID][oItOne->first] references the line...
	auto oMax = m_oLineBoundaries.at(oItOne->first).second; // get maximum

	if ( oPoint[1] >= oMax ) // in this case we are definitely below the first line
	{
		// Set oItOne, oItTwo and Min, Max such that they are different and sandwich oY, if the latter is not above the grid
		++oItTwo;
		oMin = m_oLineBoundaries.at(oItOne->first).first;
		oMax = m_oLineBoundaries.at(oItTwo->first).second;

		while ( (oItTwo != m_oGrid.end()) && !isInRange(oPoint[1], oMin, oMax) ) // find lines that sandwich Y
		{
			++oItOne; ++oItTwo;
			oMin = m_oLineBoundaries.at(oItOne->first).first;
			oMax = m_oLineBoundaries.at(oItTwo->first).second;
		}

		if ( oItTwo == m_oGrid.end() )// here we are definitely below the grid
		{
			oItTwo = oItOne;
		}
	}

	/* At this stage we know, which grid lines sandwich the sought point or whether the sought point definitely lies above or below the grid. */
	unsigned int oIdxOneLeft(0), oIdxOneRight(0), oIdxTwoLeft(0), oIdxTwoRight(0);
	int oFound(2); // to prevent infinite loops
	auto oLineOne = oItOne->second; auto oLineTwo = oItTwo->second;

	/*
	* The idea is to find the correct horizontal position given the enclosing lines found above. However,
	* not all y-coordinates of the corners of one line are identical, and as the key of the map is an average of the latter,
	* it may happen that we have to move one call up or down and repeat finding the correct x position.
	* That is, what the next while loop is all about...
	*/
	while ( oFound > 0 )
	{
		oIdxOneLeft = 0; oIdxOneRight = 0; oIdxTwoLeft = 0; oIdxTwoRight = 0;
		oLineOne = oItOne->second; oLineTwo = oItTwo->second;

		// get top line right and left cell boundaries
		while ( (oIdxOneRight < oLineOne.size()) && (oLineOne[oIdxOneRight].ScreenX < oPoint[0]) ) // are we still in the grid (not behind its rightmost corner point of the top line)?
		{
			++oIdxOneRight;
		}
		if ( oIdxOneRight >= oLineOne.size() )            // if the point in question exceeds the rightmost corner, set both right and let corner indices to match rightmost corner...
		{
			oIdxOneLeft = oLineOne.size() - 1;
			oIdxOneRight = oIdxOneLeft;
		}
		else
		{
			if ( oIdxOneRight > 0 )
			{
				oIdxOneLeft = oIdxOneRight - 1; //... otherwise we found two enclosing corners.
			}
		}

		// get bottom line vertical cell boundaries, same procedure as above
		while ( (oIdxTwoRight < oLineTwo.size()) && (oLineTwo[oIdxTwoRight].ScreenX < oPoint[0]) )
		{
			++oIdxTwoRight;
		}
		if ( oIdxTwoRight >= oLineTwo.size() )
		{
			oIdxTwoLeft = oLineTwo.size() - 1;
			oIdxTwoRight = oIdxTwoLeft;
		}
		else
		{
			if ( oIdxTwoRight > 0 )
			{
				oIdxTwoLeft = oIdxTwoRight - 1;
			}
		}

		/* As mentioned above, our inaugural search considered min top and max bottom y coord, but the point in question might actually be above or below the cell we found if
		* for instance its y coord equals top min, but the two enclosing top line corners have higher values. In this case me need to move to the correct cell and
		* repeat the x coord search.
		*/

		// Find correct line. Do we have to move one cell up or down?
		Vec3D oTopLeft(static_cast<double>(oLineOne[oIdxOneLeft].ScreenX), static_cast<double>(oLineOne[oIdxOneLeft].ScreenY), 0.0);    // top left corner
		Vec3D oTopRight(static_cast<double>(oLineOne[oIdxOneRight].ScreenX), static_cast<double>(oLineOne[oIdxOneRight].ScreenY), 0.0); // top right corner
		LineSegment oTopLine; oTopLine.preComputeSegmentData(oTopLeft, oTopRight);
		Vec3D oTopProj{oPoint.projOntoSegment(oTopLine, true)};
		int oSubstract(2);
		if ( (static_cast<int>(oTopProj[1]) - static_cast<int>(oPoint[1])) > 0 )
		{
			std::cout << "FindCell: The point " << oPoint[1] << " is above the top line " << oTopProj[1] << "\n";
			//oPoint[1] = oTopProj[1];
			oSubstract = 1;
			auto oItLast = m_oGrid.end(); --oItLast;
			if ( (oItOne != oItTwo) && (oItTwo != oItLast) ) // move to cell below, if possible and if p_oY is not above grid
			{
				++oItOne; ++oItTwo;
			}
			else
			{
				oItOne = oItTwo;
			}
			oLineOne = oItOne->second; oLineTwo = oItTwo->second;
		}
		else
		{
			//std::cout << "The point " << oPoint[1] << " is below the top line " << oTopProj[1] << ", check bottom \n";
			//the point is below the top line, check if it is above the bottom line
			Vec3D oBotLeft{static_cast<double>(oLineTwo[oIdxTwoLeft].ScreenX), static_cast<double>(oLineTwo[oIdxTwoLeft].ScreenY), 0.0};    // top left corner
			Vec3D oBotRight{static_cast<double>(oLineTwo[oIdxTwoRight].ScreenX), static_cast<double>(oLineTwo[oIdxTwoRight].ScreenY), 0.0}; // top right corner
			LineSegment oBotLine; oBotLine.preComputeSegmentData(oBotLeft, oBotRight);
			Vec3D oBotProj{oPoint.projOntoSegment(oBotLine, true)};
			if ( (static_cast<int>(oPoint[1]) - static_cast<int>(oBotProj[1])) > 0 )
			{
				std::cout << "FindCell:The point " << oPoint[1] << " is below the bottom line " << oBotProj[1] << "\n";
				//oPoint[1] = oBotProj[1];
				oSubstract = 1;
				auto oItLast = m_oGrid.end(); --oItLast;

				if ( (oItOne != oItTwo) && (oItTwo != oItLast) ) // move to cell below, if possible and if p_oY is not above grid
				{
					++oItOne; ++oItTwo;
				}
				else
				{
					oItOne = oItTwo;
				}
				oLineOne = oItOne->second; oLineTwo = oItTwo->second;
			}
		}
		oFound -= oSubstract; // 0 if all is ok in the first run, <= 0 after second run, 1 if a second run is necessary
	} // while (oFound > 0)


	// ---------- Now we should have the correct cell. If coordinates are out of the grid, identical indices and/or iterators will identify the respective situations ----------

	sCellData result {oItOne->second, oItTwo->second, 
        oIdxOneLeft, oIdxOneRight, oIdxTwoLeft, oIdxTwoRight};

  return result;
}


void CalibrationCornerGrid::printLinesInfo() const
{

    std::cout << "\n";
    std::cout << "Horizontal lines " << "\n";
    int counter = 0;
    for ( auto & oLine : m_oHorizontalLines )
    {
        counter ++;
        if ( oLine.isValid() )
        {
            std::cout << counter << ") Inclination " << oLine.getInclinationDegrees() << "degrees,  passing through  (0," << oLine.getY(0) << ")\n";
        }
        else
        {
            std::cout << counter << ") Could not fit horizontal line" << std::endl;
        }
    }
    std::cout << "Vertical lines " << "\n";
    counter = 0;
    for ( auto & oLine : m_oVerticalLines )
    {
        counter ++;
        if (oLine.isValid())
        {
            std::cout << counter << ") Inclination " << oLine.getInclinationDegrees() << "degrees,  passing through  (" << oLine.getX(0) << ", 0)\n";
        }
        else
        {
            std::cout << counter << ") Could not fit vertical line" << std::endl;
        }
    }

    std::cout << "Average: " << factor_real_to_pix()  <<  " pix / mm " << std::endl;
}

//compute factor to convert real coordinates to screen (how many pixel per mm?)
double CalibrationCornerGrid::factor_real_to_pix() const
{
    
    auto fDist = [] (double dx, double dy)
    {
      return std::sqrt(dx*dx + dy*dy);        
    };
    
    if (m_oGrid.size() < 2)
    {
        return 0.0;
    }
    
    bool oOK = true;
    
    //compute scale for each cell
    std::vector<double> real_to_pix_rows (m_oGrid.size()-1, 0.0); 
    double sum_real_to_pix = 0.0;
    int rowIndex = 0;
    for (auto oItTopLine = m_oGrid.begin(), oItBotLine = next(oItTopLine), oItEnd = m_oGrid.end();
         oItBotLine != oItEnd && oOK;
        oItTopLine++, oItBotLine++, rowIndex++)
    {

        auto & rTransform = getTransform();

		const auto & rLineTop = oItTopLine->second;
		const auto & rLineBot = oItBotLine->second;

		if ( (rLineTop.size() < 2) || (rLineBot.size() < 2) )
		{
			oOK = false;
			//"Bad calibration grid. Each line needs at least two corners.\n");
			break;
		}

        precitec::math::tCellDataArray  oRealWorldCoordinatesLeftCell; //first in row
        precitec::math::tCellDataArray  oRealWorldCoordinatesRightCell; //last in row

        bool leftCellFound = false;
        bool rightCellFound = false;
        
		//iterate checkerboard squares (from itTopLeft)
		for ( int oIdxTopLeft = 0, oIdxTopLeftMax =rLineTop.size() - 1; oIdxTopLeft < oIdxTopLeftMax; oIdxTopLeft++ )
		{
            sCellData oLastCell(oItTopLine->second, oItBotLine->second, oIdxTopLeft, sCellData::CellMode::fixedChessboardCoordinates);
            std::tie(rightCellFound, oRealWorldCoordinatesRightCell) = oLastCell.computeRealWorldCoordinatesFromCell(rTransform);
            if (!rightCellFound)
            {
                continue;
            }
            if (rightCellFound && !leftCellFound)
            {
                leftCellFound = true;
                oRealWorldCoordinatesLeftCell = oRealWorldCoordinatesRightCell;
            }
		}; //end loop x
		if (!leftCellFound || !rightCellFound)
        {
            oOK = false;
            break;
        }
		//average diagonals on current cell row
		auto & rTopLeft = oRealWorldCoordinatesLeftCell[eCornerPosition::eTopLeft];
        auto & rBotLeft = oRealWorldCoordinatesLeftCell[eCornerPosition::eBotLeft];
		auto & rTopRight = oRealWorldCoordinatesRightCell[eCornerPosition::eTopRight];
        auto & rBotRight = oRealWorldCoordinatesRightCell[eCornerPosition::eBotRight];
        
        auto rDiagonal1 = rBotRight - rTopLeft;
        auto rDiagonal2 = rTopRight - rBotLeft;
        
        auto & rCurrent_real_to_pix = real_to_pix_rows[rowIndex];
        rCurrent_real_to_pix = 0.5 * ( fDist(rDiagonal1.ScreenX, rDiagonal1.ScreenY) / fDist(rDiagonal1.RealX, rDiagonal1.RealY) +
                                            fDist(rDiagonal2.ScreenX, rDiagonal2.ScreenY) / fDist(rDiagonal2.RealX, rDiagonal2.RealY));
        sum_real_to_pix += rCurrent_real_to_pix;

	} //end loop y
	return  rowIndex > 0 ?  sum_real_to_pix / rowIndex : 0;

}



std::tuple< bool, precitec::math::tCellDataArray > CalibrationCornerGrid::findCellData(double p_oX, double p_oY) const
{
    auto oCell = findCell(p_oX, p_oY);
    if (!oCell.isValid())
    {
        return {false, {}};
    }
    auto oResult = oCell.computeRealWorldCoordinatesFromCell(m_oCornerCorrectionTransforms);
    return oResult;
}


geo2d::Size CornerGridMap::getValidArea() const{return m_oValidArea;}
}
}
