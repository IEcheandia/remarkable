#pragma once

#include <precitec/change.h>

namespace precitec
{
namespace gui
{

class EtherCATConfigurationChangeEntry : public components::userLog::Change
{
    Q_OBJECT
    Q_PROPERTY(bool wasEnabled READ wasEnabled CONSTANT)
    Q_PROPERTY(bool isEnabled READ isEnabled CONSTANT)
    Q_PROPERTY(QByteArray oldMacAddress READ oldMacAddress CONSTANT)
    Q_PROPERTY(QByteArray newMacAddress READ newMacAddress CONSTANT)
public:
    Q_INVOKABLE EtherCATConfigurationChangeEntry(QObject *parent = nullptr);
    EtherCATConfigurationChangeEntry(bool wasEnabled, bool enabled, const QByteArray &oldMacAddress, const QByteArray &newMacAddress, QObject *parent = nullptr);
    ~EtherCATConfigurationChangeEntry() override;

    bool wasEnabled() const
    {
        return m_wasEnabled;
    }
    bool isEnabled() const
    {
        return m_enabled;
    }
    QByteArray oldMacAddress() const
    {
        return m_oldMacAddress;
    }
    QByteArray newMacAddress() const
    {
        return m_newMacAddress;
    }

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;

    bool m_wasEnabled = false;
    bool m_enabled = false;
    QByteArray m_oldMacAddress;
    QByteArray m_newMacAddress;
};

}
}
