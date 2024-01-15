#include "errorGroupModel.h"

namespace precitec
{
namespace gui
{

std::map<ErrorGroupModel::ErrorGroup, std::tuple<std::string, std::string, QString>> error_groups {
    {ErrorGroupModel::ErrorGroup::LengthOutsideBoundary, 
        {QT_TRANSLATE_NOOP("", "Length Outside Boundary"), 
        QT_TRANSLATE_NOOP("", "Verifies that the <b>length</b> of a section or a sum of sections with values <b>outside</b> a set boundary range is <b>below</b> a given <b>threshold</b> value"),
        {QStringLiteral("../images/error-LOB-StatBoundFail.png")}}},
    {ErrorGroupModel::ErrorGroup::LengthInsideBoundary, 
        {QT_TRANSLATE_NOOP("", "Length Inside Boundary"), 
        QT_TRANSLATE_NOOP("", "Verifies that the <b>length</b> of a section with values <b>within</b> a set boundary range is <b>above</b> a given <b>threshold</b> value"),
        {QStringLiteral("../images/error-LIB-StatBoundFail.png")}}},
    {ErrorGroupModel::ErrorGroup::AreaOutsideBoundary, 
        {QT_TRANSLATE_NOOP("", "Area Outside Boundary"), 
        QT_TRANSLATE_NOOP("", "Verifies that the <b>area</b> of a section or a sum of sections with values <b>outside</b> a set boundary range is <b>below</b> a given <b>threshold</b> value"),
        {QStringLiteral("../images/error-AOB-StatBoundFail.png")}}},
    {ErrorGroupModel::ErrorGroup::PeakOutsideBoundary, 
        {QT_TRANSLATE_NOOP("", "Peak Outside Boundary"), 
        QT_TRANSLATE_NOOP("", "Verifies that no result <b>value</b> lies more than a given <b>threshold</b> value <b>outside</b> a set boundary range"),
        {QStringLiteral("../images/error-POB-StatBoundFail.png")}}}
};

ErrorGroupModel::ErrorGroupModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ErrorGroupModel::~ErrorGroupModel() = default;

QVariant ErrorGroupModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    if (index.row() > int(error_groups.size()))
    {
        return QVariant{};
    }
    auto it = error_groups.begin();
    std::advance(it, index.row());
    if (role == Qt::DisplayRole)
    {
         return QString::fromStdString(std::get<0>((*it).second));
    }
    if (role == Qt::UserRole)
    {
         return QString::fromStdString(std::get<1>((*it).second));
    }
    if (role == Qt::UserRole + 1)
    {
         return std::get<QString>((*it).second);
    }
    if (role == Qt::UserRole + 2)
    {
         return QVariant::fromValue((*it).first);
    }
    return QVariant{};
}

int ErrorGroupModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return error_groups.size();
}

QHash<int, QByteArray> ErrorGroupModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("description")},
        {Qt::UserRole + 1, QByteArrayLiteral("image")},
        {Qt::UserRole + 2, QByteArrayLiteral("type")}
    };
}

}
}


