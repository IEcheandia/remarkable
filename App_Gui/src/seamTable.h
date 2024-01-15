#pragma once

#include <QString>
#include <QUuid>

#include <vector>
#include <tuple>

namespace precitec
{
namespace gui
{

struct TableItem {
    int number = -1;
    uint textWidth = 0;
    QString name;
    QUuid id;
    bool selected = false;
    bool linked = false;
};

class SeamTable
{

public:
    SeamTable();

    uint rowCount() const
    {
        return m_rowCount;
    }

    uint columnCount() const
    {
        return m_columnCount;
    }

    void setSize(uint rows, uint columns);

    TableItem item(uint row, uint column) const
    {
        return m_items.at(row * m_columnCount + column);
    }

    void select(uint row, uint column, bool select);

    std::vector<TableItem> items() const
    {
        return m_items;
    }
    void setItem(uint row, uint column, int number, uint textWidth, const QString& name, const QUuid& id, bool selected, bool linked);

    bool empty()
    {
        return m_items.size() == 0;
    }
    void clear();

    bool rowSelected(uint row);
    bool columnSelected(uint column);

private:
    uint m_rowCount = 0;
    uint m_columnCount = 0;

    std::vector<TableItem> m_items;
};

}
}
