#include "ContourBufferRecorder.h"
#include <filter/buffer.h>
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{
using namespace interface;
const std::string ContourBufferRecorder::m_filterName("ContourBufferRecorder");
const std::string ContourBufferRecorder::m_paramSlot("Slot"); // Slot, into which the data is written.

ContourBufferRecorder::ContourBufferRecorder():
    TransformFilter(ContourBufferRecorder::m_filterName, Poco::UUID("873D4FDD-B21E-4BC6-98E1-0600F8568B31"))
    , m_pipeInData(nullptr)
    , m_pipeInPos(nullptr)
    , m_slot(1)
    , m_numTrigger(0)
    , m_triggerDelta(0)
    , m_seamSeries(0)
    , m_seam(0)
    , m_writeIndex(0)
{
    parameters_.add(m_paramSlot, fliplib::Parameter::TYPE_uint, m_slot);
    setInPipeConnectors({{Poco::UUID("1F47FAE8-8F53-4960-88CF-F0A21400BB05"), m_pipeInData, "Data", 1, "data"},
    {Poco::UUID("415C7765-53F5-45F8-BA67-6B95C4F4250E"), m_pipeInPos, "Position", 1, "pos"}});
    setVariantID(Poco::UUID("F5E7A6FE-7DF5-44A3-AFFE-726D122D3962"));
}

ContourBufferRecorder::~ContourBufferRecorder()
{

}

void ContourBufferRecorder::setParameter()
{
    TransformFilter::setParameter();
    m_slot = parameters_.getParameter(ContourBufferRecorder::m_paramSlot).convert<unsigned int>();
}

void ContourBufferRecorder::setProductInformationParameter()
{
    const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
    m_seamSeries = pProductData->m_oSeamSeries;
    m_seam = pProductData->m_oSeam;
    m_numTrigger = pProductData->m_oNumTrigger;
    m_triggerDelta = pProductData->m_oTriggerDelta;
}

void ContourBufferRecorder::arm(const fliplib::ArmStateBase& state)
{
    if (state.getStateID() == eSeamStart)
    {
        setProductInformationParameter();
        // reset / initialize the memory for the slot that was selected by the user ...
        auto& data = ContourBufferSingleton::getInstanceData();
        int numTriggerToStore = std::min(static_cast<int>(m_numTrigger), m_maxNumTriggers);
        data.init(m_slot, m_seamSeries, m_seam, numTriggerToStore);
        m_data = data.get(m_slot, m_seamSeries, m_seam);

        auto& pos = ContourBufferSingleton::getInstancePos();
        pos.init(m_slot, m_seamSeries, m_seam, numTriggerToStore);
        m_pos = pos.get(m_slot, m_seamSeries, m_seam);
        m_writeIndex = 0;
    }

    if(m_oVerbosity >= eHigh)
    {
        wmLog(eInfo, "ContourBufferRecorder - Filter '%s' armed at armstate %i\n", m_filterName.c_str(), state.getStateID());
    }
}

bool ContourBufferRecorder::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "data")
    {
        m_pipeInData = dynamic_cast<pipe_contour_t*>(&pipe);
    }
    if (pipe.tag() == "pos")
    {
        m_pipeInPos  = dynamic_cast<pipe_scalar_t*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);
}

void ContourBufferRecorder::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    poco_assert_dbg(m_pipeInData != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pipeInPos != nullptr);

    // checks, should never happen ...
    if (m_data->ref().size() == 0 || m_pos->ref().size() == 0)
    {
        // get product information for the debug message
        wmLog(eError, "ContourBufferRecorder - Buffer is not initialized, cannot write any elements [seamseries %u seam %u slot %u]!\n",
            m_seamSeries, m_seam, m_slot);
        preSignalAction();
        return;
    }

    auto& geoVecDPointarrayIn = m_pipeInData->read(m_oCounter);
    const GeoDoublearray& geoDoublePosIn = m_pipeInPos->read(m_oCounter);

    //overwrite previous position if necessary
    if (m_writeIndex > 0
        && geoDoublePosIn.ref().getData()[0] == m_pos->ref().getData()[m_writeIndex-1]
        && geoDoublePosIn.ref().getRank()[0] >= m_pos->ref().getRank()[m_writeIndex-1]
        && geoVecDPointarrayIn.ref()[0].getRank() >= m_data->ref()[m_writeIndex-1].getRank())
    {
        m_writeIndex--;
    }

    if (static_cast<int>(m_data->ref().size()) <= m_writeIndex || static_cast<int>(m_pos->ref().size()) <= m_writeIndex )
    {
        // get product information for the debug message
        wmLog(eError, "ContourBufferRecorder - Buffer is too small (%d), cannot write more elements (%d) [seamseries %u seam %u slot %u]!\n",
            std::min(m_data->ref().size(), m_pos->ref().size()), m_writeIndex, m_seamSeries, m_seam, m_slot);
        preSignalAction();
        return;
    }
    m_pos->ref()[m_writeIndex] = geoDoublePosIn.ref()[0];
    m_data->ref()[m_writeIndex] = geoVecDPointarrayIn.ref()[0];

    // some debug output
    if(m_oVerbosity >= eHigh)
    {
        auto& contour = m_data->ref()[m_writeIndex];
        wmLog(eInfo, "ContourBufferRecorder - Storing contour starting with %f %f (r:%d) at %f in slot %d element %d!\n",
            contour.getData()[0].x, contour.getData()[0].y, contour.getRank()[0], std::get<eData>(m_pos->ref()[m_writeIndex]), m_slot, m_writeIndex);
    }
    m_writeIndex++;
    preSignalAction();
}


} // namespace filter
} // namespace precitec
