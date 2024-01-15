#include "qualityNormModel.h"
#include "jsonSupport.h"

#include <QUuid>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSaveFile>

namespace precitec
{
namespace storage
{

QualityNormModel::QualityNormModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &QualityNormModel::configurationDirectoryChanged, this, &QualityNormModel::load);
}

QualityNormModel::~QualityNormModel() = default;

int QualityNormModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_qualityNorms.size();
}

QVariant QualityNormModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= (int)m_qualityNorms.size())
    {
        return {};
    }

    const auto& qualityNorm = m_qualityNorms.at(index.row());
    switch (role)
    {
        case Qt::DisplayRole:
            return qualityNorm->name();
        case Qt::UserRole:
            return qualityNorm->uuid();
    }
    return {};
}

QHash<int, QByteArray> QualityNormModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("uuid")}
    };
}

void QualityNormModel::setConfigurationDirectory(const QString& dir)
{
    if (m_configurationDirectory.compare(dir) == 0)
    {
        return;
    }
    m_configurationDirectory = dir;
    emit configurationDirectoryChanged();
}

void QualityNormModel::load()
{
    const auto dir = QDir{m_configurationDirectory};

    if (!dir.exists())
    {
        return;
    }

    const auto filePath = dir.filePath(QStringLiteral("qualityNorms.json"));

    if (!QFileInfo::exists(filePath))
    {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    const auto data = file.readAll();
    file.close();
    if (data.isEmpty())
    {
        return;
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(data, &error);
    if (document.isNull())
    {
        return;
    }

    beginResetModel();
    qDeleteAll(m_qualityNorms);

    const auto qualityNorms = json::parseQualityNorms(document.object(), this);

    for (auto qn : qualityNorms)
    {
        qn->addCommonResultsFromJson(document.object());
    }

    m_qualityNorms.push_back(new QualityNorm(QUuid{}, QStringLiteral("None"), this));

    m_qualityNorms.insert(m_qualityNorms.end(), qualityNorms.begin(), qualityNorms.end());

    endResetModel();
}

QModelIndex QualityNormModel::indexForQualityNorm(const QUuid& qualityNorm) const
{
    auto it = std::find_if(m_qualityNorms.begin(), m_qualityNorms.end(), [qualityNorm] (auto qn) { return qn->uuid() == qualityNorm; });
    if (it == m_qualityNorms.end())
    {
        return index(0);
    }
    return index(std::distance(m_qualityNorms.begin(), it), 0);
}

QUuid QualityNormModel::idAtIndex(int index)
{
    if (index < 0 || index >= int(m_qualityNorms.size()))
    {
        return QUuid{};
    }
    return m_qualityNorms.at(index)->uuid();
}

QualityNorm* QualityNormModel::qualityNorm(const QUuid& qualityNorm)
{
    auto it = std::find_if(m_qualityNorms.begin(), m_qualityNorms.end(), [qualityNorm] (auto qn) { return qn->uuid() == qualityNorm; });
    if (it == m_qualityNorms.end())
    {
        return nullptr;
    }
    return *it;
}

}
}

