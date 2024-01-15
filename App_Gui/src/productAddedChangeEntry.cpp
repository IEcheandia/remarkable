#include "productAddedChangeEntry.h"
#include "product.h"

namespace precitec
{
namespace gui
{

ProductAddedChangeEntry::ProductAddedChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}

ProductAddedChangeEntry::ProductAddedChangeEntry(storage::Product *product, QObject *parent)
    : components::userLog::Change(parent)
    , m_data(product->toJson())
{
    setMessage(tr("Product added"));
}

ProductAddedChangeEntry::~ProductAddedChangeEntry() = default;

void ProductAddedChangeEntry::initFromJson(const QJsonObject &data)
{
    m_product = storage::Product::fromJson(data, this);
}

QJsonObject ProductAddedChangeEntry::data() const
{
    return m_data;
}

QUrl ProductAddedChangeEntry::detailVisualization() const
{
    return QStringLiteral("qrc:///resources/qml/userLog/ProductAddedDeletedChangeEntry.qml");
}

}
}

