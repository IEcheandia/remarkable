#include "seamSeriesErrorModel.h"
#include "seamSeriesError.h"
#include "seamSeries.h"

using precitec::storage::SeamSeries;
using precitec::storage::SeamSeriesError;

namespace precitec
{
namespace gui
{

SeamSeriesErrorModel::SeamSeriesErrorModel(QObject *parent)
    : OverlyingErrorModel(parent)
{
}

SeamSeriesErrorModel::~SeamSeriesErrorModel() = default;

QVariant SeamSeriesErrorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !currentSeamSeries())
    {
        return {};
    }
    const auto &errors = currentSeamSeries()->overlyingErrors();
    if (index.row() > int(errors.size()))
    {
        return {};
    }
    auto error = errors.at(index.row());
    if (role == Qt::DisplayRole)
    {
         return error->name();
    }
    if (role == Qt::UserRole)
    {
         return QVariant::fromValue(error);
    }
    if (role == Qt::UserRole + 1)
    {
         return nameFromId(error->variantId());
    }
    if (role == Qt::UserRole + 2)
    {
         return isTypeError(error->variantId());
    }
    return {};
}

int SeamSeriesErrorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !currentSeamSeries())
    {
        return 0;
    }
    return currentSeamSeries()->overlyingErrors().size();
}

QHash<int, QByteArray> SeamSeriesErrorModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("error")},
        {Qt::UserRole + 1, QByteArrayLiteral("type")},
        {Qt::UserRole + 2, QByteArrayLiteral("isTypeError")}
    };
}

void SeamSeriesErrorModel::setCurrentSeamSeries(SeamSeries *seamSeries)
{
    if (m_currentSeamSeries == seamSeries)
    {
        return;
    }
    beginResetModel();
    m_currentSeamSeries = seamSeries;
    disconnect(m_destroyConnection);
    m_destroyConnection = QMetaObject::Connection{};
    if (m_currentSeamSeries)
    {
        m_destroyConnection = connect(m_currentSeamSeries, &QObject::destroyed, this, std::bind(&SeamSeriesErrorModel::setCurrentSeamSeries, this, nullptr));
    } else
    {
        m_destroyConnection = {};
    }
    endResetModel();
    emit currentSeamSeriesChanged();
}

SeamSeriesError *SeamSeriesErrorModel::createError(ErrorType errorType)
{
    if (!m_currentSeamSeries)
    {
        return nullptr;
    }
    beginInsertRows({}, rowCount(), rowCount());

    auto error = m_currentSeamSeries->addOverlyingError(variantId(errorType));
    error->setName(name(errorType));

    if (attributeModel())
    {
        error->initFromAttributes(attributeModel());
    }
    endInsertRows();

    return error;
}

void SeamSeriesErrorModel::removeError(SeamSeriesError *error)
{
    if (!error || !m_currentSeamSeries)
    {
        return;
    }
    const auto &errors = m_currentSeamSeries->overlyingErrors();
    auto it = std::find(errors.begin(), errors.end(), error);
    if (it == errors.end())
    {
        return;
    }
    auto index = std::distance(errors.begin(), it);
    beginRemoveRows({}, index, index);
    m_currentSeamSeries->removeOverlyingError(index);
    endRemoveRows();
}

}
}



