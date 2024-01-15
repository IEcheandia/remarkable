#include "errorSettingModel.h"
#include "event/resultType.h"
#include "jsonSupport.h"
#include "productModel.h"
#include "product.h"
#include "seamError.h"
#include "intervalError.h"
#include "productError.h"
#include "seamSeriesError.h"
#include "parameterSet.h"
#include "resultHelper.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSaveFile>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QColor>

namespace precitec
{
namespace storage
{

ErrorSettingModel::ErrorSettingModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_errorStorageFile = QDir{QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QStringLiteral("/config/")}.filePath(QStringLiteral("errorsConfig.json"));

    connect(this, &ErrorSettingModel::rowsInserted, this, &ErrorSettingModel::writeUpdatedList);
    connect(this, &ErrorSettingModel::rowsRemoved, this, &ErrorSettingModel::writeUpdatedList);
    connect(this, &ErrorSettingModel::productModelChanged, this, &ErrorSettingModel::updateUsedFlag);
}

ErrorSettingModel::~ErrorSettingModel()
{
}

void ErrorSettingModel::setProductModel(ProductModel *productModel)
{
    loadErrors();
    if (productModel == m_productModel)
    {
        return;
    }
    m_productModel = productModel;
    if (m_productModel)
    {
        connect(m_productModel, &ProductModel::dataChanged, this, &ErrorSettingModel::updateUsedFlag);
    }
    emit productModelChanged();
}

int ErrorSettingModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_errorItems.size();
}

QVariant ErrorSettingModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(row < 0 || row >= (int)m_errorItems.size()) {
        return {};
    }

    const auto &errorItem = m_errorItems.at(index.row());
    switch(role) {
        case Qt::DisplayRole:
            return errorItem->enumType();
        case Qt::UserRole:
            return errorItem->uuid();
        case Qt::UserRole + 1:
            return errorItem->name();
        case Qt::UserRole + 2:
            return errorItem->plotterNumber();
        case Qt::UserRole + 3:
            return errorItem->plottable();
        case Qt::UserRole + 4:
            return errorItem->min();
        case Qt::UserRole + 5:
            return errorItem->max();
        case Qt::UserRole + 6:
            return errorItem->lineColor();
        case Qt::UserRole + 7:
            return errorItem->visibleItem();
        case Qt::UserRole + 8:
            return QVariant::fromValue(errorItem->visualization());
        case Qt::UserRole + 9:
            return errorItem->disabled();
        case Qt::UserRole + 10:
            return QColor(errorItem->lineColor()).hslHueF();
        case Qt::UserRole + 11:
            return QColor(errorItem->lineColor()).hslSaturationF();
        case Qt::UserRole + 12:
            return QColor(errorItem->lineColor()).lightnessF();
        case Qt::UserRole + 13:
        {
            const auto qualityName = nameForQualityError(errorItem->enumType());
            return qualityName.isEmpty() ? qualityName : QStringLiteral("(%1)").arg(qualityName);
        }
    }
    return {};
}

QHash<int, QByteArray> ErrorSettingModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, QByteArrayLiteral("enumType")},
        {Qt::UserRole, QByteArrayLiteral("uuid")},
        {Qt::UserRole + 1, QByteArrayLiteral("name")},
        {Qt::UserRole + 2, QByteArrayLiteral("plotterNumber")},
        {Qt::UserRole + 3, QByteArrayLiteral("plottable")},
        {Qt::UserRole + 4, QByteArrayLiteral("min")},
        {Qt::UserRole + 5, QByteArrayLiteral("max")},
        {Qt::UserRole + 6, QByteArrayLiteral("lineColor")},
        {Qt::UserRole + 7, QByteArrayLiteral("visibleItem")},
        {Qt::UserRole + 8, QByteArrayLiteral("visualization")},
        {Qt::UserRole + 9, QByteArrayLiteral("disabled")},
        {Qt::UserRole + 10, QByteArrayLiteral("hue")},
        {Qt::UserRole + 11, QByteArrayLiteral("saturation")},
        {Qt::UserRole + 12, QByteArrayLiteral("lightness")},
        {Qt::UserRole + 13, QByteArrayLiteral("qualityName")}
    };
}

void ErrorSettingModel::loadErrors()
{
    if (!QFileInfo::exists(m_errorStorageFile))
    {
        createFileWithDefaultErrors();
    }
    QFile file(m_errorStorageFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ErrorSettingModel::loadErrors() Cannot open the file" << file.fileName();
        return;
    }
    const QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty())
    {
       return;
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(data, &error);
    if (document.isNull())
    {
        return;
    }
    beginResetModel();
    qDeleteAll(m_errorItems);
    m_errorItems = json::parseResultItems(document.object(), this);
    for (auto item : m_errorItems)
    {
        item->setVisualization(ResultSetting::Visualization::Binary);
    }
    endResetModel();

    updateUsedFlag();
    emit errorSettingModelChanged();
}

void ErrorSettingModel::writeUpdatedList()
{
    QSaveFile file(m_errorStorageFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "ErrorSettingModel::writeUpdatedList() Cannot create the file <" << m_errorStorageFile << ">";
        return;
    }
    toJson(&file);
    file.commit();
}

void ErrorSettingModel::updateUsedFlag()
{
    if (!m_productModel)
    {
        return;
    }
    // the collection of the defined error-enums
    std::vector<int> nioItemsEnums;
    for (auto product : m_productModel->products())
    {
        // check the graph parameters for defined error types
        for (auto parameterSet : product->filterParameterSets())
        {
            for (auto parameter : parameterSet->parameters())
            {
                if (parameter->name().compare(QLatin1String("NioType")) == 0)
                {
                    int enumType = parameter->value().toInt();
                    if (std::any_of(nioItemsEnums.begin(), nioItemsEnums.end(), [enumType] (auto nioItem) { return nioItem == enumType; }))
                    {
                        continue;
                    }
                    checkAndAddNewError(parameter->uuid(), enumType);
                    nioItemsEnums.push_back(enumType);
               }
            }
        }
        // check the defined errors of the product
        const auto &productErrors = product->overlyingErrors();
        for (auto error : productErrors)
        {
            int enumType = error->errorType();
            if (std::any_of(nioItemsEnums.begin(), nioItemsEnums.end(), [enumType] (auto nioItem) { return nioItem == enumType; }))
            {
                continue;
            }
            checkAndAddNewError(error->uuid(), enumType);
            nioItemsEnums.push_back(enumType);
        }
        const auto &seamSeriesErrors = product->allSeamSeriesErrors();
        for (auto error : seamSeriesErrors)
        {
            int enumType = error->errorType();
            if (std::any_of(nioItemsEnums.begin(), nioItemsEnums.end(), [enumType] (auto nioItem) { return nioItem == enumType; }))
            {
                continue;
            }
            checkAndAddNewError(error->uuid(), enumType);
            nioItemsEnums.push_back(enumType);
        }
        const auto &seamErrors = product->allSeamErrors();
        for (auto error : seamErrors)
        {
            int enumType = error->errorType();
            if (std::any_of(nioItemsEnums.begin(), nioItemsEnums.end(), [enumType] (auto nioItem) { return nioItem == enumType; }))
            {
                continue;
            }
            checkAndAddNewError(error->uuid(), enumType);
            nioItemsEnums.push_back(enumType);
        }
        const auto &intervalErrors = product->allIntervalErrors();
        for (auto error : intervalErrors)
        {
            int enumType = error->errorType();
            if (std::any_of(nioItemsEnums.begin(), nioItemsEnums.end(), [enumType] (auto nioItem) { return nioItem == enumType; }))
            {
                continue;
            }
            checkAndAddNewError(error->uuid(), enumType);
            nioItemsEnums.push_back(enumType);
        }
    }
    // set now the flag for the button enable
    for (auto it = m_errorItems.begin(); it != errorItems().end(); it++)
    {
        int enumType = (*it)->enumType();
        (*it)->setDisabled(0);
        if (std::any_of(nioItemsEnums.begin(), nioItemsEnums.end(), [enumType] (auto nioItem) { return nioItem == enumType; }))
        {
            (*it)->setDisabled(1);
        }
    }
    updateUsedFlagChanged();
}

void ErrorSettingModel:: checkAndAddNewError(const QUuid &uuid, int enumType)
{
    if (std::any_of(m_errorItems.begin(), m_errorItems.end(), [enumType] (auto nioItem) { return nioItem->enumType() == enumType; }))
    {
        return;
    }
    createAndAddItem(uuid, enumType);
}

ResultSetting *ErrorSettingModel::getItem(int enumType)
{
    if (m_errorItems.size() == 0)
    {
        loadErrors();
    }
    for (auto it = m_errorItems.begin(); it != m_errorItems.end(); ++it)
    {
        if ((*it)->enumType() == enumType)
        {
            return (*it);
        }
    }
    return nullptr;
}

int ErrorSettingModel::highestEnumValue()
{
    auto highestValue = 0;
    for (auto it = m_errorItems.begin(); it != m_errorItems.end(); it++)
    {
        if ((*it)->enumType() > highestValue)
        {
            highestValue = (*it)->enumType();
        }
    }
    return highestValue;
}

QModelIndex ErrorSettingModel::indexForResultType(int enumType) const
{
    auto it = std::find_if(m_errorItems.begin(), m_errorItems.end(), [enumType] (auto item) { return item->enumType() == enumType; });
    if (it == m_errorItems.end())
    {
        return {};
    }
    return index(std::distance(m_errorItems.begin(), it), 0);
}

void ErrorSettingModel::updateValue(const QModelIndex& modelIndex, const QVariant &data, ResultSetting::Type target)
{
    auto index = modelIndex.row();
    if (std::size_t(index) >= m_errorItems.size()) {
        return;
    }
    const auto &errorItem = m_errorItems.at(index);
    if (errorItem != nullptr)
    {
        errorItem->updateValue(data, target);
        writeUpdatedList();
        emit dataChanged(modelIndex, modelIndex);
    }
}


QJsonObject ErrorSettingModel::toJson() const
{
    return QJsonObject{
        {
            json::toJson(m_errorItems)
        }
    };
}

void ErrorSettingModel::toJson(QIODevice *device)
{
    const QJsonDocument document(toJson());
    device->write(document.toJson());
}

void ErrorSettingModel::createNewError(const QString &name, const QVariant &enumType)
{
    beginInsertRows(QModelIndex(), m_errorItems.size(), m_errorItems.size());
    auto *newError = new ResultSetting(QUuid::createUuid(), enumType.toInt(), this);
    newError->setName(name);
    newError->setLineColor(QStringLiteral("#FF0000"));
    m_errorItems.push_back(newError);
    endInsertRows();
}

void ErrorSettingModel::deleteError(const QModelIndex& modelIndex, const QVariant &data)
{
    auto index = modelIndex.row();
    if (std::size_t(index) >= m_errorItems.size()) {
        qDebug() << "ErrorSettingModel::deleteError() wrong index <" << index << "> enum <" << data << ">";
        return;
    }
    auto it = std::find_if(m_errorItems.begin(), m_errorItems.end(), [data] (auto p) { return p->enumType() == data; });
    if (it != m_errorItems.end())
    {
        beginRemoveRows({}, index, index);
        (*it)->deleteLater();
        m_errorItems.erase(it);
        endRemoveRows();
    } else
    {
        qDebug() << "ErrorSettingModel::deleteError() error!! index <" << index << "> enum <" << data << "> not found";
    }
}

void ErrorSettingModel::createFileWithDefaultErrors()
{
    createAndAddItem(QUuid{"B353B29F-A335-429D-BF42-290E2290FD25"}, 1001);
    createAndAddItem(QUuid{"7DC800E5-6645-484B-B492-C8ACDA0D1951"}, 1002);
    createAndAddItem(QUuid{"26D832F4-963E-4186-AB54-BC2EF8329B98"}, 1003);
    createAndAddItem(QUuid{"39B7487F-1929-43BF-986C-D9C4858730E1"}, 1004);
    createAndAddItem(QUuid{"E3209564-C2B0-4AB9-849C-FA5C45223F7D"}, 1005);
    createAndAddItem(QUuid{"EA0EA47F-1A02-4FEF-B865-F6368C4FD661"}, 1006);
    createAndAddItem(QUuid{"9B26D4AB-3B61-499E-9BB7-681A0C8B793B"}, 1007);
    createAndAddItem(QUuid{"A1828EA7-AC8D-4755-861A-CCDB02E73BB9"}, 1008);
    createAndAddItem(QUuid{"347EFB90-8162-49E9-96A1-46472FDECFF2"}, 1009);
    createAndAddItem(QUuid{"6349B741-00A8-4B9B-83E2-6DF8C8A32ED1"}, 1010);
    createAndAddItem(QUuid{"576CE2C0-624B-422D-A4AD-F57049F90920"}, 1011);
    createAndAddItem(QUuid{"FD4B30F1-D68D-4388-BF11-0FA50911D3BB"}, 1012);
    createAndAddItem(QUuid{"2B220F16-C525-4E35-9C53-8112F615C9C1"}, 1013);
    createAndAddItem(QUuid{"6C0D3277-6617-4936-BC91-2422C80977BA"}, 1015);
}

void ErrorSettingModel::createAndAddItem(const QUuid &uuid, int enumType)
{
    beginInsertRows(QModelIndex(), m_errorItems.size(), m_errorItems.size());
    auto *newError = new ResultSetting(uuid, enumType, this);
    newError->setName(nameForResult(enumType));
    newError->setLineColor(colorForResult(enumType).name());
    m_errorItems.push_back(newError);
    endInsertRows();
}

void ErrorSettingModel::ensureItemExists(int enumType)
{
    checkAndAddNewError(QUuid::createUuid(), enumType);
}

}
}
