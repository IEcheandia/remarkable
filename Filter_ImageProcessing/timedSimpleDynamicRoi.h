#ifndef TIMEDSIMPLEDYNAMICROI_H_
#define TIMEDSIMPLEDYNAMICROI_H_
#pragma once

/**
 *	@file
 *	@copyright      Precitec GmbH & Co. KG
 *	@author         Duw
 *	@date           06.2015
 *	@brief			Filter to create subRoi-image from image and 1 double input denoting ROI. After a given amount of time, the width of the ROI is reduced to avoid false detections.
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

	class FILTER_API TimedSimpleDynamicRoi : public fliplib::TransformFilter {
	public:
		/// std-cTor
		TimedSimpleDynamicRoi();
		/// dTor does nothing.
		virtual ~TimedSimpleDynamicRoi() {}

		static const std::string m_oFilterName;			///< Filter name.
		static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
		static const std::string m_oParamX0Name;	///< Parametername: Y-Position
		static const std::string m_oParamY0Name;	///< Parametername: Y-Position
		static const std::string m_oParamDYName;	///< Parametername: Height
		static const std::string m_oParamDXLargeName;	///< Parametername: Large width
		static const std::string m_oParamDXSmallName;	///< Parametername: Small width 
		static const std::string m_oParamThresholdName;	///< Parametername: Amount of delay [um].

		/// create Sub Image
		static ImageFrame	createSubImage(ImageFrame const& p_rIn, Rect & roi);
		/// initialize local parameters from database
		virtual void setParameter();
		/// no painintg is deemed necessary
		virtual void paint();
		virtual void arm(const fliplib::ArmStateBase& state);

	private:
		/// registration input pipe
		virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
		/// do the actual processing; N inputs so simple proceedGroup()
		virtual void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

		/// image in-pipe
		const fliplib::SynchronePipe<ImageFrame>*	m_pPipeInImageFrame;
		/// roi-x in-pipes
		const fliplib::SynchronePipe<GeoDoublearray>*	m_pPipeInX;
		
		// parameters
		int        m_oY0; ///< Parameter: Y-Position
		int        m_oX0; ///< Parameter: X-Position
		int        m_oDY; ///< Parameter: Height
		int        m_oDXLarge; ///< Parameter: Large Width
		int        m_oDXSmall; ///< Parameter: Small Width
		unsigned int m_oThreshold; ///< Parameter: Amount of delay [um].
		int m_oTriggerDelta;		///< Trigger distance [um]

		/// out pipe
		fliplib::SynchronePipe<ImageFrame>	m_oPipeSubImage;
		/// final roi; class member so paint can use precalculated value
		Rect		m_oRoi;


	}; // Scale

} // namespace filter
} // namespace precitec

#endif // DYNAMICROISIMPLE_H_
