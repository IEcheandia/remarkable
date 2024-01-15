#include "moduleModel.h"

#include <functional>

namespace precitec
{
namespace gui
{
namespace components
{
namespace logging
{

ModuleModel::ModuleModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_longestItem(index(0, 0))
{
    connect(this, &ModuleModel::rowsInserted, this,
        [this]
        {
            const auto it = std::max_element(m_modules.begin(), m_modules.end(), [] (const auto &a, const auto &b) { return a.size() < b.size(); });
            if (it == m_modules.end() || it->size() <= m_longestItem.data().toByteArray().size())
            {
                return;
            }
            m_longestItem = index(std::distance(m_modules.begin(), it) + 1, 0);
            emit longestItemChanged();
        }
    );
}

ModuleModel::~ModuleModel() = default;

int ModuleModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    // one more for the "All" item
    return m_modules.size() + 1;
}

QVariant ModuleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
    {
        return QVariant();
    }
    if (index.row() == 0)
    {
        return tr("All");
    }
    return m_modules.at(index.row() - 1);
}

void ModuleModel::setLogModel(const QPointer<QAbstractItemModel> &model)
{
    m_logModel = model;
    connect(m_logModel, &QAbstractItemModel::rowsInserted, this, std::bind(&ModuleModel::checkNewElements, this, std::placeholders::_2, std::placeholders::_3));
    auto checkAll = [this]
    {
        if (m_logModel->rowCount() > 0)
        {
            checkNewElements(0, m_logModel->rowCount() - 1);
        }
    };
    connect(m_logModel, &QAbstractItemModel::modelReset, this, checkAll);
    checkAll();
}

void ModuleModel::checkNewElements(int first, int last)
{
    std::vector<QByteArray> newElements;
    for (int i = first; i <= last; i++)
    {
        const auto name = m_logModel->index(i, 0).data(Qt::UserRole + 1).toByteArray();
        if (std::none_of(m_modules.begin(), m_modules.end(), [&name] (const auto &compare) { return compare == name; }) &&
            std::none_of(newElements.begin(), newElements.end(), [&name] (const auto &compare) { return compare == name; }))
        {
            newElements.push_back(std::move(name));
        }
    }
    if (!newElements.empty())
    {
        beginInsertRows(QModelIndex(), m_modules.size() + 1, m_modules.size() + newElements.size());
        m_modules.reserve(m_modules.size() + newElements.size());
        for (std::size_t i = 0; i < newElements.size(); i++)
        {
            m_modules.push_back(std::move(newElements.at(i)));
        }
        endInsertRows();
    }
}

QString ModuleModel::longestItem() const
{
    return m_longestItem.data().toString();
}

}
}
}
}
