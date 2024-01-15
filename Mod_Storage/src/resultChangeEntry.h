#pragma once
#include "resultSetting.h"
#include <precitec/change.h>

#include <QVariant>

namespace precitec
{
namespace storage
{

class ResultChangeEntry : public precitec::gui::components::userLog::Change
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultSetting *result READ result CONSTANT)

    Q_PROPERTY(precitec::storage::ResultSetting::Type change READ change CONSTANT)

    Q_PROPERTY(QVariant oldValue READ oldValue CONSTANT)

    Q_PROPERTY(QVariant newValue READ newValue CONSTANT)
public:
    ResultChangeEntry(ResultSetting *setting, const ResultSetting::Type &change, const QVariant &oldValue, const QVariant &newValue, QObject *parent = nullptr);
    Q_INVOKABLE ResultChangeEntry(QObject *parent = nullptr);
    ~ResultChangeEntry();

    QUrl detailVisualization() const override;

    ResultSetting *result() const
    {
        return m_result;
    }
    ResultSetting::Type change() const
    {
        return m_change;
    }
    QVariant oldValue() const
    {
        return m_oldValue;
    }
    QVariant newValue() const
    {
        return m_newValue;
    }

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;

private:
    ResultSetting *m_result = nullptr;
    ResultSetting::Type m_change;
    QVariant m_oldValue;
    QVariant m_newValue;
};

}
}
