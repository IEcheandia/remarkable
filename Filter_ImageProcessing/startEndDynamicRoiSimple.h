#ifndef STARTENDDYNAMICROISIMPLE_H_
#define STARTENDDYNAMICROISIMPLE_H_
#pragma once

/**
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         OS
 *	@date           04.2017
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
	using interface::GeoStartEndInfoarray;
	using geo2d::Doublearray;
	using image::BImage;
	using interface::ImageFrame;
	using geo2d::Rect;

class FILTER_API StartEndDynamicRoiSimple : public fliplib::TransformFilter 
{
	public:
		/// std-cTor
		StartEndDynamicRoiSimple();
		/// dTor does nothing.
		virtual ~StartEndDynamicRoiSimple() {}

	public:
		/// create Sub Image
		static ImageFrame	createSubImage(ImageFrame const& p_rIn, Rect & roi);
		/// initialize local parameters from database
		virtual void setParameter();
		
		virtual void paint();
		virtual void arm(const fliplib::ArmStateBase& state);

	private:		
		virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);		
		virtual void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

	private:
		/// image in-pipe
		const fliplib::SynchronePipe<ImageFrame>*	m_pPipeImage;
		/// roi-x in-pipe
		const fliplib::SynchronePipe<GeoDoublearray>*	m_pPipeX;
		/// roi-dx in-pipe
		const fliplib::SynchronePipe<GeoDoublearray>*	m_pPipeDx;
		/// startEndInfo in-pipe
		const fliplib::SynchronePipe<GeoStartEndInfoarray>*	m_pPipeStartEndInfo;

		// parameters
		int        m_oY0;	
		int        m_oDY;	

		/// out pipe
		fliplib::SynchronePipe<ImageFrame>	m_oPipeSubImage;
		/// final roi; class member so paint can use precalculated value
		Rect		m_oRoi;
		bool m_isCropped;

}; 


} // namespace filter
} // namespace precitec

#endif // STARTENDDYNAMICROISIMPLE_H_
