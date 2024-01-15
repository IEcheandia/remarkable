#include "seamErrorModel.h"
#include "seam.h"
#include "seamError.h"

using precitec::storage::SeamError;

namespace precitec
{
namespace gui
{

SeamErrorModel::SeamErrorModel(QObject *parent)
    : SimpleErrorModel(parent)
{
}

SeamErrorModel::~SeamErrorModel() = default;

QHash<int, QByteArray> SeamErrorModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("error")},
        {Qt::UserRole + 1, QByteArrayLiteral("type")}
    };
}

QVariant SeamErrorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !currentSeam())
    {
        return QVariant{};
    }
    const auto &errors = currentSeam()->seamErrors();
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
    return QVariant{};
}

int SeamErrorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !currentSeam())
    {
        return 0;
    }
    return currentSeam()->seamErrors().size();
}

SeamError *SeamErrorModel::createError(ErrorType errorType)
{
    if (!currentSeam())
    {
        return nullptr;
    }
    beginInsertRows({}, rowCount(), rowCount());
    auto error = addError(errorType);
    endInsertRows();

    return error;
}

void SeamErrorModel::removeError(SeamError *error)
{
    if (!error)
    {
        return;
    }
    if (!currentSeam())
    {
        return;
    }
    const auto &seamErrors = currentSeam()->seamErrors();
    auto it = std::find(seamErrors.begin(), seamErrors.end(), error);
    if (it == seamErrors.end())
    {
        return;
    }
    (*it)->unsubscribe();
    auto index = std::distance(seamErrors.begin(), it);
    beginRemoveRows({}, index, index);
    currentSeam()->removeError(index);
    endRemoveRows();
}

}
}
