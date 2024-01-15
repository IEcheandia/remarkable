#ifndef __CALIBRATIONCORNERGRID
#define __CALIBRATIONCORNERGRID

#include "calibrationCommon.h"
#include "util/camGridData.h"
#include "math/3D/projectiveMathStructures.h"
#include "math/2D/LineEquation.h"
#include "cellData.h"
#include "geo/size.h"

namespace precitec
{
namespace math
{

//forward declaration
class Calibration3DCoords;
class CornerGridMap;

//generic container for  landmark points in a reference system (mm) and in the screen system (pixel)
//assumes only "grid" like distribution, but does not try to fit lines along corners
class CornerGridMap
{
public:
    virtual void reset(geo2d::Size p_oValidArea);
    void addRow(int yAverage, int yMin, int yMax, system::t_coordScreenToPlaneLine coordinates);
    
    void setTransform(RealWorldTransform p_Transform);
    void applyTransform();
    
    
	//getters and format functions
	geo2d::Size getValidArea() const;
    bool isEmpty() const;
	void printGrid(std::ostream & out = std::cout) const;
	const system::tGridMap2D3D& getGrid2D3D() const;
	const RealWorldTransform& getTransform() const;
    const system::tLineBoundaries& getLineBoundaries() const;
	system::tGridMap getGrid2D() const;
	std::vector< geo2d::coord2DScreen > getAllNodesScreen() const ;  //for wmCalibration
    std::vector< std::array<geo2d::coord2DScreen,2> > getAllSegmentsScreen() const;  //for wmCalibration

protected:
	//corner grid is described by m_oGrid, m_oLineBoundaries, m_oTransform
	system::tGridMap2D3D m_oGrid;               ///< Computed 3D Grid, also including 2D point data from pattern. 
	system::tLineBoundaries m_oLineBoundaries;  ///
	RealWorldTransform m_oCornerCorrectionTransforms;
	geo2d::Size m_oValidArea;
    
};

 //subset of correspondneces between mm <-> pixel (at regular pixel positions for ease of interpolation -  transforms)

class UniformGridMap : public CornerGridMap
{  
    friend class UniformGridMapBuilder;
    
};


/*
 * Function that represents the corners of the chessboard pattern used for calibration.
 * The internal variable m_oGrid is compatbile with the representation used in CamGridData
 * Plausiblity functions as fitted lines
 * Navigation function (to be used during interpolation)
*/

class CalibrationCornerGrid : public CornerGridMap
{
	friend void loadCamGridDataToCornerGrid(CalibrationCornerGrid & rGrid, const system::CamGridData & p_CamGridData, bool linearize);

public:
	CalibrationCornerGrid(geo2d::Size p_oValidArea);
	void reset(geo2d::Size p_oValidArea) override;
	//populate a CalibrationCornerGridInstance from CamGridData
	static bool computeCornerData(CalibrationCornerGrid &rGrid, const system::CamGridData & p_CamGridData, bool linearize);
	static bool computeCornerData(CalibrationCornerGrid &rGrid, const system::tGridMap & rGridMap, const int oMaxXDelta, bool linearize);


	//process points
	system::t_coordScreenToPlaneLine alignPoints(double threshold); //return the original values of the corrected points, whose correction was greater then threshold
	bool getLinearizedCornerPosition(const int ChessboardX, const int ChessboardY, double & ScreenX, double & ScreenY) const;

	const LineEquation & getVerticalLineEquation(unsigned int ChessboardX) const;
	const LineEquation & getHorizontalLineEquation(unsigned int ChessboardY) const;
	std::vector<LineEquation> getAllLines() const;
    void printLinesInfo() const;
    
    double factor_real_to_pix() const;
    std::tuple<bool,tCellDataArray> findCellData(double p_oX, double p_oY) const;

private:

	/*
	* Fnds the cell enclosing point (p_oX, p_oY) for a Scheimpflug system and returns the data in p_rCellData. Currently used for finding the reference cell enclosing the origin.
	*/
	sCellData findCell(double p_oX, double p_oY) const;
	bool fitLines();
    

	unsigned int m_oNumberOfChessboardColumns;
	unsigned int m_oNumberOfChessboardRows;
	std::vector< LineEquation > m_oHorizontalLines; // equations of the lines that fit the corner position along a chessboard row
	std::vector< LineEquation > m_oVerticalLines;  // equations of the lines that fit the corner position along a chessboard column
	LineEquation m_oInvalidLine; //used to always return a valid reference in getHorizontalLineEquation and  getVerticalLineEquation
	bool m_oPrintLines = false;
};

}
}

#endif
