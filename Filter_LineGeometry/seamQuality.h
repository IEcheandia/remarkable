/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		JS
 *  @date		2015
 *  @brief		Computes Length and Quality of a valid seamline
 */

#ifndef SEAMQUALITY_H_
#define SEAMQUALITY_H_


#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <image/image.h>				///< BImage
#include "filter/parameterEnums.h"		///< enum ExtremumType

#include <common/frame.h>				///< ImageFrame
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray


namespace precitec {
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief This filter finds the maximum or minimum of a laser line and returns it as a point output pipe.
 */
class FILTER_API SeamQuality  : public fliplib::TransformFilter
{
public:

	/// CTor.
	SeamQuality();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string PIPENAME_LENGTH;		///< Name Out-Pipe 1
	static const std::string PIPENAME_DEVIATION;	///< Name Out-Pipe 2

	// 2 new Out-Pipes
	static const std::string PIPENAME_TRACKED_LENGTH; ///< Name Out-Pipe 3
	static const std::string PIPENAME_GAP_DETECTED;	  ///< Name Out-Pipe 4

	/// Set filter parameters defined in database / xml file
	void setParameter();

	/// paints overlay
	void paint();

	virtual void arm(const fliplib::ArmStateBase& state);	///< arm filter

	/**
	 * @brief Calculates length of a line in respect of the distance between laserline points. 
	 *
	 * @param p_rLineIn    		(Laser)-Line input object.
	 * @param p_oDirection      Count direction from first valid to first invalid point(rank==0) in the line
	 * @param p_rLengthOut		Length of valid line
	 * @return void
	 */
	void calcLength(const geo2d::VecDoublearray &p_rLineIn, 
					SearchDirType p_oDirection, 
					geo2d::Doublearray &p_rLengthOut, 
					geo2d::Doublearray &p_rTrackedLengthOut,
					geo2d::Doublearray &p_rGapDetectedOut);


protected:

	/// in pipe registration
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/// in pipe event processing
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE);

private:

	//vektor von double array
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > *m_pPipeInLaserline;			///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    *m_pPipeInXLeft;				///< In pipe xcoord left point
	const fliplib::SynchronePipe< interface::GeoDoublearray >    *m_pPipeInXRight;				///< In pipe xcoord right point

	// double array
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipeOutLength;			///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipeOutDeviation;	    ///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipeOutTrackedLength;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipeOutGapDetected;	    ///< Out pipe

	geo2d::TPoint<double>											m_oJumpPos;

	geo2d::Doublearray								  				m_oLengthOut;				///< Output value.
	geo2d::Doublearray								  				m_oDeviationOut;			///< Output value.
	geo2d::Doublearray								  				m_oTrackedLengthOut;		///< Output value.
	geo2d::Doublearray								  				m_oGapDetectedOut;			///< Output value.

	// parameters
	int 															m_oMaxJump;
	int 															m_oMaxDistance;
	double 															m_oMaxDeviation;
	int 															m_oFitMethod;
	SearchDirType													m_oDirection;				///< left to right, or right to left
	double																m_oGapThreshold;

	// internal variables
	interface::SmpTrafo												m_oSpTrafo;					///< roi translation


	double															m_oPositionInX1;
	double															m_oPositionInX2;
	geo2d::TPoint<double>	m_oSensorPos;		//	Position auf dem Sensor (kompletter Chip i.d.R. 1024x1024 pixel
	geo2d::TPoint<double>	m_oHWRoiPos;		//	Position auf dem Hw Roi
	
};
} // namespace precitec
} // namespace filter

#endif /* LINEEXTREMUM_H_ */
