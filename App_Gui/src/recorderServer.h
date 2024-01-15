#pragma once
#include "event/recorder.interface.h"
#include "event/recorderPoll.interface.h"

// Qt
#include <QObject>
// std
#include <memory>
#include <vector>

using namespace precitec::interface;

class QMutex;
class QTimer;

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{
using ImageData = std::tuple<ImageContext, precitec::image::BImage, precitec::image::OverlayCanvas>;
}
}

class RecorderServer : public QObject, public TRecorder<AbstractInterface>
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::image::ImageData imageData READ getImage NOTIFY imageDataChanged)
    /**
     * The image number of the last received image.
     **/
    Q_PROPERTY(int imageNumber READ imageNumber NOTIFY imageDataChanged)

    /**
     * Throttles the image polling to only every 100 msec.
     **/
    Q_PROPERTY(bool throttle READ isThrottled WRITE setThrottled NOTIFY throttledChanged)
public:
    explicit RecorderServer(QObject *parent = nullptr);
    ~RecorderServer() override;

    /**
     * Iniitializes the shared memory of this RecorderServer.
     * @param stationName The station name which needs to be added to the Shared memory name
     * @param size The size of the shared memory segment
     **/
    void init(const std::string &stationName, int size);

    void data(int sensorId, ImageContext const& context, image::BImage const& data, image::OverlayCanvas const& canvas) override;
    void data(int sensorId, ImageContext const& context, image::Sample const& data, image::OverlayCanvas const& canvas) override;
    void multiSampleData(const std::vector<interface::SampleFrame> &samples, image::OverlayCanvas const& canvas) override;
    void simulationDataMissing(ImageContext const& context) override;

    precitec::gui::components::image::ImageData getImage() const;

    void setHasFrameGrabber(bool set)
    {
        m_hasFrameGrabber = set;
    }

    Q_INVOKABLE void requestNextImage();

    void setRecorderPollProxy(const std::shared_ptr<interface::TRecorderPoll<interface::AbstractInterface>> &proxy)
    {
        m_pollProxy = proxy;
    }

    int imageNumber() const;

    bool isThrottled() const
    {
        return m_throttle;
    }
    void setThrottled(bool throttle);

Q_SIGNALS:
    void imageReceived();
    void imageDataChanged(int imageNumber);
    void sampleData(int sensorId, const precitec::image::Sample &data, const precitec::interface::ImageContext &context);
    void throttledChanged();

private:
    void doRequestNextImage();
    std::unique_ptr<QMutex> m_mutex;
    std::unique_ptr<precitec::system::SharedMem> m_imageSharedMemory;
    precitec::gui::components::image::ImageData m_imageData;
    std::shared_ptr<interface::TRecorderPoll<interface::AbstractInterface>> m_pollProxy;
    bool m_hasFrameGrabber = false;
    bool m_throttle = false;
    QTimer *m_timer = nullptr;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::RecorderServer*)
