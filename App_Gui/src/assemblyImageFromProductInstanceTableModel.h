#pragma once

#include "assemblyImageInspectionModel.h"
#include "productInstanceTableModel.h"

#include <QAbstractListModel>

namespace precitec
{
namespace gui
{

/**
 * Model providing data similar to the AssemblyImageInspectionModel filled from a ProductInstanceTableModel.
 **/
class AssemblyImageFromProductInstanceTableModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The ProductInstanceTableModel from which data to fill this model is taken
     **/
    Q_PROPERTY(precitec::storage::ProductInstanceTableModel *productInstanceTableModel READ productInstanceTableModel WRITE setProductInstanceTableModel NOTIFY productInstanceTableModelChanged)
public:
    explicit AssemblyImageFromProductInstanceTableModel(QObject *parent = nullptr);
    ~AssemblyImageFromProductInstanceTableModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    storage::ProductInstanceTableModel *productInstanceTableModel() const
    {
        return m_productInstanceTableModel;
    }
    void setProductInstanceTableModel(storage::ProductInstanceTableModel *productInstanceTableModel);

    /**
     * Loads the data from the @link{productInstanceTableModel} for the given @p row.
     **/
    Q_INVOKABLE void init(int row);

Q_SIGNALS:
    void productInstanceTableModelChanged();

private:
    storage::ProductInstanceTableModel *m_productInstanceTableModel = nullptr;
    QMetaObject::Connection m_modelDestroyedConnection;

    std::vector<AssemblyImageInspectionModel::Data> m_seams;
};

}
}
