#pragma once

#include <QIdentityProxyModel>

namespace precitec
{
namespace gui
{

/**
 * Identity proxy model for a FileModel from WobbleFigureEditor.
 * Extends the roles by a checked role to track which of the basic figures is checked.
 **/
class BasicFigureSelectionModel : public QIdentityProxyModel
{
    Q_OBJECT
    /**
     * Id of the selected figure. If none is selected, the value is @c -1. Otherwise it matches
     * the data for which the checked role is @c true.
     **/
    Q_PROPERTY(int selectedId READ selectedId WRITE setSelectedId NOTIFY selectedIdChanged)
public:
    explicit BasicFigureSelectionModel(QObject* parent = nullptr);
    ~BasicFigureSelectionModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * Marks the index identified by @p row as checked if it is not already checked.
     **/
    Q_INVOKABLE void markAsChecked(int row);

    int selectedId() const
    {
        return m_selectedId;
    }
    void setSelectedId(int index);

Q_SIGNALS:
    void selectedIdChanged();

private:
    int getId(const QModelIndex& index) const;
    // Default id of first basic figure
    int m_selectedId = 1;
};

}
}
