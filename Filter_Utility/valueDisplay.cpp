#include "valueDisplay.h"
#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <regex>

#define FILTER_NAME "ValueDisplay"


namespace precitec
{
namespace filter
{

ValueDisplay::ValueDisplay()
    : TransformFilter(FILTER_NAME, Poco::UUID("8fa043cf-03e9-4527-83cd-ed5c01b04f80"))
    , m_pPipeInDataX(nullptr)
    , m_pPipeInDataY(nullptr)
    , m_pPipeInDataValue(nullptr)
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
        {Poco::UUID("10189ac5-c40e-4cb0-a0af-99f9d8e0d998"), m_pPipeInDataX, "PositionX", 1, "pos_x"},
        {Poco::UUID("768882bc-5928-4817-aaaf-cead43b195a7"), m_pPipeInDataY, "PositionY", 1, "pos_y"},
        {Poco::UUID("9f8fec8c-d4eb-4875-ace4-f7f2806dc2f2"), m_pPipeInDataValue, "Value", 1, "value"}
    });
    setVariantID(Poco::UUID("8378c880-ea6a-4ca4-ae88-2d00b38c1f98"));
}

void ValueDisplay::setParameter()
{
    TransformFilter::setParameter();

    m_textPattern = parameters_.getParameter("TextPattern").convert<std::string>();
    m_textColor.red = parameters_.getParameter("Red").convert<byte>();
    m_textColor.green = parameters_.getParameter("Green").convert<byte>();
    m_textColor.blue = parameters_.getParameter("Blue").convert<byte>();
    m_displayValueInLog = parameters_.getParameter("DisplayValueInLog").convert<bool>();
}

bool ValueDisplay::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
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
    return BaseFilter::subscribe( p_rPipe, p_oGroup );
}

void ValueDisplay::paint()
{
    if (m_oVerbosity == eNone || m_pTrafo.isNull() || std::isnan(m_value))  // filter should not paint anything on verbosity eNone
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
        oMsg << FILTER_NAME << ": " << text << " [" << (int)m_pos_x << ", " << (int)m_pos_y << "]" << std::endl;
        wmLog(eInfo, oMsg.str());
    }
}

void ValueDisplay::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent)
{
    poco_assert_dbg(m_pPipeInDataX != nullptr);
    poco_assert_dbg(m_pPipeInDataY != nullptr);
    poco_assert_dbg(m_pPipeInDataY != nullptr);

    if(m_oVerbosity == eNone)
    {
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
        oMsg << FILTER_NAME << ": Different contexts for x and y value: '" << rGeoPosXIn.context() << "', '" << rGeoPosYIn.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }

    m_pTrafo = rGeoPosXIn.context().trafo();

    m_pos_x = !rPosXIn.getData().empty() ? rPosXIn.getData().front() : 0;
    m_pos_y = !rPosYIn.getData().empty() ? rPosYIn.getData().front() : 0;
    m_value = !rValueIn.getData().empty() ? rValueIn.getData().front() : std::numeric_limits<double>::quiet_NaN();

    preSignalAction();
}

} //namespace filter
} //namespace precitec
