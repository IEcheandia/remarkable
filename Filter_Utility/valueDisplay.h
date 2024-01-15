#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "overlay/color.h"
#include "geo/geo.h"

namespace precitec
{
namespace filter
{

class FILTER_API ValueDisplay : public fliplib::TransformFilter
{
public:
    ValueDisplay();

    void setParameter();
    void paint();

    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent);

private:
    const fliplib::SynchronePipe< precitec::interface::GeoDoublearray >* m_pPipeInDataX;
    const fliplib::SynchronePipe< precitec::interface::GeoDoublearray >* m_pPipeInDataY;
    const fliplib::SynchronePipe< precitec::interface::GeoDoublearray >* m_pPipeInDataValue;

    std::string m_textPattern;
    image::Color m_textColor;
    bool m_displayValueInLog;

    interface::SmpTrafo m_pTrafo;
    double m_pos_x;
    double m_pos_y;
    double m_value;
};

}
}
