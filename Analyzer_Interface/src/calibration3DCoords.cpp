#include "calibration3DCoords.h"

#include <fstream>

#include "common/defines.h"
#include "image/image.h"
#include "calibrationCornerGrid.h"
#include "system/tools.h"  //pwdToStr
#include "common/calibrationConfiguration.h" //toWMCalibFile
#include "coordinates/linearMagnificationModel.h"

namespace precitec
{
namespace math
{

using geo2d::Point;
using geo2d::Size;
using geo2d::Rect;
using image::BImage;
using math::clip;
using geo2d::eCoordDimension;
using geo2d::coordScreenToPlaneDouble;
using geo2d::coord2D;
using filter::LaserLine;

Calibration3DCoords::Calibration3DCoords():
m_oSensorWidth(0),
m_oSensorHeight(0),
m_oSensorModel(SensorModel::eUndefined)
{}

void Calibration3DCoords::serialize ( system::message::MessageBuffer &buffer ) const
{
    marshal(buffer, m_oSensorWidth );
    marshal(buffer, m_oSensorHeight);
    if (!usesOrientedLineCalibration())
    {
        assert(buffer.hasSpace(2 * sizeof(float) * m_oCoordsArrayX.size()));
        marshal(buffer, m_oCoordsArrayX, m_oCoordsArrayX.size());
        marshal(buffer, m_oCoordsArrayY, m_oCoordsArrayX.size());
    }
    else
    {
        marshal(buffer, m_oCoordsArrayX, 0);
        marshal(buffer, m_oCoordsArrayY, 0);
    }

    //deserialize mapTriangAngles
    marshal(buffer, int(m_oTriangAngles.size()));
    for (auto & entry: m_oTriangAngles)
    {
        marshal(buffer, int(entry.first));
        marshal(buffer, float(entry.second));
    }
    int oSensorModel = int(m_oSensorModel);
    marshal(buffer, oSensorModel );

    //deserialize m_oLinearMagnificationModels
    marshal(buffer, int(m_oLinearMagnificationModels.size()));
    for (auto & entry: m_oLinearMagnificationModels)
    {
        marshal(buffer, int(entry.first));
        marshal(buffer, entry.second);
    }
}

void Calibration3DCoords::deserialize( system::message::MessageBuffer const&buffer )
{
    deMarshal(buffer, m_oSensorWidth);
    deMarshal(buffer, m_oSensorHeight);
    deMarshal(buffer, m_oCoordsArrayX );
    deMarshal(buffer, m_oCoordsArrayY );

    m_oCoordsArrayX.resize(m_oSensorWidth * m_oSensorHeight);
    m_oCoordsArrayY.resize(m_oSensorWidth * m_oSensorHeight);
    
    int oNumAngles;
    mapTriangAngles_t tmp_mapTriangAngles;
    deMarshal(buffer, oNumAngles);
    for (int i=0; i< oNumAngles; i++)
    {
        int oKey;
        float oValue;
        deMarshal(buffer, oKey);
        deMarshal(buffer, oValue);
        tmp_mapTriangAngles[LaserLine(oKey)] = oValue;
    }
    
    int oSensorModel;
    deMarshal(buffer, oSensorModel);
    
    completeInitialization(SensorModel(oSensorModel));
    for (auto & entry: tmp_mapTriangAngles)
    {       
        setTriangulationAngle(entry.second, angleUnit::eRadians, entry.first);
    }
    //serialize m_oLinearMagnificationModels
    resetOrientedLineCalibration();
    int oNumModels;
    deMarshal(buffer, oNumModels);
    for (int i=0; i< oNumModels; i++)
    {
        int oKey;
        coordinates::LinearMagnificationModel oValue;
        deMarshal(buffer, oKey);
        deMarshal(buffer, oValue);
        setOrientedLineCalibration(LaserLine(oKey), oValue);
    }
    
}


Vec3D Calibration3DCoords::RotateY(const Vec3D& p_rInput, double p_oBeta)
{
	// Fuehrt eine Drehung um die Y-Achse aus.
	double oX = cos(p_oBeta) * p_rInput[0] + sin(p_oBeta) * p_rInput[2];
	double oZ = -sin(p_oBeta) * p_rInput[0] + cos(p_oBeta) * p_rInput[2];

	return Vec3D(oX, p_rInput[1], oZ);
}

Vec3D Calibration3DCoords::FromCalibratedToRotated(const Vec3D& p_rPointInCalibratedCoordinateSystem, double p_oBeta)
{
	// Eingabepunkt ist im kalibrierten Koordinatensystem. Die Methode berechnet den entsprechenden Punkt
	// im gedrehten Koordinatensystem (welches auf dem gedrehten Blech liegt).
	// p_oBeta bezeichnet den Anstellwinkel des Lasers um die positive Y-Achse im mathematisch positiven Drehsinn.
	return RotateY(p_rPointInCalibratedCoordinateSystem, p_oBeta);  // wird so weitergereicht
}

Vec3D  Calibration3DCoords::FromRotatedToCalibrated(const Vec3D& p_rPointInRotatedCoordinateSystem, double p_oBeta)
{
	// Eingabepunkt ist im gedrehten Koordinatensystem (welches auf dem gedrehten Blech liegt).
	// Die Methode berechnet den entsprechenden Punkt im kalibrierten Koordinatensystem.
	// Dabei handelt es sich einfach um die inverse Drehung.
	// p_oBeta bezeichnet den Anstellwinkel des Lasers um die positive Y-Achse im mathematisch positiven Drehsinn.
	return RotateY(p_rPointInRotatedCoordinateSystem, -p_oBeta);
}


bool Calibration3DCoords::convertScreenTo3D(float &p_rX3d, float &p_rY3d, float &p_rZ3d,
	const int p_oX, const int p_oY, filter::LaserLine p_oLaserLine) const
{
    // if possible, directly use the linear magnification model
    auto itModel = m_oLinearMagnificationModels.find(p_oLaserLine);
    if (itModel != m_oLinearMagnificationModels.end())
    {
        auto result = itModel->second.laserScreenCoordinatesTo3D(p_oX, p_oY);
        if (!result.first)
        {
            return false;
        }
        p_rX3d = result.second.x;
        p_rY3d = result.second.y;
        p_rZ3d = result.second.z;
        return true;
    }

	int oWidth, oHeight;
	getSensorSize(oWidth, oHeight);

	float oXplane, oYPlane;
	bool valid = getCoordinates(oXplane, oYPlane, p_oX, p_oY);

	if ( !valid )
	{
		return false;
	}

	if ( p_oLaserLine >= filter::LaserLine::NumberLaserLines )
	{
		wmLog(eWarning, "Wrong laser line requested\n");
        return false;
	}

	
	auto oTriangAngle = getTriangulationAngle(angleUnit::eRadians, p_oLaserLine);


	//We need to go from the plane coordinates to the world coordinates, and then solve the equation for the laser plane
	// -sin(alpha) y + cos(alpha) z = 0

	//In the coax case, the world coordinates x and y are equal to the plane coordinates, and the equation gives z = tg(alpha) y 
	//In the scheimpflug case , the plane it's already the coordinate plane (the laser plane condition is already satisfied), 
	// therefore it's only a matter of plane to real world transformation 

    p_rX3d = oXplane;

    //Internal plane and laser plane are the same 
    //(in the extreme case  of triangulation angle equal to zero, the second coordinate of the plane corresponds to the z coordinate of the world)
    if ( isScheimpflugCase() )
    {
        p_rY3d = oYPlane * sin(oTriangAngle);
        p_rZ3d = oYPlane * cos(oTriangAngle);
    }
    else
    {
        assert(m_oSensorModel == SensorModel::eLinearMagnification);
        //coax case
        //the internal plane and the laser plane are not coincident, but the internal plane is the horizontal one
        
        float oTan = static_cast<float>(std::tan(oTriangAngle));
        if ( oTan == 0.0 )
        {
            wmLog(eWarning, "Triangulation angle is 0 \n");
            oTan = 1.0;
        }

        p_rY3d = oYPlane;
        p_rZ3d = p_rY3d / oTan;
    }
    return true;
    
    
}

	





bool Calibration3DCoords::convertScreenToLaserPlane(float &p_rX2d, float &p_rY2d,
		const int p_oX, const int p_oY, filter::LaserLine p_oLaserLine) const
{
    // if possible, directly use the linear magnification model
    auto itModel = m_oLinearMagnificationModels.find(p_oLaserLine);
    if (itModel != m_oLinearMagnificationModels.end())
    {
        auto result = itModel->second.laserScreenCoordinatesToLaserPlane(p_oX, p_oY);
        if (!result.first)
        {
            return false;
        }
        p_rX2d = result.second.x;
        p_rY2d = result.second.y;
        return true;
    }
	//p_rX2d, p_rY2d are in mm, but on the laser plane (z is not defined, 2d reference system)
	int oWidth, oHeight;
	getSensorSize(oWidth, oHeight);

	float oXplane, oYPlane;
	bool valid = getCoordinates(oXplane, oYPlane, p_oX, p_oY);

	if ( !valid )
	{
		return false;
	}


	if ( p_oLaserLine >= filter::LaserLine::NumberLaserLines )
	{
		wmLog(eDebug, "Wrong laser line requested\n");
	}

    p_rX2d = oXplane;
    if ( m_oSensorModel == SensorModel::eCalibrationGridOnLaserPlane )
    {
        //we are already in the laser plane
        p_rY2d = oYPlane;
    }
    else
    {	
        assert(m_oSensorModel == SensorModel::eLinearMagnification);
        auto oTriangAngle = getTriangulationAngle(angleUnit::eRadians, p_oLaserLine);
        p_rY2d = oTriangAngle == 0 ? oYPlane : (oYPlane / sin(oTriangAngle));
    }
    return true;
    

}

bool Calibration3DCoords::convertScreenToHorizontalPlane(float &p_rX2d, float &p_rY2d, const int p_oX, const int p_oY) const
{
    // if possible, directly use the linear magnification model
    if (usesOrientedLineCalibration())
    {
        auto & model = m_oLinearMagnificationModels.begin()->second;
        auto result = model.imageCoordinatesToGrayscaleImagePlane(p_oX, p_oY);
        if (!result.first)
        {
            return false;
        }
        p_rX2d = result.second.x;
        p_rY2d = result.second.y;
        return true;
    }
	float oXplane, oYPlane;
    
    //like coord2dTo3d before using the triangulation angle
	bool valid = getCoordinates(oXplane, oYPlane, p_oX, p_oY);

	if ( !valid )
	{
		return false;
	}


	switch(m_oSensorModel)
    {
        case SensorModel::eLinearMagnification:
        {                
            //we are already in the horizontal plane
            p_rX2d = oXplane;
            p_rY2d = oYPlane;
            return true;
        }
        case SensorModel::eCalibrationGridOnLaserPlane:
        {
            //approximate magnification with the one computed on an horizontal line 
            double pixel_to_mm = pixel_to_mm_OnHorizontalPlane( 100, p_oX, p_oY);
            p_rX2d = oXplane;
            p_rY2d = pixel_to_mm * p_oY; //TODO use ycc (but usually we don't use the absolute value of a coordinate, only the distance beetween 2 coordinates)
            return true;
        }
        case SensorModel::eUndefined:
        case SensorModel::eNumSensorModels:
        {
            assert(false && "invalid sensor model");
            return false;
        }
    }
    assert(false && "not all switch cases handled");
    return false;
    
}

//euclidean dist sqrt(x^1+y^2+z^2)
double Calibration3DCoords::dist(const double &p_oX3d1, const double &p_oY3d1, const double &p_oZ3d1,
	const double &p_oX3d2, const double &p_oY3d2, const double &p_oZ3d2) const
{
	double oDX(p_oX3d1 - p_oX3d2);
	double oDY(p_oY3d1 - p_oY3d2);
	double oDZ(p_oZ3d1 - p_oZ3d2);

	return std::sqrt((oDX * oDX) + (oDY * oDY) + (oDZ * oDZ));
}


double Calibration3DCoords::distFrom2D(const int p_oX1, const int p_oY1, const int p_oX2, const int p_oY2) const
{
    //back-compatibility
	Vec3D oP1 = to3D(static_cast<int>(p_oX1), static_cast<int>(p_oY1), filter::LaserLine::FrontLaserLine);
	Vec3D oP2 = to3D(static_cast<int>(p_oX2), static_cast<int>(p_oY2), filter::LaserLine::FrontLaserLine);

	return dist(oP1[0],oP1[1] , oP1[2], oP2[0], oP2[1], oP2[2]);
}


bool Calibration3DCoords::getCoordinates(float & x_plane, float & y_plane, const int x_pixel, const int y_pixel) const
{
	//in the coax case I need to look only at the size of the sensor, in the scheimplfug case see calibrationGrid::testposition
	int oWidth, oHeight;
	getSensorSize(oWidth, oHeight);


	if ( !(x_pixel >= 0 && x_pixel < oWidth && y_pixel >= 0 && y_pixel < oHeight) )
	{
        x_plane = 0.0;
        y_plane = 0.0;
		assert(!isValidCoord(x_plane, y_plane));
		return false;
	}

	auto index = getIndex(x_pixel, y_pixel);
    x_plane = m_oCoordsArrayX[index];
	y_plane = m_oCoordsArrayY[index];
    
	bool oValid = isValidCoord(x_plane, y_plane);
	if ( !oValid )
	{
		//check also the neighbours, if this point it's really the origin it will have 4 valid neighbors
		// (assuming not at the image edge and that  all points computed)
		if ( x_pixel > 0 && x_pixel < (oWidth - 1) && y_pixel > 0 && y_pixel < (oHeight - 1) )
		{
			bool isOrigin = true;
			for ( auto local_index : { getIndex(x_pixel - 1, y_pixel), getIndex(x_pixel + 1, y_pixel),
								getIndex(x_pixel, y_pixel-1), getIndex(x_pixel, y_pixel+1)} )
			{
				if ( !isValidCoord(m_oCoordsArrayX[local_index], m_oCoordsArrayY[local_index]))
				{
					isOrigin = false;
					break;
				}
			}
			if ( isOrigin ) //4 valid neighbours
			{
				oValid = true;
			}
		}
	}

	assert(oValid || !isValidCoord(x_plane, y_plane));
	return oValid;

}


Size Calibration3DCoords::getSensorSize() const
{
	int oWidth, oHeight;
	getSensorSize(oWidth, oHeight);
	return Size(oWidth, oHeight);
}


void Calibration3DCoords::getSensorSize(int &p_rWidth, int &p_rHeight) const
{
    p_rWidth = m_oSensorWidth;
    p_rHeight = m_oSensorHeight;
	assert((int) m_oCoordsArrayX.size() == p_rHeight* p_rWidth); 
    assert( m_oCoordsArrayY.size() == m_oCoordsArrayX.size());
}


bool Calibration3DCoords::isValidCoord(const float & x_plane, const float & y_plane)
{
	//use getCoordinate to make sure that a point at the origin is valid
	return (!(x_plane == 0.0  && y_plane == 0.0));
}


float Calibration3DCoords::getTriangulationAngle(const angleUnit p_oAngleUnit,
	const filter::LaserLine  oLaserLine) const
{
    assert(oLaserLine < filter::LaserLine::NumberLaserLines);   
    float oAngle;
    auto search = m_oTriangAngles.find(oLaserLine);
    if (search != m_oTriangAngles.end()) 
    {
        oAngle = search->second;
    } 
    else 
    {
		oAngle = 0;
		wmLog(eError, "Unknown laser line %s for getTriangulationAngle \n", laserLineName(oLaserLine).c_str());
    }
    
	if ( p_oAngleUnit == angleUnit::eDegrees )
	{
		oAngle = (oAngle*180.0 / math::pi);
	}
	return oAngle;

}


bool Calibration3DCoords::isScheimpflugCase() const
{
    assert((m_oSensorModel == SensorModel::eCalibrationGridOnLaserPlane 
    || m_oSensorModel == SensorModel::eLinearMagnification) && "other sensor models must be considered, isScheimpflugCase is not enough");
    return m_oSensorModel == SensorModel::eCalibrationGridOnLaserPlane;
}

void Calibration3DCoords::completeInitialization (math::SensorModel pSensorModel)
{
    
    m_oSensorModel = pSensorModel;
}

size_t Calibration3DCoords::getIndex(unsigned int x_pixel, unsigned int y_pixel) const
{
    assert(m_oCoordsArrayX.size() == m_oSensorWidth * m_oSensorHeight);
    assert(m_oCoordsArrayX.size() == m_oCoordsArrayY.size());
	assert(y_pixel < m_oSensorHeight);
	assert(x_pixel < m_oSensorWidth);
    return m_oSensorWidth * y_pixel + x_pixel;
    
}


float & Calibration3DCoords::X(unsigned int x_pixel, unsigned int y_pixel)
{
    auto & r_x = m_oCoordsArrayX[getIndex(x_pixel, y_pixel)];
    return r_x;
}
float & Calibration3DCoords::Y(unsigned int x_pixel, unsigned int y_pixel)
{
    auto & r_y = m_oCoordsArrayY[getIndex(x_pixel, y_pixel)];
    return r_y;
}
    
void Calibration3DCoords::resetGridCellData(const int & pWidth, const int & pHeight, const math::SensorModel & pSensorModel)
{
    m_oSensorWidth = pWidth;
    m_oSensorHeight = pHeight;
	if ( pWidth == 0 && pHeight == 0 )
	{
		m_oCoordsArrayX.clear();
        m_oCoordsArrayY.clear();
	}
	else
	{
        auto area = pWidth * pHeight;
		m_oCoordsArrayX.assign(area, 0);
        m_oCoordsArrayY.assign(area, 0);
        
		assert(X(pWidth - 1, pHeight - 1) == 0);
		assert(Y(pWidth - 1, pHeight - 1) == 0);
	}

	assert((int) m_oCoordsArrayX.size() == pWidth * pHeight);
    assert(m_oCoordsArrayX.size() == m_oCoordsArrayY.size());
    completeInitialization(pSensorModel);
}

void Calibration3DCoords::setTriangulationAngle(const float p_oAngle, const angleUnit p_angleUnit,
	const filter::LaserLine  oLaserLine)
{

    double oAngleRad = p_angleUnit == angleUnit::eDegrees ? p_oAngle*math::pi / 180 : p_oAngle;
    
	switch ( m_oSensorModel )
	{
		case SensorModel::eCalibrationGridOnLaserPlane:
		case SensorModel::eLinearMagnification:
			if ( oLaserLine < filter::LaserLine::NumberLaserLines )
			{              
				m_oTriangAngles[oLaserLine] = oAngleRad;
			}
			else
			{
				wmLog(eError, "Unknown laser line %s for setTriangulationAngle", laserLineName(oLaserLine).c_str());
			}
			break;
        case SensorModel::eUndefined:
        case SensorModel::eNumSensorModels:
            wmLog(eError, "Unknown laser line %s for setTriangulationAngle", laserLineName(oLaserLine).c_str());
			break;
	}
}

void Calibration3DCoords::setAllTriangulationAngles(const float p_oAngle, const angleUnit p_angleUnit)
{
    for (int i = 0; i < (int)(LaserLine::NumberLaserLines); i++)
    {       
        setTriangulationAngle(p_oAngle, p_angleUnit, LaserLine(i));
    }

}


bool Calibration3DCoords::to3D(float &p_rX3d, float &p_rY3d, float &p_rZ3d,
	const int p_oX, const int p_oY, filter::LaserLine p_oLaserLine) const
{
	return convertScreenTo3D(p_rX3d, p_rY3d, p_rZ3d,
		p_oX, p_oY, p_oLaserLine);
}


Vec3D Calibration3DCoords::to3D(const int p_oX, const int p_oY, filter::LaserLine p_oLaserLine) const
{
	float oX(0), oY(0), oZ(0);
	UNUSED bool oOK = to3D(oX, oY, oZ, p_oX, p_oY, p_oLaserLine);

	return Vec3D(oX, oY, oZ); // remains unchanged if oOK == false
}

Vec3D Calibration3DCoords::to3D(const Vec3D &p_rCoord2D, filter::LaserLine p_oLaserLine) const
{
	float oX(0), oY(0), oZ(0);
    to3D(oX, oY, oZ, static_cast<int>(p_rCoord2D[0]), static_cast<int>(p_rCoord2D[1]), p_oLaserLine);

	return Vec3D(oX, oY, oZ);  // remains unchanged if oOK == false
}

void Calibration3DCoords::coordsToCheckerBoardImage(
	BImage & pImage,
	const Point & pRoiOrigin, const Size & pRoiSize,
	const float & pSquareSide,
	const Calibration3DCoords::CoordinatePlaneMode mode
	) const
{
	pImage.resizeFill(pRoiSize, 123);
	assert(pImage.width() == pRoiSize.width);
	assert(pImage.height() == pRoiSize.height);
	//to facilitate counting, I show a grey line every greyIndex-th coordinate (circa every 10 mm)
	const int greyIndex(static_cast<int>(std::ceil(10 / pSquareSide)));

	std::function<bool(float &, float &, const int &, const int &)>  fConvertCoordinate; //get coordinates in mm from pixelposition

	switch ( mode )
	{
		default:
			wmLog(eWarning, "Compute Checkerboard unknown modality, default to 0\n");
			assert(false); 
			//FALLTHROUGH
		case CoordinatePlaneMode::LineLaser1Plane:
			fConvertCoordinate = [this] (float & Xmm, float & Ymm, const int & i, const int &j)
			{
				bool validCoord = convertScreenToLaserPlane(Xmm, Ymm, i, j, filter::LaserLine::FrontLaserLine);
				return validCoord;
			};
			break;
		case CoordinatePlaneMode::LineLaser2Plane:
			fConvertCoordinate = [this] (float & Xmm, float & Ymm, const int & i, const int &j)
			{
				bool validCoord = convertScreenToLaserPlane(Xmm, Ymm, i, j, filter::LaserLine::BehindLaserLine);
				return validCoord;
			};
			break;
		case CoordinatePlaneMode::LineLaser3Plane:
			fConvertCoordinate = [this] (float & Xmm, float & Ymm, const int & i, const int &j)
			{
				bool validCoord = convertScreenToLaserPlane(Xmm, Ymm, i, j, filter::LaserLine::CenterLaserLine);
				return validCoord;
			};
			break;
		case CoordinatePlaneMode::XYPlane:  
			fConvertCoordinate = [this] (float & Xmm, float & Ymm, const int & i, const int &j) 
			{
				float Zmm;
				bool validCoord = convertScreenTo3D(Xmm, Ymm, Zmm, i, j, filter::LaserLine::FrontLaserLine);
				return validCoord;
			};
			break;
		case CoordinatePlaneMode::InternalPlane:
			fConvertCoordinate = [this] (float & Xmm, float & Ymm, const int & i, const int &j) 
			{
				//get representation on internal plane, indipendently from the meaning
				getCoordinates(Xmm, Ymm, i, j);
				return true;
			};
			break;
	}

	for ( int x = 0; x < pImage.width(); x++ )
	{
		for ( int y = 0; y < pImage.height(); y++ )
		{
			float oCoordImg[2] = {0, 0};
			int i = 0, j = 0;
			byte pixel = 0;
			auto actualX = x + pRoiOrigin.x;
			auto actualY = y + pRoiOrigin.y;
			bool validCoord = fConvertCoordinate(oCoordImg[0], oCoordImg[1], actualX, actualY);
			if ( validCoord )
			{
				//coordinate of virtual square
				i = static_cast<int> (std::abs(std::floor(oCoordImg[0] / pSquareSide)));
				j = static_cast <int> (std::abs(std::floor(oCoordImg[1] / pSquareSide)));

				byte offset = 0;
				if ( i % greyIndex == 0 || j % greyIndex == 0 )
				{
					offset = 70;
				}
				//in a checkerboard, a square is black when both coordinates are even or both are odd
				if ( (i % 2) == (j % 2) )
				{
					pixel = 0 + offset;
				}
				else
				{
					pixel = 255 - offset;
				}
				pImage.setValue(x, y, pixel);
			}
		}

	}

} 


template<int t_coord>
void Calibration3DCoords::coordsToMagnification(image::BImage & rImage,
		 const geo2d::Point & pRoiOrigin, const geo2d::Size & pRoiSize,
		 int p_oRadius) const
{
    int oSensorWidth, oSensorHeight;
	getSensorSize(oSensorWidth, oSensorHeight);
    
    
    auto fConvertCoordinate = [this] (float & Xmm, float & Ymm, const int & i, const int &j) 
    {
        //get representation on internal plane, indipendently from the meaning
        return getCoordinates(Xmm, Ymm, i, j);
    };

    std::vector<double> lengths(pRoiSize.area());
    
    for ( int y = 0, index = 0; y < pRoiSize.height; y++ )
    {
        for ( int x = 0; x < pRoiSize.width; x++, index++ )
        {
            float oCoordImg1[2] = {0, 0};
            float oCoordImg2[2] = {0, 0};
                
            auto actualX = x + pRoiOrigin.x;
            auto actualY = y + pRoiOrigin.y;
            
            auto x1 = t_coord == 0 ?  std::max(0, actualX - p_oRadius) : actualX;
            auto x2 = t_coord == 0 ?  std::min(oSensorWidth-1, actualX + p_oRadius) : actualX;
            auto y1 = t_coord == 1 ?  std::max(0, actualY - p_oRadius) : actualY;
            auto y2 = t_coord == 1 ?  std::min(oSensorHeight-1, actualY + p_oRadius) : actualY;
            auto oDistance = t_coord == 0 ? double(x2-x1) : double (y2-y1);
            
            const bool validCoord1 = fConvertCoordinate(oCoordImg1[0], oCoordImg1[1], x1, y1);
            const bool validCoord2 = fConvertCoordinate(oCoordImg2[0], oCoordImg2[1], x2, y2);

            (void)validCoord1;
            (void)validCoord2;
            assert(validCoord1 && validCoord2 && (oDistance>0));
            assert(index == y *  pRoiSize.width + x);
            
            lengths[index] = dist(oCoordImg1[0], oCoordImg1[1], 0.0, oCoordImg2[0], oCoordImg2[1], 0)/std::abs(oDistance);
            assert(lengths[index] > 0);
            
        }
    }
    
    auto bounds = std::minmax_element(lengths.begin(), lengths.end());
    auto oRange = *bounds.second - *bounds.first;
    auto oMin = *bounds.first;
    std::cout << "magnification range: " << *bounds.first << " " << *bounds.second << std::endl;
    auto fTransform = [&oMin, &oRange] (double length) -> byte
    {
      length = std::round(length*1e6) * 1e-6; //rounding
      return static_cast<byte>( (length - oMin) * 255 / oRange) ;
    };
    
    
    rImage.resizeFill(pRoiSize, 123);
	assert(rImage.width() == pRoiSize.width);
	assert(rImage.height() == pRoiSize.height);
    
    BImage::transform(lengths.begin(), lengths.end(), rImage, fTransform);
            
            
}

void Calibration3DCoords::coordsToTable(const std::string & pFilenamePrefix, const geo2d::Point & pRoiOrigin, const geo2d::Size & pRoiSize) const
{
    using precitec::coordinates::CalibrationConfiguration;
    
	const auto oSensorSize = getSensorSize();
	if ( oSensorSize.area() == 0 )
	{
		wmLog(eError, "Requested empty grid in coordsToTable\n");
		return;
	}

	if ( pRoiSize.width > oSensorSize.width || pRoiSize.height > oSensorSize.height )
	{
		wmLog(eError, "Range out of index \n");
		return;
	}

	static int oCnt = 0; //used to write a new file every time this function is called

	std::stringstream oFilename("");
	oFilename << "screen" << pFilenamePrefix << oCnt << ".txt";
	wmLog(eInfo, "Printing 3D grid to  %s \n", oFilename.str().c_str());

	std::ofstream oF(CalibrationConfiguration::toWMCalibFile(oFilename.str(), system::wmBaseDir()));

	oF << std::fixed << std::setprecision(4);
	oF << " Range: x " << pRoiOrigin.x << " " << pRoiOrigin.x + pRoiSize.width
		<< " y " << pRoiOrigin.y << " " << pRoiOrigin.y + pRoiSize.height << "\n";

	for ( auto j = pRoiOrigin.y, jMaxLimit = pRoiOrigin.y + pRoiSize.height; j < jMaxLimit; ++j )
	{
		for ( auto i = pRoiOrigin.x, iMaxLimit = pRoiOrigin.x + pRoiSize.width; i < iMaxLimit; ++i )
		{
			//getCoordinates would also check if the coordinate is valid, here I just dump the content of the internal array
			auto ind = getIndex(i,j);
			oF << m_oCoordsArrayX[ind]<< "," << m_oCoordsArrayY[ind] << ";";
		}
		oF << std::endl;
	}

	++oCnt;
	oF.close();
}

std::vector<geo2d::coord2D> Calibration3DCoords::checkDiscontinuities(bool invalidCoordsAllowed,
	bool notContigousValidCoordinatesAllowed) const
{
	std::vector<coord2D> oDiscontinuityPoints;
	auto oSensorSize = getSensorSize();
	std::vector<float> lastColumn_X(oSensorSize.width, NAN);
	
	for ( int i = 0; i < oSensorSize.width; ++i )
	{
		std::vector<float> curColumn_X(oSensorSize.width, NAN);
		float prev_upY(NAN);
		
		int firstValidj = oSensorSize.height;
		int lastValidj = oSensorSize.height;
		
		for ( int j = 0; j < oSensorSize.height; ++j )
		{
			float x, y;
			bool validCoord = getCoordinates(x, y, i, j);
			bool isDiscontinuityPoint = false;
			if ( validCoord )
			{
				if ( j < firstValidj )
				{
					//this is the first valid coordinate in the current column
					firstValidj = j;
				}
				else
				{
					//check if already exited the valid coordinates fields
					if ( j> lastValidj && !notContigousValidCoordinatesAllowed )
					{
						std::cout << "Discontinuity because of valid coord outside of valid coord field [" << i << " " << j << "]  " <<
							"contiguous valid coords between [" << i << " " << firstValidj << "] and [" << i << " " << lastValidj << "] "<< std::endl;
						isDiscontinuityPoint = true;
					}
				}

				curColumn_X[j] = x;
				//evaluate discontinuities
				float leftX = lastColumn_X[j];
				if ( !std::isnan(leftX) )
				{
					auto index_left = getIndex(i-1,j);
					assert(leftX == m_oCoordsArrayX [index_left]);
					if ( leftX > x )
					{
						auto leftY =m_oCoordsArrayY [index_left];
						std::cout << "Discontinuity in horizontal direction [" << i - 1 << " " << j << "] = " << leftX << " " << leftY
							<< "  [" << i << " " << j << "] = " << x << " " << y
							<< std::endl;
						isDiscontinuityPoint = true;
					}
				}
				if ( !std::isnan(prev_upY) )
				{
					auto index_up = getIndex(i, j-1);
					assert(prev_upY == m_oCoordsArrayY[index_up]);
					if ( prev_upY < y )
					{
						auto prev_upX = m_oCoordsArrayX[index_up];
						std::cout << "Discontinuity in vertical direction [" << i << " " << j - 1 << "] = " << prev_upX << " " << prev_upY
							<< "  [" << i << " " << j << "] = " << x << " " << y
							<< std::endl;
						isDiscontinuityPoint = true;
					}
				}
			}
			else
			{
				if ( !invalidCoordsAllowed )
				{
					std::cout << "Discontinuity because of invalid coord [" << i << " " << j  << "]  " << std::endl;
					isDiscontinuityPoint = true;
				}

				if ( j > firstValidj  //we already entered the valid field
					&& j < lastValidj ) // and we havent updated yet lastValidj
				{
					lastValidj = j-1;
				}

			}
			//update the values for next loop
			prev_upY = validCoord? y: NAN;
			if ( isDiscontinuityPoint )
			{
				oDiscontinuityPoints.push_back(coord2D(i, j));
			}

		} //end iteration along row

		//update the values for next loop
		lastColumn_X = std::move(curColumn_X);
	}//end iteration along row

	return oDiscontinuityPoints;
}

double Calibration3DCoords::factorHorizontal(int pLength, int pCenterX, int pCenterY) const
{
    pLength = std::min(std::max(std::abs(pLength),10), int(m_oSensorWidth) - 1);
    int oLength1 = pLength / 2;
	int oLength2 = pLength - oLength1;

	auto x0 = std::min(std::max(0, pCenterX - oLength1), int(m_oSensorWidth) - pLength-1) ;
	auto x1 = std::min(std::max(0, pCenterX + oLength2), int(m_oSensorWidth) - 1);

	auto oDist = distFrom2D(x0, pCenterY, x1, pCenterY);
    auto oFact = std::abs(oDist) > math::eps ? ((x1-x0) / oDist) : 1;
    return oFact;
}

double Calibration3DCoords::factorVertical(int pLength, int pCenterX, int pCenterY, filter::LaserLine p_oLaserLine) const
{
    pLength = std::min(std::max(std::abs(pLength),10), int(m_oSensorHeight) - 1);
    int oLength1 = pLength / 2;
    int oLength2 = pLength - oLength1;

	auto y0 = std::min(std::max(0, pCenterY - oLength1), int(m_oSensorHeight) - pLength -1);
	auto y1 = std::min(std::max(0, pCenterY + oLength2), int(m_oSensorHeight) - 1);

    //reimplementation of distFrom2D(pCenterX, y0, pCenterX, y1), specifying the laser plane
    Vec3D oP1 = to3D(static_cast<int>(pCenterX), static_cast<int>(y0), p_oLaserLine);
    Vec3D oP2 = to3D(static_cast<int>(pCenterX), static_cast<int>(y1), p_oLaserLine);
    double oDist = dist(oP1[0], oP1[1], oP1[2], oP2[0], oP2[1], oP2[2]);
    auto oFact = std::abs(oDist) > math::eps ? ((y1-y0) / oDist) : 1;
    return oFact;
}

double Calibration3DCoords::factorVertical_Z(int pLength, int pCenterX, int pCenterY, filter::LaserLine p_oLaserLine) const
{
    pLength = std::min(std::max(std::abs(pLength),10), int(m_oSensorHeight) - 1);
    int oLength1 = pLength / 2;
    int oLength2 = pLength - oLength1;

	auto y0 = std::min(std::max(0, pCenterY - oLength1), int(m_oSensorHeight) - pLength -1);
	auto y1 = std::min(std::max(0, pCenterY + oLength2), int(m_oSensorHeight) - 1);

    //reimplementation of distFrom2D(pCenterX, y0, pCenterX, y1), specifying the laser plane
    Vec3D oP1 = to3D(static_cast<int>(pCenterX), static_cast<int>(y0), p_oLaserLine);
    Vec3D oP2 = to3D(static_cast<int>(pCenterX), static_cast<int>(y1), p_oLaserLine);
    double oDist = dist(oP1[0], 0, oP1[2], oP2[0], 0, oP2[2]);
    auto oFact = std::abs(oDist) > math::eps ? ((y1-y0) / oDist) : 1;
    return oFact;
}
//used only in calibrationResult, in Scheimpflug case
double Calibration3DCoords::distanceOnInternalPlane(const int & p_Xpix1, const int & p_Ypix1, const int & p_Xpix2, const int & p_Ypix2) const
{
    float oX1, oY1, oX2, oY2;
    bool valid = getCoordinates(oX1, oY1, p_Xpix1, p_Ypix1);
    valid &= getCoordinates(oX2, oY2, p_Xpix2, p_Ypix2);
    if ( !valid )
    {
        return 0;
    }
    double dX = (oX2 - oX1);
    double dY = (oY2 - oY1);
    return sqrt(dX*dX + dY*dY);
}

double Calibration3DCoords::distanceOnHorizontalPlane(int x0, int y0, int x1, int y1) const
{
    switch(m_oSensorModel)
    {
        case SensorModel::eLinearMagnification:
        {
            if (usesOrientedLineCalibration())
            {
                auto & model = m_oLinearMagnificationModels.begin()->second;
                auto result0 = model.imageCoordinatesToGrayscaleImagePlane(x0,y0);
                auto result1 = model.imageCoordinatesToGrayscaleImagePlane(x1,y1);
                assert(result0.first && result1.first);
                float oDX = result1.second.x - result0.second.x;
                float oDY = result1.second.y - result0.second.y;
                return std::sqrt((oDX * oDX) + (oDY * oDY));
            }
            //like coord2dTo3d before using the triangulation angle
            float X0, Y0, X1, Y1;
            bool valid = getCoordinates(X0,Y0, x0, y0);
            valid = valid && getCoordinates(X1, Y1, x1, y1);
            assert(valid);

            float oDX = X1 - X0;
            float oDY = Y1 - Y0;
            return std::sqrt((oDX * oDX) + (oDY * oDY));
        }
        case SensorModel::eCalibrationGridOnLaserPlane:
        {
            //approximate magnification with the one computed on an horizontal line 
            double pixel_to_mm = pixel_to_mm_OnHorizontalPlane( std::abs(x1-x0)+2, (x0+x1)/2, (y0+y1)/2);
            double dx = std::abs(x1-x0);
            double dy = std::abs(y1-y0);
            return pixel_to_mm * std::sqrt((dx * dx) + (dy * dy));
        }
        case SensorModel::eUndefined:
        case SensorModel::eNumSensorModels:
        {
            assert(false && "invalid sensor model");
            return -1000.0;
        }
    }
    assert(false && "missing cases in switch");
    return - 1000.0;
}

double Calibration3DCoords::pixel_to_mm_OnHorizontalPlane(int pLength, int pCenterX, int pCenterY) const
{
    pLength = std::min(std::max(std::abs(pLength),10), int(m_oSensorHeight) - 1);
    int oLength1 = pLength / 2;
    int oLength2 = pLength - oLength1;

    auto x0 = std::min(std::max(0, pCenterX - oLength1), int(m_oSensorWidth) - pLength-1);
    auto x1 = std::min(std::max(0, pCenterX + oLength2), int(m_oSensorWidth) - 1);

    double oDist; 
    if (usesOrientedLineCalibration())
    {
        oDist = distanceOnHorizontalPlane(x0, pCenterY, x1, pCenterY);
    }
    else
    {
            //like coord2dTo3d before using the triangulation angle
        float X0, Y0, X1, Y1;
        bool valid = getCoordinates(X0, Y0, x0, pCenterY);
        valid = valid && getCoordinates(X1, Y1, x1, pCenterY);
        assert(valid);

        float oDX = X1 - X0;
        float oDY = Y1 - Y0;
        oDist =  std::sqrt((oDX * oDX) + (oDY * oDY));
        
    }
    double pixel_to_mm = oDist / double(x1 - x0);

    return pixel_to_mm;

}



void Calibration3DCoords::coordsToMagnificationX(image::BImage& pImage, const geo2d::Point& pRoiOrigin, const geo2d::Size& pRoiSize, int p_oRadius) const
{
    return  coordsToMagnification<0>(pImage, pRoiOrigin, pRoiSize, p_oRadius);
}


void Calibration3DCoords::coordsToMagnificationY(image::BImage& pImage, const geo2d::Point& pRoiOrigin, const geo2d::Size& pRoiSize, int p_oRadius) const
{
    return  coordsToMagnification<1>(pImage, pRoiOrigin, pRoiSize, p_oRadius);
}


std::vector< precitec::geo2d::DPoint > Calibration3DCoords::distanceTCPmmToSensorCoordCoax(const std::vector< precitec::geo2d::DPoint >& contour_mm, double tcpSensorX, double tcpSensorY) const
{
    if (isScheimpflugCase())
    {
        wmLog(eWarning, "Not implemented \n");
        return {};
    }
    std::vector<geo2d::DPoint> contour_pix;
    contour_pix.reserve(contour_mm.size());

    // if possible, directly use the linear magnification model
    if (usesOrientedLineCalibration())
    {
        auto & model = m_oLinearMagnificationModels.begin()->second;
        geo2d::DPoint tcp_sensorPix{tcpSensorX, tcpSensorY};
        for (auto & rdist_mm : contour_mm)
        {
            auto result = model.distanceTCPmmToSensorCoord({static_cast<float>(rdist_mm.x), static_cast<float>(rdist_mm.y)}, tcp_sensorPix);
            if (!result.first)
            {
                return {};
            }
            contour_pix.push_back(result.second);
#ifndef NDEBUG
            {
                //FIXME remove debug
                auto pos_pix = result.second;
                auto tcp_mm = model.imageCoordinatesToGrayscaleImagePlane(tcp_sensorPix);
                auto pos_mm = model.imageCoordinatesToGrayscaleImagePlane(pos_pix);
                assert(tcp_mm.first && pos_mm.first);
                auto computedDistance = geo2d::DPoint(pos_mm.second.x, pos_mm.second.y) - geo2d::DPoint(tcp_mm.second.x, tcp_mm.second.y);
                assert(math::isClose(computedDistance.x, rdist_mm.x, 1e-5));
                assert(math::isClose(computedDistance.y, rdist_mm.y, 1e-5));
            }
#endif
        }
        return contour_pix;
    }

    geo2d::DPoint coordsTCP_mm;
    {
        geo2d::TPoint<float> coordsTCP_mm_float;
        bool ok = convertScreenToHorizontalPlane(coordsTCP_mm_float.x, coordsTCP_mm_float.y, (int)std::round(tcpSensorX), (int)std::round(tcpSensorY));
        if (!ok)
        {
            wmLog(eWarning, "TCP %f %f is not a valid point (probably outside the sensor size) \n",tcpSensorX, tcpSensorY);
            return {};
        }
        coordsTCP_mm = geo2d::DPoint{ (double) coordsTCP_mm_float.x, (double) coordsTCP_mm_float.y};
    }
    
    //this could be computed with CoaxCalibrationData
    double scale_x, scale_y;
    {
        auto oSensorSize = getSensorSize();
        geo2d::Point p1 {(int) tcpSensorX, (int) tcpSensorY};

        int maxDeltaPositive = std::min(oSensorSize.width - p1.x, oSensorSize.height - p1.y);
        int maxDeltaNegative = std::max(p1.x, p1.y);
        int delta = (maxDeltaPositive > 100) ? 100 :
                    (maxDeltaNegative > 100) ? - 100 :
                    (maxDeltaPositive > maxDeltaNegative) ? maxDeltaPositive : -maxDeltaNegative;
        geo2d::Point p2 {p1.x + delta, p1.y + delta};
        geo2d::TPoint<float> p1_mm;
        geo2d::TPoint<float> p2_mm;

        bool ok = convertScreenToHorizontalPlane(p1_mm.x, p1_mm.y, p1.x, p1.y);
        ok &= convertScreenToHorizontalPlane(p2_mm.x, p2_mm.y, p2.x, p2.y);

        scale_x = double(p2_mm.x - p1_mm.x) / double(p2.x - p1.x);
        scale_y = double(p2_mm.y - p1_mm.y) / double(p2.y - p1.y);

    }

    
    for (auto & rdist_mm : contour_mm)
    {
        geo2d::DPoint dist_pix{rdist_mm.x / scale_x, rdist_mm.y / scale_y }; 
        geo2d::DPoint point_pix { dist_pix.x + tcpSensorX, dist_pix.y + tcpSensorY};
        contour_pix.push_back(point_pix);
        
#ifndef NDEBUG        
        //check if the found point sits really at the expected distance from TCP, accounting for rounding errors
        
        auto coordsPoint_mm = rdist_mm + coordsTCP_mm; 
        
        geo2d::TPoint<float> approxpos_mm_float;
        bool valid = convertScreenToHorizontalPlane( approxpos_mm_float.x, approxpos_mm_float.y, std::round(point_pix.x), std::round(point_pix.y));
        if (!valid )
        {
            wmLog(eError, "Could not get actual point coordinates for %f %f \n", point_pix.x, point_pix.y);
        }
        geo2d::DPoint approxpos_mm {(double)approxpos_mm_float.x, (double)approxpos_mm_float.y};
        
        
        geo2d::TPoint<float> floor_mm, ceil_mm;
        valid = convertScreenToHorizontalPlane( floor_mm.x,floor_mm.y, std::floor(point_pix.x -0.5), std::floor(point_pix.y -0.5));
        valid &= convertScreenToHorizontalPlane( ceil_mm.x,ceil_mm.y, std::ceil(point_pix.x + 0.5), std::ceil(point_pix.y + 0.5));
   
        if( valid )
        {
            bool ok = true;
            if ( floor_mm.x < ceil_mm.x)
            {
                ok &= ((( floor_mm.x - 1e-6) < coordsPoint_mm.x) && (coordsPoint_mm.x < ( ceil_mm.x + 1e-6)));
            }
            else
            {
                ok &= ((( floor_mm.x + 1e-6) > coordsPoint_mm.x) && ( coordsPoint_mm.x > ceil_mm.x  - 1e-6));            
            }

            if ( floor_mm.y < ceil_mm.y)
            {
                ok &= ((( floor_mm.y  - 1e-6) < coordsPoint_mm.y) && ( coordsPoint_mm.y < ( ceil_mm.y + 1e-6)));
            }
            else
            {
                ok &= ((( floor_mm.y + 1e-6) > coordsPoint_mm.y) && (coordsPoint_mm.y > ( ceil_mm.y  - 1e-6 )));
            }
            if (!ok)
            {
                wmLog(eError, "Error computing distance to TCP %f %f pix (%f %f mm) \n", tcpSensorX, tcpSensorY, coordsTCP_mm.x, coordsTCP_mm.y);

                std::ostringstream oMsg;
                oMsg <<  "Error with Input: TCP [" << tcpSensorX << "," << tcpSensorY << "]  dist mm ("<< rdist_mm.x << "," << rdist_mm.y <<") ->"
                << "dist_pix " << dist_pix << " out pix " << point_pix << " in mm "  << floor_mm << " " << approxpos_mm << " " << ceil_mm 
                << " actual dist " << approxpos_mm - coordsTCP_mm << "\n";
                wmLog(eDebug, oMsg.str());
            }
        }
        else
        {
            std::ostringstream oMsg;
            oMsg << "Not valid point position " << rdist_mm << " + " << coordsTCP_mm << " -> " << point_pix.x << " " << point_pix.y << " pix" << std::endl;
            wmLog(eError, oMsg.str());
        }
#endif
    }
    return contour_pix;
}


bool Calibration3DCoords::usesOrientedLineCalibration() const
{
    return !m_oLinearMagnificationModels.empty();
}

void Calibration3DCoords::resetOrientedLineCalibration()
{
    m_oLinearMagnificationModels.clear();
}

void Calibration3DCoords::setOrientedLineCalibration(filter::LaserLine line, coordinates::LinearMagnificationModel model)
{
    m_oLinearMagnificationModels[line] = model;
}


const coordinates::LinearMagnificationModel& Calibration3DCoords::getOrientedLineCalibration ( filter::LaserLine line ) const
{
    return m_oLinearMagnificationModels.at ( line );
}


void Calibration3DCoords::adjustZPointForOrientedLaserLine(filter::LaserLine line, geo2d::DPoint pix)
{
    m_oLinearMagnificationModels[line].adjustLineToReferenceZ(pix);
}

} //namespace math
} //namespace precitec
