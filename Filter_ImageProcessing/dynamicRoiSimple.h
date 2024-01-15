#ifndef DYNAMICROISIMPLE_H_
#define DYNAMICROISIMPLE_H_
#pragma once

/**
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         JS
 *	@date           06.2015
 *	@brief			Filter to create subRoi-image from image and 2 double inputs denoting ROI
 */


#include <vector>

#include "fliplib/Fliplib.h"			// fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/PipeEventArgs.h"		// event processing
#include "fliplib/SynchronePipe.h"		// in- / output

#include "common/frame.h"				// ImageFrame
#include "image/image.h"				// BImage
#include "common/geoContext.h"
#include "geo/geo.h"				// BImage, GeoVector...


namespace precitec {

namespace filter {
	/// all scalars are doubles, and may show up in groups
	//typedef std::vector <double> PipedScalar;
	using interface::GeoDoublearray;
	using geo2d::Doublearray;
	using image::BImage;
	using interface::ImageFrame;
	using geo2d::Rect;

	class FILTER_API DynamicRoiSimple : public fliplib::TransformFilter {
	public:
		/// std-cTor
		DynamicRoiSimple();
		/// dTor does nothing.
		virtual ~DynamicRoiSimple() {}

	public:
		/// create Sub Image
		ImageFrame	createSubImage(ImageFrame const& imageIn);
		/// initialize local parameters from database
		virtual void setParameter();
		/// no painintg is deemed necessary
		virtual void paint();
		virtual void arm(const fliplib::ArmStateBase& state);

        static const std::string m_filterName;
	private:
		/// registration input pipe
		virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
		/// do the actual processing; N inputs so simple proceedGroup()
		virtual void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

	private:
        void translateIntoInputRoi(const Rect& inRoi);

		/// image in-pipe
		const fliplib::SynchronePipe<ImageFrame>*	m_pPipeImage;
		/// roi-x in-pipes
		const fliplib::SynchronePipe<GeoDoublearray>*	m_pPipeX;
		/// roi-dx in-pipes
		const fliplib::SynchronePipe<GeoDoublearray>*	m_pPipeDx;

        interface::SmpTrafo m_smpTrafo;
		
		// parameters
		int        m_oY0;	///< Searching for the maximum or minimum?
		int        m_oDY;	///< left to right, or right to left
		bool m_xIsStart; ///< true: input x is start of roi, false: x is center of roi

		/// out pipe
		fliplib::SynchronePipe<ImageFrame>	m_oPipeSubImage;
		/// final roi; class member so paint can use precalculated value
		Rect		m_oRoi;
        bool m_isRoiValid = false;
	}; // Scale

} // namespace filter
} // namespace precitec

#endif // DYNAMICROISIMPLE_H_
