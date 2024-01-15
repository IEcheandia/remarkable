#include "simpleErrorModel.h"
#include "attributeModel.h"
#include "errorGroupModel.h"
#include "seam.h"
#include "seamError.h"
#include "intervalError.h"

using precitec::storage::AttributeModel;
using precitec::storage::AbstractMeasureTask;
using precitec::storage::Seam;
using precitec::storage::SeamError;
using precitec::storage::IntervalError;

namespace precitec
{
namespace gui
{

std::map<SimpleErrorModel::ErrorType, std::tuple<std::string, std::string, std::string, QString, QUuid, ErrorGroupModel::ErrorGroup, SeamError::BoundaryType> > error_keys {
    {SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary, {QT_TRANSLATE_NOOP("", "Length Outside Static Boundary Failure"), QT_TRANSLATE_NOOP("", "Static Boundary Failure"), QT_TRANSLATE_NOOP("", "The length of a single, continuous sector outside the range defined by <b>Min</b> and <b>Max</b> is greater than the <b>Threshold</b> value"), {QStringLiteral("../images/error-LOB-StatBoundFail.png")}, {QByteArrayLiteral("3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D")}, ErrorGroupModel::ErrorGroup::LengthOutsideBoundary, SeamError::BoundaryType::Static}},
    {SimpleErrorModel::ErrorType::LengthOutsideReferenceBoundary, {QT_TRANSLATE_NOOP("", "Length Outside Reference Boundary Failure"), QT_TRANSLATE_NOOP("", "Reference Boundary Failure"), QT_TRANSLATE_NOOP("", "The length of a single, continuous sector outside of a <b>Reference Curve</b> is greater than the <b>Threshold</b> value"), {QStringLiteral("../images/error-LOB-RefBoundFail.png")}, {QByteArrayLiteral("5EB04560-2641-4E64-A016-14207E59A370")}, ErrorGroupModel::ErrorGroup::LengthOutsideBoundary, SeamError::BoundaryType::Reference}},
    {SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary, {QT_TRANSLATE_NOOP("", "Accumulated Length Outside Static Boundary Failure"), QT_TRANSLATE_NOOP("", "Accumulated Static Boundary Failure"), QT_TRANSLATE_NOOP("", "The accumulated length of all sectors outside the range defined by <b>Min</b> and <b>Max</b> is greater than the <b>Threshold</b> value"), {QStringLiteral("../images/error-LOB-AccStatBoundFail.png")}, {QByteArrayLiteral("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6")}, ErrorGroupModel::ErrorGroup::LengthOutsideBoundary, SeamError::BoundaryType::Static}},
    {SimpleErrorModel::ErrorType::AccumulatedLengthOutsideReferenceBoundary,  {QT_TRANSLATE_NOOP("", "Accumulated Length Outside Reference Boundary Failure"), QT_TRANSLATE_NOOP("", "Accumulated Reference Boundary Failure"), QT_TRANSLATE_NOOP("", "The accumulated length of all sectors outside of a <b>Reference Curve</b> is greater than the <b>Threshold</b> value"), {QStringLiteral("../images/error-LOB-AccRefBoundFail.png")}, {QByteArrayLiteral("F8F4E0A8-D259-40F9-B134-68AA24E0A06C")}, ErrorGroupModel::ErrorGroup::LengthOutsideBoundary, SeamError::BoundaryType::Reference}},
    {SimpleErrorModel::ErrorType::LengthInsideStaticBoundary, {QT_TRANSLATE_NOOP("", "Length Inside Static Boundary Failure"), QT_TRANSLATE_NOOP("", "Static Boundary Inside Failure"), QT_TRANSLATE_NOOP("", "The length of every single, continuous sector inside the range defined by <b>Min</b> and <b>Max</b> is shorter than the <b>Threshold</b> value"), {QStringLiteral("../images/error-LIB-StatBoundFail.png")}, {QByteArrayLiteral("3AF9EF6A-A4E9-4234-8BA5-7B42D3E58B2C")}, ErrorGroupModel::ErrorGroup::LengthInsideBoundary, SeamError::BoundaryType::Static}},
    {SimpleErrorModel::ErrorType::LengthInsideReferenceBoundary, {QT_TRANSLATE_NOOP("", "Length Inside Reference Boundary Failure"), QT_TRANSLATE_NOOP("", "Reference Boundary Inside Failure"), QT_TRANSLATE_NOOP("", "The length of every single, continuous sector inside of a <b>Reference Curve</b> is shorter than the <b>Threshold</b> value"), {QStringLiteral("../images/error-LIB-RefBoundFail.png")}, {QByteArrayLiteral("4A6AE9B0-3A1A-427F-8D58-2D0205452377")}, ErrorGroupModel::ErrorGroup::LengthInsideBoundary, SeamError::BoundaryType::Reference}},
    {SimpleErrorModel::ErrorType::AreaStaticBoundary, {QT_TRANSLATE_NOOP("", "Area Outside Static Boundary Failure"), QT_TRANSLATE_NOOP("", "Static Boundary Area Failure"), QT_TRANSLATE_NOOP("", "The area of every single, continuous sector outside the range defined by <b>Min</b> and <b>Max</b> is greater than the <b>Threshold</b> value"), {QStringLiteral("../images/error-AOB-StatBoundFail.png")}, {QByteArrayLiteral("73708EA1-580A-4660-8D80-63622670BC7C")}, ErrorGroupModel::ErrorGroup::AreaOutsideBoundary, SeamError::BoundaryType::Static}},
    {SimpleErrorModel::ErrorType::AreaReferenceBoundary, {QT_TRANSLATE_NOOP("", "Area Outside Reference Boundary Failure"), QT_TRANSLATE_NOOP("", "Reference Boundary Area Failure"), QT_TRANSLATE_NOOP("", "The area of a single, continuous sector outside of a <b>Reference Curve</b> is greater than the <b>Threshold</b> value"), {QStringLiteral("../images/error-AOB-RefBoundFail.png")}, {QByteArrayLiteral("D36ECEBA-286B-4D06-B596-0491B6544F40")}, ErrorGroupModel::ErrorGroup::AreaOutsideBoundary, SeamError::BoundaryType::Reference}},
    {SimpleErrorModel::ErrorType::AccumulatedAreaStaticBoundary, {QT_TRANSLATE_NOOP("", "Accumulated Area Outside Static Boundary Failure"), QT_TRANSLATE_NOOP("", "Accumulated Static Boundary Area Failure"), QT_TRANSLATE_NOOP("", "The accumulated area of all sectors outside the range defined by <b>Min</b> and <b>Max</b> is greater than the <b>Threshold</b> value"), {QStringLiteral("../images/error-AOB-AccStatBoundFail.png")}, {QByteArrayLiteral("740FD8B3-852C-485A-BC24-6C67A36DABD2")}, ErrorGroupModel::ErrorGroup::AreaOutsideBoundary, SeamError::BoundaryType::Static}},
    {SimpleErrorModel::ErrorType::AccumulatedAreaReferenceBoundary, {QT_TRANSLATE_NOOP("", "Accumulated Area Outside Reference Boundary Failure"),QT_TRANSLATE_NOOP("", "Accumulated Reference Boundary Area Failure"), QT_TRANSLATE_NOOP("", "The accumulated area of all sectors outside of a <b>Reference Curve</b> is greater than the <b>Threshold</b> value"), {QStringLiteral("../images/error-AOB-AccRefBoundFail.png")}, {QByteArrayLiteral("527B7421-5DDD-436C-BE33-C1A359A736F6")}, ErrorGroupModel::ErrorGroup::AreaOutsideBoundary, SeamError::BoundaryType::Reference}},
    {SimpleErrorModel::ErrorType::PeakStaticBoundary, {QT_TRANSLATE_NOOP("", "Peak Outside Static Boundary Failure"), QT_TRANSLATE_NOOP("", "Static Boundary Peak Failure"), QT_TRANSLATE_NOOP("", "A single value is detected outside the range defined by <b>Min</b> and <b>Max</b>"), {QStringLiteral("../images/error-POB-StatBoundFail.png")}, {QByteArrayLiteral("396CA433-AD11-4073-A2B2-5314CC41D152")}, ErrorGroupModel::ErrorGroup::PeakOutsideBoundary, SeamError::BoundaryType::Static}},
    {SimpleErrorModel::ErrorType::PeakReferenceBoundary, {QT_TRANSLATE_NOOP("", "Peak Outside Reference Boundary Failure"), QT_TRANSLATE_NOOP("", "Reference Boundary Peak Failure"), QT_TRANSLATE_NOOP("", "A single value is detected outside of a <b>Reference Curve</b>"), {QStringLiteral("../images/error-POB-RefBoundFail.png")}, {QByteArrayLiteral("7CF9F16D-36DE-4840-A2EA-C41979F91A9B")}, ErrorGroupModel::ErrorGroup::PeakOutsideBoundary, SeamError::BoundaryType::Reference}},
    {SimpleErrorModel::ErrorType::DualOutlierStaticBoundary, {QT_TRANSLATE_NOOP("", "Dual Length Outside Static Boundary Failure"), QT_TRANSLATE_NOOP("", "Static Boundary Dual Outlier Failure"), QT_TRANSLATE_NOOP("", "Two sectors over <b>Threshold A</b> length outside the range defined by <b>Min</b> and <b>Max</b> are less than <b>Threshold B</b> length after another"), {QStringLiteral("../images/error-LOB-DualStatBoundFail.png")}, {QByteArrayLiteral("55DCC3D9-FE50-4792-8E27-460AADDDD09F")}, ErrorGroupModel::ErrorGroup::LengthOutsideBoundary, SeamError::BoundaryType::Static}},
    {SimpleErrorModel::ErrorType::DualOutlierReferenceBoundary, {QT_TRANSLATE_NOOP("", "Dual Length Outside Reference Boundary Failure"), QT_TRANSLATE_NOOP("", "Reference Boundary Dual Outlier Failure"), QT_TRANSLATE_NOOP("", "Two sectors over <b>Threshold A</b> length outside of a <b>Reference Curve</b> are less than <b>Threshold B</b> length after another"), {QStringLiteral("../images/error-LOB-DualRefBoundFail.png")}, {QByteArrayLiteral("C0C80DA1-4E9D-4EC0-859A-8D43A0674571")}, ErrorGroupModel::ErrorGroup::LengthOutsideBoundary, SeamError::BoundaryType::Reference}}
};

SimpleErrorModel::SimpleErrorModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &SimpleErrorModel::rowsInserted, this, &SimpleErrorModel::markAsChanged);
    connect(this, &SimpleErrorModel::rowsRemoved, this, &SimpleErrorModel::markAsChanged);
}

SimpleErrorModel::~SimpleErrorModel() = default;

QVariant SimpleErrorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    if (index.row() > int(error_keys.size()))
    {
        return QVariant{};
    }
    auto it = error_keys.begin();
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
         return QString::fromStdString(std::get<2>((*it).second));
    }
    if (role == Qt::UserRole + 2)
    {
         return std::get<QString>((*it).second);
    }
    if (role == Qt::UserRole + 3)
    {
         return QVariant::fromValue((*it).first);
    }
    if (role == Qt::UserRole + 4)
    {
         return QVariant::fromValue(std::get<ErrorGroupModel::ErrorGroup>((*it).second));
    }
    if (role == Qt::UserRole + 5)
    {
         return QVariant::fromValue(std::get<SeamError::BoundaryType>((*it).second));
    }
    return QVariant{};
}

int SimpleErrorModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return error_keys.size();
}

QHash<int, QByteArray> SimpleErrorModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("title")},
        {Qt::UserRole + 1, QByteArrayLiteral("description")},
        {Qt::UserRole + 2, QByteArrayLiteral("image")},
        {Qt::UserRole + 3, QByteArrayLiteral("type")},
        {Qt::UserRole + 4, QByteArrayLiteral("group")},
        {Qt::UserRole + 5, QByteArrayLiteral("boundary")}
    };
}

SeamError* SimpleErrorModel::addError(ErrorType errorType)
{
    if (!m_currentSeam)
    {
        return nullptr;
    }
    auto error = m_currentSeam->addError(std::get<QUuid>(error_keys.at(errorType)));
    error->setName(QString::fromStdString(std::get<0>(error_keys.at(errorType))));
    if (m_attributeModel)
    {
        error->initFromAttributes(m_attributeModel);
    }

    return error;
}

IntervalError* SimpleErrorModel::addIntervalError(ErrorType errorType)
{
    if (!m_currentSeam)
    {
        return nullptr;
    }
    auto error = m_currentSeam->addIntervalError(std::get<QUuid>(error_keys.at(errorType)));
    error->setName(QString::fromStdString(std::get<0>(error_keys.at(errorType))));
    if (m_attributeModel)
    {
        error->initFromAttributes(m_attributeModel);
    }

    return error;
}

QString SimpleErrorModel::nameFromId(const QUuid &id) const
{
    for (auto &key : error_keys)
    {
        if (id == std::get<QUuid>(key.second))
        {
            return QString::fromStdString(std::get<0>(key.second));
        }
    }
    return QStringLiteral("");
}

void SimpleErrorModel::setCurrentSeam(Seam *seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }
    beginResetModel();
    m_currentSeam = seam;
    disconnect(m_destroyConnection);
    if (m_currentSeam)
    {
        m_destroyConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&SimpleErrorModel::setCurrentSeam, this, nullptr));
    } else
    {
        m_destroyConnection = {};
    }
    endResetModel();
    emit currentSeamChanged();
}

void SimpleErrorModel::setAttributeModel(AttributeModel *attributeModel)
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
        m_attributeModelDestroyedConnection = connect(m_attributeModel, &QObject::destroyed, this, std::bind(&SimpleErrorModel::setAttributeModel, this, nullptr));
    } else
    {
        m_attributeModelDestroyedConnection = {};
    }
    emit attributeModelChanged();
}

}
}

