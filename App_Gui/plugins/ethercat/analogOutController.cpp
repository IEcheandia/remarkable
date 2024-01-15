#include "analogOutController.h"
#include "slaveInfoModel.h"
#include "../../src/serviceToGuiServer.h"

#include <QMutex>

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

AnalogOutController::AnalogOutController(QObject *parent)
    : AbstractAnalogInOutController(parent)
{
}

AnalogOutController::~AnalogOutController() = default;

static bool s_recursionBlocker = false;

void AnalogOutController::fetchData()
{
    QMutexLocker lock{mutex()};
    // TODO: how to validate whether slave is correct?
    if (!service() || slave().dwVendorId == 0)
    {
        return;
    }
    const auto dataElements = service()->outputData(slave());
    if (dataElements.empty())
    {
        return;
    }

    auto calculateData = [] (const QByteArray &data, uint32_t offset)
    {
        if (data.count() < 4)
        {
            return 0.0;
        }
        int16_t binaryData = data.at(offset + 1);
        binaryData = binaryData << 8;
        binaryData = binaryData & 0xFF00;
        int16_t lower = data.at(offset);
        lower = lower & 0xFF;
        binaryData = binaryData | lower;
        // see https://download.beckhoff.com/download/document/io/ethercat-terminals/el41xxen.pdf (version 4.3) page 134
        const qreal factor = (binaryData >= 0) ? 3276.7 : 3276.8;
        return qreal(binaryData) / factor;
    };

    s_recursionBlocker = true;
    const qreal channel1 = calculateData(dataElements.back(), 0);
    const qreal channel2 = calculateData(dataElements.back(), 2);
    if (qFuzzyCompare(channel1, m_channel1) || (qFuzzyIsNull(channel1) && qFuzzyIsNull(m_channel1)))
    {
        m_channel1 = channel1;
        emit channel1Changed();
    }
    if (qFuzzyCompare(channel2, m_channel2) || (qFuzzyIsNull(channel2) && qFuzzyIsNull(m_channel2)))
    {
        m_channel2 = channel2;
        emit channel2Changed();
    }
    s_recursionBlocker = false;
}

void AnalogOutController::setChannel1(qreal value)
{
    setChannel(value, 1);
}

void AnalogOutController::setChannel2(qreal value)
{
    setChannel(value, 2);
}

void AnalogOutController::setChannel(qreal value, int channel)
{
    if (!service() || s_recursionBlocker)
    {
        return;
    }
    qint16 transformed = value * 3276.7;
    QByteArray data(2, 0);
    reinterpret_cast<uint16_t*>(data.data())[0] = transformed;
    QByteArray mask(2, 0);
    mask.data()[0] = 0xFF;
    mask.data()[1] = 0xFF;
    service()->setOutputData(slave(), data, mask, channel);
}

}
}
}
}
