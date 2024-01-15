#include "referenceImageController.h"
#include "seam.h"
#include "seamSeries.h"
#include "product.h"
#include "parameterSet.h"
#include "parameter.h"
#include "attributeModel.h"
#include "attribute.h"
#include "jsonSupport.h"

#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QMetaEnum>

using precitec::storage::Seam;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::AttributeModel;

namespace precitec
{
namespace gui
{

ReferenceImageController::ReferenceImageController(QObject* parent)
    : AbstractHardwareParameterModel(parent)
    , m_fileWatcher(new QFileSystemWatcher(this))
{
    connect(this, &ReferenceImageController::currentSeamChanged, this, &ReferenceImageController::updateImagePath);
    connect(this, &ReferenceImageController::currentSeamChanged, this, &ReferenceImageController::updateModel);
    connect(this, &ReferenceImageController::attributeModelChanged, this, &ReferenceImageController::updateModel);
    connect(this, &ReferenceImageController::referenceImageDirChanged, this, &ReferenceImageController::updateImagePath);
    connect(this, &ReferenceImageController::imagePathChanged, this, &ReferenceImageController::updateHardwareParameters);
    connect(this, &ReferenceImageController::referenceHardwareParametersChanged, this, &ReferenceImageController::updateModel);
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, &ReferenceImageController::imageFilePathChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, &ReferenceImageController::updateHardwareParameters);
}

ReferenceImageController::~ReferenceImageController() = default;

QVariant ReferenceImageController::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto info = m_parametersDiff.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return tr(std::get<std::string>(info).c_str());
    case Qt::UserRole:
        return QVariant::fromValue(std::get<HardwareParameters::Key>(info));
    case Qt::UserRole + 1:
        return QVariant::fromValue(findAttribute(std::get<HardwareParameters::Key>(info)));
    case Qt::UserRole + 2:
        return std::get<2>(info) != nullptr;
    case Qt::UserRole + 3:
        return QVariant::fromValue(std::get<2>(info));
    case Qt::UserRole + 4:
        return std::get<3>(info) != nullptr;
    case Qt::UserRole + 5:
        return QVariant::fromValue(std::get<3>(info));
    case Qt::UserRole + 6:
        return std::get<HardwareParameters::UnitConversion>(info) == HardwareParameters::UnitConversion::MilliFromMicro;
    }
    return {};
}

int ReferenceImageController::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_parametersDiff.size();
}

QHash<int, QByteArray> ReferenceImageController::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("key")},
        {Qt::UserRole + 1, QByteArrayLiteral("attribute")},
        {Qt::UserRole + 2, QByteArrayLiteral("seamEnabled")},
        {Qt::UserRole + 3, QByteArrayLiteral("seamParameter")},
        {Qt::UserRole + 4, QByteArrayLiteral("referenceEnabled")},
        {Qt::UserRole + 5, QByteArrayLiteral("referenceParameter")},
        {Qt::UserRole + 6, QByteArrayLiteral("milliFromMicro")}
    };
}

void ReferenceImageController::setReferenceImageDir(const QString& dir)
{
    if (m_referenceImageDir.compare(dir) == 0)
    {
        return;
    }
    m_referenceImageDir = dir;
    emit referenceImageDirChanged();
}

void ReferenceImageController::setCurrentSeam(Seam* seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }

    if (m_currentSeam)
    {
        disconnect(m_seamDestroyed);
        disconnect(m_currentSeam, &Seam::hardwareParametersChanged, this, &ReferenceImageController::updateModel);
    }

    m_currentSeam = seam;

    if (m_currentSeam)
    {
        m_seamDestroyed = connect(m_currentSeam, &Seam::destroyed, this, std::bind(&ReferenceImageController::setCurrentSeam, this, nullptr));
        connect(m_currentSeam, &Seam::hardwareParametersChanged, this, &ReferenceImageController::updateModel);
    } else
    {
        m_seamDestroyed = QMetaObject::Connection{};
    }

    emit currentSeamChanged();
}

void ReferenceImageController::setAttributeModel(AttributeModel* model)
{
    if (attributeModel() == model)
    {
        return;
    }
    if (attributeModel())
    {
        disconnect(attributeModel(), &AttributeModel::modelReset, this, &ReferenceImageController::updateModel);
    }

    AbstractHardwareParameterModel::setAttributeModel(model);

    if (model)
    {
        connect(model, &AttributeModel::modelReset, this, &ReferenceImageController::updateModel);
    }
}

void ReferenceImageController::updateImagePath()
{
    auto imagePath = QStringLiteral("");
    if (!m_referenceImageDir.isEmpty() && m_currentSeam)
    {
        const auto seamSeries = m_currentSeam->seamSeries();
        const auto product = seamSeries->product();

        imagePath = QStringLiteral("%1/%2/seam_series%3/seam%4/").arg(m_referenceImageDir)
                                                               .arg(product->uuid().toString(QUuid::WithoutBraces))
                                                               .arg(seamSeries->number(), 4, 10, QLatin1Char('0'))
                                                               .arg(m_currentSeam->number(), 4, 10, QLatin1Char('0'));
    }

    if (m_imagePath.compare(imagePath) == 0)
    {
        return;
    }

    if (!m_imagePath.isEmpty())
    {
        m_fileWatcher->removePath(m_imagePath);
    }

    m_imagePath = imagePath;

    if (!m_imagePath.isEmpty())
    {
        const auto dir = QDir{m_imagePath};
        if (!dir.exists())
        {
            dir.mkpath(dir.absolutePath());
        }
        m_fileWatcher->addPath(m_imagePath);
    }

    emit imagePathChanged();
    emit imageFilePathChanged(m_imagePath);
}

QString ReferenceImageController::imageFilePath() const
{
    if (m_imagePath.isEmpty())
    {
        return QStringLiteral("");
    }

    const auto files = QDir{m_imagePath}.entryInfoList(QStringList() << QStringLiteral("*.png"), QDir::Files, QDir::Time);

    if (files.isEmpty())
    {
        return QStringLiteral("");
    }

    if (files.size() > 1)
    {
        for (auto i = 1; i < files.size(); i++)
        {
            QFile::remove(files.at(i).absoluteFilePath());
        }
    }

    return QStringLiteral("file://").append(files.at(0).absoluteFilePath());
}

void ReferenceImageController::saveHardwareParameters()
{
    if (!m_currentSeam)
    {
        return;
    }

    auto parameterSet = m_currentSeam->hardwareParameters();
    if (!parameterSet)
    {
        return;
    }

    auto document = QJsonDocument{
        QJsonObject{
            storage::json::hardwareParametersToJson(parameterSet)
        }
    };

    QSaveFile file{m_imagePath + QStringLiteral("parameters.json")};
    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }
    file.write(document.toJson());
    file.commit();
}

void ReferenceImageController::updateHardwareParameters()
{
    QFile file(m_imagePath + QStringLiteral("parameters.json"));

    if(!file.exists())
    {
        return;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    const QByteArray data = file.readAll();
    if (data.isEmpty())
    {
        return;
    }

    const auto document = QJsonDocument::fromJson(data);
    if (document.isNull())
    {
        return;
    }

    m_referenceHardwareParameters = storage::json::parseHardwareParameters(document.object(), this);
    emit referenceHardwareParametersChanged();
}

ParameterSet* ReferenceImageController::getParameterSet() const
{
    return nullptr;
}

bool ReferenceImageController::isValid() const
{
    return true;
}

ParameterSet* ReferenceImageController::getParameterSetDirect() const
{
    return nullptr;
}

void ReferenceImageController::updateModel()
{
    beginResetModel();

    for (auto parameterPair : m_parametersDiff)
    {
        if (std::get<2>(parameterPair))
        {
            std::get<2>(parameterPair)->deleteLater();
        }
        if (std::get<3>(parameterPair))
        {
            std::get<3>(parameterPair)->deleteLater();
        }
    }
    m_parametersDiff.clear();

    if (m_currentSeam && m_referenceHardwareParameters && attributeModel())
    {
        for (auto i = 0; i < HardwareParameters::instance()->keyCount(); i++)
        {
            const auto key = HardwareParameters::Key(i);
            const auto& keyInfo = HardwareParameters::instance()->properties(key);

            auto seamParameter = findParameter(m_currentSeam->hardwareParameters(), key);
            auto referenceParameter = findParameter(m_referenceHardwareParameters, key);

            if (!seamParameter && !referenceParameter)
            {
                continue;
            }

            if (seamParameter && !referenceParameter)
            {
                m_parametersDiff.push_back({keyInfo.name, key, seamParameter->duplicate(), nullptr, keyInfo.conversion});
                continue;
            }

            if (!seamParameter && referenceParameter)
            {
                m_parametersDiff.push_back({keyInfo.name, key, nullptr, referenceParameter->duplicate(), keyInfo.conversion});
                continue;
            }

            if (seamParameter->value() == referenceParameter->value())
            {
                continue;
            }

            m_parametersDiff.push_back({keyInfo.name, key, seamParameter->duplicate(), referenceParameter->duplicate(), keyInfo.conversion});
        }
    }
    endResetModel();
}


}
}
