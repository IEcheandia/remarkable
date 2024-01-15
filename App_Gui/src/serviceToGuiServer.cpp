#include "serviceToGuiServer.h"

#include <QtConcurrentRun>
#include <QMutex>
#include <QMutexLocker>

#include <bitset>

using namespace precitec::interface;

namespace precitec
{
namespace gui
{

ServiceToGuiServer::ServiceToGuiServer(const ServiceFromGuiProxy &proxy, QObject *parent)
    : QObject(parent)
    , m_proxy(proxy)
    , m_mutex(std::make_unique<QMutex>())
{
    connect(this, &ServiceToGuiServer::monitoringEventsChanged, this,
        [this]
        {
            if (!m_proxy)
            {
                return;
            }
            QtConcurrent::run(m_proxy.get(), &TviServiceFromGUI<AbstractInterface>::SetTransferMode, m_monitoring);
        }
    );
}

ServiceToGuiServer::~ServiceToGuiServer() = default;


void ServiceToGuiServer::ProcessImage(precitec::interface::ProcessDataVector& input, precitec::interface::ProcessDataVector& output)
{
    QMutexLocker lock{m_mutex.get()};
    m_inputData = input;
    m_outputData = output;
    lock.unlock();
    emit processDataChanged();
}

void ServiceToGuiServer::SlaveInfoECAT(short count, precitec::interface::SlaveInfo info)
{
    Q_UNUSED(count)
    QMutexLocker lock{m_mutex.get()};
    m_slaveInfo = info.GetSlaveInfoVector();
    emit slaveInfoChanged();
}

void ServiceToGuiServer::ConfigInfo(std::string config)
{
    Q_UNUSED(config)
}

std::vector<EC_T_GET_SLAVE_INFO> ServiceToGuiServer::slaveInfo() const
{
    QMutexLocker lock{m_mutex.get()};
    return m_slaveInfo;
}

namespace
{
std::vector<QByteArray> copyData(const precitec::interface::ProcessDataVector &data, quint32 offset, quint32 size)
{
    std::vector<QByteArray> ret;
    if (size == 0)
    {
        return ret;
    }
    ret.reserve(data.size());

    for (const auto &data : data)
    {
        QByteArray rawData = QByteArray::fromRawData(data.GetData(), data.GetSize());
        if (quint32(rawData.length()) >= offset + size)
        {
            ret.emplace_back(rawData.mid(offset, size));
        }
    }
    return ret;
}

}

std::vector<QByteArray> ServiceToGuiServer::inputData(const EC_T_GET_SLAVE_INFO &slave) const
{
    QMutexLocker lock{m_mutex.get()};
    const quint32 offset = slave.dwPdOffsIn / 8;
    const quint32 size = slave.dwPdSizeIn / 8;
    return copyData(m_inputData, offset, size);
}

std::vector<QByteArray> ServiceToGuiServer::outputData(const EC_T_GET_SLAVE_INFO &slave) const
{
    QMutexLocker lock{m_mutex.get()};
    const quint32 offset = slave.dwPdOffsOut / 8;
    const quint32 size = slave.dwPdSizeOut / 8;
    return copyData(m_outputData, offset, size);
}

void ServiceToGuiServer::setMonitoringEvents(bool set)
{
    if (m_monitoring == set)
    {
        return;
    }
    m_monitoring = set;
    emit monitoringEventsChanged();
}

void ServiceToGuiServer::setOutputBit(const EC_T_GET_SLAVE_INFO &slave, quint32 byteIndex, quint8 bitIndex, bool state)
{
    if (!m_proxy)
    {
        return;
    }
    const quint32 size = slave.dwPdSizeOut / 8;
    if (byteIndex >= size)
    {
        return;
    }
    ProcessData data;
    ProcessData mask;
    data.SetSize(size);
    mask.SetSize(size);

    std::bitset<8> bitSet;
    bitSet.set(bitIndex, state);

    QByteArray outData(size, 0);
    outData[byteIndex] = static_cast<uint8_t>(bitSet.to_ulong());
    data.FillBuffer(outData.data());

    bitSet.set(bitIndex, true);

    QByteArray maskData(size, 0);
    maskData[byteIndex] = static_cast<uint8_t>(bitSet.to_ulong());
    mask.FillBuffer(maskData.data());

    m_proxy->OutputProcessData(slave.wCfgPhyAddress, data, mask, 1);
}

void ServiceToGuiServer::setOutputData(const EC_T_GET_SLAVE_INFO &slave, const QByteArray &data, const QByteArray &mask, int channel)
{
    if (!m_proxy)
    {
        return;
    }
    ProcessData processData(data.size(), data.constData());
    ProcessData maskData(data.size(), mask.constData());
    m_proxy->OutputProcessData(slave.wCfgPhyAddress, processData, maskData, channel);
}

}
}
