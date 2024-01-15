/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB)
 *  @date		2011
 *  @brief		Extracts a portion from a laserline object.
 */

#ifndef LINEEXTRACTDYNAMIC_H_
#define LINEEXTRACTDYNAMIC_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray
#include "overlay/overlayCanvas.h"


namespace precitec {
	using interface::GeoVecDoublearray;
	using geo2d::VecDoublearray;
	using interface::GeoVecDoublearray;
	using geo2d::VecDoublearray;
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief This filter extracts a sub line from a longer (laser-)line object. The start and end indices have to be specified by the user as filter parameter "Start" and "End".
 */
class FILTER_API LineExtractDynamic  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineExtractDynamic();
	/// DTor.
	virtual ~LineExtractDynamic();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe

	/// Set filter parameters defined in database / xml file
	void setParameter();
	/// paints overerlay primitives
	void paint();

	/**
	 * @brief Extracts a sub line from a longer laser line.
	 *
	 * @param p_rLineIn 	(Laser)-Line input object.
	 * @param p_oStart  	Start index.
	 * @param p_oEnd		End index.
	 * @param p_rLineOut    Output object.
	 */
	static void extract( const VecDoublearray &p_rLineIn, int p_oStart, int p_oEnd, VecDoublearray &p_rLineOut);

protected:

	/// in pipe registration
	virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	virtual void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeLineIn;    	///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >*       	m_pPipeLineOut;		///< Out pipe
	interface::SmpTrafo											m_oSpTrafo;			///< roi translation
	const fliplib::SynchronePipe< interface::GeoDoublearray >*       	m_pPipeStart;			///< Start index
	const fliplib::SynchronePipe< interface::GeoDoublearray >*       	m_pPipeEnd;				///< End index
	geo2d::VecDoublearray											m_oLineOut;			///< Output signal
	image::Color m_oColor; ///< Color of the overlay
	bool m_oPaint;  ///< Do NOT paint in case of error in preceeding filters

};

} // namespace precitec
} // namespace filter

#endif /* LINEEXTRACTDYNAMIC_H_ */
