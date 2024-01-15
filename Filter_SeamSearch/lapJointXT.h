/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS/CB
 * 	@date		2021
 * 	@brief 		This filter calculates the seam pos out of the laserline, the image and to lines for lap joint seams (Audi)
 */

#ifndef LAPJOINTXT_H_
#define LAPJOINTXT_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

namespace precitec {
namespace filter {

class FILTER_API LapJointXT  : public fliplib::TransformFilter
{

public:

	/// CTor.
	LapJointXT();
	/// DTor.
	virtual ~LapJointXT();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_SEAMPOS_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_SEAMLEFT_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_SEAMRIGHT_OUT;	///< Name Out-Pipe
	static const std::string PIPENAME_AREALEFT_OUT;	///< Name Out-Pipe
	static const std::string PIPENAME_AREARIGHT_OUT;	///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	/**
	 * @brief 
	 *
	 * @param p_rImageIn       Input image.
	 * @param p_rLaserLineIn   LaserLine input object.
	 * @param p_oLineHeight    Height of the laser line object.
	 * @param p_rProfileOut    Profile output object.
	 * @param p_oProfileHeight Height of the output profile (for each of the upper and lower band).
	 */
	void calcSeam( const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn,
		geo2d::Doublearray &seamPos, geo2d::Doublearray &seamLeft, geo2d::Doublearray &seamRight, geo2d::Doublearray &areaLeft, geo2d::Doublearray &areaRight );

	//void calcParabel(const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn, geo2d::Doublearray seamLeft, geo2d::Doublearray seamRight,
	//	geo2d::Doublearray & outA, geo2d::Doublearray & outB, geo2d::Doublearray & outC);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;				///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;				///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInLeftLineSlope;			///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInLeftLineYIntercept;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInRightLineSlope;			///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInRightLineYIntercept;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInLeftMaxLineDiff;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInRightMaxLineDiff;	///< In pipe

	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamPos;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamLeft;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamRight;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutAreaLeft;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutAreaRight;	///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation

	int m_oMode;
	int m_oSeamAreaStart;
	int m_oSeamAreaEnd;
	int m_oThresholdLeft;
	int m_oThresholdRight;
	int m_oNumberOfPixLeft;
	int m_oNumberOfPixRight;

	int m_oLineThreshold;
	int m_oMaxLineDiff_left, m_oMaxLineDiff_right;

	bool m_doLeft, m_doRight;
	bool m_isNoSeamDetection, m_isTracking;
	int m_AbortType = 0;			// 0: both direction /1: only to down /2: to up
	bool m_isTrackingWithGap = false;

	
	geo2d::Doublearray m_oSeamPosOut;			
	geo2d::Doublearray m_oSeamLeftOut;			
	geo2d::Doublearray m_oSeamRightOut;		
	geo2d::Doublearray m_oAreaLeftOut;		
	geo2d::Doublearray m_oAreaRightOut;	

	double m_paintA, m_paintB, m_paintC;

	int m_resultSeamLeft, m_resultSeamRight, m_resultSeamPos;
	int m_correctedSeamLeft, m_correctedSeamRight;
	int m_seamStartX, m_seamEndX;
	int m_lastSeamLeft, m_lastSeamRight;

	//int m_numberOfDebugPoints;

	void setModeVariables();

	double m_leftLineSlope, m_leftLineYIntercept, m_rightLineSlope, m_rightLineYIntercept;
};

} // namespace precitec
} // namespace filter

#endif /* LAPJOINTXT_H_ */
