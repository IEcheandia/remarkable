#include "dataSubsampling2.h"
#include "dataSubsampling.h"

#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{

DataSubsampling2::DataSubsampling2()
    : TransformFilter("DataSubsampling2", Poco::UUID{"cd5df271-5c5b-40fd-bfe9-d4cc5589b9cd"})
    , m_pPipeInData1(nullptr)
    , m_pPipeInData2(nullptr)
    , m_oPipeOutData1(this, "Data1")
    , m_oPipeOutData2(this, "Data2")
    , m_oOperation(0)
    , m_oPassThroughBadRank(false)
{
    parameters_.add("Operation", fliplib::Parameter::TYPE_int, m_oOperation);
    parameters_.add("PassThroughBadRank", fliplib::Parameter::TYPE_bool, m_oPassThroughBadRank);

    setInPipeConnectors({{Poco::UUID("2da6675c-5413-4837-a1d4-1e3584c81c80"), m_pPipeInData1, "Input1", 1, "input1"},
                         {Poco::UUID("4cc3a030-983d-4977-ae08-af1476ae72b5"), m_pPipeInData2, "Input2", 1, "input2"}});
    setOutPipeConnectors({
        {Poco::UUID("2b13e971-2d78-4e50-af1c-8cf5986e32c5"), &m_oPipeOutData1, "Data1"},
        {Poco::UUID("90986c64-ca6e-401e-8ffe-d9679e462d8d"), &m_oPipeOutData2, "Data2"}
    });
    setVariantID(Poco::UUID("565fea93-a42b-41f6-950d-5be7d56ec155"));
}

void DataSubsampling2::setParameter()
{
    TransformFilter::setParameter();

    m_oOperation = parameters_.getParameter("Operation").convert<int>();
    m_oPassThroughBadRank = parameters_.getParameter("PassThroughBadRank").convert<bool>();
}

bool DataSubsampling2::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    const auto& inPipes = inPipeConnectors();

    if (p_rPipe.tag() == inPipes[0].tag())
    {
        m_pPipeInData1 = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == inPipes[1].tag())
    {
        m_pPipeInData2 = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
    }
    else
    {
        return false;
    }
    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void DataSubsampling2::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    poco_assert_dbg(m_pPipeInData1 != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInData2 != nullptr); // to be asserted by graph editor

    const auto& geoDoubleArrayIn1 = m_pPipeInData1->read(m_oCounter);
    const auto geoDoubleOut1 = DataSubsampling::subsample(geoDoubleArrayIn1, static_cast<SubsamplingOperation>(m_oOperation), m_oPassThroughBadRank);

    const auto& geoDoubleArrayIn2 = m_pPipeInData2->read(m_oCounter);
    const auto geoDoubleOut2 = DataSubsampling::subsample(geoDoubleArrayIn2, static_cast<SubsamplingOperation>(m_oOperation), m_oPassThroughBadRank);

    // send the data out ...
    preSignalAction();
    m_oPipeOutData1.signal(geoDoubleOut1);
    m_oPipeOutData2.signal(geoDoubleOut2);
}

}
}
