#include "seamSelectionModel.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "linkedSeam.h"
#include "seamPropertyModel.h"
#include "productController.h"

#include <QFontMetrics>

using precitec::storage::Seam;
using precitec::storage::LinkedSeam;
using precitec::storage::Product;

namespace precitec
{
namespace gui
{

SeamSelectionModel::SeamSelectionModel ( QObject* parent )
    : QAbstractTableModel(parent)
{
    connect(this, &SeamSelectionModel::productChanged, this, &SeamSelectionModel::updateTable);
    connect(this, &SeamSelectionModel::fontChanged, this, &SeamSelectionModel::updateTable);
    connect(this, &SeamSelectionModel::modelReset, this, &SeamSelectionModel::rowCountChanged);
    connect(this, &SeamSelectionModel::modelReset, this, &SeamSelectionModel::columnCountChanged);
    connect(this, &SeamSelectionModel::currentSeamChanged, this, std::bind(&SeamSelectionModel::dataChanged, this, index(0,0), index(rowCount() - 1, columnCount() - 1), QVector<int>{Qt::UserRole + 6}));
}

SeamSelectionModel::~SeamSelectionModel() = default;

QHash<int, QByteArray> SeamSelectionModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("enabled")},
        {Qt::UserRole + 1, QByteArrayLiteral("selected")},
        {Qt::UserRole + 2, QByteArrayLiteral("top")},
        {Qt::UserRole + 3, QByteArrayLiteral("bottom")},
        {Qt::UserRole + 4, QByteArrayLiteral("left")},
        {Qt::UserRole + 5, QByteArrayLiteral("right")},
        {Qt::UserRole + 6, QByteArrayLiteral("current")},
        {Qt::UserRole + 7, QByteArrayLiteral("linked")}
    };
}

QVariant SeamSelectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_currentSeam || index.row() >= int(m_seamTable.rowCount()) || index.column() >= int(m_seamTable.columnCount()))
    {
        return {};
    }
    switch (role)
    {
        case Qt::DisplayRole:
        {
            return m_seamTable.item(index.row(), index.column()).name;
        }
        case Qt::UserRole:
            return m_seamTable.item(index.row(), index.column()).number != -1;
        case Qt::UserRole + 1:
            return m_seamTable.item(index.row(), index.column()).selected;
        case Qt::UserRole + 2:
            if (m_seamTable.item(index.row(), index.column()).selected)
            {
                return index.row() == 0u ? true : !m_seamTable.item(index.row() - 1, index.column()).selected;
            }
            return false;
        case Qt::UserRole + 3:
            if (m_seamTable.item(index.row(), index.column()).selected)
            {
                return index.row() == int(m_seamTable.rowCount()) - 1 ? true : !m_seamTable.item(index.row() + 1, index.column()).selected;
            }
            return false;
        case Qt::UserRole + 4:
            if (m_seamTable.item(index.row(), index.column()).selected)
            {
                return index.column() == 0u ? true : !m_seamTable.item(index.row(), index.column() - 1).selected;
            }
            return false;
        case Qt::UserRole + 5:
            if (m_seamTable.item(index.row(), index.column()).selected)
            {
                return index.column() == int(m_seamTable.columnCount()) - 1 ? true : !m_seamTable.item(index.row(), index.column() + 1).selected;
            }
            return false;
        case Qt::UserRole + 6:
            return m_seamTable.item(index.row(), index.column()).id == m_currentSeam->uuid();
        case Qt::UserRole + 7:
            return m_seamTable.item(index.row(), index.column()).linked;
        default:
            return {};
    }
}

bool SeamSelectionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::UserRole + 1 || index.row() >= int(m_seamTable.rowCount()) || index.column() >= int(m_seamTable.columnCount()))
    {
        return false;
    }

    m_seamTable.select(index.row(), index.column(), value.toBool());

    const auto topIdx = index.siblingAtRow(index.row() + 1);
    const auto bottomIdx = index.siblingAtRow(index.row() - 1);
    const auto leftIdx = index.siblingAtColumn(index.column() + 1);
    const auto rightIdx = index.siblingAtColumn(index.column() - 1);

    emit dataChanged(index, index, {Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3, Qt::UserRole + 4, Qt::UserRole + 5});
    emit dataChanged(topIdx, topIdx, {Qt::UserRole + 2});
    emit dataChanged(bottomIdx, bottomIdx, {Qt::UserRole + 3});
    emit dataChanged(leftIdx, leftIdx, {Qt::UserRole + 4});
    emit dataChanged(rightIdx, rightIdx, {Qt::UserRole + 5});

    return true;
}

int SeamSelectionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_currentSeam)
    {
        return 0;
    }
    return m_seamTable.rowCount();
}

int SeamSelectionModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_currentSeam)
    {
        return 0;
    }
    return m_seamTable.columnCount();
}

Qt::ItemFlags SeamSelectionModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant SeamSelectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return {};
    }
    if (orientation == Qt::Horizontal)
    {
        return QStringLiteral("Seam %1").arg(QString::number(section + 1));
    } else
    {
        return QStringLiteral("Series %1").arg(QString::number(section + 1));
    }
}

void SeamSelectionModel::setCurrentSeam(Seam* seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }

    m_currentSeam = seam;
    disconnect(m_seamDestroyed);
    if (m_currentSeam)
    {
        m_seamDestroyed = connect(m_currentSeam, &Seam::destroyed, this, std::bind(&SeamSelectionModel::setCurrentSeam, this, nullptr));
    } else
    {
        m_seamDestroyed = QMetaObject::Connection{};
    }

    emit currentSeamChanged();
}

void SeamSelectionModel::setProduct(Product* product)
{
    if (m_product == product)
    {
        return;
    }

    m_product = product;
    disconnect(m_productDestroyed);
    if (m_product)
    {
        m_productDestroyed = connect(m_product, &Product::destroyed, this, std::bind(&SeamSelectionModel::setProduct, this, nullptr));
    } else
    {
        m_productDestroyed = {};
    }

    emit productChanged();
}

void SeamSelectionModel::setPropertyModel(SeamPropertyModel* propertyModel)
{
    if (m_propertyModel == propertyModel)
    {
        return;
    }

    m_propertyModel = propertyModel;
    disconnect(m_propertyModelDestroyed);
    if (m_propertyModel)
    {
        m_propertyModelDestroyed = connect(m_propertyModel, &SeamPropertyModel::destroyed, this, std::bind(&SeamSelectionModel::setPropertyModel, this, nullptr));
    } else
    {
        m_propertyModelDestroyed = {};
    }

    emit propertyModelChanged();
}

void SeamSelectionModel::setProductController(ProductController* productController)
{
    if (m_productController == productController)
    {
        return;
    }

    m_productController = productController;
    disconnect(m_productControllerDestroyed);
    if (m_productController)
    {
        m_productControllerDestroyed = connect(m_productController, &ProductController::destroyed, this, std::bind(&SeamSelectionModel::setProductController, this, nullptr));
    } else
    {
        m_productControllerDestroyed = {};
    }

    emit productControllerChanged();
}

void SeamSelectionModel::setFont(const QFont& font)
{
    if (m_font == font)
    {
        return;
    }
    m_font = font;
    emit fontChanged();
}

void SeamSelectionModel::updateTable()
{
    if (!m_product)
    {
        m_seamTable.clear();
        return;
    }

    const auto rowCount = m_product->seamSeries().size();
    auto columnCount = 0u;
    for (auto series : m_product->seamSeries())
    {
        if (series->seams().size() > columnCount)
        {
            columnCount = series->seams().size();
        }
    }

    QFontMetrics fontMetrics(m_font);

    beginResetModel();
    m_seamTable.setSize(rowCount, columnCount);

    for (auto i = 0u; i < rowCount; i++)
    {
        const auto series = m_product->seamSeries().at(i);

        for (auto j = 0u; j < series->seams().size(); j++)
        {
            const auto seam = series->seams().at(j);

            m_seamTable.setItem(i, j, seam->number(), uint(fontMetrics.horizontalAdvance(seam->name())), seam->name(), seam->uuid(), false, seam->metaObject()->inherits(&LinkedSeam::staticMetaObject));
        }
    }
    endResetModel();
}

void SeamSelectionModel::selectRow(int row)
{
    const auto selected = m_seamTable.rowSelected(row);
    for (auto i = 0u; i < m_seamTable.columnCount(); i++)
    {
        const auto& seamId = m_seamTable.item(row, i).id;
        if (seamId == m_currentSeam->uuid() || seamId.isNull())
        {
            continue;
        }
        m_seamTable.select(row, i, !selected);
    }

    emit dataChanged(index(row, 0), index(row, columnCount() - 1), {Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3, Qt::UserRole + 4, Qt::UserRole + 5});
    emit dataChanged(index(row + 1, 0), index(row + 1, columnCount() - 1), {Qt::UserRole + 2});
    emit dataChanged(index(row - 1, 0), index(row - 1, columnCount() - 1), {Qt::UserRole + 3});
}

void SeamSelectionModel::selectColumn(int column)
{
    const auto selected = m_seamTable.columnSelected(column);
    for (auto i = 0u; i < m_seamTable.rowCount(); i++)
    {
        const auto& seamId = m_seamTable.item(i, column).id;
        if (seamId == m_currentSeam->uuid() || seamId.isNull())
        {
            continue;
        }
        m_seamTable.select(i, column, !selected);
    }

    emit dataChanged(index(0, column), index(rowCount() - 1, column), {Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3, Qt::UserRole + 4, Qt::UserRole + 5});
    emit dataChanged(index(0, column + 1), index(rowCount() - 1, column + 1), {Qt::UserRole + 4});
    emit dataChanged(index(0, column - 1), index(rowCount() - 1, column - 1), {Qt::UserRole + 5});
}

void SeamSelectionModel::selectAll()
{
    for (auto i = 0u; i < m_seamTable.rowCount(); i++)
    {
        for (auto j = 0u; j < m_seamTable.columnCount(); j++)
        {
            if (m_seamTable.item(i, j).id.isNull() || m_seamTable.item(i, j).id == m_currentSeam->uuid())
            {
                continue;
            }
            m_seamTable.select(i, j, true);
        }
    }

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3, Qt::UserRole + 4, Qt::UserRole + 5});
}

void SeamSelectionModel::selectNone()
{
    for (auto i = 0u; i < m_seamTable.rowCount(); i++)
    {
        for (auto j = 0u; j < m_seamTable.columnCount(); j++)
        {
            m_seamTable.select(i, j, false);
        }
    }

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3, Qt::UserRole + 4, Qt::UserRole + 5});
}

void SeamSelectionModel::save()
{
    if (m_seamTable.empty() || !m_productController || !m_propertyModel || !m_product)
    {
        return;
    }

    // acquire items before modified product
    // creating the modified product leads to the productController sending a dataChanged signal, which resets the product and that resets the table
    const auto tableItems = m_seamTable.items();

    auto modifiedProduct = m_productController->modifiedProduct(m_product->uuid());

    if (!modifiedProduct)
    {
        return;
    }

    auto changed = false;

    for (const auto& item: tableItems)
    {
        if (!item.selected)
        {
            continue;
        }

        auto seam = modifiedProduct->findSeam(item.id);

        if (seam == m_currentSeam)
        {
            continue;
        }

        changed |= m_propertyModel->copy(m_currentSeam, seam);
    }

    if (changed)
    {
        emit markAsChanged();
    }
}

uint SeamSelectionModel::columnWidth(uint column)
{
    if (!m_product || column >= m_seamTable.columnCount())
    {
        return 0;
    }

    auto width = 0u;

    for (auto i = 0u; i < m_seamTable.rowCount(); i++)
    {
        width = std::max(m_seamTable.item(i, column).textWidth, width);
    }

    return width;
}

}
}

