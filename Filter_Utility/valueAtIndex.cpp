#include "valueAtIndex.h"
#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#define FILTER_ID            "32d9f6da-e550-4962-b6b6-88b593c72523"
#define VARIANT_ID           "ecb0ad62-eeaa-46bc-8907-1922fef96c5e"

#define PIPE_ID_ARRAYIN      "1363d97f-3d7c-4442-849a-69f43d0fbf54"
#define PIPE_ID_INDEXIN      "5c2a4514-b3e9-4474-9132-0c3a1e913418"
#define PIPE_ID_VALUEOUT     "1f5a8109-d6e6-403e-88e8-46dc026c4875"

namespace precitec
{
namespace filter
{

ValueAtIndex::ValueAtIndex()
    : TransformFilter("ValueAtIndex", Poco::UUID(FILTER_ID))
    , m_arrayIn(nullptr)
    , m_indexIn(nullptr)
    , m_valueOut(this, "ValueOut")
{
    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_ARRAYIN), m_arrayIn, "ArrayIn", 1, "ArrayIn"},
        {Poco::UUID(PIPE_ID_INDEXIN), m_indexIn, "IndexIn", 1, "IndexIn"},
    });
    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_VALUEOUT), &m_valueOut, "ValueOut", 1, "ValueOut"},
    });
    setVariantID(Poco::UUID(VARIANT_ID));
}

void ValueAtIndex::setParameter()
{
    TransformFilter::setParameter();
}

bool ValueAtIndex::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ArrayIn")
    {
        m_arrayIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == "IndexIn")
    {
        m_indexIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else
    {
        return false;
    }

    return BaseFilter::subscribe(pipe, group);
}

void ValueAtIndex::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto array = m_arrayIn->read(m_oCounter);
    const auto indexIn = m_indexIn->read(m_oCounter);

    interface::GeoDoublearray valueOut = array; // copy

    int index = indexIn.ref().empty() ? 0 : indexIn.ref().getData()[0];
    index = index >= 0 ? index : array.ref().size() + index;

    valueOut.ref().resize(1);

    if (index < 0 || index >=  (int)array.ref().size()) // out of range!
    {
        valueOut.ref().getData()[0] = 0.0;
        valueOut.ref().getRank()[0] = 0;
        valueOut.rank(0.0);
    }
    else
    {
        valueOut.ref().getData()[0] = array.ref().getData()[index];
        valueOut.ref().getRank()[0] = array.ref().getRank()[index];
    }

    preSignalAction();
    m_valueOut.signal(valueOut);
}

} //namespace filter
} //namespace precitec
