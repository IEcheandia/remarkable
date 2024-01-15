#pragma once

#include <QObject>
#include <QUuid>
#include <QVector2D>
#include <forward_list>

class QDataStream;

namespace precitec
{
namespace storage
{

class ReferenceCurveData : public QObject
{
    Q_OBJECT

public:
    explicit ReferenceCurveData(QObject* parent = nullptr);
    explicit ReferenceCurveData(const QUuid& uuid, QObject* parent = nullptr);
    ~ReferenceCurveData() override;

    ReferenceCurveData* duplicate(const QUuid& id, QObject* parent) const;

    QUuid uuid() const
    {
        return m_uuid;
    }

    template <typename T>
    void setSamples(const T &samples, std::size_t count)
    {
        static_assert(std::is_convertible<typename T::value_type, QVector2D>::value, "Template argument must be a container of a type convertible to QVector2D");
        m_data.reserve(count);
        std::copy(samples.begin(), samples.end(), std::back_inserter(m_data));
    }

    void setSamples(const std::vector<QVector2D> &samples)
    {
        setSamples(samples, samples.size());
    }

    void clear()
    {
        m_data.clear();
    }

    const std::vector<QVector2D>& samples() const
    {
        return m_data;
    }

    friend QDataStream &operator<<(QDataStream &out, ReferenceCurveData* referenceCurve);
    friend QDataStream &operator>>(QDataStream &in, ReferenceCurveData* referenceCurve);

private:
    QUuid m_uuid;
    std::vector<QVector2D> m_data;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::ReferenceCurveData*)

