#pragma once

#include <QSortFilterProxyModel>
#include <QPointF>

namespace precitec
{
namespace gui
{

/**
 * @brief Filter proxy model to filter on InfoBoxes with a position inside their bounding box
 **/
class InfoBoxFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool multipleSelection READ multipleSelection WRITE setMultipleSelection NOTIFY multipleSelectionChanged)

public:
    InfoBoxFilterModel(QObject *parent = nullptr);
    ~InfoBoxFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool multipleSelection() const
    {
        return m_multipleSelection;
    }
    void setMultipleSelection(bool set);

    Q_INVOKABLE void addPosition(const QPointF &position);

    Q_INVOKABLE void clear();

Q_SIGNALS:
    void positionAdded();
    void multipleSelectionChanged();

private:
    std::vector<QPointF> m_positions;
    bool m_multipleSelection = false;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::InfoBoxFilterModel*)


