#include "assemblyImageFromProductInstanceTableModel.h"
#include "seam.h"

namespace precitec
{

using storage::Product;
using storage::ProductInstanceTableModel;
using storage::Seam;

namespace gui
{

AssemblyImageFromProductInstanceTableModel::AssemblyImageFromProductInstanceTableModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

AssemblyImageFromProductInstanceTableModel::~AssemblyImageFromProductInstanceTableModel() = default;

QVariant AssemblyImageFromProductInstanceTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    const auto &data = m_seams.at(index.row());
    if (!data.seam)
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return data.seam->name();
    case Qt::UserRole:
        return data.seam->positionInAssemblyImage();
    case Qt::UserRole + 2:
        return QVariant::fromValue(data.state);
    case Qt::UserRole + 3:
        return QVariant::fromValue(data.seam.data());
    }
    return {};
}

int AssemblyImageFromProductInstanceTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_seams.size();
}

QHash<int, QByteArray> AssemblyImageFromProductInstanceTableModel::roleNames() const
{
    // roles as provided by the AssemblyImageInspectionModel
    return {
        {Qt::DisplayRole, QByteArrayLiteral("seamName")},
        {Qt::UserRole, QByteArrayLiteral("position")},
        {Qt::UserRole + 2, QByteArrayLiteral("state")},
        {Qt::UserRole + 3, QByteArrayLiteral("seam")},
    };
}

void AssemblyImageFromProductInstanceTableModel::init(int row)
{
    beginResetModel();
    m_seams.clear();

    if (m_productInstanceTableModel)
    {
        m_seams.reserve(m_productInstanceTableModel->columnCount() - 1);

        // i == 0 is the complete product instance
        for (int i = 1; i < m_productInstanceTableModel->columnCount(); i++)
        {
            const auto index = m_productInstanceTableModel->index(row, i);
            if (!index.isValid())
            {
                continue;
            }
            auto seam = m_productInstanceTableModel->data(index, Qt::UserRole + 10).value<Seam*>();
            if (!seam)
            {
                continue;
            }
            AssemblyImageInspectionModel::Data data;
            data.seam = seam;
            switch (m_productInstanceTableModel->data(index, Qt::UserRole + 2).value<ProductInstanceTableModel::State>())
            {
            case ProductInstanceTableModel::State::Io:
                data.state = AssemblyImageInspectionModel::SeamState::Success;
                break;
            case ProductInstanceTableModel::State::Nio:
                data.state = AssemblyImageInspectionModel::SeamState::Failure;
                break;
            default:
                data.state = AssemblyImageInspectionModel::SeamState::Uninspected;
                break;
            }
            m_seams.push_back(data);
        }
    }

    endResetModel();
}

void AssemblyImageFromProductInstanceTableModel::setProductInstanceTableModel(storage::ProductInstanceTableModel *productInstanceTableModel)
{
    if (m_productInstanceTableModel == productInstanceTableModel)
    {
        return;
    }
    m_productInstanceTableModel = productInstanceTableModel;
    disconnect(m_modelDestroyedConnection);
    m_modelDestroyedConnection = {};

    if (m_productInstanceTableModel)
    {
        m_modelDestroyedConnection = connect(m_productInstanceTableModel, &ProductInstanceTableModel::destroyed, this, std::bind(&AssemblyImageFromProductInstanceTableModel::setProductInstanceTableModel, this, nullptr));
    }

    emit productInstanceTableModelChanged();
}

}
}
