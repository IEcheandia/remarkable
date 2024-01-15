
#include "bufferPlayer.h"
#include <filter/buffer.h>
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <image/image.h>
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{

const std::string BufferPlayer::m_filterName("BufferPlayer");
const std::string BufferPlayer::m_pipeOutName("Data");
const std::string BufferPlayer::m_paramSlot("Slot");
const std::string BufferPlayer::m_paramDataOffset("DataOffset");
const std::string BufferPlayer::m_paramSeamSeriesOffset("SeamSeriesOffset");
const std::string BufferPlayer::m_paramSeamOffset("SeamOffset");


BufferPlayer::BufferPlayer():
    TransformFilter(BufferPlayer::m_filterName, Poco::UUID{"2C49DC9E-9216-414A-A134-A0CBF89FD36D"})
    , m_pipeInImage(nullptr)
    , m_pipeInPos(nullptr)
    , m_pipeOutData(this, BufferPlayer::m_pipeOutName)
    , m_slot(1)
    , m_dataOffset(0)
    , m_seamSeriesOffset(0)
    , m_seamOffset(0)
    , m_requestedSeamSeries(0)
    , m_requestedSeam(0)
    , m_image(0)
    , m_count(0)
    , m_triggerDelta(1000)
{
    parameters_.add(m_paramSlot, fliplib::Parameter::TYPE_uint, m_slot);
    parameters_.add(m_paramDataOffset, fliplib::Parameter::TYPE_int, m_dataOffset);
    parameters_.add(m_paramSeamSeriesOffset, fliplib::Parameter::TYPE_int, m_seamSeriesOffset);
    parameters_.add(m_paramSeamOffset, fliplib::Parameter::TYPE_int, m_seamOffset);

    setInPipeConnectors({{Poco::UUID("540CFF31-B5FE-4EF6-B30D-FE7A5AD5910D"), m_pipeInImage, "Image", 1024, "image"},
    {Poco::UUID("62D61E83-0AFF-4B38-9D51-B9B5DCAF8DD6"), m_pipeInPos, "Position", 1024, "pos"}});
    setOutPipeConnectors({{Poco::UUID("CCEB7DE0-DEC2-49B2-BC2A-30BC5A947E10"), &m_pipeOutData, m_pipeOutName, 0, "data"}});
    setVariantID(Poco::UUID("04D19F78-3FCF-4443-88C5-E63D8E9249D1"));
}

BufferPlayer::~BufferPlayer()
{
}

void BufferPlayer::setParameter()
{
    TransformFilter::setParameter();
    m_slot = parameters_.getParameter(BufferPlayer::m_paramSlot).convert<unsigned int>();
    m_dataOffset = parameters_.getParameter(BufferPlayer::m_paramDataOffset).convert<int>();
    m_seamSeriesOffset = parameters_.getParameter(BufferPlayer::m_paramSeamSeriesOffset).convert<int>();
    m_seamOffset = parameters_.getParameter(BufferPlayer::m_paramSeamOffset).convert<int>();
    m_image = m_dataOffset / m_triggerDelta;
}

void BufferPlayer::setProductInformationParameter()
{
        const analyzer::ProductData* productData = externalData<analyzer::ProductData>();
        m_triggerDelta = productData->m_oTriggerDelta;
        m_seamSeries = productData->m_oSeamSeries;
        m_seam = productData->m_oSeam;
        m_requestedSeamSeries = m_seamSeries + m_seamSeriesOffset;
        m_requestedSeam = m_seam + m_seamOffset;
}

void BufferPlayer::arm(const fliplib::ArmStateBase& state)
{
    if (state.getStateID() == eSeamStart)
    {
        setProductInformationParameter();

        //seamSerie offset correct for current serie?
        if (m_requestedSeamSeries < 0)
        {
            m_ready = false;
            wmLog(eDebug, "BufferPlayer::arm - seamserie-offset not correct: seamserie %d + seamseriesoffset %d = %d! [slot %d]\n",
                m_seamSeries,
                m_seamSeriesOffset,
                m_requestedSeamSeries,
                m_slot);
        }

        // seam-offset correct for current seam?
        if (m_requestedSeam < 0)
        {
            m_ready = false;
            wmLog(eDebug, "BufferPlayer::arm - seam-offset not correct: seam %d + seamoffset %d = %d! [slot %d]\n",
                m_seam,
                m_seamOffset,
                m_requestedSeam,
                m_slot);
        }

        // reset / initialize the memory for the slot that was selected by the user ...
        Buffer& data = BufferSingleton::getInstanceData();
        Buffer& pos = BufferSingleton::getInstancePos();
        if (!data.exists(m_slot, m_requestedSeamSeries, m_requestedSeam) ||
            !pos.exists(m_slot, m_requestedSeamSeries, m_requestedSeam))
        {
            m_ready = false;
            wmLog(eDebug, "BufferPlayer::arm - buffer does not exist! [seamseries %u seamseriesoffset %d seam %u seamoffset %d slot %d]\n",
                m_seamSeries,
                m_seamSeriesOffset,
                m_seam,
                m_seamOffset,
                m_slot);
        }
        else
        {
            m_data = data.get(m_slot, m_requestedSeamSeries, m_requestedSeam);
            m_pos = pos.get(m_slot, m_requestedSeamSeries, m_requestedSeam);
            m_count = 0;
            m_image = 0;
            m_ready = true;
        }
    }

    if(m_oVerbosity >= eHigh)
    {
        // get product information for the debug message
        wmLog(eInfo, "BufferPlayer - Filter '%s' armed at armstate %i [req seamseries %d req seam %d slot %u]\n",
            m_filterName.c_str(),
            state.getStateID(),
            m_requestedSeamSeries,
            m_requestedSeam,
            m_slot);
    }

}

bool BufferPlayer::subscribe(fliplib::BasePipe& pipe, int group)
{
    using namespace interface;
    if (pipe.tag() == "image")
    {
        m_pipeInImage = dynamic_cast< fliplib::SynchronePipe<ImageFrame>*>(&pipe);
    }
    if (pipe.tag() == "pos")
    {
        m_pipeInPos  = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);
}



void BufferPlayer::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    using namespace interface;
    poco_assert_dbg(m_pipeInImage != nullptr); // to be asserted by graph editor

    // is the buffer ready?
    const ImageFrame& frame(m_pipeInImage->read(m_oCounter));
    const geo2d::Doublearray outDummy{1, 0, eRankMin};
    GeoDoublearray geoDoubleOut(frame.context(), outDummy, frame.analysisResult(), NotPresent); // bad rank

    if (!m_ready)
    {
        wmLog(eWarning, "BufferPlayer not ready (req seamseries %d req seam %d, slot %d)\n",
            m_requestedSeamSeries,
            m_requestedSeam,
            m_slot);

        // send dummy data out ...
        preSignalAction();
        m_pipeOutData.signal(geoDoubleOut);
        return;
    }
    m_image = frame.context().imageNumber();

    double posBufPrev = 0.0; // position that was stored by the recorder, perhaps slightly before the position we would like to access here.
    double posBufNext = 0.0; // position that was stored by the recorder, perhaps slightly behind the position we would like to access here.
    double pos = 0.0;
    if ( m_pipeInPos == nullptr )
    {
        pos = double(m_image * m_triggerDelta);
    }
    else
    {
        // position comes from an external encoder
        const GeoDoublearray& geoDoubleArrayIn = m_pipeInPos->read(m_oCounter);
        pos = std::get<eData>(geoDoubleArrayIn.ref()[0]);
    }
    pos += m_dataOffset;
    // find the read-index, based on last iteration and position array
    m_count = 0;
    std::vector<double>& posData = m_pos->ref().getData();
    for(auto iter = posData.begin(); iter != posData.end(); iter++)
    {
        // did we find the position? // only positiv positions :) may use Encoder "mal -1" to avoid this
        if ((*iter - pos) >= 0)
        {
            posBufNext = *iter;
            break;
        }
        else
        {
            posBufPrev = *iter;
            m_count++;
        }
    }
    // get the actual information at this position.
    double data = 0.0; // This is what we are looking for, the final, interpolated data value.
    double dataBufPrev = 0.0; // Data value, stored by the recorder, perhaps slightly before the position we want to access here.
    double rankBufPrev = eRankMin; // Corresponding rank.
    double dataBufNext = 0.0; // Data value, stored by the recorder, perhaps slightly behind the position we want to access here.
    double rankBufNext = eRankMin; // Corresponding rank.

    if (m_data->ref().size() > m_count)
    {
        if (m_count > 0)
        {
            dataBufPrev = std::get<eData>(m_data->ref()[m_count-1]);
            rankBufPrev = std::get<eRank>(m_data->ref()[m_count-1]);
        }
        else
        {
            dataBufPrev = std::get<eData>(m_data->ref()[m_count]);
            rankBufPrev = std::get<eRank>(m_data->ref()[m_count]);
        }

        dataBufNext = std::get<eData>(m_data->ref()[m_count]);
        rankBufNext = std::get<eRank>(m_data->ref()[m_count]);
    }
    else
    {
        // get product information for the debug message
        wmLog(eDebug, "BufferPlayer - cannot find any data for position %f! [req seamseries %d req seam %d slot %u]\n",
            pos,
            m_requestedSeamSeries,
            m_requestedSeam,
            m_slot);
        preSignalAction();
        m_pipeOutData.signal(geoDoubleOut);
        return;
    }

    // store final data element in the output array.
    if (posBufNext == posBufPrev)
    {
        posBufNext += 0.01;
    }
    double left = double(pos - posBufPrev) / double(posBufNext - posBufPrev);
    data = (dataBufNext * left) + (dataBufPrev * (1.0-left));

    // compute rank
    int rank = static_cast<int>(std::min(rankBufPrev, rankBufNext));
    // store result
    geo2d::Doublearray out;
    out.assign(1);
    out[0] = std::tie(data, rank);

    // some debug output
    if(m_oVerbosity >= eHigh)
    {
        wmLog(eInfo, "BufferPlayer - Reading from slot %d element %d / %d: %f (r:%d)!\n", m_slot, m_count, m_data->ref().size(), std::get<eData>(out[0]), std::get<eRank>(out[0]) );
    }

    // send the data out ...
    geoDoubleOut = GeoDoublearray(frame.context(), out, frame.analysisResult(), (double)(rank) / 255.0);
    preSignalAction();
    m_pipeOutData.signal(geoDoubleOut);
}

} // namespace filter
} // namespace precitec

