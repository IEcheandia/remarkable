#include "math/Calibration3DCoordsLoader.h"
#include "math/calibration3DCoords.h"
#include "math/calibrationCornerGrid.h"
#include "math/calibration3DCoordsInterpolator.h"



namespace precitec {
namespace math {
    
using precitec::geo2d::coordScreenToPlaneDouble;


//helper functions to initialize for calibration3DCoords

void loadCamGridDataToCornerGrid(CalibrationCornerGrid & rGrid, const system::CamGridData & p_CamGridData, bool linearize)
	{
	/*   given o as origin and knowing the cell it lies in (bounded by 4 corner points) as follows:
	*
	*       (oXIdxLeftStart, oLineTop)                                  (oXIdxLeftEnd, oLineTop)
	*
	*                                        o
	*
	*
	*    [(oXIdxRightStart, rLineBot)]
	*                                                                         (oXIdxRightEnd, rLineBot)
	*
	* we compute the reference corner (i.e. bottom left of origin), denoted by [(,)]. This serves as a base for computing all other real world corner coordinates later. */

	// returns false and coordinates (0, 0, 0) for invalid cell data or projection problems
	
	rGrid.m_oValidArea.width = p_CamGridData.sensorWidth();
	rGrid.m_oValidArea.height = p_CamGridData.sensorHeight();
	
	rGrid.m_oCornerCorrectionTransforms = RealWorldTransform{};
	rGrid.m_oCornerCorrectionTransforms.m_oScale = p_CamGridData.gridDelta() * p_CamGridData.correctionFactor();
	assert(rGrid.m_oCornerCorrectionTransforms.m_oScale > 0);

    
    // compute the position in mm of the chessboard corners. 
    //In this first run position 0,0 is at the upper left corner, it will be corrected by m_oCornerCorrectionTransforms
	CalibrationCornerGrid::computeCornerData(rGrid, p_CamGridData, linearize);

	const int oOrigX = p_CamGridData.sensorWidth()/2;
	const int oOrigY = p_CamGridData.sensorHeight()/2;


	//"QnxMsg.Calib.OrigInfo",
	wmLog(eDebug, "loadCamGridDataToCornerGrid: Origin is %d, %d. Looking for enclosing cell...\n", oOrigX, oOrigY);

	sCellData oCellDataOrigin = rGrid.findCell(oOrigX, oOrigY);

	// find cell enclosing origin
	if ( !oCellDataOrigin.isValid() )
	{
		//"QnxMsg.Calib.BadCalData"
		wmLog(eError, "Invalid calibration data: Origin must be enclosed by a complete cell.\n");
		assert(false);
		return;
	}

	//Compute the coordinates of the pixels enclosed in oCellDataOrigin, 
	//so that coordinates(oOriginX, oOriginY) = (0.0, 0.0)
	float oOriginXmm(0), oOriginYmm(0);
    {
		//create a temporary grid enclosing the origin, compute interpolated value
		double Xmin, Xmax, Ymin, Ymax;
		oCellDataOrigin.findMaximumPixelRange(Xmin, Xmax, Ymin, Ymax);
		Xmin = std::floor(Xmin);
		Ymin = std::floor(Ymin);
		Xmax = std::ceil(Xmax);
		Ymax = std::ceil(Ymax);

		for ( int i = 0; i < eCornerPosition::eNumCornersInCell; i++ )
		{
			std::cout << oCellDataOrigin.getCorner(eCornerPosition(i)) << " ";
		}
		std::cout << std::endl;

		{
			// compute correct real world data of cell corners, setting origin to (0,0,0)
            geo2d::Size oTmpSensorSize(Xmax - Xmin + 1, Ymax - Ymin + 1);
			coordScreenToPlaneDouble oTmpOffset(-Xmin, -Ymin, 0, 0);
			
            Calibration3DCoords oTmp;
			Calibration3DCoordsInterpolator oTmp3DCoords(oTmp);
            
			oTmp3DCoords.resetGridCellData(oTmpSensorSize.width, oTmpSensorSize.height);


			bool oOK = oTmp3DCoords.cellTo3D(
                {oTmpOffset + oCellDataOrigin.getCorner(eCornerPosition::eTopLeft),
                    oTmpOffset + oCellDataOrigin.getCorner(eCornerPosition::eTopRight),
                    oTmpOffset + oCellDataOrigin.getCorner(eCornerPosition::eBotLeft),
                    oTmpOffset + oCellDataOrigin.getCorner(eCornerPosition::eBotRight)});
            
			if ( !oOK )
			{
				std::cout << "Interpolation error at cell ";
				for ( int i = 0; i < eCornerPosition::eNumCornersInCell; i++ )
				{
					std::cout << oCellDataOrigin.getCorner(i) << " ";
				}
				std::cout << std::endl;
				return;
			}
			if ( oTmp3DCoords.getTC(oOrigX - Xmin, oOrigY - Ymin) < 1 )
			{
				//"QnxMsg.Calib.NoOrig"
				wmLog(eError, "Cannot determine Origin. Grid too deformed!\n");
				return;
			}
			oTmp3DCoords.normalize3DData();

			oTmp.getCoordinates(oOriginXmm, oOriginYmm, oOrigX - Xmin, oOrigY - Ymin);
		}
	}

	rGrid.m_oCornerCorrectionTransforms.m_oTx = -oOriginXmm;
	rGrid.m_oCornerCorrectionTransforms.m_oTy = -oOriginYmm;

	}

    

bool loadCamGridData(Calibration3DCoords & r3DCoords, const system::CamGridData & rCamGridData) //former imgtodata
{
	//load the cells from camgriddata 
	CalibrationCornerGrid oCornerGrid({rCamGridData.sensorWidth(), rCamGridData.sensorHeight()}); 
	loadCamGridDataToCornerGrid(oCornerGrid, rCamGridData, /*linearize*/ true);
	if ( oCornerGrid.getGrid2D3D().size() == 0 )
	{
		//"QnxMsg.Calib.BadGrid",
		return false;
	}

	//prepare the coordinates lookup table
	Calibration3DCoordsInterpolator oInterpolator(r3DCoords);
	oInterpolator.resetGridCellData(rCamGridData.sensorWidth(), rCamGridData.sensorHeight());
    assert(r3DCoords.getSensorSize().area() == rCamGridData.sensorWidth() * rCamGridData.sensorHeight());
	//precomputecalib3dcoords
	bool ok = oInterpolator.allCellsTo3D(oCornerGrid, /*extrapolate*/ true, /*rectify*/ true);
	if ( !ok )
	{
		//QnxMsg.Calib.BadGrid
		return false;
	}
	assert(r3DCoords.getSensorSize().area() == rCamGridData.sensorWidth() * rCamGridData.sensorHeight());
    //this angle corresponds to the key-value "scheimTriangAngle"
    //to simplify filter parameterization and measure dialog, assign the same angle to every laser line - even if physically 
    // there is only one laser line
    
    r3DCoords.setAllTriangulationAngles(rCamGridData.triangulationAngle_rad(), angleUnit::eRadians);
    

	return true;
}

    
bool loadCoaxModel(Calibration3DCoords & r3DCoords, const CoaxCalibrationData & oCoaxData, bool useOrientedLine)
{
    using filter::LaserLine;

	if ( oCoaxData.m_oBeta0 == 0 || oCoaxData.m_oDpixX == 0 || oCoaxData.m_oDpixY == 0 )
	{
		return false;
	}
    r3DCoords.resetGridCellData(oCoaxData.m_oWidth, oCoaxData.m_oHeight, SensorModel::eLinearMagnification);
    r3DCoords.resetOrientedLineCalibration();
    if (useOrientedLine)
    {
        if (oCoaxData.m_oDpixX != oCoaxData.m_oDpixY)
        {
            wmLog(eError,"Only square pixels supported \n");
            return false;
        }
        for (auto whichLine : {LaserLine::FrontLaserLine, LaserLine::CenterLaserLine,LaserLine::BehindLaserLine })
        {
            double betaZ;
            bool isHighPlaneOnImageTop;
            math::LineEquation lineXYEquation;
            oCoaxData.getLineDependentParameters(betaZ, isHighPlaneOnImageTop, lineXYEquation, whichLine);
            if (!lineXYEquation.isValid())
            {
                wmLog(eError, "LineXY equation for laser line %d is not valid (all coefficients are 0), using an horizontal line as default\n", int(whichLine));
                lineXYEquation = {0.0,1.0,0.0};
            }

            coordinates::LinearMagnificationModel oModel(oCoaxData.m_oBeta0,betaZ,
                                oCoaxData.m_oInvertX, oCoaxData.m_oInvertY, isHighPlaneOnImageTop,
                                oCoaxData.m_oDpixX,
                                geo2d::Point(oCoaxData.m_oOrigX, oCoaxData.m_oOrigY),
                                lineXYEquation);
            r3DCoords.setOrientedLineCalibration(whichLine, oModel);
           
            r3DCoords.setTriangulationAngle(oCoaxData.computeTriangulationAngle(whichLine), angleUnit::eDegrees, whichLine);
        }
        return true;
    }



	//former createCoaxGrid

	//in the coax case, this grid is an horizontal plane

    
    //what about triang angle? is it always already initialized when i call this function?
    for (auto & oLaserLine : { LaserLine::FrontLaserLine, LaserLine::CenterLaserLine, LaserLine::BehindLaserLine} )
    {
        r3DCoords.setTriangulationAngle(oCoaxData.computeTriangulationAngle(oLaserLine), angleUnit::eDegrees, oLaserLine);
 
    }

	const float oFactorX = (oCoaxData.m_oInvertX? -1.0 : 1.0) * oCoaxData.m_oDpixX / oCoaxData.m_oBeta0;
	const float oFactorY = (oCoaxData.m_oInvertY? -1.0 : 1.0) * oCoaxData.m_oDpixY / oCoaxData.m_oBeta0;
    

	//cast to signed
	int oOrigX = oCoaxData.m_oOrigX;
	int oOrigY = oCoaxData.m_oOrigY;

	for ( int y = 0, yMax = oCoaxData.m_oHeight ; y < yMax ; ++y )
	{
		for ( int x = 0, xMax = oCoaxData.m_oWidth ; x < xMax ; ++x )
	{
			r3DCoords.X(x,y) = (x - oOrigX) * oFactorX;
			r3DCoords.Y(x,y) = -(y - oOrigY) * oFactorY;
	}
}

#ifndef NDEBUG
	//equivalent to interpolating a grid with one cell
	// we will create a grid with one cell
	float oLeftRealX = -(oOrigX * oFactorX);
	float oRightRealX = (int(oCoaxData.m_oWidth) - 1 - oOrigX) * oFactorX;

	float oTopRealY = oOrigY * oFactorY;
	float oBotRealY = -(int(oCoaxData.m_oHeight) - 1 - oOrigY) * oFactorY;

	assert(r3DCoords.X(oCoaxData.m_oOrigX,oCoaxData.m_oOrigY)  == 0);
	assert(r3DCoords.Y(oCoaxData.m_oOrigX,oCoaxData.m_oOrigY)  == 0);

	assert(r3DCoords.X(0,0) == oLeftRealX);
	assert(r3DCoords.Y(0,0) == oTopRealY);

    assert(r3DCoords.X(oCoaxData.m_oWidth - 1,0) == oRightRealX);
	assert(r3DCoords.Y(oCoaxData.m_oWidth - 1,0) == oTopRealY);

    assert(r3DCoords.X(0,oCoaxData.m_oHeight-1) == oLeftRealX);
	assert(r3DCoords.Y(0,oCoaxData.m_oHeight-1) == oBotRealY);

	assert(r3DCoords.X(oCoaxData.m_oWidth - 1, oCoaxData.m_oHeight - 1) == oRightRealX);
	assert(r3DCoords.Y(oCoaxData.m_oWidth - 1, oCoaxData.m_oHeight - 1) == oBotRealY);
#endif
    
	return true;
}

}
}
