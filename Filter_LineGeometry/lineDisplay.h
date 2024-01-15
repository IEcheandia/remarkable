/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB)
 *  @date		2011
 *  @brief		Simple display filter for laser-line objects.
 */

#ifndef LINEDISPLAY_H_
#define LINEDISPLAY_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray

namespace precitec {
	using image::BImage;
	using interface::ImageFrame;
	using interface::Size2D;
	using interface::GeoVecDoublearray;
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief This filter displays a 1D signal / laser line pipe.
 */
class FILTER_API LineDisplay  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineDisplay();
	/// DTor.
	virtual ~LineDisplay();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe

	/// Set filter parameters defined in database / xml file.
	void setParameter();
	/// Draw the filter results.
	void paint();

protected:
	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// In-pipe event processing.
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);

private:

	const fliplib::SynchronePipe< GeoVecDoublearray >* 	m_pPipeLineIn;    	///< In pipe
	fliplib::SynchronePipe< GeoVecDoublearray >*       	m_pPipeLineOut;		///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;			///< roi translation
	int															m_oStart;			///< Start index
	int															m_oEnd;				///< End index
	int															m_oHeight;			///< The height of the output plot
	bool														m_oFrame;			///< Shall the filter draw a frame?
	int															m_oRed;				///< Color of the plotted curve (red   component)
	int															m_oGreen;			///< Color of the plotted curve (green component)
	int															m_oBlue;			///< Color of the plotted curve (blue  component)
	bool														m_oCross;			///< Shall we draw a cross for each point?
	unsigned int												m_oPaintLayer;		///< Layer to paint on

	geo2d::VecDoublearray										m_oLineOut;			///< Output signal
};

} // namespace precitec
} // namespace filter

#endif /* LINEDISPLAY_H_ */
