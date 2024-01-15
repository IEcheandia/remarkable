/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter extracs the width of the laserline, so seams can be found in thin line areas
 */

#ifndef LINEWIDTH_H_
#define LINEWIDTH_H_

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

class FILTER_API LineWidth  : public fliplib::TransformFilter
{
public:
	LineWidth();
	virtual ~LineWidth();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT;		///< Name Out-Pipe

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
	bool calcLineWidth( const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn, int p_oThreshold, 
		geo2d::VecDoublearray &p_rProfileOut, int p_oSearchHeightint);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;	///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >       * m_pPipeOutLineWidth;	///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation
	int 														m_oMode;				///< Mode for different calculations
	int 														m_oThreshold;			///< Threshold what belongs to the laserline
	int															m_oSearchHeight;		///< Height of the area 
	int															m_oResolution;			///< Threshold for soot detection
	int															m_oSootThresh;			///< Threshold for soot detection
	int															m_oSootThresh2;			///< Threshold for soot detection
	int															m_oSootFac;			///< Threshold for soot detection
	int															m_oSootFac2;			///< Threshold for soot detection
	geo2d::VecDoublearray										m_oLineWidthOut;			///< Output profile

	int m_overlayMin, m_overlayMax;
    int m_overlayHeight;
};

} // namespace precitec
} // namespace filter

#endif /* LINEWIDTH_H_ */
