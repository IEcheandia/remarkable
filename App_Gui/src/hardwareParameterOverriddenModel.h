#pragma once
#include <QAbstractListModel>

namespace precitec
{

namespace storage
{
class AbstractMeasureTask;
class Parameter;
class Product;
class SeamSeries;
}

namespace gui
{
class AbstractHardwareParameterModel;

/**
 * This model provides information whether a hardware parameter is overridden in a lower level.
 * E.g. whether a hardware parameter defined on Product level is overridden in a seam series or seam.
 **/
class HardwareParameterOverriddenModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The model providing the hardware Parameter.
     **/
    Q_PROPERTY(precitec::gui::AbstractHardwareParameterModel *hardwareParameterModel READ hardwareParameterModel WRITE setHardwareParameterModel NOTIFY hardwareParameterModelChanged)
    /**
     * The index for @link{hardwareParameterIndex} for which the overridden values should be found.
     **/
    Q_PROPERTY(QModelIndex hardwareParameterIndex READ hardwareParameterIndex WRITE setHardwareParameterIndex NOTIFY hardwareParameterIndexChanged)
public:
    HardwareParameterOverriddenModel(QObject *parent = nullptr);
    ~HardwareParameterOverriddenModel() override;

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
    void findForProduct(storage::Product *product, storage::Parameter *parameter);
    void findForSeamSeries(storage::SeamSeries *seamSeries, storage::Parameter *parameter);
    precitec::gui::AbstractHardwareParameterModel *m_hardwareParameterModel = nullptr;
    QMetaObject::Connection m_hardwareParameterModelDestroyedConnection;
    QMetaObject::Connection m_hardwareParameterModelDataChanged;
    QModelIndex m_parameterModelIndex;

    struct OverriddenData
    {
        OverriddenData(storage::Parameter *parameter, storage::AbstractMeasureTask *measureTask)
            : parameter{parameter}
            , measureTask{measureTask}
            {
            }
        storage::Parameter *parameter = nullptr;
        storage::AbstractMeasureTask *measureTask = nullptr;
    };
    std::vector<OverriddenData> m_data;
};

}
}
