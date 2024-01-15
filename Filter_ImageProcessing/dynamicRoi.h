#ifndef DYNAMICROI_H_
#define DYNAMICROI_H_
#pragma once

/**
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR)
 *	@date           19.10.2012
 *	@brief					Filter to create subRoi-image from image and 4 double inputs denoting ROI
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
#include "overlay/overlayCanvas.h"


namespace precitec {

namespace filter {
	/// all scalars are doubles, and may show up in groups
	//typedef std::vector <double> PipedScalar;
	using interface::GeoDoublearray;
	using geo2d::Doublearray;
	using image::BImage;
	using interface::ImageFrame;
	using geo2d::Rect;

	class FILTER_API DynamicRoi : public fliplib::TransformFilter {
	public:
		/// std-cTor
		DynamicRoi();
		/// dTor does nothing.
		virtual ~DynamicRoi() {}

	public:
		/// calculate out = A * in  +  B
		ImageFrame	createSubImage(ImageFrame const& p_rIn, Rect & roi, double angle = 0);
		/// initialize local parameters from database
		virtual void setParameter();
		/// no painintg is deemed necessary
		virtual void paint();
		virtual void arm (const fliplib::ArmStateBase& state) {}
	private:
		/// registration input pipe
		virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
		/// do the actual processing; N inputs so simple proceedGroup()
		virtual void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

	private:
		/// image in-pipe
		const fliplib::SynchronePipe<ImageFrame>*	m_pPipeImage;
		/// roi-x in-pipes
		const fliplib::SynchronePipe<GeoDoublearray>*	m_pPipeX;
		/// roi-y in-pipes
		const fliplib::SynchronePipe<GeoDoublearray>*	m_pPipeY;
		/// roi-dx in-pipes
		const fliplib::SynchronePipe<GeoDoublearray>*	m_pPipeDx;
		/// roi-dy in-pipes
		const fliplib::SynchronePipe<GeoDoublearray>*	m_pPipeDy;
        const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeAngle;
		///color
		image::Color	m_oColor;
		/// out pipe
		fliplib::SynchronePipe<ImageFrame>	m_oPipeSubImage;
		/// final roi; class member so paint can use precalculated value
		Rect		m_oRoi;
        double m_angle;
        image::BImage m_image; // paint info
		/// trafo of image; class member so paint can use precalculated value
		interface::SmpTrafo	m_pTrafo;
        bool m_oSupportNestedROI;

		bool m_badInput;
	}; // Scale

} // namespace filter
} // namespace precitec

#endif // DYNAMICROI_H_
