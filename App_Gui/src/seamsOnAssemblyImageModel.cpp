#include "seamsOnAssemblyImageModel.h"

#include "linkedSeam.h"
#include "seam.h"
#include "seamSeries.h"
#include "product.h"

using precitec::storage::Product;

namespace precitec
{
namespace gui
{

SeamsOnAssemblyImageModel::SeamsOnAssemblyImageModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &SeamsOnAssemblyImageModel::seamChanged, this,
        [this]
        {
            disconnect(m_productConnection);
            if (m_seam)
            {
                m_productConnection = connect(m_seam->seamSeries()->product(), &Product::seamsChanged, this,
                    [this]
                    {
                        beginResetModel();
                        initSeams();
                        endResetModel();
                    }
                );
            } else
            {
                m_productConnection = {};
            }
            initSeams();
        });
}

SeamsOnAssemblyImageModel::~SeamsOnAssemblyImageModel() = default;

QHash<int, QByteArray> SeamsOnAssemblyImageModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("seamName")},
        {Qt::UserRole, QByteArrayLiteral("position")},
        {Qt::UserRole + 1, QByteArrayLiteral("current")},
        {Qt::UserRole + 2, QByteArrayLiteral("linkedToCurrent")},
    };
}

QVariant SeamsOnAssemblyImageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    auto seam = m_allSeams.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return seam ? seam->name() : QString();
    case Qt::UserRole:
        return seam ? seam->positionInAssemblyImage() : QPointF{-1, -1};
    case Qt::UserRole + 1:
        return seam.data() == m_seam;
    case Qt::UserRole + 2:
        return std::find(m_seam->linkedSeams().begin(), m_seam->linkedSeams().end(), seam.data()) != m_seam->linkedSeams().end();
    }
    return {};
}

int SeamsOnAssemblyImageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_allSeams.size();
}

void SeamsOnAssemblyImageModel::setSeam(precitec::storage::Seam *seam)
{
    if (m_seam == seam)
    {
        return;
    }
    beginResetModel();
    m_seam = seam;

    disconnect(m_seamDestroyedConnection);
    if (m_seam)
    {
        m_seamDestroyedConnection = connect(m_seam, &QObject::destroyed, this, std::bind(&SeamsOnAssemblyImageModel::setSeam, this, nullptr));
    } else
    {
        m_seamDestroyedConnection = {};
    }

    emit seamChanged();
    endResetModel();
}

void SeamsOnAssemblyImageModel::setSeamPosition(qreal x, qreal y)
{
    if (!m_seam)
    {
        return;
    }
    auto it = std::find_if(m_allSeams.begin(), m_allSeams.end(), [this] (auto &seam) { return seam.data() == m_seam; });
    if (it == m_allSeams.end())
    {
        return;
    }
    m_seam->setPositionInAssemblyImage({x, y});
    const auto i = index(std::distance(m_allSeams.begin(), it), 0);
    emit markAsChanged();
    emit dataChanged(i, i, {Qt::UserRole});
}

void SeamsOnAssemblyImageModel::initSeams()
{
    m_allSeams.clear();
    if (!m_seam)
    {
        return;
    }
    auto p = m_seam->seamSeries()->product();
    for (auto series : p->seamSeries())
    {
        const auto &seams = series->seams();
        std::copy(seams.begin(), seams.end(), std::back_inserter(m_allSeams));
    }
}

}
}
