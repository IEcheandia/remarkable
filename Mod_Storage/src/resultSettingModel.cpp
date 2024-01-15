#include "resultSettingModel.h"
#include "jsonSupport.h"

#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSaveFile>
#include <QFileSystemWatcher>
#include <QMutexLocker>
#include <QColor>

namespace precitec
{
namespace storage
{

// TODO Handle this in a more flexible way
static const std::vector<int> s_lwmResultTypes = {737, 738, 739, 740};

ResultSettingModel::ResultSettingModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_resultStorageFile = QDir{QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QStringLiteral("/config/")}.filePath(QStringLiteral("resultConfigOverlay.json"));

    connect(this, &ResultSettingModel::rowsInserted, this, &ResultSettingModel::writeUpdatedList);
    connect(this, &ResultSettingModel::rowsRemoved, this, &ResultSettingModel::writeUpdatedList);
    connect(this, &ResultSettingModel::dataChanged, this, &ResultSettingModel::writeUpdatedList);

    loadResults();
    import(m_resultStorageFile);
}

ResultSettingModel::~ResultSettingModel()
{
}

int ResultSettingModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_resultItems.size();
}

QVariant ResultSettingModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= (int)m_resultItems.size())
    {
        return {};
    }

    const auto &result = m_resultItems.at(index.row());
    switch (role)
    {
        case Qt::DisplayRole:
            return result->enumType();
        case Qt::UserRole:
            return result->uuid();
        case Qt::UserRole + 1:
            return result->name();
        case Qt::UserRole + 2:
            return result->plotterNumber();
        case Qt::UserRole + 3:
            return result->plottable();
        case Qt::UserRole + 4:
            return result->min();
        case Qt::UserRole + 5:
            return result->max();
        case Qt::UserRole + 6:
            return result->lineColor();
        case Qt::UserRole + 7:
            return result->visibleItem();
        case Qt::UserRole + 8:
            return QVariant::fromValue(result->visualization());
        case Qt::UserRole + 9:
            return result->disabled();
        case Qt::UserRole + 10:
            return QColor(result->lineColor()).hslHueF();
        case Qt::UserRole + 11:
            return QColor(result->lineColor()).hslSaturationF();
        case Qt::UserRole + 12:
            return QColor(result->lineColor()).lightnessF();
    }
    return {};
}

QHash<int, QByteArray> ResultSettingModel::roleNames() const
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
        {Qt::UserRole + 12, QByteArrayLiteral("lightness")}
    };
}

void ResultSettingModel::loadResults()
{
    QFile file{QDir{QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QStringLiteral("/system_graphs/")}.filePath(QStringLiteral("resultConfig.json"))};
    if (!file.open(QIODevice::ReadOnly))
    {
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
    qDeleteAll(m_resultItems);
    m_resultItems = json::parseResultItems(document.object(), this);
    endResetModel();
}

void ResultSettingModel::import(const QString &path)
{
    QFile file{path};
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    const QByteArray data{file.readAll()};
    if (data.isEmpty())
    {
        return;
    }
    const QJsonDocument document = QJsonDocument::fromJson(data);
    if (document.isNull())
    {
        return;
    }
    const auto newItems = json::parseResultItems(document.object(), this);
    if (newItems.empty())
    {
        return;
    }
    beginResetModel();
    for (auto item : newItems)
    {
        item->setStorageType(ResultSetting::StorageType::UserOverlay);
        auto it = std::find_if(m_resultItems.begin(), m_resultItems.end(), [item] (auto existing) { return existing->enumType() == item->enumType(); });
        if (it != m_resultItems.end())
        {
            (*it)->deleteLater();
            m_resultItems.erase(it);
        }
        m_resultItems.push_back(item);
    }
    endResetModel();
}

void ResultSettingModel::updateValue(const QModelIndex& modelIndex, const QVariant &data, ResultSetting::Type target)
{
    int index = modelIndex.row();
    if (std::size_t(index) >= m_resultItems.size())
    {
        return;
    }
    const auto &result = m_resultItems.at(index);
    if (result != nullptr)
    {
        result->updateValue(data, target);
        result->setStorageType(ResultSetting::StorageType::UserOverlay);
        emit dataChanged(modelIndex, modelIndex, {});
    }
}

QJsonObject ResultSettingModel::toJson() const
{
    std::vector<ResultSetting*> userOverlay;
    std::copy_if(m_resultItems.begin(), m_resultItems.end(), std::back_inserter(userOverlay),
                 [] (auto *result) { return result->storageType() == ResultSetting::StorageType::UserOverlay; });
    return QJsonObject{
        {
            json::toJson(userOverlay)
        }
    };
}

void ResultSettingModel::toJson(QIODevice *device)
{
    const QJsonDocument document(toJson());
    device->write(document.toJson());
}

ResultSetting *ResultSettingModel::checkAndAddItem(int enumType, QString name)
{
    for (auto it = m_resultItems.begin(); it != m_resultItems.end(); ++it)
    {
        if ((*it)->enumType() == enumType) 
        {
            return (*it);
        }
    }

    beginInsertRows(QModelIndex(), m_resultItems.size(), m_resultItems.size());
    auto *newValue = new ResultSetting(QUuid::createUuid(), enumType, this);
    newValue->setStorageType(ResultSetting::StorageType::UserOverlay);
    newValue->setName(name);
    newValue->setMin(-1000);
    newValue->setMax(1000);
    newValue->setPlottable(1);
    newValue->setPlotterNumber(1);
    newValue->setLineColor(QStringLiteral("#ee6622"));
    newValue->setVisibleItem(1);

    m_resultItems.push_back(newValue);
    endInsertRows();

    return newValue;
}

void ResultSettingModel::ensureItemExists(int enumType)
{
    checkAndAddItem(enumType, QString::number(enumType));
}

ResultSetting *ResultSettingModel::getItem(int enumType) const
{
    for (auto it = m_resultItems.begin(); it != m_resultItems.end(); ++it)
    {
        if ((*it)->enumType() == enumType) 
        {
            return (*it);
        }
    }
    return nullptr;
}

void ResultSettingModel::deleteResult(const QModelIndex& modelIndex, const QVariant &data){
    auto index = modelIndex.row();
    if (std::size_t(index) >= m_resultItems.size()) {
        qDebug() << "ResultSettingModel::deleteResult() wrong index <" << index << "> enum <" << data << ">";
        return;
    }
    auto it = std::find_if(m_resultItems.begin(), m_resultItems.end(), [data] (auto p) { return p->enumType() == data; });
    if (it != m_resultItems.end())
    {
        beginRemoveRows({}, index, index);
        (*it)->deleteLater();
        m_resultItems.erase(it);
        endRemoveRows();
    } else
    {
        qDebug() << "ResultSettingModel::deleteResult() error!! index <" << index << "> enum <" << data << "> not found";
    }
}
void ResultSettingModel::createNewResult(const QString &name, const QVariant &enumType)
{
    checkAndAddItem(enumType.toInt(),name);
}
int ResultSettingModel::highestEnumValue(){
    auto highestValue = 0;
    for (auto it = m_resultItems.begin(); it != m_resultItems.end(); it++)
    {
        if ((*it)->enumType() > highestValue)
        {
            highestValue = (*it)->enumType();
        }
    }
    return highestValue;
}

QModelIndex ResultSettingModel::indexForResultType(int enumType) const
{
    auto it = std::find_if(m_resultItems.begin(), m_resultItems.end(), [enumType] (auto item) { return item->enumType() == enumType; });
    if (it == m_resultItems.end())
    {
        return {};
    }
    return index(std::distance(m_resultItems.begin(), it), 0);
}

QString ResultSettingModel::nameForResultType(int enumType) const
{
    if (auto result = getItem(enumType))
    {
        return result->name();
    }
    return QStringLiteral("Enum %1").arg(enumType);
}

void ResultSettingModel::writeUpdatedList()
{
    QSaveFile file(m_resultStorageFile);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    toJson(&file);
    file.commit();
}

bool ResultSettingModel::isLwmType(int enumType)
{
    return std::any_of(s_lwmResultTypes.begin(), s_lwmResultTypes.end(), [enumType] (auto lwmResult) { return enumType == lwmResult; });
}

}
}
