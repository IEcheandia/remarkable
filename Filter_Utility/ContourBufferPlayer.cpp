#include "ContourBufferPlayer.h"
#include <numeric>
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
using namespace interface;
using namespace image;

const std::string ContourBufferPlayer::m_filterName("ContourBufferPlayer");
const std::string ContourBufferPlayer::m_pipeOutName("Data");
const std::string ContourBufferPlayer::m_paramSlot("Slot");
const std::string ContourBufferPlayer::m_paramDataOffset("DataOffset");
const std::string ContourBufferPlayer::m_paramSeamSeriesOffset("SeamSeriesOffset");
const std::string ContourBufferPlayer::m_paramSeamOffset("SeamOffset");


ContourBufferPlayer::ContourBufferPlayer():
    TransformFilter(ContourBufferPlayer::m_filterName, Poco::UUID("94C87352-0BF4-4230-A055-E72E1B02C1E2"))
    , m_pipeInImage(nullptr)
    , m_pipeInPos(nullptr)
    , m_pipeOutData(this, ContourBufferPlayer::m_pipeOutName)
    , m_slot(1)
    , m_dataOffset(0)
    , m_seamSeriesOffset(0)
    , m_seamOffset(0)
    , m_seamSeries(0)
    , m_seam(0)
    , m_requestedSeamSeries(0)
    , m_requestedSeam(0)
{
    parameters_.add(m_paramSlot, fliplib::Parameter::TYPE_uint, m_slot);
    parameters_.add(m_paramDataOffset, fliplib::Parameter::TYPE_int, m_dataOffset);
    parameters_.add(m_paramSeamSeriesOffset, fliplib::Parameter::TYPE_int, m_seamSeriesOffset);
    parameters_.add(m_paramSeamOffset, fliplib::Parameter::TYPE_int, m_seamOffset);

    setInPipeConnectors({{Poco::UUID("A49A6CA5-F357-4C1C-B235-FEFE5B8BBCC8"), m_pipeInImage, "Image", 1, "image"},
    {Poco::UUID("C96E032A-82CA-415D-A169-955F69817ECB"), m_pipeInPos, "Position", 1, "pos"}});
    setOutPipeConnectors({{Poco::UUID("B815B87B-293E-4D11-852D-F752F5065D93"), &m_pipeOutData, m_pipeOutName, 0, "data"}});
    setVariantID(Poco::UUID("355EB4D9-A094-4038-9606-AD771A162AA6"));
}

ContourBufferPlayer::~ContourBufferPlayer()
{
}

void ContourBufferPlayer::setParameter()
{
    TransformFilter::

    setParameter();
    m_slot = parameters_.getParameter(ContourBufferPlayer::m_paramSlot).convert<unsigned int>();
    m_dataOffset = parameters_.getParameter(ContourBufferPlayer::m_paramDataOffset).convert<int>();
    m_seamSeriesOffset = parameters_.getParameter(ContourBufferPlayer::m_paramSeamSeriesOffset).convert<int>();
    m_seamOffset = parameters_.getParameter(ContourBufferPlayer::m_paramSeamOffset).convert<int>();
}

void ContourBufferPlayer::setProductInformationParameter()
{
    const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
    m_seamSeries = pProductData->m_oSeamSeries;
    m_seam = pProductData->m_oSeam;
    m_requestedSeamSeries = m_seamSeries + m_seamSeriesOffset;
    m_requestedSeam = m_seam + m_seamOffset;
}

void ContourBufferPlayer::arm(const fliplib::ArmStateBase& state)
{
    if (state.getStateID() == eSeamStart)
    {
        setProductInformationParameter();
        // serie-offset correct for current seam?
        if (m_requestedSeamSeries < 0)
        {
            m_ready = false;
            wmLog(eDebug, "ContourBufferPlayer::arm - seamSeries-offset not correct: seamSeries %d + seamSeriesOffset %d = %d! [slot %d]\n", m_seamSeries, m_seamSeriesOffset, m_requestedSeamSeries, m_slot);
        }

        // seam-offset correct for current seam?
        if (m_requestedSeam < 0)
        {
            m_ready = false;
            wmLog(eDebug, "ContourBufferPlayer::arm - seam-offset not correct: seam %d + seamoffset %d = %d! [slot %d]\n", m_seam, m_seamOffset, m_requestedSeam, m_slot);
        }

        // reset / initialize the memory for the slot that was selected by the user ...
        ContourBuffer& data = ContourBufferSingleton::getInstanceData();
        Buffer& pos = ContourBufferSingleton::getInstancePos();

        if (!data.exists(m_slot, m_requestedSeamSeries, m_requestedSeam) ||
            !pos.exists(m_slot, m_requestedSeamSeries, m_requestedSeam))
        {
            wmLog(eDebug, "ContourBufferPlayer::arm - buffer does not exist! [seamseries %d seamseriesoffset %d seam %d seamoffset %d slot %d]\n", m_seamSeries, m_seamSeriesOffset, m_seam, m_seamOffset, m_slot);
            m_ready = false;
        }
        else
        {
            m_data = data.get(m_slot, m_requestedSeamSeries, m_requestedSeam);
            m_pos = pos.get(m_slot, m_requestedSeamSeries, m_requestedSeam);
            m_ready = true;
        }
    }

    if(m_oVerbosity >= eHigh)
    {
        wmLog(eInfo, "ContourBufferPlayer - Filter '%s' armed at armstate %i [seamseries %d seamseriesoffset %d seam %d seamoffset %d slot %d]\n", m_filterName.c_str(), state.getStateID(), m_seamSeries, m_seamSeriesOffset, m_seam, m_seamOffset, m_slot);
    }
}

bool ContourBufferPlayer::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "image")
    {
        m_pipeInImage = dynamic_cast<fliplib::SynchronePipe<ImageFrame>*>(&pipe);
    }
    if (pipe.tag() == "pos")
    {
        m_pipeInPos = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);
}

void ContourBufferPlayer::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    poco_assert_dbg(m_pipeInImage != nullptr); // to be asserted by graph editor
    const std::vector<geo2d::AnnotatedDPointarray> emptyResult(1, geo2d::AnnotatedDPointarray(0));
    const ImageFrame& frame(m_pipeInImage->read(m_oCounter));
    // is the buffer ready?
    if (!m_ready)
    {
        wmLog(eWarning, "ContourBufferPlayer not ready(series offset %d, seam offset %d, slot %d)\n", m_seamSeriesOffset, m_seamOffset, m_slot);
        // send dummy data out ...
        const GeoVecAnnotatedDPointarray geoOut(frame.context(), emptyResult, frame.analysisResult(), NotPresent); // bad rank
        preSignalAction();
        m_pipeOutData.signal(geoOut);
        return;
    }

    // read image frame and retrieve image number
    double requestedPosition = 0.0;
    double posBufPrev = 0.0; // position that was stored by the recorder, perhaps slightly before the position we would like to access here.
    double posBufNext = 0.0; // position that was stored by the recorder, perhaps slightly behind the position we would like to access here.

    const GeoDoublearray& geoDoubleArrayIn = m_pipeInPos->read(m_oCounter);
    requestedPosition = std::get<eData>(geoDoubleArrayIn.ref()[0]);

    // here the data-offset is added. This is the only remaining time-component in case an encoder is used ...
    requestedPosition += m_dataOffset;

    // find the read-index, based on last iteration and position array
    int readIndex = 0;
    std::vector<double>& storedPositions = m_pos->ref().getData();
    for(auto iter = storedPositions.begin(); iter != storedPositions.end(); iter++)
    {
        // did we find the position? // only positiv positions :) may use Encoder "mal -1" to avoid this
        if ((*iter - requestedPosition) >= 0)
        {
            posBufNext = *iter;
            break;
        }
        else
        {
            posBufPrev = *iter;
            readIndex++;
        }
    }

    // get the actual information at this position.
    if (readIndex >= static_cast<int>(m_data->ref().size()))
    {
        // get product information for the debug message
        wmLog(eDebug, "ContourBufferPlayer - cannot find any data for position %f! [req seamseries %u req seam %u slot %u]\n", requestedPosition, m_requestedSeamSeries, m_requestedSeam, m_slot);
        // send dummy data out
        const GeoVecAnnotatedDPointarray geoOut(frame.context(), emptyResult, frame.analysisResult(), NotPresent); // bad rank
        preSignalAction();
        m_pipeOutData.signal(geoOut);
        return;
    }

    // Data value, stored by the recorder, perhaps slightly before the position we want to access here.
    geo2d::AnnotatedDPointarray& dataBufPrev = m_data->ref()[readIndex > 0 ? readIndex-1 : readIndex];
    // Data value, stored by the recorder, perhaps slightly behind the position we want to access here.
    geo2d::AnnotatedDPointarray& dataBufNext = m_data->ref()[readIndex];

    // store final data element in the output array.
    // This is what we are looking for, the final, interpolated data value (nearest neighbor interpolation)
    geo2d::AnnotatedDPointarray data = (requestedPosition - posBufPrev ) < ( posBufNext - requestedPosition) ?  dataBufPrev : dataBufNext;

    // compute rank
    double meanRank = data.getRank().size() > 0 ? std::accumulate(data.getRank().begin(), data.getRank().end(), 0.0) / data.getRank().size() : 0.0;

    // store result
    std::vector<geo2d::AnnotatedDPointarray> out;
    out.assign(1, data);

        // some debug output
    if(m_oVerbosity >= eHigh)
    {
        auto& storedContour = out[0];
        auto& firstPoint = storedContour.getData()[0];
        auto& firstPointRank = storedContour.getRank()[0];

        wmLog(eInfo, "ContourBufferPlayer: Reading from slot %d element %d / %d: %f %f (r:%d)!\n",
            m_slot, readIndex, m_data->ref().size(), firstPoint.x, firstPoint.y, firstPointRank );
    }
    // send the data out ...
    const GeoVecAnnotatedDPointarray geoOut(frame.context(), out, frame.analysisResult(), meanRank / 255.0);
    preSignalAction();
    m_pipeOutData.signal(geoOut);
}

} // namespace filter
} // namespace precitec
