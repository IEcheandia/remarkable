#pragma once

#include <QAbstractListModel>
#include <QPointer>

#include <vector>

namespace precitec
{
namespace gui
{
namespace components
{
namespace logging
{

/**
 * @brief Model containing all the unique module names in the LogModel.
 * First item has special value All.
 **/
class ModuleModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The currently longest text item in this model.
     **/
    Q_PROPERTY(QString longestItem READ longestItem NOTIFY longestItemChanged)
public:
    explicit ModuleModel(QObject *parent = nullptr);
    ~ModuleModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Sets the @p model from which the ModuleModel retrieves the module names
     **/
    void setLogModel(const QPointer<QAbstractItemModel> &model);

    QString longestItem() const;

Q_SIGNALS:
    void longestItemChanged();

private:
    void checkNewElements(int first, int last);
    std::vector<QByteArray> m_modules;
    QPointer<QAbstractItemModel> m_logModel;
    QModelIndex m_longestItem;
};

}
}
}
}
