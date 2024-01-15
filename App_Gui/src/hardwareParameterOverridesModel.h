#pragma once
#include <QAbstractListModel>

namespace precitec
{

namespace storage
{
class AbstractMeasureTask;
class Parameter;
class Product;
class Seam;
class SeamSeries;
}

namespace gui
{
class AbstractHardwareParameterModel;

/**
 * This model provides information whether a hardware parameter overrides a parameter defined on a higher level.
 * E.g. whether a hardware parameter defined on Seam level overrides a parameter defined on SeamSeries or Product.
 **/
class HardwareParameterOverridesModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The model providing the hardware Parameter.
     **/
    Q_PROPERTY(precitec::gui::AbstractHardwareParameterModel *hardwareParameterModel READ hardwareParameterModel WRITE setHardwareParameterModel NOTIFY hardwareParameterModelChanged)
    /**
     * The index for @link{hardwareParameterIndex} for which the values the @link{hardwareParameterModel} overrides are extracted.
     **/
    Q_PROPERTY(QModelIndex hardwareParameterIndex READ hardwareParameterIndex WRITE setHardwareParameterIndex NOTIFY hardwareParameterIndexChanged)
public:
    HardwareParameterOverridesModel(QObject *parent = nullptr);
    ~HardwareParameterOverridesModel() override;

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    precitec::gui::AbstractHardwareParameterModel *hardwareParameterModel() const
    {
        return m_hardwareParameterModel;
    }
    void setHardwareParameterModel(AbstractHardwareParameterModel *model);

    QModelIndex hardwareParameterIndex() const
    {
        return m_parameterModelIndex;
    }
    void setHardwareParameterIndex(const QModelIndex &index);

Q_SIGNALS:
    void hardwareParameterModelChanged();
    void hardwareParameterIndexChanged();

private:
    void init();
    void findForSeam(storage::Seam *seam, storage::Parameter *parameter);
    void findForSeamSeries(storage::SeamSeries *seamSeries, storage::Parameter *parameter);
    precitec::gui::AbstractHardwareParameterModel *m_hardwareParameterModel = nullptr;
    QMetaObject::Connection m_hardwareParameterModelDestroyedConnection;
    QMetaObject::Connection m_hardwareParameterModelDataChanged;
    QModelIndex m_parameterModelIndex;

    struct OverridesData
    {
        OverridesData(storage::Parameter *parameter, storage::SeamSeries *seamSeries)
            : parameter{parameter}
            , seamSeries{seamSeries}
            {
            }
        OverridesData(storage::Parameter *parameter, storage::Product *product)
            : parameter{parameter}
            , product{product}
            {
            }
        storage::Parameter *parameter = nullptr;
        storage::SeamSeries *seamSeries = nullptr;
        storage::Product *product = nullptr;
    };
    std::vector<OverridesData> m_data;
};

}
}

