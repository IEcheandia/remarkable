#pragma once

#include "resultSetting.h"

#include <QSortFilterProxyModel>

namespace precitec
{
namespace storage
{
class ResultSettingModel;
}
namespace gui
{

class ResultSettingFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultSetting::Type sortOnType MEMBER m_sortOnType NOTIFY sortOnTypeChanged)

    Q_PROPERTY(bool sortAsc MEMBER m_sortAsc NOTIFY sortAscChanged)

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    ResultSettingFilterModel(QObject *parent = nullptr);
    ~ResultSettingFilterModel() override;

    precitec::storage::ResultSettingModel* resultModel() const;
    void setResultModel(precitec::storage::ResultSettingModel* newModel);

    QString searchText() const
    {
        return m_searchText;
    }
    void setSearchText(const QString &searchText);

    /**
     * returns the index of item in the filterd list, given by the key (the enum)
     */
    Q_INVOKABLE int findIndex(int value);

    /**
     * connect the errorSettingModel::updateUsedFlagChange() after creation of the class
     */
    Q_INVOKABLE void updateUsedFlags();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
    void sortingChanged();

Q_SIGNALS:
    void sortOnTypeChanged();
    void sortAscChanged();
    void searchTextChanged();

private:
    precitec::storage::ResultSetting::Type m_sortOnType;
    bool m_sortAsc;
    QString m_searchText;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ResultSettingFilterModel*)
