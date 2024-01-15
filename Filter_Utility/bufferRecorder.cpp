#include "bufferRecorder.h"
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

const std::string BufferRecorder::m_filterName("BufferRecorder");
const std::string BufferRecorder::m_paramSlot("Slot");


BufferRecorder::BufferRecorder() :
    TransformFilter(BufferRecorder::m_filterName, Poco::UUID{"D36E075D-3BA4-46FB-8682-6E21FE70741A"})
    , m_pipeInPos(nullptr)
    , m_pipeInData(nullptr)
    , m_slot(1)
    , m_numTrigger(0)
    , m_seamSeries(0)
    , m_seam(0)
    , m_writeIndex(0)
{
    parameters_.add(m_paramSlot, fliplib::Parameter::TYPE_uint, m_slot);
    setInPipeConnectors({{Poco::UUID("69988F6D-EDF7-470A-B6A7-DFF9AAE663B0"), m_pipeInData, "Data", 1024, "data"},
    {Poco::UUID("1716DE0E-5BDF-4E93-930E-9524C42E6C34"), m_pipeInPos, "Position", 1024, "pos"}});
    setVariantID(Poco::UUID("2D2D51FD-625D-4EF2-BDF6-C742C30E9DF0"));
}

BufferRecorder::~BufferRecorder()
{
}

void BufferRecorder::setParameter()
{
    TransformFilter::setParameter();
    m_slot = parameters_.getParameter(BufferRecorder::m_paramSlot).convert<unsigned int>();
}

void BufferRecorder::setProductInformationParameter()
{
    const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
    m_triggerDelta = pProductData->m_oTriggerDelta;
    m_numTrigger = pProductData->m_oNumTrigger;
    m_seamSeries = pProductData->m_oSeamSeries;
    m_seam = pProductData->m_oSeam;
}

int BufferRecorder::getDataSize()
{
    return m_data->ref().size();
}

int BufferRecorder::getPosSize()
{
    return m_pos->ref().size();
}

void BufferRecorder::arm(const fliplib::ArmStateBase& state)
{
    if (state.getStateID() == eSeamStart)
    {
        const unsigned int maxNumTrigger = 100000;
        setProductInformationParameter();
        // initialize the memory for the slot that was selected by the user ...
        Buffer& data = BufferSingleton::getInstanceData();
        if (m_numTrigger < maxNumTrigger)
        {
            data.init(m_slot, m_seamSeries, m_seam, m_numTrigger);
        }
        else
        {
            data.init(m_slot, m_seamSeries, m_seam, maxNumTrigger);
        }
        m_data = data.get(m_slot, m_seamSeries, m_seam);

        Buffer& pos = BufferSingleton::getInstancePos();
        if (m_numTrigger < maxNumTrigger)
        {
            pos.init(m_slot, m_seamSeries, m_seam, m_numTrigger);
        }
        else
        {
            pos.init(m_slot, m_seamSeries, m_seam, maxNumTrigger);
        }
        m_pos = pos.get(m_slot, m_seamSeries, m_seam);
        m_writeIndex = 0;
    }

    if(m_oVerbosity >= eHigh)
    {
        wmLog(eInfo, "BufferRecorder - Filter '%s' armed at armstate %i\n", m_filterName.c_str(), state.getStateID());
    }
}

bool BufferRecorder::subscribe(fliplib::BasePipe& pipe, int group)
{
    using namespace interface;
    if (pipe.tag() == "data")
    {
        m_pipeInData = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(&pipe);
    }
    if (pipe.tag() == "pos")
    {
        m_pipeInPos = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);

}

void BufferRecorder::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    using namespace interface;
    if (m_pipeInData == nullptr)
    {
        wmLog(eError, "BufferRecorder - No incoming data, cannot write any elements [slot %d]!\n", m_slot);
        preSignalAction();
        return;
    }
    // checks, should never happen ...
    if (m_data == std::shared_ptr<interface::GeoDoublearray>() || m_pos == std::shared_ptr<interface::GeoDoublearray>())
    {
        // get product information for the debug message
        wmLog(eError, "BufferRecorder - Buffer is not initialized, cannot write any elements [seamserie %d seam %d slot %d]!\n",
        m_seamSeries, m_seam, m_slot);
        preSignalAction();
        return;
    }

    const GeoDoublearray& geoDoubleArrayIn = m_pipeInData->read(m_oCounter);
    // position information - do we have actual information from the in-pipe, or do we simply calculate the position based on the time?
    //overwrite previous position if necessary

    double pos = m_oCounter * m_triggerDelta;
    int rank = eRankMax;

    if ( m_pipeInPos != nullptr )
    {
        const GeoDoublearray &rGeoDoublePosIn  = m_pipeInPos->read(m_oCounter);
        std::tie(pos, rank) = rGeoDoublePosIn.ref()[0];
    }

    //overwrite previous position if necessary

    if (m_writeIndex > 0
        && pos == m_pos->ref().getData()[m_writeIndex-1]
        && rank >= m_pos->ref().getRank()[m_writeIndex-1]
        && geoDoubleArrayIn.ref().getRank()[0] >= m_data->ref().getRank()[m_writeIndex-1] )
    {
        m_writeIndex--;
    }

    if (getDataSize() <= m_writeIndex || getPosSize() <= m_writeIndex)
    {
        // get product information for the debug message
        wmLog(eError, "BufferRecorder - Buffer is too small (%d), cannot write more elements (%d) [seamserie %u seam %u slot %u]!\n",
            std::min(getDataSize(), getPosSize()), m_writeIndex, m_seamSeries, m_seam, m_slot);
        preSignalAction();
        return;
    }

    m_pos->ref()[m_writeIndex] = std::tie(pos, rank);
    m_data->ref()[m_writeIndex] = geoDoubleArrayIn.ref()[0];
    // some debug output
    if(m_oVerbosity >= eHigh)
    {
        wmLog(eInfo, "BufferRecorder - Storing %f (r:%d) at %f in slot %u element %u!\n",
            std::get<eData>(m_data->ref()[m_writeIndex]),
            std::get<eRank>(m_data->ref()[m_writeIndex]),
            std::get<eData>(m_pos->ref()[m_writeIndex]),
            m_slot,
            m_writeIndex
            );
    }

    // ok, we are done here, lets increase write-index
    m_writeIndex++;
    preSignalAction();
}


} // namespace filter
} // namespace precitec
