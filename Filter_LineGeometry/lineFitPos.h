/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		CB
 * 	@date		2020
 * 	@brief 		This filter calculates a fitting line out of the laser line on a certain pos
 */

#ifndef LINEFITPOS_H_
#define LINEFITPOS_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

#include "LineFitter.h"

namespace precitec {
namespace filter {


class FILTER_API LineFitPos  : public fliplib::TransformFilter
{

public:

	/// CTor.
	LineFitPos();
	/// DTor.
	virtual ~LineFitPos();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_SLOPE_OUT; ///< Name Out-Pipe
	static const std::string PIPENAME_YINTERSEPT_OUT; ///< Name Out-Pipe
    static const std::string PIPENAME_LINE_OUT; ///< Name Out-Pipe
    static const std::string PIPENAME_ERROR_OUT; ///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	void calcLine(const geo2d::VecDoublearray &p_rLaserLineIn, int roiLength, bool roitoRight, int position, bool computeError,
		geo2d::Doublearray & slopeOut, geo2d::Doublearray & yInterceptionOut, geo2d::Doublearray & errorOut);
protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE);

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInPosition;		///< In pipe

	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutSlope;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutYIntercept;		///< Data out-pipe.
    fliplib::SynchronePipe<interface::GeoLineModelarray>			m_oPipeOutLineEquation;	///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutError;		///< Data out-pipe.
    fliplib::SynchronePipe<interface::GeoVecDoublearray> m_pipeOutAbsoluteError;
    fliplib::SynchronePipe<interface::GeoVecDoublearray> m_pipeOutSumSquaredError;

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation
	int 														m_oRoiLength;			///< area length for line fit
	bool														m_oRoiToRight;				///< 0: Area from Pos to the left, 1: area to the right 
	bool														m_oHorizontalMean;			///< 0: LineFit free orientation, 1: LineFit horizontal (Mean) 

	geo2d::Doublearray										m_oSlopeOut;			///< Output profile
	geo2d::Doublearray										m_oYInterceptOut;			///< Output profile

	bool m_isValid;

    int m_paintLineSize, m_paintStartX, m_paintEndX;
	double m_paintSlope, m_paintYIntercept;
    std::vector<double> m_paintArea;
};

} // namespace precitec
} // namespace filter

#endif /* LINEFITPOS_H_ */
