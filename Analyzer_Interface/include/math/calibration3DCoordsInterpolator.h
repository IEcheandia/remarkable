#ifndef _CALIBRATION3DCOORDSINTERPOLATOR_H
#define _CALIBRATION3DCOORDSINTERPOLATOR_H

#include "calibration3DCoords.h"
#include "calibrationCornerGrid.h"
#include "geo/coordinate.h"

namespace precitec {
namespace math {

class Calibration3DCoordsInterpolator
{
public:
	Calibration3DCoordsInterpolator(Calibration3DCoords & p3DCoords)
		:m_o3DCoords(p3DCoords),
		mWidth(m_o3DCoords.getSensorSize().width),
		mHeight(m_o3DCoords.getSensorSize().height)
	{
		m_oTimesComputed.resize(mWidth*mHeight, 0);
	};

    bool allCellsTo3D(const CornerGridMap & pCornerGrid, bool pExtrapolate, bool pRectify);

	void resetGridCellData(const int & pWidth, const int & pHeight);
	std::vector<unsigned int > m_oTimesComputed; ///< Field telling us how many timed 3D coords for screen coord x, y and sensor 0..2 have been computed. Needed internally.

    static geo2d::coordScreenToPlaneDouble cellTo3D(const tCellDataArray & rChessBoardCoordinates , double screenX, double screenY);

    static bool applyTranslationToSetOrigin(CalibrationCornerGrid & pCornerGridToUpdate, double xScreenOrigin, double yScreenOrigin, double expected_xReal = 0.0, double expected_yReal = 0.0);
    
	/**
	* Interpolates 3D data for all 2D screen pixels in a cell, if the cell is not too deformed, i.e. left and right cell bound length differences are not too big.
	*/
	bool cellTo3D(const tCellDataArray & rCellDataArray);
	/**
	* If, during computation, any pixel's 3D data has been computed multiple times, the result will be normalized here via arithmetic avg.
	* In details: All coordinate computations are simply summed up, and in this function will be divided by the number of computations.
	*/
	//used only in allCellsT3D
	bool normalize3DData();

    
	/**
	* Determine, if possible, 3D data that could not be computed in callCellsTo3D, using getValueByNeighbours for potential interpolations.
	*/
	bool computeMissingGridPoints();
	/**
	* If a pixel's 3D data could not be computed during computation but has valid neighbours (left and right  OR/AND  bottom and top), we interpolate it in this method.
	* Called by computeMissingGridPoints.
	*/
	bool getValueByNeighbours(const unsigned int p_oX, const unsigned int p_oY);
	//utility function to get times computed (no range checking)
	unsigned int getTC(unsigned int x, unsigned int y) const;

private:
	Calibration3DCoords &  m_o3DCoords; 
	size_t mWidth;
	size_t mHeight;

};

}
}
#endif
