/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter determines the top and lower boundaries of the laserline
 */

#ifndef BOUNDARYTRACKING_H_
#define BOUNDARYTRACKING_H_

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


class FILTER_API BoundaryTracking  : public fliplib::TransformFilter
{

public:

	/// CTor.
	BoundaryTracking();
	/// DTor.
	virtual ~BoundaryTracking();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT_TOPLINE;		///< Name Out-Pipe
	static const std::string PIPENAME_OUT_BOTTOMLINE;		///< Name Out-Pipe
	static const std::string PIPENAME_OUT_CENTERLINE;		///< Name Out-Pipe

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
void calcLines( const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn, int p_oMode, int p_oThreshold,
	geo2d::VecDoublearray &p_rTopLineOut, geo2d::VecDoublearray &p_rBottomLineOut, geo2d::VecDoublearray &p_rCenterLineOut);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;	///< In pipe

	fliplib::SynchronePipe< interface::GeoVecDoublearray >       * m_pPipeOutTopLine;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >       * m_pPipeOutBottomLine;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >       * m_pPipeOutCenterLine;	///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation
	int															m_oMode;
	int 														m_oThreshold;			///< Threshold what belongs to the laserline
	int															m_oSearchHeight;		///< Height of the area 

	geo2d::VecDoublearray										m_oTopLineOut;			///< Output profile
	geo2d::VecDoublearray										m_oBottomLineOut;			///< Output profile
	geo2d::VecDoublearray										m_oCenterLineOut;			///< Output profile

	int m_overlayMin, m_overlayMax;

	bool m_paintAvailable;

	std::vector<int> m_paintRankVec;
	std::vector<int> m_paintPosTopVec;
	std::vector<int> m_paintPosBottomVec;
	std::vector<int> m_paintPosCenterVec;

};

} // namespace precitec
} // namespace filter

#endif /* BOUNDARYTRACKING_H_ */
