#include "productDeletedChangeEntry.h"
#include "product.h"

namespace precitec
{
namespace gui
{

ProductDeletedChangeEntry::ProductDeletedChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}

ProductDeletedChangeEntry::ProductDeletedChangeEntry(storage::Product *product, QObject *parent)
    : components::userLog::Change(parent)
    , m_data(storage::Product::fromJson(product->filePath())->toJson())
{
    setMessage(tr("Product deleted"));
}

ProductDeletedChangeEntry::~ProductDeletedChangeEntry() = default;

void ProductDeletedChangeEntry::initFromJson(const QJsonObject &data)
{
    m_product = storage::Product::fromJson(data, this);
}

QJsonObject ProductDeletedChangeEntry::data() const
{
    return m_data;
}

QUrl ProductDeletedChangeEntry::detailVisualization() const
{
    return QStringLiteral("qrc:///resources/qml/userLog/ProductAddedDeletedChangeEntry.qml");
}

}
}
