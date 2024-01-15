#ifndef PARAMETERFILTER_H_
#define PARAMETERFILTER_H_
#pragma once

/**
 *  Filter_Utility::parameterFilter.h
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR)
 *	@date           25.10.2012
 *	@brief					Parmeterfilter takes a parameter and outputs it in a pipe for debug purposes.
 */

#include <vector>

#include "fliplib/Fliplib.h"			// fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/PipeEventArgs.h"		// event processing
#include "fliplib/SynchronePipe.h"		// in- / output

#include "common/frame.h"				// ImageFrame
#include "image/image.h"				// BImage
#include "common/geoContext.h"
#include "geo/geo.h"				// BImage

namespace precitec {
namespace filter {
	/// all scalars are doubles, and may show up in groups
	//typedef std::vector <double> PipedScalar;
	/// Filter_Utility::ParameterFilter
	using interface::ImageFrame;
	/**
	 * Parameterfilter has
	 *  - a dummy pipe input, so the filter can be triggered
	 *  - a double parameter
	 *  - an output pipe, which reproduces the parameter as an array of 16 doubles
	 *  It is used to test other filters with scalar input pipes.
	 *  The array length of 16 is somewhat arbitrary but should suffice for most
	 *  test scenarois.
	 */
	class FILTER_API ParameterFilter : public fliplib::TransformFilter  {
	public:
		ParameterFilter();
		virtual ~ParameterFilter() {}
	public:
		/// initialize local parameters from database
		virtual void setParameter();
        bool isValidConnected() const override;
		/// no painintg is deemed necessary
		virtual void paint() {}
		virtual void arm (const fliplib::ArmStateBase& state) {}
	private:
		/// registration input pipe
		virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
		/// do the actual processing; N inputs so simple proceedGroup()
		virtual void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);
	private:
		/// image in-pipe (needed for trigger)
		fliplib::SynchronePipe<ImageFrame>	*m_pPipeIn;
		/// out pipe
		fliplib::SynchronePipe<interface::GeoDoublearray>	m_oPipeOut;
		/// scale factor
		double	m_oParam;

	};

} // namespace filter
} // namespace precitec

#endif // PARAMETERFILTER_H_
