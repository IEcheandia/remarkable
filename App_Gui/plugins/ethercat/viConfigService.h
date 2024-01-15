#pragma once

#include <QObject>

#include <memory>

class QXmlStreamReader;

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

/**
 * Service which parses the VI_Config.xml found in the @link{configurationDir}.
 **/
class ViConfigService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configurationDir READ configurationDir WRITE setConfigurationDir NOTIFY configurationDirChanged)
public:
    ViConfigService(QObject *parent = nullptr);
    ~ViConfigService() override;

    QString configurationDir() const
    {
        return m_configurationDir;
    }
    void setConfigurationDir(const QString &config);


    enum class SignalType {
        Input,
        Output
    };
    Q_ENUM(SignalType)
    struct Signal {
        SignalType type;
        quint32 productCode = -1;
        quint32 slaveType = -1;
        quint32 vendorId = -1;
        quint32 instance = -1;
        quint32 startBit = -1;
        quint32 length = -1;
        QString name;
    };
    /**
     * Gets all Signals for the @p vendorId, @p productCode and @p instance.
     * If there is no Signal defined in the VI_Config this will return an empty list.
     **/
    std::vector<Signal> getSignals(quint32 vendorId, quint32 productCode, quint32 instance) const;

    const std::vector<Signal> &allSignals() const
    {
        return m_signals;
    }

Q_SIGNALS:
    void configurationDirChanged();
    /**
     * Signal emitted whenever a VI_Config got parsed.
     **/
    void signalsChanged();

private:
    void init();
    void parseWeldHeadControl(QXmlStreamReader &xml, std::vector<Signal> &newSignals);
    void parseInspectionControl(QXmlStreamReader &xml, std::vector<Signal> &newSignals);
    void parseInspectionSignal(QXmlStreamReader &xml, const QStringRef &signalName, std::vector<Signal> &newSignals);
    void parseSignal(QXmlStreamReader &xml, Signal &signal);
    void parseHeadMonitor(QXmlStreamReader &xml, std::vector<Signal> &newSignals);
    void parseHeadMonitorSignals(QXmlStreamReader &xml, SignalType type, std::vector<Signal> &newSignals);
    QString m_configurationDir;
    std::vector<Signal> m_signals;

};

}
}
}
}

Q_DECLARE_METATYPE(precitec::gui::components::ethercat::ViConfigService*)
