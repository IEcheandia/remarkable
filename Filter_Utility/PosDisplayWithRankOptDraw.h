/**
 *  @file
 *  @copyright Precitec Vision GmbH & Co. KG
 *  @author    MM
 *  @date      2021
 *  @brief     This filter visualizes a position using overlay primitives. Analogue to PosDisplayWithRank with additional input to turn off the painting.
 */

#pragma once

#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/SynchronePipe.h>
#include <overlay/overlayCanvas.h>
#include <geo/geo.h>

namespace precitec
{
namespace filter
{

/**
 * @brief This filter visualizes a position using overlay primitives.
 */
class FILTER_API PosDisplayWithRankOptDraw : public fliplib::TransformFilter
{
public:
    PosDisplayWithRankOptDraw();

    virtual ~PosDisplayWithRankOptDraw();

    // Declare constants
    static const std::string m_oFilterName;      ///< Filter name.
    static const std::string m_oParamStyle;      ///< Parameter: Rendering style, e.g. crosshairs.
    static const std::string m_oParamColorRed;   ///< Parameter: Color of overlay primitives - red component.
    static const std::string m_oParamColorGreen; ///< Parameter: Color of overlay primitives - green component.
    static const std::string m_oParamColorBlue;  ///< Parameter: Color of overlay primitives - blue component.
    static const std::string m_oParamColorAlpha; ///< Parameter: Color of overlay primitives - alpha component.

    static const std::string m_oParamBadColorRed;   ///< Parameter: Color of overlay primitives for bad rank- red component.
    static const std::string m_oParamBadColorGreen; ///< Parameter: Color of overlay primitives for bad rank- green component.
    static const std::string m_oParamBadColorBlue;  ///< Parameter: Color of overlay primitives for bad rank- blue component.
    static const std::string m_oParamBadColorAlpha; ///< Parameter: Color of overlay primitives for bad rank- alpha component.
    static const std::string m_oParamRankThreshold; ///< Parameter: Rank Threshold

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

    const fliplib::SynchronePipe< interface::GeoDoublearray >* m_pPipeInDataX;    ///< Position x-coordinate in-pipe.
    const fliplib::SynchronePipe< interface::GeoDoublearray >* m_pPipeInDataY;    ///< Position y-coordinate in-pipe.
    const fliplib::SynchronePipe< interface::GeoDoublearray >* m_pPipeInDataDraw; ///< In-pipe if drawing is asked for.

    int  m_oStyle;                 ///< Parameter: Rendering style, e.g. crosshairs.
    image::Color m_oColorOK;       ///< Parameter: Color of overlay primitives.
    image::Color m_oColorKO;       ///< Parameter: Color of overlay primitive in case of bad rank
    image::Color m_oColor;         ///< Color to use in paint
    interface::SmpTrafo m_pTrafo;  ///< ROI trafo.
    interface::GeoPoint m_oPoint;  ///< Point that the filter is supposed
    int m_oRankThreshold;
};

}
}
