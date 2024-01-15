#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

class SettingNameFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    SettingNameFilterModel(QObject *parent = nullptr);
    ~SettingNameFilterModel() override;

    void setSearchText(const QString& text);
    QString searchText() const;

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

Q_SIGNALS:
    void searchTextChanged();

private:
    QString m_searchText;
};
}
}
Q_DECLARE_METATYPE(precitec::gui::SettingNameFilterModel*)


