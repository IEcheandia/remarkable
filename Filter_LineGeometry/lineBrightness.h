/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter extracs the brightness of the laserline, so seams can be found in darker areas
 */

#ifndef LINEBRIGHTNESS_H_
#define LINEBRIGHTNESS_H_

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


class FILTER_API LineBrightness  : public fliplib::TransformFilter
{

public:

	/// CTor.
	LineBrightness();
	/// DTor.
	virtual ~LineBrightness();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe
	static const std::string PIPENAME_OUT2;		///< Name Out-Pipe

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
	 static void calcLineBrightness(const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn,
		 geo2d::VecDoublearray &p_rProfileOut, int p_oSearchHeightint, int &overlayMin, int &overlayMax, geo2d::Doublearray &p_rLineMeanOut);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;			///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;			///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >       * m_pPipeOutLineBrightness;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			 * m_pPipeOutLineMeanBrightness;///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation
	int															m_oSearchHeight;		///< Height of the area 
	geo2d::VecDoublearray										m_oLineBrightnessOut;			///< Output profile
	geo2d::Doublearray											m_oLineMeanBrightnessOut;		///< Output Mean

	int m_overlayMin, m_overlayMax;

};

} // namespace precitec
} // namespace filter

#endif /* LINEBRIGHTNESS_H_ */
