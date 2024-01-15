#include "laserControlModel.h"
#include "laserControlPreset.h"
#include "productController.h"
#include "productModel.h"
#include "product.h"
#include "seam.h"
#include "seamSeries.h"

using precitec::storage::LaserControlPreset;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;

namespace precitec
{
namespace gui
{

LaserControlModel::LaserControlModel(QObject* parent)
    : AbstractLaserControlModel(parent)
{
}

LaserControlModel::~LaserControlModel() = default;

void LaserControlModel::setProductController(ProductController *controller)
{
    if (m_productController == controller)
    {
        return;
    }
    disconnect(m_productControllerDestroyed);
    m_productControllerDestroyed = {};
    m_productController = controller;
    if (m_productController)
    {
        m_productControllerDestroyed = connect(m_productController, &QObject::destroyed, std::bind(&LaserControlModel::setProductController, this, nullptr));
    }
    emit productControllerChanged();
}

void LaserControlModel::setEditing(bool editing)
{
    if (m_isEditing == editing)
    {
        return;
    }
    m_isEditing = editing;
    emit editingChanged();
}

void LaserControlModel::load()
{
    QUuid currentUuid = QUuid();
    if (currentPreset() >= 0 && currentPreset() < rowCount())
    {
        currentUuid = presets().at(currentPreset())->uuid();
    }

    AbstractLaserControlModel::load();

    for (auto preset : presets())
    {
        if (preset)
        {
            connect(preset, &LaserControlPreset::editStarted, this, &LaserControlModel::disablePresets);
            connect(preset, &LaserControlPreset::editStopped, this, &LaserControlModel::enablePresets);
        }
    }

    if (!currentUuid.isNull())
    {
        setCurrentPreset(findPresetIndex(currentUuid));
    }
}

void LaserControlModel::deletePreset(LaserControlPreset* preset)
{
    if (!preset)
    {
        return;
    }

    const auto& presetList = presets();
    const auto it = std::find(presetList.begin(), presetList.end(), preset);
    if (it == presetList.end())
    {
        return;
    }

    const auto row = std::distance(presetList.begin(), it);

    beginRemoveRows(QModelIndex(), row, row);

    const auto filePath = preset->filePath();
    disconnect(preset, &LaserControlPreset::editStarted, this, &LaserControlModel::disablePresets);
    disconnect(preset, &LaserControlPreset::editStopped, this, &LaserControlModel::enablePresets);
    removePathFromWatcher(filePath);
    removePresetInstance(row);

    LaserControlPreset::deleteFile(filePath);

    endRemoveRows();

    enablePresets();
    setCurrentPreset(-1);
}

void LaserControlModel::enablePresets()
{
    for (auto preset : presets())
    {
        preset->setEnabled(true);
    }
    setEditing(false);
}

void LaserControlModel::disablePresets()
{
    for (auto preset : presets())
    {
        preset->setEnabled(false);
    }
    setEditing(true);
}

bool LaserControlModel::findPreset(const QUuid &uuid)
{
    for (auto preset : presets())
    {
        if (preset->uuid() == uuid)
        {
            return true;
        }
    }
    return false;
}

int LaserControlModel::findPresetIndex(const QUuid& uuid)
{
    for (auto i = 0u; i < presets().size(); i++)
    {
        const auto preset = presets().at(i);
        if (preset->uuid() == uuid)
        {
            return i;
        }
    }
    return -1;
}

LaserControlPreset* LaserControlModel::createNewPreset()
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    auto preset = new LaserControlPreset(this);

    const auto dir = QDir{laserControlPresetDir()};
    const auto filePath = dir.absoluteFilePath(preset->uuid().toString(QUuid::WithoutBraces) + QStringLiteral(".json"));

    preset->setName(QStringLiteral("New Preset"));
    preset->setFilePath(filePath);
    addPathToWatcher(filePath);

    connect(preset, &LaserControlPreset::editStarted, this, &LaserControlModel::disablePresets);
    connect(preset, &LaserControlPreset::editStopped, this, &LaserControlModel::enablePresets);
    addPresetInstance(preset);
    endInsertRows();

    preset->setState(LaserControlPreset::State::New);
    setCurrentPreset(rowCount() - 1);

    return preset;
}

void LaserControlModel::updateHardwareParameters(const QUuid& presetId)
{
    if (!m_productController)
    {
        return;
    }
    if (!m_productController->productModel())
    {
        return;
    }

    const auto products = m_productController->productModel()->products();

    for (auto product : products)
    {
        bool productChanged = false;

        m_productController->selectProduct(product->uuid());
        if (product->laserControlPreset() == presetId)
        {
            const auto row = findPresetIndex(presetId);
            if (row >= 0)
            {
                auto ps = m_productController->currentProduct()->hardwareParameters();

                updateParameterSet(ps, index(row, 0));
                productChanged = true;
            }
        }

        for (auto series : product->seamSeries())
        {
            m_productController->selectSeamSeries(series->uuid());

            if (series->laserControlPreset() == presetId)
            {
                const auto row = findPresetIndex(presetId);
                if (row >= 0)
                {
                    auto ps = m_productController->currentSeamSeries()->hardwareParameters();

                    updateParameterSet(ps, index(row, 0));
                    productChanged = true;
                }
            }

            for (auto seam : series->seams())
            {
                m_productController->selectSeam(seam->uuid());

                if (seam->laserControlPreset() == presetId)
                {
                    const auto row = findPresetIndex(presetId);
                    if (row >= 0)
                    {
                        auto ps = m_productController->currentSeam()->hardwareParameters();

                        updateParameterSet(ps, index(row, 0));
                        productChanged = true;
                    }
                }
            }
        }

        if (productChanged)
        {
            m_productController->markAsChanged();
        }
    }

    if (m_productController->hasChanges())
    {
        m_productController->saveChanges();
    }
}

}
}
