#include "valueDisplayOnOff.h"
#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <regex>

namespace precitec
{
namespace filter
{

const std::string ValueDisplayOnOff::m_filter_name("ValueDisplayOnOff");

ValueDisplayOnOff::ValueDisplayOnOff()
    : TransformFilter(m_filter_name, Poco::UUID("578a983a-8858-4edb-ad09-d3922b8671d1"))
    , m_pPipeInDataX(nullptr)
    , m_pPipeInDataY(nullptr)
    , m_pPipeInDataValue(nullptr)
    , m_pPipeInDataOnOff(nullptr)
    , m_textPattern("")
    , m_textColor(image::Color::Green())
    , m_displayValueInLog(false)
    , m_pos_x(0)
    , m_pos_y(0)
    , m_value(0)
{
    parameters_.add("TextPattern", fliplib::Parameter::TYPE_string, m_textPattern);
    parameters_.add("Red", fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_textColor.red));
    parameters_.add("Green", fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_textColor.green));
    parameters_.add("Blue", fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_textColor.blue));
    parameters_.add("DisplayValueInLog", fliplib::Parameter::TYPE_bool, static_cast<bool>(m_displayValueInLog));

    setInPipeConnectors({
        {Poco::UUID("b15d4aef-dfbf-4a6e-ba25-b669776edf6a"), m_pPipeInDataX, "PositionX", 1, "pos_x"},
        {Poco::UUID("9341898d-396d-4d24-aa8b-3de2b6fc1d0c"), m_pPipeInDataY, "PositionY", 1, "pos_y"},
        {Poco::UUID("e688daa1-bb9a-41d3-a899-a7cca56ad312"), m_pPipeInDataValue, "Value", 1, "value"},
        {Poco::UUID("3229ccef-8f44-40d9-b59d-b39921789080"), m_pPipeInDataOnOff, "OnOff", 1, "onOff"}
    });
    setVariantID(Poco::UUID("381ca026-8917-4c50-b69c-ee5d7b31a52e"));
}

void ValueDisplayOnOff::setParameter()
{
    TransformFilter::setParameter();

    m_textPattern = parameters_.getParameter("TextPattern").convert<std::string>();
    m_textColor.red = parameters_.getParameter("Red").convert<byte>();
    m_textColor.green = parameters_.getParameter("Green").convert<byte>();
    m_textColor.blue = parameters_.getParameter("Blue").convert<byte>();
    m_displayValueInLog = parameters_.getParameter("DisplayValueInLog").convert<bool>();
}

bool ValueDisplayOnOff::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if (p_rPipe.tag() == "pos_x")
    {
        m_pPipeInDataX = dynamic_cast< fliplib::SynchronePipe < precitec::interface::GeoDoublearray > * >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "pos_y")
    {
        m_pPipeInDataY = dynamic_cast< fliplib::SynchronePipe < precitec::interface::GeoDoublearray > * >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "value")
    {
        m_pPipeInDataValue = dynamic_cast< fliplib::SynchronePipe < precitec::interface::GeoDoublearray > * >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "onOff")
    {
        m_pPipeInDataOnOff = dynamic_cast< fliplib::SynchronePipe < precitec::interface::GeoDoublearray > * >(&p_rPipe);
    }
    return BaseFilter::subscribe( p_rPipe, p_oGroup );
}

void ValueDisplayOnOff::paint()
{
    if (m_oVerbosity == eNone || m_pTrafo.isNull() || std::isnan(m_value) || !m_valueDisplayOn)  // filter should not paint anything on verbosity eNone
    {
        return;
    }

    const interface::Trafo &rTrafo (*m_pTrafo);
    image::OverlayCanvas &rCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer &rLayer (rCanvas.getLayerText());

    std::ostringstream value_string;
    value_string.precision(3);
    value_string << std::fixed << m_value;
    std::regex value_exp("%%");
    std::string text = std::regex_replace(m_textPattern, value_exp, value_string.str());
    rLayer.add<image::OverlayText>(text, image::Font(16),
        rTrafo(geo2d::Point((int)m_pos_x, (int)m_pos_y)), m_textColor);

    if (m_displayValueInLog)
    {
        std::ostringstream oMsg;
        oMsg << m_filter_name << ": " << text << " [" << (int)m_pos_x << ", " << (int)m_pos_y << "]" << std::endl;
        wmLog(eInfo, oMsg.str());
    }
}

void ValueDisplayOnOff::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent)
{
    if(m_oVerbosity == eNone)
    {
        preSignalAction();
        return;
    }

    m_valueDisplayOn = true;
    const auto& rOnOff = m_pPipeInDataOnOff->read(m_oCounter).ref().getData();
    if (rOnOff.empty() || rOnOff.front() == 0)
    {
        m_valueDisplayOn = false;
        preSignalAction();
        return;
    }

    const interface::GeoDoublearray &rGeoPosXIn = m_pPipeInDataX->read(m_oCounter);
    const interface::GeoDoublearray &rGeoPosYIn = m_pPipeInDataY->read(m_oCounter);
    const interface::GeoDoublearray &rGeoValueIn = m_pPipeInDataValue->read(m_oCounter);

    const geo2d::Doublearray &rPosXIn = rGeoPosXIn.ref();
    const geo2d::Doublearray &rPosYIn = rGeoPosYIn.ref();
    const geo2d::Doublearray &rValueIn = rGeoValueIn.ref();

    if (rGeoPosXIn.context() != rGeoPosYIn.context())
    { // contexts expected to be equal
        std::ostringstream oMsg;
        oMsg << m_filter_name << ": Different contexts for x and y value: '" << rGeoPosXIn.context() << "', '" << rGeoPosYIn.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }

    m_pTrafo = rGeoPosXIn.context().trafo();

    m_pos_x = !rPosXIn.getData().empty() ? rPosXIn.getData().front() : 0;
    m_pos_y = !rPosYIn.getData().empty() ? rPosYIn.getData().front() : 0;
    m_value = !rValueIn.getData().empty() ? rValueIn.getData().front() : std::numeric_limits<double>::quiet_NaN();

    preSignalAction();
}

}
}
