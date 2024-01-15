/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2014
 * 	@brief		This filter visualizes a position using overlay primitives.
 */

#ifndef POS_DISPLAY_H_
#define POS_DISPLAY_H_

// WM includes
#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/SynchronePipe.h>
#include <overlay/overlayCanvas.h>
#include <geo/geo.h>

namespace precitec {
namespace filter {

/**
 * @brief This filter visualizes a position using overlay primitives.
 */
class FILTER_API PosDisplay : public fliplib::TransformFilter
{
public:

	/**
	 * CTor.
	 */
	PosDisplay();
	/**
	 * @brief DTor.
	 */
	virtual ~PosDisplay();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oParamStyle;			///< Parameter: Rendering style, e.g. crosshairs.
	static const std::string m_oParamColorRed;		///< Parameter: Color of overlay primitives - red component.
	static const std::string m_oParamColorGreen;	///< Parameter: Color of overlay primitives - green component.
	static const std::string m_oParamColorBlue;		///< Parameter: Color of overlay primitives - blue component.

    //paint routines
    static void paintCrosshairMedium(image::OverlayLayer &rLayer, const geo2d::Point & rPoint, const image::Color & rColor, const interface::Trafo & rTrafo);
    static void paintCrosshairLarge(image::OverlayLayer &rLayer, const geo2d::Point & rPoint, const image::Color & rColor, const interface::Trafo & rTrafo);


	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	void paint();

protected:

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/**
	 * @brief Processing routine.
	 * @param p_pSender
	 * @param p_rEvent
	 */
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent);

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInDataX;			///< Position x-coordinate in-pipe.
	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInDataY;			///< Position y-coordinate in-pipe.

	int 														m_oStyle;				///< Parameter: Rendering style, e.g. crosshairs.
	image::Color												m_oColor;				///< Parameter: Color of overlay primitives.
	interface::SmpTrafo											m_pTrafo;				///< ROI trafo.
	interface::GeoPoint 										m_oPoint;				///< Point that the filter is supposed

}; // class PosDisplay

} // namespace filter
} // namespace precitec

#endif // POS_DISPLAY_H_
