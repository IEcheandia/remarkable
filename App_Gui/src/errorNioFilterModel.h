#pragma once
#include "nioSettingModel.h"
#include <QObject>
#include <QSortFilterProxyModel>

namespace precitec {
   

namespace gui {

class ErrorNioFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool sortAsc MEMBER m_sortAsc NOTIFY sortAscChanged)

public:
    ErrorNioFilterModel(QObject *parent = nullptr);
    ~ErrorNioFilterModel() override;

    /**
     * returns the index of item in the filterd list, given by the key (the enum)
     */
    Q_INVOKABLE int findIndex(int value);
    
protected:
    virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
    void sortingChanged();

Q_SIGNALS:
    void sortAscChanged();

private:
    bool m_sortAsc;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ErrorNioFilterModel*)


