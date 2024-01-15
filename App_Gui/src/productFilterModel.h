#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * A filter proxy model to sort on products. Excludes the default product (optionally).
 **/
class ProductFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * Whether the default product should be included, default is @c false.
     **/
    Q_PROPERTY(bool includeDefaultProduct READ includeDefaultProduct WRITE setIncludeDefaultProduct NOTIFY includeDefaultProductChanged)
public:
    explicit ProductFilterModel(QObject *parent = nullptr);
    ~ProductFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool includeDefaultProduct() const
    {
        return m_includeDefaultProduct;
    }
    void setIncludeDefaultProduct(bool set);

protected:
    bool lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const override;

Q_SIGNALS:
    void includeDefaultProductChanged();

private:
    bool m_includeDefaultProduct = false;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ProductFilterModel*)
