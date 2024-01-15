#pragma once

#include <QObject>

namespace precitec
{

namespace gui
{

class EtherCATConfigurationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray macAddress READ macAddress WRITE setMacAddress NOTIFY macAddressChanged)
    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
public:
    EtherCATConfigurationController(QObject *parent = nullptr);
    ~EtherCATConfigurationController() override;

    QByteArray macAddress() const
    {
        return m_macAddress;
    }
    void setMacAddress(const QByteArray &macAddress);

    bool isModified()
    {
        return m_modified;
    }

    bool isEnabled() const
    {
        return m_enabled;
    }
    void setEnabled(bool enabled);

    Q_INVOKABLE void save();

Q_SIGNALS:
    void macAddressChanged();
    void modifiedChanged();
    void enabledChanged();

private:
    void markAsModified();
    QByteArray initMacAddress() const;
    bool initEnabled();

    QByteArray m_macAddress;
    bool m_modified = false;
    bool m_enabled;
};

}
}
