#include "lineModel.h"

#include "filter/parameterEnums.h"

using precitec::filter::LaserLine;

namespace precitec
{
namespace gui
{

LineModel::LineModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

LineModel::~LineModel() = default;

QHash<int, QByteArray> LineModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")}
    };
}

int LineModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return (int)(LaserLine::NumberLaserLines);
}

QVariant LineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    switch (role)
    {
    case Qt::DisplayRole:
        return QStringLiteral("Laser Line ").append(QString::number(index.row() + 1));
    default:
        return {};
    }
}

}
}

