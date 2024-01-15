/*!
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			KIR, WoR, HS
 *  @date			2010-2011
 *  @file
 *  @brief			Fliplib filter 'RoiSelector' in component 'Filter_ImageProcessing'. Clips image to roi dimension.
 */

#ifndef ROISELECTOR_H_
#define ROISELECTOR_H_

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
 * Schneidet aus einem ImageFrame den Region of intrests aus und sendet eine neue ImageFrame an den
 * naechsten Filter
 */
class FILTER_API ROISelector : public fliplib::TransformFilter
{
public:
	static const std::string m_oFilterName;
	static const std::string PIPENAME1;

	ROISelector();

	void setParameter();							///< parameter setzen
	void paint();									///< Zeichnet die ROI in Overlay
	void arm (const fliplib::ArmStateBase& state);	///< arm filter

private:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& pipe, int group);

	/// Verarbeitungsmethode IN Pipe
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

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
	SyncPipeImgFrame		m_oPipeOutImageFrame;	///< outpipe 

	interface::SmpTrafo		m_oSpTrafo;				///< roi translation
	geo2d::Rect				m_oRoi;					///< Roi
	image::Color			m_oColor;				///< Farbe des overlay

	image::SmpBImage oSmpNewImage;
};



} // namespace filter
} // namespace precitec

#endif /*ROISELECTOR_H_*/
