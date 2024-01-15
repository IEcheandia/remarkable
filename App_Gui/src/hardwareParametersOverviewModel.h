#pragma once

#include <QAbstractTableModel>

#include <set>
#include <vector>

namespace precitec
{

namespace storage
{
class AttributeModel;
class AbstractMeasureTask;
class ParameterSet;
class Parameter;
class Product;
class SeamSeries;
class Seam;
}

namespace gui
{

/**
 * A table model to represent all hardware parameters of a @link{product}.
 *
 * As rows all hardware parameters are listed which are either used on product, seam series or seam level.
 * As columns product, seam series and seams are listed. Each cell contains the value of the hardware parameter at row
 * for the product or seam series or seam in the column.
 *
 * The model is not able to detect changes in the product. It assumes that this is only used as read only and the view
 * is destroyed whenever the product changes.
 **/
class HardwareParametersOverviewModel : public QAbstractTableModel
{
    Q_OBJECT
    /**
     * The hardware key value attribute model. Required for translating the hardware keys to human readable names.
     **/
    Q_PROPERTY(precitec::storage::AttributeModel *keyValueAttributeModel READ keyValueAttributeModel WRITE setKeyValueAttributeModel NOTIFY keyValueAttributeModelChanged)
    /**
     * The Product for which the HardwareParameter values should be taken.
     **/
    Q_PROPERTY(precitec::storage::Product *product READ product WRITE setProduct NOTIFY productChanged)
public:
    explicit HardwareParametersOverviewModel(QObject *parent = nullptr);
    ~HardwareParametersOverviewModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    precitec::storage::Product *product() const
    {
        return m_product;
    }
    void setProduct(precitec::storage::Product *product);

    precitec::storage::AttributeModel *keyValueAttributeModel() const
    {
        return m_keyValueAttributeModel;
    }
    void setKeyValueAttributeModel(precitec::storage::AttributeModel *model);

    /**
     * @returns the width of the @p column. Assumes that the visualization uses the default font for cells and a bold default font for the header.
     **/
    Q_INVOKABLE int columnWidth(int column);

Q_SIGNALS:
    void productChanged();
    void keyValueAttributeModelChanged();

private:
    void init();
    void initForProduct();
    void initForSeamSeries(storage::SeamSeries *seamSeries);
    void initForSeam(storage::Seam *seam);
    void initParameterSet(storage::ParameterSet *set);
    precitec::storage::Parameter* dataForParameterSet(const QModelIndex &index, storage::ParameterSet *set) const;
    int calculateColumnWidth(int column) const;
    storage::Product *m_product = nullptr;
    QMetaObject::Connection m_productDestroyed;
    std::set<QString> m_hardwareParameterKeys;
    std::vector<storage::AbstractMeasureTask*> m_measureTasks;
    std::map<int, int> m_columnWidth;
    precitec::storage::AttributeModel *m_keyValueAttributeModel = nullptr;
    QMetaObject::Connection m_keyValueAttributeModelDestroyed;
};

}
}
