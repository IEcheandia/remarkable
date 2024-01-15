#pragma once

#include "seamTable.h"

#include <QAbstractTableModel>
#include <QFont>

namespace precitec
{
namespace storage
{

class Seam;
class Product;

}
namespace gui
{

class SeamPropertyModel;
class ProductController;

class SeamSelectionModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::Seam* currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)

    Q_PROPERTY(precitec::storage::Product* product READ product WRITE setProduct NOTIFY productChanged)

    Q_PROPERTY(precitec::gui::SeamPropertyModel* propertyModel READ propertyModel WRITE setPropertyModel NOTIFY propertyModelChanged)

    Q_PROPERTY(precitec::gui::ProductController* productController READ productController WRITE setProductController NOTIFY productControllerChanged)

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

    Q_PROPERTY(int columnCount READ columnCount NOTIFY columnCountChanged)

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)

public:
    explicit SeamSelectionModel(QObject *parent = nullptr);
    ~SeamSelectionModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    precitec::storage::Seam* currentSeam() const
    {
        return m_currentSeam;
    }
    void setCurrentSeam(precitec::storage::Seam* seam);

    precitec::storage::Product* product() const
    {
        return m_product;
    }
    void setProduct(precitec::storage::Product* product);

    SeamPropertyModel* propertyModel() const
    {
        return m_propertyModel;
    }
    void setPropertyModel(SeamPropertyModel* propertyModel);

    ProductController* productController() const
    {
        return m_productController;
    }
    void setProductController(ProductController* productController);

    QFont font()
    {
        return m_font;
    }
    void setFont(const QFont& font);

    Q_INVOKABLE void selectRow(int row);

    Q_INVOKABLE void selectColumn(int column);

    Q_INVOKABLE void selectAll();

    Q_INVOKABLE void selectNone();

    Q_INVOKABLE void save();

    Q_INVOKABLE uint columnWidth(uint column);

Q_SIGNALS:
    void currentSeamChanged();
    void productChanged();
    void rowCountChanged();
    void columnCountChanged();
    void propertyModelChanged();
    void productControllerChanged();
    void fontChanged();
    void markAsChanged();

private:
    void updateTable();

    SeamTable m_seamTable;
    precitec::storage::Seam* m_currentSeam = nullptr;
    QMetaObject::Connection m_seamDestroyed;
    precitec::storage::Product* m_product;
    QMetaObject::Connection m_productDestroyed;
    SeamPropertyModel* m_propertyModel;
    QMetaObject::Connection m_propertyModelDestroyed;
    ProductController* m_productController;
    QMetaObject::Connection m_productControllerDestroyed;

    QFont m_font = QFont(QStringLiteral("Noto Sans"));
};

}
}

Q_DECLARE_METATYPE(precitec::gui::SeamSelectionModel*)
