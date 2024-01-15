/*!
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			OS
*  @date			2016
*  @file
*  @brief			Analog to Roi-Selector. Clips Roi to Begin/End-Detection result
*/

// VariantID: C95F9D22-3DB6-4F43-956C-6EDE2753C3C8

#ifndef STARTENDROISELECTOR_H_
#define STARTENDROISELECTOR_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"
#include "fliplib/ArmState.h"

#include "common/frame.h"
#include "overlay/overlayCanvas.h"
#include "common/geoContext.h"

namespace precitec {
	namespace filter {


		/**
		* Schneidet aus einem ImageFrame den Region of interests aus und sendet einen neuen ImageFrame an den
		* naechsten Filter
		*/
		class FILTER_API StartEndROISelector : public fliplib::TransformFilter
		{
		public:
			static const std::string m_oFilterName;
			static const std::string PIPENAME1, PIPENAME2;

			StartEndROISelector();

			void setParameter();							///< parameter setzen
			void paint();									///< Zeichnet die ROI in Overlay
			void arm(const fliplib::ArmStateBase& state);	///< arm filter

		private:
			/// in pipe registrastion
			bool subscribe(fliplib::BasePipe& pipe, int group);

			/// Verarbeitungsmethode IN Pipe
			void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

			//! die eigentliche Filterfuktion
			/*!
			\param p_rImgIn	Input image
			\param p_rRoi	Roi. Is adjusted if necessary.
			\return			Image clipped to roi dimension
			\sa -			BImage
			*/
			image::SmpBImage selectRoi(image::BImage const& p_rImgIn, geo2d::Rect &p_rRoi);

			typedef fliplib::SynchronePipe< interface::ImageFrame >	SyncPipeImgFrame;

			const SyncPipeImgFrame*	m_pPipeInImageFrame;	///< inpipe
			const fliplib::SynchronePipe< interface::GeoStartEndInfoarray > * m_pPipeInStartEndInfo;	///< inpipe

			SyncPipeImgFrame		m_oPipeOutImageFrame;	///< outpipe 
            SyncPipeImgFrame		m_oPipeOutSetROI;	    ///< outpipe 

			interface::SmpTrafo		m_oSpTrafo;				///< roi translation
			geo2d::Rect				m_oRoi;					///< Roi
			image::Color			m_oColor;				///< Farbe des overlay

			precitec::geo2d::Rect m_croppedRoi;
			image::SmpBImage oSmpNewImage;
            image::SmpBImage oSmpImgROIParaSet;
			bool m_isCropped;
		};



	} // namespace filter
} // namespace precitec

#endif /*STARTENDROISELECTOR_H_*/
