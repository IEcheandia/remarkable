#ifndef VERTICALSHADING_H_
#define VERTICALSHADING_H_
#pragma once

/**
 *  Filter_PoreAnalysis::verticalShading.h
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR)
 *	@date           23.10.2012
 *	@brief					A Filter to detect and eleminate column noise in an image
 */

#include <vector>

#include "fliplib/Fliplib.h"			// fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/PipeEventArgs.h"		// event processing
#include "fliplib/SynchronePipe.h"		// in- / output

#include "common/frame.h"				// ImageFrame
#include "image/image.h"				// BImage
#include "common/geoContext.h"


namespace precitec {
	/// Filter_PoreAnalysis::filter::VerticalShading
namespace filter {

		/**
		 * Pattern is the data-structure passed between the shading functinos
		 * The two arrays contain vectors (an entry for each column) with
		 * the column offset (==minimum) and the scaling factor.
		 * This structure allows these values to be a simple function-return-value.
		 */
		struct Pattern {
			Pattern(uInt size) : offset(size), factor(size) {}
			std::vector<uInt> offset;
			std::vector<uInt> factor;
		};

		using image::BImage;
		using interface::ImageFrame;
		using geo2d::Rect;

		class FILTER_API VerticalShading : public fliplib::TransformFilter {
		public:
			VerticalShading();
			/// does nothing
			virtual ~VerticalShading() {}
		public:
			/// initialize local parameters from database
			virtual void setParameter();
			/// no painintg is deemed necessary
			virtual void paint();
			virtual void arm (const fliplib::ArmStateBase& state) {}
		private:
			/// detect and eliminate column patterns in image
			ImageFrame shade(ImageFrame const& p_rIn);
			/// detect column patterns in image
			static Pattern	detectPattern(BImage const& p_rIn);
			/// eliminate column patterns in image
			static void eliminatePattern(BImage const& p_rIn, Pattern	const& p_rPattern, BImage& p_rOut);
			/// registration input pipe
			virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
			/// do the actual processing; 1 input so simple proceed()
			virtual void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);
		private:
			/// in pipe
			const fliplib::SynchronePipe<ImageFrame>*	m_pPipeIn;
			interface::SmpTrafo							m_oSpTrafo;				///< roi translation
			std::array<image::BImage, g_oNbParMax>		m_oShadedImageOut;		///< shaded image out
			/// out pipe
			fliplib::SynchronePipe<ImageFrame>			m_oPipeOut;
		};


} // namespace filter
} // namespace precitec

#endif // VERTICALSHADING_H_
