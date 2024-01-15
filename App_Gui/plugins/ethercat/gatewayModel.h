#pragma once
#include "viConfigService.h"

#include <QAbstractListModel>

#include <map>

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

/**
 * Model providing input/output signal information for an ethercat gateway.
 * Each row is one bit, the first 20 bytes are input (that is 160 rows), the upper 20 bytes are output
 *
 * The model provides the following roles:
 * @li display: human readable and translated name for the signal
 * @li byte: number of the byte (starting at 0)
 * @li bit: number of the bit in the byte (starting at 0)
 * @li type: ViConfigService::SignalType
 * @li state: the state of the bit identified by the row
 *
 * To init the model use the @link{setGateway} method.
 **/
class GatewayModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * ViConfigService to fetch signal information for the gateway
     **/
    Q_PROPERTY(precitec::gui::components::ethercat::ViConfigService *viConfig READ viConfig WRITE setViConfig NOTIFY viConfigChanged)
    /**
     * The input data for this gateway.
     **/
    Q_PROPERTY(QByteArray inputData READ inputData WRITE setInputData NOTIFY inputDataChanged)
    /**
     * The output data for this gateway.
     **/
    Q_PROPERTY(QByteArray outputData READ outputData WRITE setOutputData NOTIFY outputDataChanged)
public:
    GatewayModel(QObject *parent = nullptr);
    ~GatewayModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    ViConfigService *viConfig() const
    {
        return m_viConfig;
    }
    void setViConfig(ViConfigService *service);

    QByteArray inputData() const
    {
        return m_inputData;
    }
    QByteArray outputData() const
    {
        return m_outputData;
    }
    void setInputData(const QByteArray &data);
    void setOutputData(const QByteArray &data);

    /**
     * Initializes this model by looking in the ViConfigService for the signals of the gateway
     * with the @p instance, @p inputSize and @p outputSize in bits. VendorId and ProductCode are hardcoded
     **/
    Q_INVOKABLE void setGateway(quint32 instance, quint32 inputSize, quint32 outputSize);

    /**
     * @param skipNumbers ignore the @p bit
     **/
    static QString localizeSignal(const ViConfigService::Signal &signal, int bit, bool skipNumbers = false);

Q_SIGNALS:
    void viConfigChanged();
    void inputDataChanged();
    void outputDataChanged();

private:
    void initGateway();
    std::map<int, std::pair<ViConfigService::Signal, quint32>> m_signals;
    quint32 m_instance = 0;
    ViConfigService *m_viConfig = nullptr;
    QMetaObject::Connection m_viConfigDestroyed;
    QMetaObject::Connection m_viConfigSignalsChanged;
    QByteArray m_inputData;
    QByteArray m_outputData;
    std::uint32_t m_inputSize = 0;
    std::uint32_t m_outputSize = 0;
};

}
}
}
}
