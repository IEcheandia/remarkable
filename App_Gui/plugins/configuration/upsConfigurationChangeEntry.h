#pragma once

#include <precitec/change.h>

namespace precitec
{
namespace gui
{

class UpsConfigurationChangeEntry : public components::userLog::Change
{
    Q_OBJECT
    Q_PROPERTY(bool wasEnabled READ wasEnabled CONSTANT)
    Q_PROPERTY(bool isEnabled READ isEnabled CONSTANT)
    Q_PROPERTY(QString oldName READ oldName CONSTANT)
    Q_PROPERTY(QString newName READ newName CONSTANT)
public:
    Q_INVOKABLE UpsConfigurationChangeEntry(QObject *parent = nullptr);
    UpsConfigurationChangeEntry(bool wasEnabled, bool enabled, const QString &oldName, const QString &newName, QObject *parent = nullptr);
    ~UpsConfigurationChangeEntry() override;

    bool wasEnabled() const
    {
        return m_wasEnabled;
    }
    bool isEnabled() const
    {
        return m_enabled;
    }
    QString oldName() const
    {
        return m_oldName;
    }
    QString newName() const
    {
        return m_newName;
    }

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;

    bool m_wasEnabled = false;
    bool m_enabled = false;
    QString m_oldName;
    QString m_newName;
};

}
}
