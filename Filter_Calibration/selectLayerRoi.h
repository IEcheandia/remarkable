/**
 *  @file       selectLayerRoi.h
 *  @ingroup    Filter_Utility
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Andreas Beschorner (AB)
 *  @date		2013
 *  @brief		Select top or bottom roi from automatic (calibration workpiece) layer roi detection.
 */


#ifndef SELECTLAYERROI_H_
#define SELECTLAYERROI_H_


#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray

namespace precitec {
namespace filter {

/**
 * @brief   Select top or bottom roi from automatic (calibration workpiece) layer roi detection.
 * @details The primay usecase this filter is aiming at is selecting one of the two ROIs from automatic calibration workpiece layer ROI detection.
 *          The output is a ROI which can be forwarded to the dynamic roi filter.
 * @param p_rVecCoord     IN pipe, containing the complete vector of two ROIs for top and bottom layer.
 * @param p_oTopOrBottom  PARAMETER for choosing either top or bottom layer.
 */
class FILTER_API SelectLayerRoi  : public fliplib::TransformFilter
{
public:

	static const std::string m_oFilterName;
	static const std::string m_oNamePipeXTop;
	static const std::string m_oNamePipeYTop;
	static const std::string m_oNamePipeXBot;
	static const std::string m_oNamePipeYBot;

	/// CTor.
	SelectLayerRoi();
	/// DTor.
	virtual ~SelectLayerRoi();

	/// Set filter parameters as defined in database / xml file.
	void setParameter();

protected:
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/// In-pipe event processing.
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

private:
	const fliplib::SynchronePipe< interface::ImageFrame >*	m_pPipeInImageFrame;    ///< in pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >*   m_pPipeROIsIn;   ///< In pipe

	fliplib::SynchronePipe< interface::GeoDoublearray >            m_oPipeOutXTop;  ///< Out pipe X top (upper left corner)
	fliplib::SynchronePipe< interface::GeoDoublearray >            m_oPipeOutYTop;  ///< Out pipe Y top
	fliplib::SynchronePipe< interface::GeoDoublearray >            m_oPipeOutXBot;  ///< Out pipe X bottom (lower right corner)
	fliplib::SynchronePipe< interface::GeoDoublearray >            m_oPipeOutYBot;  ///< Out pipe Y bottom

	void signalSend(const interface::ImageContext &p_rImgContext, const double p_oWidth, const int p_oIO);

	// parameters
	int m_oTopLayer; ///< Top layer <= 0, Bottom layer > 0

	// internal variables
	interface::SmpTrafo    m_oSpTrafo;			///< roi translation
	double m_oYTop, m_oYBottom;                ///< Output profile
	interface::ResultType m_oRes;                          ///< Passthrough for result(s) from previous filter(s)
};

} // namespace precitec
} // namespace filter

#endif /* SelectLayerRoi_H_ */

