#include "abstractLaserControlModel.h"
#include "attribute.h"
#include "attributeModel.h"
#include "parameter.h"
#include "parameterSet.h"
#include "precitec/dataSet.h"

#include <QFileSystemWatcher>
#include <QUuid>
#include <QDir>

using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::LaserControlPreset;

namespace precitec
{
namespace gui
{

std::map<LaserControlPreset::Key, std::tuple<std::string, QUuid> > channel1_preset_keys {
    {LaserControlPreset::Key::LC_Parameter_No3, {QT_TRANSLATE_NOOP("", "Laser Control Channel 1 Power 1"), {QByteArrayLiteral("16DFEBA1-0A86-40C5-856D-D8949BEE803B")}}},
    {LaserControlPreset::Key::LC_Parameter_No4, {QT_TRANSLATE_NOOP("", "Laser Control Channel 1 Power 2"), {QByteArrayLiteral("DC32C906-0731-445A-A92E-4D222E6A2F34")}}},
    {LaserControlPreset::Key::LC_Parameter_No5, {QT_TRANSLATE_NOOP("", "Laser Control Channel 1 Power 3"), {QByteArrayLiteral("3ACDE9A3-8217-4225-9105-67DF1F8F5BA8")}}},
    {LaserControlPreset::Key::LC_Parameter_No6, {QT_TRANSLATE_NOOP("", "Laser Control Channel 1 Power 4"), {QByteArrayLiteral("9A59DA4D-1C41-43AA-BCA6-657587C3D038")}}},
    {LaserControlPreset::Key::LC_Parameter_No11, {QT_TRANSLATE_NOOP("", "Laser Control Channel 1 Offset 1"), {QByteArrayLiteral("0E1E3F32-5A7F-4A4A-A957-91F685F1EA23")}}},
    {LaserControlPreset::Key::LC_Parameter_No12, {QT_TRANSLATE_NOOP("", "Laser Control Channel 1 Offset 2"), {QByteArrayLiteral("B10FDAB7-5A76-4EA6-9F07-2E3259FCB1FB")}}},
    {LaserControlPreset::Key::LC_Parameter_No13, {QT_TRANSLATE_NOOP("", "Laser Control Channel 1 Offset 3"), {QByteArrayLiteral("82FD87BF-B3EC-4B0F-855E-A21F480EB04B")}}},
    {LaserControlPreset::Key::LC_Parameter_No14, {QT_TRANSLATE_NOOP("", "Laser Control Channel 1 Offset 4"), {QByteArrayLiteral("D403DDCF-4182-46FB-A779-F43D28966167")}}},
};

std::map<LaserControlPreset::Key, std::tuple<std::string, QUuid> > channel2_preset_keys {
    {LaserControlPreset::Key::LC_Parameter_No16, {QT_TRANSLATE_NOOP("", "Laser Control Channel 2 Power 1"), {QByteArrayLiteral("DD52C706-ECA4-4012-8893-E35C9350F46B")}}},
    {LaserControlPreset::Key::LC_Parameter_No17, {QT_TRANSLATE_NOOP("", "Laser Control Channel 2 Power 2"), {QByteArrayLiteral("E4396066-7FEC-4756-82EB-DADABE44D5DD")}}},
    {LaserControlPreset::Key::LC_Parameter_No18, {QT_TRANSLATE_NOOP("", "Laser Control Channel 2 Power 3"), {QByteArrayLiteral("406A93EE-D9E7-44D5-979C-391FB47347AA")}}},
    {LaserControlPreset::Key::LC_Parameter_No19, {QT_TRANSLATE_NOOP("", "Laser Control Channel 2 Power 4"), {QByteArrayLiteral("6773212B-3C55-4A6F-984B-926996FDAFA7")}}},
    {LaserControlPreset::Key::LC_Parameter_No24, {QT_TRANSLATE_NOOP("", "Laser Control Channel 2 Offset 1"), {QByteArrayLiteral("F001DBFF-EB99-4B4C-8705-6BDFBD38E1F5")}}},
    {LaserControlPreset::Key::LC_Parameter_No25, {QT_TRANSLATE_NOOP("", "Laser Control Channel 2 Offset 2"), {QByteArrayLiteral("FE413097-4738-46F5-A3A1-2941D958A90C")}}},
    {LaserControlPreset::Key::LC_Parameter_No26, {QT_TRANSLATE_NOOP("", "Laser Control Channel 2 Offset 3"), {QByteArrayLiteral("4E77FD69-4B78-4BAD-81C2-12CFA6F79F8D")}}},
    {LaserControlPreset::Key::LC_Parameter_No27, {QT_TRANSLATE_NOOP("", "Laser Control Channel 2 Offset 4"), {QByteArrayLiteral("237638AB-BAF6-4952-A793-AB3BB5AB091A")}}}
};

AbstractLaserControlModel::AbstractLaserControlModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_jsonWatcher(new QFileSystemWatcher(this))
{
    connect(m_jsonWatcher, &QFileSystemWatcher::fileChanged, this, &AbstractLaserControlModel::load);
    connect(m_jsonWatcher, &QFileSystemWatcher::directoryChanged, this, &AbstractLaserControlModel::load);
    connect(this, &AbstractLaserControlModel::laserControlPresetDirChanged, this, &AbstractLaserControlModel::load);
    connect(this, &AbstractLaserControlModel::currentPresetChanged, [this] {
        emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole + 4});
    });
}

AbstractLaserControlModel::~AbstractLaserControlModel() = default;

QHash<int, QByteArray> AbstractLaserControlModel::roleNames() const
{
    // LaserControlPresetFilterModel depends on isCurrent role number
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("preset")},
        {Qt::UserRole + 1, QByteArrayLiteral("uuid")},
        {Qt::UserRole + 2, QByteArrayLiteral("channel1Power")},
        {Qt::UserRole + 3, QByteArrayLiteral("channel2Power")},
        {Qt::UserRole + 4, QByteArrayLiteral("isCurrent")}
    };
}

QVariant AbstractLaserControlModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    const auto preset = m_presets.at(index.row());
    if (role == Qt::DisplayRole)
    {
        return preset->name();
    }
    if (role == Qt::UserRole)
    {
        return QVariant::fromValue(preset);
    }
    if (role == Qt::UserRole + 1)
    {
        return preset->uuid();
    }
    if (role == Qt::UserRole + 2)
    {
        return QVariant::fromValue(preset->channel1PowerDataSet());
    }
    if (role == Qt::UserRole + 3)
    {
        return QVariant::fromValue(preset->channel2PowerDataSet());
    }
    if (role == Qt::UserRole + 4)
    {
        return index.row() == m_currentPreset;
    }

    return QVariant{};
}

int AbstractLaserControlModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_presets.size();
}

void AbstractLaserControlModel::setCurrentPreset(int idx)
{
    if (m_currentPreset == idx)
    {
        return;
    }
    if (idx >= int(m_presets.size()))
    {
        return;
    }
    m_currentPreset = idx;
    emit currentPresetChanged();
}

LaserControlPreset* AbstractLaserControlModel::currentPresetItem() const
{
    if (m_currentPreset < 0 || m_currentPreset >= int(m_presets.size()))
    {
        return nullptr;
    }
    return m_presets.at(m_currentPreset);
}

void AbstractLaserControlModel::setLaserControlPresetDir(const QString& dir)
{
    if (m_laserControlPresetDir == dir)
    {
        return;
    }

    m_laserControlPresetDir = dir;
    clearJsonWatcher();
    addPathToWatcher(QDir{m_laserControlPresetDir}.absolutePath());
    emit laserControlPresetDirChanged();
}

void AbstractLaserControlModel::setAttributeModel(AttributeModel *model)
{
    if (m_attributeModel == model)
    {
        return;
    }
    m_attributeModel = model;
    disconnect(m_attributeModelDestroyed);
    if (m_attributeModel)
    {
        m_attributeModelDestroyed = connect(m_attributeModel, &AttributeModel::destroyed, this, std::bind(&AbstractLaserControlModel::setAttributeModel, this, nullptr));
    } else
    {
        m_attributeModelDestroyed = QMetaObject::Connection{};
    }
    emit attributeModelChanged();
}

void AbstractLaserControlModel::setChannel2Enabled(bool enabled)
{
    if (m_channel2Enabled == enabled)
    {
        return;
    }
    m_channel2Enabled = enabled;
    emit channel2EnabledChanged();
}

void AbstractLaserControlModel::addPathToWatcher(const QString& path)
{
    m_jsonWatcher->addPath(path);
}

void AbstractLaserControlModel::removePathFromWatcher(const QString& path)
{
    m_jsonWatcher->removePath(path);
}

void AbstractLaserControlModel::clearJsonWatcher()
{
    const auto directories = m_jsonWatcher->directories();
    if (!directories.isEmpty())
    {
        m_jsonWatcher->removePaths(directories);
    }
    const auto files = m_jsonWatcher->files();
    if (!files.empty())
    {
        m_jsonWatcher->removePaths(files);
    }
}

void AbstractLaserControlModel::connectPreset(LaserControlPreset* preset)
{
    connect(preset, &LaserControlPreset::nameChanged, [this, preset] {
        const auto it = std::find(m_presets.begin(), m_presets.end(), preset);
        if (it != m_presets.end())
        {
            const auto idx = std::distance(m_presets.begin(), it);
            emit dataChanged(index(idx), index(idx), {Qt::DisplayRole});
        }
    });
    connect(preset, &LaserControlPreset::channel1SamplesChanged, [this, preset] {
        const auto it = std::find(m_presets.begin(), m_presets.end(), preset);
        if (it != m_presets.end())
        {
            const auto idx = std::distance(m_presets.begin(), it);
            emit dataChanged(index(idx), index(idx), {Qt::UserRole + 2});
        }
    });
    connect(preset, &LaserControlPreset::channel2SamplesChanged, [this, preset] {
        const auto it = std::find(m_presets.begin(), m_presets.end(), preset);
        if (it != m_presets.end())
        {
            const auto idx = std::distance(m_presets.begin(), it);
            emit dataChanged(index(idx), index(idx), {Qt::UserRole + 3});
        }
    });
}

void AbstractLaserControlModel::addPresetInstance(LaserControlPreset* preset)
{
    connectPreset(preset);
    m_presets.push_back(preset);
}

void AbstractLaserControlModel::insertPresetInstance(uint i, LaserControlPreset* preset)
{
    beginInsertRows(QModelIndex(), i, i);
    connectPreset(preset);
    m_presets.insert(m_presets.begin() + i, preset);
    endInsertRows();
}

void AbstractLaserControlModel::removePresetInstance(uint index)
{
    m_presets.at(index)->deleteLater();
    m_presets.erase(m_presets.begin() + index);
}

void AbstractLaserControlModel::clearPresets()
{
    beginResetModel();
    setCurrentPreset(-1);
    qDeleteAll(m_presets);
    m_presets.clear();
    endResetModel();
}

Attribute *AbstractLaserControlModel::findPresetAttribute(LaserControlPreset::Key key) const
{
    if (!m_attributeModel)
    {
        return nullptr;
    }
    const auto channel1_it = channel1_preset_keys.find(key);
    if (channel1_it != channel1_preset_keys.end())
    {
        return m_attributeModel->findAttribute(std::get<1>((*channel1_it).second));
    }
    const auto channel2_it = channel2_preset_keys.find(key);
    if (channel2_it != channel2_preset_keys.end())
    {
        return m_attributeModel->findAttribute(std::get<1>((*channel2_it).second));
    }
    return nullptr;
}

Parameter *AbstractLaserControlModel::findPresetParameter(ParameterSet* ps, LaserControlPreset::Key key) const
{
    if (!ps)
    {
        return nullptr;
    }

    auto attribute = findPresetAttribute(key);

    if (!attribute)
    {
        return nullptr;
    }
    const auto &parameters = ps->parameters();

    auto it = std::find_if(parameters.begin(), parameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
    if (it == parameters.end())
    {
        return nullptr;
    }
    return *it;
}

bool AbstractLaserControlModel::updateParameterSet(ParameterSet* ps, QModelIndex row)
{
    if (!m_attributeModel || !ps || !row.isValid())
    {
        return false;
    }

    auto preset = row.data(Qt::UserRole).value<LaserControlPreset*>();

    if (!preset)
    {
        return false;
    }

    auto changed = false;

    for (auto key : channel1_preset_keys)
    {
        auto parameter = findPresetParameter(ps, std::get<0>(key));
        const auto attribute = findPresetAttribute(std::get<0>(key));

        if (!attribute)
        {
            continue;
        }

        if (!parameter)
        {
            parameter = ps->createParameter(QUuid::createUuid(), attribute, {});
        }

        if (parameter->value().toInt() == preset->getValue(std::get<0>(key)))
        {
            continue;
        }

        parameter->setValue(preset->getValue(std::get<0>(key)));

        changed = true;
    }

    for (auto key : channel2_preset_keys)
    {
        auto parameter = findPresetParameter(ps, std::get<0>(key));

        const auto attribute = findPresetAttribute(std::get<0>(key));

        if (!attribute)
        {
            continue;
        }

        if (!parameter)
        {
            parameter = ps->createParameter(QUuid::createUuid(), attribute, {});
        }

        if (parameter->value().toInt() == preset->getValue(std::get<0>(key)))
        {
            continue;
        }

        parameter->setValue(preset->getValue(std::get<0>(key)));

        changed = true;
    }

    return changed;
}

bool AbstractLaserControlModel::hasPresetParameters(ParameterSet* ps)
{
    if (!ps)
    {
        return false;
    }

    for (auto key : channel1_preset_keys)
    {
        const auto parameter = findPresetParameter(ps, std::get<0>(key));

        if (!parameter)
        {
            return false;
        }
    }
    if (m_channel2Enabled)
    {
        for (auto key : channel2_preset_keys)
        {
            const auto parameter = findPresetParameter(ps, std::get<0>(key));

            if (!parameter)
            {
                return false;
            }
        }
    }

    return true;
}

bool AbstractLaserControlModel::compareParameterSet(ParameterSet* ps, QModelIndex row)
{
    if (!ps)
    {
        return false;
    }
    if (!row.isValid())
    {
        return false;
    }

    auto preset = row.data(Qt::UserRole).value<LaserControlPreset*>();

    if (!preset)
    {
        return false;
    }

    for (auto key : channel1_preset_keys)
    {
        auto parameter = findPresetParameter(ps, std::get<0>(key));

        if (!parameter)
        {
            return false;
        }

        if (parameter->value().toInt() != preset->getValue(std::get<0>(key)))
        {
            return false;
        }
    }
    if (m_channel2Enabled)
    {
        for (auto key : channel2_preset_keys)
        {
            auto parameter = findPresetParameter(ps, std::get<0>(key));

            if (!parameter)
            {
                return false;
            }

            if (parameter->value().toInt() != preset->getValue(std::get<0>(key)))
            {
                return false;
            }
        }
    }
    return true;
}

bool AbstractLaserControlModel::updatePresetValues(ParameterSet* ps, LaserControlPreset* preset)
{
    if (!ps)
    {
        return false;
    }

    if (!preset)
    {
        return false;
    }

    auto changed = false;

    for (auto key : channel1_preset_keys)
    {
        if (!findPresetParameter(ps, std::get<0>(key)))
        {
            return false;
        }
    }

    if (m_channel2Enabled)
    {
        for (auto key : channel2_preset_keys)
        {
            if (!findPresetParameter(ps, std::get<0>(key)))
            {
                return false;
            }
        }
    }

    for (auto key : channel1_preset_keys)
    {
        auto parameter = findPresetParameter(ps, std::get<0>(key));

        if (parameter->value().toInt() == preset->getValue(std::get<0>(key)))
        {
            continue;
        }

        preset->setValue(std::get<0>(key), parameter->value().toInt());

        changed = true;
    }

    if (m_channel2Enabled)
    {
        for (auto key : channel2_preset_keys)
        {
            auto parameter = findPresetParameter(ps, std::get<0>(key));

            if (parameter->value().toInt() == preset->getValue(std::get<0>(key)))
            {
                continue;
            }

            preset->setValue(std::get<0>(key), parameter->value().toInt());

            changed = true;
        }
    }

    return changed;
}

bool AbstractLaserControlModel::clearPresetValues(ParameterSet* ps)
{
    if (!ps)
    {
        return false;
    }

    auto changed = false;

    for (auto key : channel1_preset_keys)
    {
        auto parameter = findPresetParameter(ps, std::get<0>(key));

        if (parameter)
        {
            ps->removeParameter(parameter);

            changed = true;
        }
    }

    for (auto key : channel2_preset_keys)
    {
        auto parameter = findPresetParameter(ps, std::get<0>(key));

        if (parameter)
        {
            ps->removeParameter(parameter);

            changed = true;
        }
    }

    return changed;
}

void AbstractLaserControlModel::load()
{
    auto dir = QDir{laserControlPresetDir()};

    if (!dir.exists())
    {
        dir.mkpath(dir.absolutePath());
        addPathToWatcher(dir.absolutePath());
    }

    clearPresets();

    beginResetModel();
    const auto files = dir.entryInfoList(QStringList{QStringLiteral("*.json")}, QDir::Files | QDir::Readable);
    for (const auto &file : files)
    {
        addPathToWatcher(file.absoluteFilePath());
        auto preset = LaserControlPreset::load(file.absoluteFilePath(), this);

        if (preset)
        {
            addPresetInstance(preset);
        }
    }
    std::sort(m_presets.begin(), m_presets.end(), [] (const auto p1, const auto p2) {
        return p1->name() < p2->name();
    });
    endResetModel();
}

}
}
