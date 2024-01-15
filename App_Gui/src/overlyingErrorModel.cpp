#include "overlyingErrorModel.h"
#include "attributeModel.h"

#include <QUuid>

using precitec::storage::AttributeModel;

namespace precitec
{
namespace gui
{

std::map<OverlyingErrorModel::ErrorType, std::tuple<std::string, std::string, QString, QUuid> > overlying_error_keys {
    {OverlyingErrorModel::ErrorType::ConsecutiveTypeErrors, {QT_TRANSLATE_NOOP("", "Consecutive Type Errors"), QT_TRANSLATE_NOOP("", "The number of <b>Consecutive Seams</b> in which a <b>Certain Type</b> of error occurs exceeds a predefined <b>Threshold</b> value"), QStringLiteral("../images/error-ConsecSeamErr.png"), QUuid("37E21057-EFD4-4C18-A298-BE9F804C6C04")}},
    {OverlyingErrorModel::ErrorType::AccumulatedTypeErrors, {QT_TRANSLATE_NOOP("", "Accumulated Type Errors"), QT_TRANSLATE_NOOP("", "The number of <b>Accumulated Seams</b> in which a <b>Certain Type</b> of error occurs exceeds a predefined <b>Threshold</b> value"), QStringLiteral("../images/error-AccuSeamErr.png"), QUuid("C19E43C7-EBC0-4771-A701-BA102511AD9F")}},
    {OverlyingErrorModel::ErrorType::ConsecutiveSeamErrors, {QT_TRANSLATE_NOOP("", "Consecutive Seam Errors"), QT_TRANSLATE_NOOP("", "The number of <b>Consecutive Seams</b> in which <b>Any Type</b> of error occurs exceeds a predefined <b>Threshold</b> value"), QStringLiteral("../images/error-ConsecSeamErr.png"), QUuid("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50")}},
    {OverlyingErrorModel::ErrorType::AccumulatedSeamErrors, {QT_TRANSLATE_NOOP("", "Accumulated Seam Errors"), QT_TRANSLATE_NOOP("", "The number of <b>Accumulated Seams</b> in which <b>Any Type</b> of error occurs exceeds a predefined <b>Threshold</b> value"), QStringLiteral("../images/error-AccuSeamErr.png"), QUuid("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F")}}
};

OverlyingErrorModel::OverlyingErrorModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &OverlyingErrorModel::rowsInserted, this, &OverlyingErrorModel::markAsChanged);
    connect(this, &OverlyingErrorModel::rowsRemoved, this, &OverlyingErrorModel::markAsChanged);
}

OverlyingErrorModel::~OverlyingErrorModel() = default;

QVariant OverlyingErrorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() > int( overlying_error_keys.size()))
    {
        return {};
    }
    auto it = overlying_error_keys.begin();
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
    if (role == Qt::UserRole + 3)
    {
        return (*it).first == ErrorType::ConsecutiveTypeErrors || (*it).first == ErrorType::AccumulatedTypeErrors;
    }

    return {};
}

int OverlyingErrorModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return overlying_error_keys.size();
}

QHash<int, QByteArray> OverlyingErrorModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("description")},
        {Qt::UserRole + 1, QByteArrayLiteral("image")},
        {Qt::UserRole + 2, QByteArrayLiteral("type")},
        {Qt::UserRole + 3, QByteArrayLiteral("isTypeError")}
    };
}

void OverlyingErrorModel::setAttributeModel(AttributeModel *attributeModel)
{
    if (m_attributeModel == attributeModel)
    {
       return;
    }
    disconnect(m_attributeModelDestroyedConnection);
    m_attributeModelDestroyedConnection = QMetaObject::Connection{};
    m_attributeModel = attributeModel;
    if (m_attributeModel)
    {
        m_attributeModelDestroyedConnection = connect(m_attributeModel, &QObject::destroyed, this, std::bind(&OverlyingErrorModel::setAttributeModel, this, nullptr));
    } else
    {
        m_attributeModelDestroyedConnection = {};
    }
    emit attributeModelChanged();
}

QUuid OverlyingErrorModel::variantId(ErrorType type) const
{
    return std::get<QUuid>(overlying_error_keys.at(type));
}

QString OverlyingErrorModel::name(ErrorType type) const
{
    return QString::fromStdString(std::get<0>(overlying_error_keys.at(type)));
}

QString OverlyingErrorModel::nameFromId(const QUuid &id) const
{
    for (auto &key : overlying_error_keys)
    {
        if (id == std::get<QUuid>(key.second))
        {
            return QString::fromStdString(std::get<0>(key.second));
        }
    }
    return QStringLiteral("");
}

bool OverlyingErrorModel::isTypeError(const QUuid &id) const
{
    return id == variantId(ErrorType::ConsecutiveTypeErrors) || id == variantId(ErrorType::AccumulatedTypeErrors);
}

}
}




