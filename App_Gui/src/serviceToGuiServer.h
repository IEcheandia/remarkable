#pragma once
#include "event/viServiceToGUI.interface.h"
#include "event/viServiceFromGUI.interface.h"

#include <QObject>

#include <vector>

class QMutex;

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TviServiceFromGUI<precitec::interface::AbstractInterface>> ServiceFromGuiProxy;

namespace gui
{

/**
 * Server implementing the TviServiceToGUI interface and provides it as a QObject.
 **/
class ServiceToGuiServer : public QObject, public precitec::interface::TviServiceToGUI<precitec::interface::AbstractInterface>
{
    Q_OBJECT
    /**
     * Whether the server is monitoring events from Service.
     * This needs to be set to @c true in order to receive @link{processDataChanged} signals.
     **/
    Q_PROPERTY(bool monitorEvents READ isMonitoringEvents WRITE setMonitoringEvents NOTIFY monitoringEventsChanged)
public:
    ServiceToGuiServer(const ServiceFromGuiProxy &proxy, QObject *parent = nullptr);
    ~ServiceToGuiServer() override;

    void ProcessImage(precitec::interface::ProcessDataVector& input, precitec::interface::ProcessDataVector& output) override;
    void SlaveInfoECAT(short count, precitec::interface::SlaveInfo info) override;
    void ConfigInfo(std::string config) override;

    /**
     * Access to the slaveInfo provided through the SlaveInfoECAT event.
     * @see slaveInfoChanged
     **/
    std::vector<EC_T_GET_SLAVE_INFO> slaveInfo() const;

    /**
     * The input data for the @p slave.
     * @see outputData
     * @see processDataChanged
     **/
    std::vector<QByteArray> inputData(const EC_T_GET_SLAVE_INFO &slave) const;
    /**
     * The output data for the @p slave.
     * @see inputData
     * @see processDataChanged
     **/
    std::vector<QByteArray> outputData(const EC_T_GET_SLAVE_INFO &slave) const;

    /**
     * Set the output bit at @p byteIndex / @p bitIndex of the @p slave to the new value @p state.
     **/
    void setOutputBit(const EC_T_GET_SLAVE_INFO &slave, quint32 byteIndex, quint8 bitIndex, bool state);

    /**
     * Sets the output @p data with the given @p mask for @p channel of the EtherCAT @p slave.
     **/
    void setOutputData(const EC_T_GET_SLAVE_INFO &slave, const QByteArray &data, const QByteArray &mask, int channel);

    bool isMonitoringEvents() const
    {
        return m_monitoring;
    }
    void setMonitoringEvents(bool set);

Q_SIGNALS:
    /**
     * Emitted when the slaveInfo changes. Might be emitted from a thread, use Qt::QueuedConnection.
     * @see slaveInfo
     **/
    void slaveInfoChanged();
    /**
     * Emitted whenever new process data (input/output) has been received.
     * @see inputData
     * @see outputData
     **/
    void processDataChanged();
    void monitoringEventsChanged();

private:
    ServiceFromGuiProxy m_proxy;
    std::unique_ptr<QMutex> m_mutex;
    std::vector<EC_T_GET_SLAVE_INFO> m_slaveInfo;
    precitec::interface::ProcessDataVector m_inputData;
    precitec::interface::ProcessDataVector m_outputData;
    bool m_monitoring = false;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ServiceToGuiServer*)
