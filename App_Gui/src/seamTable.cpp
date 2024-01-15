#include "seamTable.h"

namespace precitec
{
namespace gui
{

SeamTable::SeamTable()
{
}

void SeamTable::setSize(uint rows, uint columns)
{
    m_items.clear();
    m_rowCount = rows;
    m_columnCount = columns;
    m_items.resize(rows * columns);
}

void SeamTable::setItem(uint row, uint column, int number, uint textWidth, const QString& name, const QUuid& id, bool selected, bool linked)
{
    m_items.at(row * m_columnCount + column) = {number, textWidth, name, id, selected, linked};
}

void SeamTable::select(uint row, uint column, bool select)
{
    auto& item = m_items.at(row * m_columnCount + column);

    if (item.linked)
    {
        return;
    }

    item.selected = select;
}

bool SeamTable::rowSelected(uint row)
{
    auto selected = 0u;
    auto not_selected = 0u;
    for (auto i = 0u; i < m_columnCount; i++)
    {
        if (m_items.at(row * m_columnCount + i).id.isNull() || m_items.at(row * m_columnCount + i).linked)
        {
            continue;
        }

        if (m_items.at(row * m_columnCount + i).selected)
        {
            selected++;
        } else
        {
            not_selected++;
        }
    }
    return selected > not_selected;
}

bool SeamTable::columnSelected(uint column)
{
    auto selected = 0u;
    auto not_selected = 0u;
    for (auto i = 0u; i < m_rowCount; i++)
    {
        if (m_items.at(i * m_columnCount + column).id.isNull() || m_items.at(i * m_columnCount + column).linked)
        {
            continue;
        }

        if (m_items.at(i * m_columnCount + column).selected)
        {
            selected++;
        } else
        {
            not_selected++;
        }
    }
    return selected > not_selected;
}

void SeamTable::clear()
{
    setSize(0, 0);
}

}
}
