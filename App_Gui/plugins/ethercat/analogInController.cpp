#include "analogInController.h"
#include "slaveInfoModel.h"
#include "../../src/serviceToGuiServer.h"

#include <precitec/dataSet.h>

#include <QMutex>
#include <QMutexLocker>
#include <QVector2D>

#include <cmath>

using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

AnalogInController::AnalogInController(QObject *parent)
    : AbstractAnalogInOutController(parent)
    , m_channel1DataSet(new DataSet{this})
    , m_channel2DataSet(new DataSet{this})
{
    m_channel1DataSet->setSmooth(false);
    m_channel2DataSet->setSmooth(false);
    connect(this, &AnalogInController::newData, this, &AnalogInController::updateDataSet, Qt::QueuedConnection);
    connect(this, &AnalogInController::slaveChanged, this,
        [this]
        {
            m_oversampling = model()->data(index(), Qt::UserRole + 5).value<SlaveInfoModel::Type>() == SlaveInfoModel::Type::AnalogOversamplingIn;
            if (m_oversampling)
            {
                m_channel1DataSet->setMaxElements(m_channel1DataSet->maxElements() * 100u);
                m_channel2DataSet->setMaxElements(m_channel2DataSet->maxElements() * 100u);
            }
            emit oversamplingChanged();
        }
    );
}

AnalogInController::~AnalogInController() = default;

void AnalogInController::fetchData()
{
    QMutexLocker lock{mutex()};
    // TODO: how to validate whether slave is correct?
    if (!service() || slave().dwVendorId == 0)
    {
        return;
    }
    if (m_oversampling)
    {
        fetchOversampling();
    } else
    {
        fetchNormal();
    }
}

void AnalogInController::fetchNormal()
{
    const auto dataElements = service()->inputData(slave());
    m_channel1.reserve(m_channel1.size() + dataElements.size());
    m_channel2.reserve(m_channel2.size() + dataElements.size());

    auto calculateData = [] (const QByteArray &data, uint32_t offset, uint32_t timeStamp) -> QVector2D
    {
        if (data.count() < 6)
        {
            return {};
        }
        // three byte layout, first byte is status, 2 byte values
        int16_t binaryData = data.at(offset + 2);
        binaryData = binaryData << 8;
        binaryData = binaryData & 0xFF00;
        int16_t lower = data.at(offset + 1);
        lower = lower & 0xFF;
        binaryData = binaryData | lower;
        // see https://download.beckhoff.com/download/document/io/ethercat-terminals/el31xxen.pdf (version 5.8) page 162
        const auto factor = (binaryData >= 0) ? 3276.7f : 3276.8f;
        return {timeStamp / 1000.0f, float(binaryData) / factor};
    };

    quint32 timeStamp = m_timeStamp;
    std::transform(dataElements.begin(), dataElements.end(), std::back_inserter(m_channel1),
        [calculateData, &timeStamp] (const auto &data)
        {
            return calculateData(data, 0, timeStamp++);
        });
    timeStamp = m_timeStamp;
    std::transform(dataElements.begin(), dataElements.end(), std::back_inserter(m_channel2),
        [calculateData, &timeStamp] (const auto &data)
        {
            return calculateData(data, 3, timeStamp++);
        });
    m_timeStamp = timeStamp;
    emit newData();
}

void AnalogInController::fetchOversampling()
{
    // input: n x 2 x 16 bit data; optionally 2 x 16 bit cycle counter, 4 byte StartNextLatch time
    const int n = (slave().dwPdSizeIn - 32 - 32) / 32;
    const auto dataElements = service()->inputData(slave());
    m_channel1.reserve(m_channel1.size() + dataElements.size() * n);
    m_channel2.reserve(m_channel2.size() + dataElements.size() * n);

    for (const auto &dataElement : dataElements)
    {
        const auto data = reinterpret_cast<const int16_t*>(dataElement.constData());
        // layout: 16 bit cycle counter channel 1, n * 16 bit values channel 1, 16 bit cycle counter channel 2, n * 16 bit values channel 2
        for (int i = 0; i < n; i++)
        {
            const auto channel1 = data[i + 1];
            const auto channel2 = data[i + n + 2];
            const auto timestamp = (m_timeStamp + 1.0f / n) / 1000.0f;
            const auto factor1 = (channel1 >= 0) ? 3276.7f : 3276.8f;
            const auto factor2 = (channel2 >= 0) ? 3276.7f : 3276.8f;
            m_channel1.emplace_back(timestamp, float(channel1) / factor1);
            m_channel2.emplace_back(timestamp, float(channel2) / factor2);
        }
        m_timeStamp++;
    }

    emit newData();
}

void AnalogInController::updateDataSet()
{
    QMutexLocker lock{mutex()};
    std::vector<QVector2D> channel1;
    if (!m_channel1.empty())
    {
        channel1 = std::move(m_channel1);
        m_channel1.clear();
    }
    std::vector<QVector2D> channel2;
    if (!m_channel2.empty())
    {
        channel2 = std::move(m_channel2);
        m_channel2.clear();
    }
    lock.unlock();

    if (!channel1.empty())
    {
        const qreal currentValue = channel1.back().y();
        if (!qFuzzyCompare(currentValue, m_channel1CurrentValue))
        {
            m_channel1CurrentValue = currentValue;
            emit channel1CurrentValueChanged();
        }
        m_channel1DataSet->addSamples(std::move(channel1));
    }
    if (!channel2.empty())
    {
        const qreal currentValue = channel2.back().y();
        if (!qFuzzyCompare(currentValue, m_channel2CurrentValue))
        {
            m_channel2CurrentValue = currentValue;
            emit channel2CurrentValueChanged();
        }
        m_channel2DataSet->addSamples(std::move(channel2));
    }
}

void AnalogInController::clear()
{
    QMutexLocker lock{mutex()};
    m_channel1DataSet->clear();
    m_channel2DataSet->clear();
    m_timeStamp = 0;
}

}
}
}
}
