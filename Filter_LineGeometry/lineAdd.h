/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter summarize two laserline and/or width/brightness arrays
 */

#ifndef LINEADD_H_
#define LINEADD_H_

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


class FILTER_API LineAdd  : public fliplib::TransformFilter
{

public:

	/// CTor.
	LineAdd();
	/// DTor.
	virtual ~LineAdd();

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
	void calcLineAdd(const geo2d::VecDoublearray &p_rLaserLine1In, const geo2d::VecDoublearray &p_rLaserLine2In, int p_oFac1, int p_oFac2, 
		geo2d::VecDoublearray &p_rSumOut);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	//const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine1;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine2;	///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >       * m_pPipeOutLine;	///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation
	int 														m_oFac1;				///< Factor for Line 1
	int 														m_oFac2;				///< Factor for Line 2
	geo2d::VecDoublearray										m_oLineOut;				///< Output profile

	int m_overlayMin, m_overlayMax;
};

} // namespace precitec
} // namespace filter

#endif /* LINEADD_H_ */
