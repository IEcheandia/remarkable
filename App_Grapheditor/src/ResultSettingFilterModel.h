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
namespace components
{
namespace grapheditor
{

class ResultSettingFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultSettingModel* resultModel READ resultModel WRITE setResultModel NOTIFY resultModelChanged)

    Q_PROPERTY(precitec::storage::ResultSetting::Type sortOnType MEMBER m_sortOnType NOTIFY sortOnTypeChanged)

    Q_PROPERTY(bool sortAsc MEMBER m_sortAsc NOTIFY sortAscChanged)

public:
    ResultSettingFilterModel(QObject *parent = nullptr);
    ~ResultSettingFilterModel() override;

    precitec::storage::ResultSettingModel* resultModel() const;
    void setResultModel(precitec::storage::ResultSettingModel* newModel);

    /**
     * returns the index of item in the filterd list, given by the key (the enum)
     */
    Q_INVOKABLE int findIndex(int value);

//From resultSettingModel to work as expected.
    Q_INVOKABLE QModelIndex indexForResultType(int enumType) const;
    Q_INVOKABLE void ensureItemExists(int enumType);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
    void sortingChanged();

Q_SIGNALS:
    void sortOnTypeChanged();
    void sortAscChanged();
    void searchTextChanged();
    void resultModelChanged();

private:
    precitec::storage::ResultSetting::Type m_sortOnType;
    bool m_sortAsc;
    precitec::storage::ResultSettingModel* m_resultModel = nullptr;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::ResultSettingFilterModel*)
