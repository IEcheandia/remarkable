#include "productInstancesDeletedChangeEntry.h"
#include "product.h"

namespace precitec
{
namespace gui
{

ProductInstancesDeletedChangeEntry::ProductInstancesDeletedChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}


ProductInstancesDeletedChangeEntry::ProductInstancesDeletedChangeEntry(storage::Product *product, const std::forward_list<std::tuple<QFileInfo, QString, QDateTime>> &instances, QObject *parent)
    : components::userLog::Change(parent)
    , m_productId(product->uuid())
    , m_productName(product->name())
{
    std::transform(instances.begin(), instances.end(), std::back_inserter(m_instances),
        [] (const auto &instance)
        {
            return QJsonObject{{
                qMakePair(QStringLiteral("serialNumber"), std::get<1>(instance)),
                qMakePair(QStringLiteral("date"), std::get<2>(instance).toString(Qt::ISODateWithMs))
            }};
        }
    );
    setMessage(tr("Product instance videos deleted"));
}

ProductInstancesDeletedChangeEntry::~ProductInstancesDeletedChangeEntry() = default;

QJsonObject ProductInstancesDeletedChangeEntry::data() const
{
    return {{
        qMakePair(QStringLiteral("product"), QJsonObject{{
            qMakePair(QStringLiteral("uuid"), m_productId.toString()),
            qMakePair(QStringLiteral("name"), m_productName)
        }}),
        qMakePair(QStringLiteral("instances"), m_instances)
    }};
}

void ProductInstancesDeletedChangeEntry::initFromJson(const QJsonObject &data)
{
    // TODO: implement
    Q_UNUSED(data)
}

}
}
