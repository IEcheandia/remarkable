/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Andreas Beschorner (BA)
 * 	@date		03/2013
 *  @brief      Click class links for respective information, more for usage examples.
 *
 *  \section Notation Notation
 *  \htmlonly
 *  Before we start with some examples, let us clarify notational aspects:<br />
 *  <table border>
 *	 <tr style="background-color&#58;#0000cc">
 *    <td><font color="white"><b> Class </b></font></td>
 *    <td><font color="white"><b> Letter(s)/ Identifier</b></font></td>
 *    <td><font color="white"><b> Description  </b></font></td>
 *   </tr><br />
 *   <tr>
 *    <td>CalibCamData</td>
 *    <td>M, N</td>
 *    <td>Class for calibration and camera data and certain operations for converting points to and from camera sensor space to real space.</td>
 *   </tr><br />
 *    <td>Vec3D</td>
 *    <td>v, w</td>
 *    <td>Class for (inhomogeneous, ordinary) 3D vectors including some functions in 3D or conversion to the projective 2D space, given calibration and camera data.</td>
 *   <tr>
 *   </tr><br />
 *    <td>Vec2DHomogeneous</td>
 *    <td>d, e</td>
 *    <td>Class for homogeneous 2D vectors in the projective 2D space including conversion to the real 3D space, given calibration and camera data.</td>
 *   <tr>
 *   </tr><br />
 *    <td>LineSegment</td>
 *    <td>l</td>
 *    <td>Class representing line segments in the real 3D space.</td>
 *   <tr>
 *  </td>
 *  </table><br />
 *  \endhtmlonly
 *
 *  \section Converting Converting vectors to and from camera sensor space / real space
 *  Naturally, for conversions to and from either space, calibration and camera data is necessary. An object representing this is given by the class precitec::math::CalibCamData and
 *  initialized either by using the appropriate CTor precitec::math::CalibCamData(system::CalibrationDataT &p_rCalData) or the setter function precitec::math::CalibCamData::setData(system::CalibrationDataT &p_rCalData).
 *
 *  \subsection C2Dto3D Converting 2D points to 3D data
 *  Given a calibration file "/tmp/calData.xml", the first step is, to read appropriate data into a precitec::math::CalibCamData object. The latter afterwards holds necessary information such as the calibration
 *  matrix \f$ M \f$ and the laser plane and is necessary for conversions from the real world onto the camera plane. Second, we will also need its inverse matrix. This is easily archieved as follows:
 *
 *  \code
 * system::CalibrationDataT calib;
 * CalibCamData oCalibCamData, oCalibCamInv;
 
 * if( calib.load("/tmp/calData.xml") )
 * {
 *    oCalibCamData.setCalibrationMatrix(calib);
 *    oCalibCamInv.invert(m_oCalibCamData);
 * }
 * \endcode
 *
 *  Now, given a screen pixel \f$ (x, y) \f$ of the laserline (keep in mind that only those points have a meaningful real world 3D representation!), we first build a homogeneous 2D vector \f$ d \f$ which
 *  afterwards is used to compute the normalized 3D vector \f$ v \f$ in the camera coordinate system, given the data from the inverse calibration matrix \f$ N=M^{-1} \f$, which is encapsulated within oCalibCamInv.
 *  In form of an equation:
 *  \f[ v = M^{-1}d \f]
 *
 * By means of a code example:
 *  \code
 * // screen coords from laser, given as integers int x, y
 * // JUST FOR DEMONSTRATION, SHOULD BE USED LIKE THIS ONLY WHEN CAMERA COORDINATES ARE NEEDED, SEE BELOW. 
 * Vec2DHomogeneous oPointToConvert(x, y);
 * Vec3D oRealPoint;
 * oPointToConvert.applyCamera(oRealPoint, oCalibCamInv); // result in oRealPoint
 *  \endcode
 *
 * This coordinate now needs further transformation (iow one should NOT perform a call like the one given above if real world 3D coords are needed --
 * the above code only computes normalized screen coordinates) from camera coordinate system to the real world. Following Hartley, pp. 70 and 156 to 157, this is achieved
 * by using information from system calibration about the laser plane, which itself is contained in objects of type precitec::math::CalibCamData::point2dTo3d
 * after setting the calibration data, see above. This is done by calling the precitec::math::CalibCamData::point2dTo3d function:
 *
 * \code
 * // 2d screen -> 3d real world
 * oCalibCamInv.point2dTo3d(oRealPoint, oPointToConvert); // result in oRealPoint
 * \endcode
 *
 *  \subsection C3Dto2D Converting 3D points to 2D screen pixel
 *  In contrast to the opposite operation, this operation gives meaningful results for all pixels within the camera focus, as there is no "aritificial" plane necessary here such as the laserline
 *  and laserplane. It is fairly simple and straightforward using the camera calibration matrix \f$ M \f$:
 *
 * \code
 * Vec2DHomogeneous oBackConversion;
 * oRealPoint.applyCamera(oBackConversion, oCalibCamData); // result in oBackConversion
 * \endcode
 *
 * Alternatively, this function can be called via using the calibration camera object
 * \code
 * oCalibCamData.point3dTo2d(oBackConversion, oRealPoint); // result in oBackConversion
 * \endcode
 *
 * \section line3D Transforming into the real world coordinate system and projecting D points onto 3D lines
 * The following example shows how to compute 3D line segment data and project real word 3D points onthogonally onto the segment.
 * Within this process, precitec::math::CalibCamData::point2dTo3d is used to completely transform 2D screen data to real world 3D data.
 *
 * oXLeft, oYLeft, oXRight and oYRight are integer, float or double values with 2D (sub)pixel screen coordinates, oCalibCamInv the
 * inverse matrix object (of type precitec::math::CalibCamData) of a camera calibration object (of type precitec::math::CalibCamData). oCoord is some arbitrary real world 3D point.
 * \code
 * // given: double oXLeft, oYLeft, oXRight, oYRight; Vec3D oCoord
 * Vec3D oStart, oEnd, oCoord, oProj;
 * LineSegment oSegment;

 * // transform start and end coordinate of segment from 2D to 3D...
 * Vec2DHomogeneous oStart2D(oXLeft, oYLeft); Vec2DHomogeneous oEnd2D(oXRight, oYRight]);
 * oCalibCamInv.point2dTo3d(oStart, oStart2D); oCalibCamInv.point2dTo3d(oEnd, oEnd2D);

 * // ...and compute segment data (direction, squared length, ...)
 * oSegment.preComputeSegmentData( oStart, oEnd );
 * // now oSegment.m_oSegment holds the difference vetor (oEnd - oStart), that is the direction vector of the segment, oSegment.m_oLen the segment's length squared.

 * // compute orthogonal projection of oCoord onto oSegment and calculate its the distance of the points
 * oProj = oCoord.projOntoSegment(oSegment);            // project onto segment in real world 3D space
 * double oDist = std::sqrt( (oCoord-oProj).norm2() );  // compute distance of projection from original point
 * \endcode
 */


#ifndef PROJECTIVEMATHSTRUCTURES_H_
#define PROJECTIVEMATHSTRUCTURES_H_

#include <array>
#include <iomanip>// wg setprecision()
#include "Analyzer_Interface.h"
#include <vector>
#include <map>
#include <iostream>
#include <cstdlib>

namespace precitec {
namespace math {

typedef std::array<double, 3> tHVecData;      ///< Data bucket typedef for homogenous 2D vectors
typedef std::array<double, 6> tCCMatrixData;  ///< Data bucket typedef for upper triangle camera calib matrices

class Vec2DHomogeneous;
class LineSegment;  // forward decl. for Vec3D


//rigid transform
struct RealWorldTransform
{
	// output = scale * ( Rotation * input + translation), but rotation not yet implemented
public:
	float m_oTx = 0.0;
	float m_oTy = 0.0;
	float m_oTz = 0.0;
	float m_oScale = 1.0;
    bool m_invertX = false;
    bool m_invertY = false;
    
	
	bool hasOnlyScale() const
	{
		for ( auto & i : {m_oTx, m_oTy, m_oTz} )
		{
			if ( i != 0 )
			{
				return false;
			}
		}
		return true;
	}

	friend std::ostream& operator<<(std::ostream& out, const RealWorldTransform& pTransform)
	{
		out << "Translation : "
			<< pTransform.m_oTx << " " << pTransform.m_oTy << " " << pTransform.m_oTz 
			<< " [mm] Scale: " << 
			pTransform.m_oScale;
		return out;
	}
};

/**
 *  @brief	Base ( inhomogeneous ) Homogeneous 2D vector class for vectors in the projective space \f$ \mathbb{P}^2 \f$.
 *
 *  This class offers base structures and functions for projective geometry as well as simple vector arithmetics.
 *
 */
class ANALYZER_INTERFACE_API Vec3D
{
public:
	Vec3D();                                            ///< CTOR with default data init
	Vec3D(const double, const double, const double);    ///< CTOR with user data init
	Vec3D(const Vec3D&);                                ///< Copy CTOR
	virtual ~Vec3D() {};                                ///< DTOR

	/// Index operator for write and read access
	double &operator[] (int p_oIdx)
	{
		const int oIdx = std::abs(p_oIdx)%3;
		return m_oEntries[oIdx];
	}

	/// Const index operator for read only access
	double const &operator[] (int p_oIdx) const
	{
		const int oIdx = std::abs(p_oIdx)%3;
		return m_oEntries[oIdx];
	}

	/// Setter copying from Vec3D. Does not create a new instance!
	inline Vec3D &operator = (const Vec3D &p_Vec)
	{
		m_oEntries[0] = p_Vec[0]; m_oEntries[1] = p_Vec[1]; m_oEntries[2] = p_Vec[2];
		return *this;
	}

	/// += operator
	template<typename T>
	inline Vec3D &operator += (const T p_oScalar)
	{
		m_oEntries[0] += p_oScalar; m_oEntries[1] += p_oScalar; m_oEntries[2] += p_oScalar;
		return *this;
	}

	/// -= operator
	template<typename T>
	inline Vec3D &operator -= (const T p_oScalar)
	{
		m_oEntries[0] -= p_oScalar; m_oEntries[1] -= p_oScalar; m_oEntries[2] -= p_oScalar;
		return *this;
	}
	
	/// *= operator
	template<typename T>
	inline Vec3D &operator *= (const T p_oScalar)
	{
		m_oEntries[0] *= p_oScalar; m_oEntries[1] *= p_oScalar; m_oEntries[2] *= p_oScalar;
		return *this;
	}

	/// Pointwise multiplication of coordinates.
	inline Vec3D &operator *= (const Vec3D &p_rRight)
	{
		m_oEntries[0] *= p_rRight[0]; m_oEntries[1] *= p_rRight[1]; m_oEntries[2] *= p_rRight[2];
		return *this;
	}

	/// Binary + operator
	inline friend Vec3D operator+ (const Vec3D &p_rLeft, const Vec3D &p_rRight)
	{
		Vec3D oRet;
		oRet[0] = p_rLeft[0] + p_rRight[0];
		oRet[1] = p_rLeft[1] + p_rRight[1];
		oRet[2] = p_rLeft[2] + p_rRight[2];
		return oRet;
	}

	/// Binary - operator
	inline friend Vec3D operator- (const Vec3D &p_rLeft, const Vec3D &p_rRight)
	{
		Vec3D oRet;
		oRet[0] = p_rLeft[0] - p_rRight[0];
		oRet[1] = p_rLeft[1] - p_rRight[1];
		oRet[2] = p_rLeft[2] - p_rRight[2];
		return oRet;
	}

	/// Binary + operator with scalar
	template<typename T>
	inline friend Vec3D &operator+ (const T p_rLeft, const Vec3D &p_rRight)
	{
		Vec3D oRet;
		oRet[0] = p_rLeft + p_rRight[0];
		oRet[1] = p_rLeft + p_rRight[1];
		oRet[2] = p_rLeft + p_rRight[2];
		return oRet;
	}

	/// Binary - operator with scalar
	template<typename T>
	inline friend Vec3D operator- (const T p_rLeft, const Vec3D &p_rRight)
	{
		Vec3D oRet;
		oRet[0] = p_rLeft - p_rRight[0];
		oRet[1] = p_rLeft - p_rRight[1];
		oRet[2] = p_rLeft - p_rRight[2];
		return oRet;
	}

	/// Binary * operator with scalar
	template<typename T>
	inline friend Vec3D operator* (const T p_rLeft, const Vec3D &p_rRight)
	{
		Vec3D oRet;
		oRet[0] = p_rLeft * p_rRight[0];
		oRet[1] = p_rLeft * p_rRight[1];
		oRet[2] = p_rLeft * p_rRight[2];
		return oRet;
	}

	/// Setter for scalar values. Does not create a new instance!
	virtual inline void set(const double p_oC1, const double p_oC2, const double p_oC3)
	{
		m_oEntries[0] = p_oC1; m_oEntries[1] = p_oC2; m_oEntries[2] = p_oC3;
	}

	/// Calculates the square of the norm of a vector.
	inline double norm2()
	{
		return ( (m_oEntries[0]*m_oEntries[0]) + (m_oEntries[1]*m_oEntries[1]) + (m_oEntries[2]*m_oEntries[2]) );
	}

	/// Calculate square of distance
	inline double dist2(const Vec3D &p_rDistTo)
	{
		double oDeltaX = p_rDistTo[0] - m_oEntries[0];
		double oDeltaY = p_rDistTo[1] - m_oEntries[1];
		double oDeltaZ = p_rDistTo[2] - m_oEntries[2];

		return ( (oDeltaX * oDeltaX) + (oDeltaY * oDeltaY) + (oDeltaZ * oDeltaZ) );
	}


	/** @brief Normalizes given vector \f$ v\in\mathbb{R}^3 \f$ by \f$ z \f$-coordinate if the latter is not zero.
	 * The result is a homogeneous vector in \f$ \mathbb{P}^2 \f$.
	 * If the \f$ z \f$-coordinate is zero ( ie. not greater than precitec::math::eps ), the function returns false otherwise and leaves \f$ v \f$ untouched.
	 */
	bool normalizeByZ(Vec2DHomogeneous &p_rDest);
	/// Computes the crossproduct \f$ v = v_1 \times v_2 \f$.
	Vec3D crossProduct(Vec3D const &p_rRightVec);
	/// Computes the standard euclidean dot product \f$ c = \langle v_1, v_2 \rangle \f$.
	double dotProduct(Vec3D const &p_rRight);
	/// Computes the orthogonal projection of a 3D vector onto a given line segment. The latter needs to be initialized correctly using  the function VecSegment::preComputeSegmentData.
	Vec3D projOntoSegment( LineSegment &p_rSeg, bool oRestrictToSegment);

	bool writeToFile(std::ofstream &p_rFile);

protected:
	tHVecData m_oEntries; ///< Coordinates of the vector
};





/**
 * @brief Tiny class for line segment data.
 *
 * Helper class for computing projections of points in \f$ \mathbb{R}^3\f$ ( class Vec3D ) onto lines and segments in the same space.
 *
 */
class ANALYZER_INTERFACE_API LineSegment
{
public:
	/// CTor
	LineSegment():m_oStart(0.0, 0.0, 0.0), m_oEnd(0.0, 0.0, 0.0), m_oSegment(0.0, 0.0, 0.0), m_oLen(0.0), m_oLastProjectionValid(false) {};
	/// DTor
	~LineSegment(){};

	/// Start point (given as direction vector from origin) of a line semgent in \f$ \mathbb{R}^3 \f$.
	Vec3D m_oStart;
	/// End point (given as direction vector from origin) of a line semgent in \f$ \mathbb{R}^3 \f$.
	Vec3D m_oEnd;
	/// Direcion of line segment. This difference vector is computed by LineSegment::preComputeSegmentData and should NOT be set directly.
	Vec3D m_oSegment;
	/// Length of the line segment. The length is computed by LineSegment::preComputeSegmentData and should NOT be set directly.
	double m_oLen;
	/// invalidates last projection (points left or right from segment)
	void invalidateLastProjection();
	void setLastProjectionValid();
	bool lastProjectionValid();

	/** @brief Given start and endpoint, computes the segment vector (endpoint - startpoint) m_oSegment and its squared length LineSegment::m_oLen.
	*
	* An exmaple is given above the file documentation projectiveMathStructures.h
	*/
	void preComputeSegmentData( Vec3D const &p_rSegStart, Vec3D const &p_rSegEnd );
private:
	bool m_oLastProjectionValid;
};


/**
 *  @brief	 Base ( inhomogeneous ) 3D vector class for projective geometry.
 *
 *  This class offers base structures and functions for projective geometry as well as simple vector arithmetics.
 *
 */
class ANALYZER_INTERFACE_API Vec2DHomogeneous
{
public:
	Vec2DHomogeneous(); ///< CTor
	Vec2DHomogeneous(const double, const double); ///< CTor with initializing data
	Vec2DHomogeneous(const Vec2DHomogeneous &p_rVec); ///< Copy CTor
	virtual ~Vec2DHomogeneous() {}; ///< DTor

	/// Index operator for write and read access. The z coordinate can not be overwritten and remains 1.
	double &operator[] (int p_oIdx)
	{
		const int oIdx = std::abs(p_oIdx)%3;
		if (oIdx == 2)
		{
			p_oRetDummy = m_oEntries[2];
			return p_oRetDummy;
		}
		return m_oEntries[oIdx];
	}

	/// Const. index operator for read only access
	double const &operator[] (int p_oIdx) const
	{
		const int oIdx = std::abs(p_oIdx)%3;
		return m_oEntries[oIdx];
	}

	/// Setter copying from Vec2DHomogeneous. Does not create a new instance!
	inline Vec2DHomogeneous &operator = (const Vec2DHomogeneous &p_rVec)
	{
		m_oEntries[0] = p_rVec[0]; m_oEntries[1] = p_rVec[1]; m_oEntries[2] = 1.0;
		return *this;
	}

	/// += operator
	inline Vec2DHomogeneous &operator+= (const Vec2DHomogeneous &p_Vec)
	{
		m_oEntries[0] += p_Vec[0]; m_oEntries[1] += p_Vec[1];
		return *this;
	}

	/// -= operator
	inline Vec2DHomogeneous &operator-= (const Vec2DHomogeneous &p_Vec)
	{
		m_oEntries[0] -= p_Vec[0]; m_oEntries[1] -= p_Vec[1];
		return *this;
	}

	/// Multiply vector with scalar, z coordinate remains 1.
	template<typename T>
	inline Vec2DHomogeneous &operator*= (const T p_oScalar)
	{
		m_oEntries[0] *= p_oScalar; m_oEntries[1] *= p_oScalar;
		return *this;
	}

	/// Substract scalar from vector, z coordinate remains 1.
	template<typename T>
	inline Vec2DHomogeneous &operator -= (const T p_oScalar)
	{
		m_oEntries[0] -= p_oScalar; m_oEntries[1] -= p_oScalar;
		return *this;
	}

	/// Add scalar to vector, z coordinate remains 1.
	template<typename T>
	inline Vec2DHomogeneous &operator+= (const T p_oScalar)
	{
		m_oEntries[0] += p_oScalar; m_oEntries[1] += p_oScalar;
		return *this;
	}

	/// Pointwise multiplication of two vectors, z coordinate remains 1.
	inline Vec2DHomogeneous &operator *= (const Vec2DHomogeneous p_oRight)
	{
		m_oEntries[0] *= p_oRight[0]; m_oEntries[1] *= p_oRight[1];
		return *this;
	}

	/// Binary + operator
	inline friend Vec2DHomogeneous operator+ (const Vec2DHomogeneous &p_rLeft, const Vec2DHomogeneous &p_rRight)
	{
		Vec2DHomogeneous oRet;
		oRet[0] = p_rLeft[0] + p_rRight[0];
		oRet[1] = p_rLeft[0] + p_rRight[1];
		oRet[2] = 1.0;
		return oRet;
	}

	/// Binary - operator
	inline friend Vec2DHomogeneous operator- (const Vec2DHomogeneous &p_rLeft, const Vec2DHomogeneous &p_rRight)
	{
		Vec2DHomogeneous oRet;
		oRet[0] = p_rLeft[0] - p_rRight[0];
		oRet[1] = p_rLeft[1] - p_rRight[1];
		oRet[2] = 1.0;
		return oRet;
	}

	/// Binary + operator with scalar
	template<typename T>
	inline friend Vec2DHomogeneous operator+ (const T p_rLeft, const Vec2DHomogeneous &p_rRight)
	{
		Vec2DHomogeneous oRet;
		oRet[0] = p_rLeft + p_rRight[0];
		oRet[1] = p_rLeft + p_rRight[1];
		oRet[2] = 1.0;
		return oRet;
	}

	/// Binary - operator with scalar
	template<typename T>
	inline friend Vec2DHomogeneous operator- (const T p_rLeft, const Vec2DHomogeneous &p_rRight)
	{
		Vec2DHomogeneous oRet;
		oRet[0] = p_rLeft - p_rRight[0];
		oRet[1] = p_rLeft - p_rRight[1];
		oRet[2] = 1.0;
		return oRet;
	}


	/// Setter function
	inline void set(const double p_oC1, const double p_oC2)
	{
		m_oEntries[0] = p_oC1; m_oEntries[1] = p_oC2; m_oEntries[2] = 1.0;
	}

	/// Setter function, overload of parent class which leaves the z-coordinate unchanged.
	virtual inline void set(const double p_oC1, const double p_oC2, const double p_oC3)
	{
		set(p_oC1, p_oC2);
	}

protected:
	/// Coordinate vector data.
	tHVecData m_oEntries;

private:
	double p_oRetDummy; ///< Internal helper for read only access of last vector entry
};



} // namespaces
}

#endif /* PROJECTIVEMATHSTRUCTURES_H_ */
