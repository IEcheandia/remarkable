#pragma once

#include <precitec/change.h>

namespace precitec
{
namespace gui
{

class QuitSystemFaultChangeEntry : public components::userLog::Change
{
    Q_OBJECT
    Q_PROPERTY(QString station READ station CONSTANT)
public:
    Q_INVOKABLE QuitSystemFaultChangeEntry(QObject *parent = nullptr);
    QuitSystemFaultChangeEntry(const QString &station, QObject *parent = nullptr);
    ~QuitSystemFaultChangeEntry() override;

    QUrl detailVisualization() const override;

    QString station() const
    {
        return m_station;
    }

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;

private:
    QString m_station;
};

}
}
