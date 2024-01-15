#include "recorderServer.h"

#include "event/imageShMem.h"

#include <QMutex>
#include <QTimer>

#include <functional>

using namespace precitec::system;
using namespace precitec::greyImage;

using precitec::gui::components::image::ImageData;

namespace precitec
{
namespace gui
{

RecorderServer::RecorderServer(QObject *parent)
    : QObject(parent)
    , m_mutex(new QMutex())
{
}

RecorderServer::~RecorderServer() = default;

void RecorderServer::init(const std::string &stationName, int size)
{
    QMutexLocker locker(m_mutex.get());
    m_imageSharedMemory = std::make_unique<SharedMem>(ImageShMemHandle + std::hash<std::string>{}(stationName), ImageShMemName + stationName, SharedMem::StdServer, size);
}

void RecorderServer::data(int sensorId, ImageContext const& context, image::BImage const& data, image::OverlayCanvas const& canvas)
{
    if (!m_imageSharedMemory)
    {
        return;
    }
    Q_UNUSED(sensorId)

    {
        QMutexLocker locker(m_mutex.get());
        m_imageData = std::make_tuple(context, data, canvas);

        emit imageReceived();
        emit imageDataChanged(context.imageNumber());
    }

    // on system start we get null images, so just request more
    static bool s_first = true;
    if (data.width() == 0 && data.height() == 0 && s_first)
    {
        s_first = false;
        doRequestNextImage();
    }
}

precitec::gui::components::image::ImageData RecorderServer::getImage() const
{
    QMutexLocker locker(m_mutex.get());
    return m_imageData;
}

int RecorderServer::imageNumber() const
{
    QMutexLocker locker(m_mutex.get());
    return std::get<ImageContext>(m_imageData).imageNumber();
}

void RecorderServer::data(int sensorId, ImageContext const& context, image::Sample const& data, image::OverlayCanvas const& canvas)
{
    if (!m_hasFrameGrabber)
    {
        static image::BImage s_fakeImage{{1024, 512}};
        static bool s_initialized = false;
        if (!s_initialized)
        {
            s_fakeImage.fill(0);
            s_initialized = true;
        }
        QMutexLocker locker(m_mutex.get());
        m_imageData = std::make_tuple(context, s_fakeImage, canvas);
        emit imageReceived();
        emit imageDataChanged(context.imageNumber());
    }

    if (sensorId != -1)
    {
        emit sampleData(sensorId, data, context);
    }
}

void RecorderServer::multiSampleData(const std::vector<precitec::interface::SampleFrame> &samples, image::OverlayCanvas const& canvas)
{
    for (const auto &sample : samples)
    {
        data(sample.sensorId(), sample.context(), sample.data(), canvas);
    }
}

void RecorderServer::simulationDataMissing(ImageContext const& context)
{
    emit imageDataChanged(context.imageNumber());
}

void RecorderServer::requestNextImage()
{
    if (!m_pollProxy)
    {
        return;
    }
    if (m_throttle)
    {
        if (!m_timer)
        {
            m_timer = new QTimer{this};
            m_timer->setSingleShot(true);
            m_timer->setInterval(std::chrono::milliseconds{100});
            connect(m_timer, &QTimer::timeout, this, &RecorderServer::doRequestNextImage);
        }
        if (!m_timer->isActive())
        {
            m_timer->start();
        }
    } else
    {
        doRequestNextImage();
    }
}

void RecorderServer::doRequestNextImage()
{
    if (m_pollProxy)
    {
        m_pollProxy->signalReadyForFrame();
    }
}

void RecorderServer::setThrottled(bool throttle)
{
    if (m_throttle == throttle)
    {
        return;
    }
    m_throttle = throttle;
    emit throttledChanged();
}

}
}
