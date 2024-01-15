#pragma once
#include <QSortFilterProxyModel>
#include <QUuid>

namespace precitec
{
namespace storage
{

class ProductInstanceSeamModel;

/**
 * The ProductInstanceSeamFilterModel sorts a ProductInstanceSeamModel based on the
 * processedSeams property of the SeamSeries
 **/
class ProductInstanceSeamSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::storage::ProductInstanceSeamModel *productInstanceSeamModel READ productInstanceSeamModel WRITE setProductInstanceSeamModel NOTIFY productInstanceSeamModelChanged)
public:
    ProductInstanceSeamSortModel(QObject *parent = nullptr);
    ~ProductInstanceSeamSortModel() override;

    ProductInstanceSeamModel *productInstanceSeamModel() const
    {
        return m_sourceModel;
    }
    void setProductInstanceSeamModel(ProductInstanceSeamModel *model);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

Q_SIGNALS:
    void productInstanceSeamModelChanged();

private:
    void init();
    ProductInstanceSeamModel *m_sourceModel = nullptr;
    std::vector<QMetaObject::Connection> m_sourceModelConnections;
    std::vector<QUuid> m_uuids;
};

}
}
