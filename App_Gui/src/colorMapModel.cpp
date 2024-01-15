#include "colorMapModel.h"
#include "product.h"
#include "precitec/colorMap.h"
#include "precitec/multicolorSet.h"

#include <math.h>

using precitec::storage::Product;
using precitec::gui::components::plotter::ColorMap;
using precitec::gui::components::plotter::MulticolorSet;

namespace precitec
{
namespace gui
{

ColorMapModel::ColorMapModel ( QObject* parent )
    : QAbstractListModel(parent)
    , m_example(new MulticolorSet(this))
{
    m_example->setDrawingOrder(MulticolorSet::DrawingOrder::OnTop);
    connect(this, &ColorMapModel::binaryChanged, this, &ColorMapModel::prepareExample);
    connect(this, &ColorMapModel::currentProductChanged, this, &ColorMapModel::prepareExample);
    connect(this, &QAbstractListModel::rowsInserted, this, &ColorMapModel::rowCountChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this, &ColorMapModel::rowCountChanged);
    connect(this, &QAbstractListModel::modelReset, this, &ColorMapModel::rowCountChanged);
}

ColorMapModel::~ColorMapModel() = default;

QHash<int, QByteArray> ColorMapModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("value")},
        {Qt::UserRole, QByteArrayLiteral("color")},
        {Qt::UserRole + 1, QByteArrayLiteral("hue")},
        {Qt::UserRole + 2, QByteArrayLiteral("saturation")},
        {Qt::UserRole + 3, QByteArrayLiteral("lightness")}
    };
}

QVariant ColorMapModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_currentProduct || !m_colorMap)
    {
        return QVariant{};
    }
    if (index.row() >= int(m_colorMap->size()))
    {
        return QVariant{};
    }
    auto color = m_colorMap->colors().at(index.row());
    switch (role)
    {
        case Qt::DisplayRole:
            return std::round(100.0f * color.first);
        case Qt::UserRole:
            return color.second;
        case Qt::UserRole + 1:
            return color.second.hslHueF();
        case Qt::UserRole + 2:
            return color.second.hslSaturationF();
        case Qt::UserRole + 3:
            return color.second.lightnessF();
        default:
            return QVariant{};
    }
}

int ColorMapModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_currentProduct || !m_colorMap)
    {
        return 0;
    }
    return m_colorMap->size();
}

void ColorMapModel::setCurrentProduct(Product *product)
{
    if (m_currentProduct == product)
    {
        return;
    }
    beginResetModel();

    m_currentProduct = product;
    disconnect(m_productDestroyed);
    if (m_currentProduct)
    {
        m_productDestroyed = connect(m_currentProduct, &Product::destroyed, this, std::bind(&ColorMapModel::setCurrentProduct, this, nullptr));
        if (m_binary)
        {
            m_colorMap = m_currentProduct->errorLevelColorMap();
        } else
        {
            m_colorMap = m_currentProduct->signalyQualityColorMap();
        }
    } else
    {
        m_productDestroyed = QMetaObject::Connection{};
    }
        prepareExample();

    endResetModel();
    emit currentProductChanged();
}

void ColorMapModel::prepareExample()
{
    if (!m_currentProduct || !m_colorMap)
    {
        m_example->clear();
        m_example->setColorMap(nullptr);
    } else
    {
        m_example->clear();
        m_example->setColorMap(m_colorMap);
        if (m_binary)
        {
            m_example->setDrawingMode(MulticolorSet::DrawingMode::SimpleBlock);
            for (auto i = 0; i < 201; i++)
            {
                m_example->addSample(QVector2D{float(i), 1}, i / 100.0f);
            }
        } else
        {
            m_example->setDrawingMode(MulticolorSet::DrawingMode::LineWithPoints);
            auto angle = 0.0;
            for (auto i = 0; i < 201; i++)
            {
                const auto sinus = sinf(angle);
                m_example->addSample(QVector2D{float(i), 200.0f * sinus}, 2 * sinus);
                angle += (2 * M_PI) / 200;
            }
        }
    }
}

void ColorMapModel::setBinary(bool binary)
{
    if (m_binary == binary)
    {
        return;
    }
    m_binary = binary;
    emit binaryChanged();
}

void ColorMapModel::addColor(float value, const QColor& color)
{
    if (!m_colorMap)
    {
        return;
    }
    beginResetModel();
    m_colorMap->addColor(value, color);
    endResetModel();
    emit markAsChanged();
}

void ColorMapModel::updateValue(quint32 idx, float value)
{
    if (!m_colorMap || idx >= m_colorMap->size())
    {
        return;
    }
    beginResetModel();
    m_colorMap->updateValue(idx, value);
    endResetModel();
    emit markAsChanged();
}

void ColorMapModel::updateColor(quint32 idx, const QColor &color)
{
    if (!m_currentProduct || !m_colorMap  || idx >= m_colorMap->size())
    {
        return;
    }
    m_colorMap->updateColor(idx, color);
    emit dataChanged(index(idx, 0), index(idx, 0), {Qt::UserRole});
    emit markAsChanged();
}

void ColorMapModel::removeColor(quint32 idx)
{
    if (!m_currentProduct || !m_colorMap || idx >= m_colorMap->size())
    {
        return;
    }
    beginRemoveRows(QModelIndex(), idx, idx);
    m_colorMap->removeColor(idx);
    endRemoveRows();
    emit markAsChanged();
}

void ColorMapModel::reset()
{
    if (!m_currentProduct || !m_colorMap )
    {
        return;
    }
    beginResetModel();
    m_colorMap->resetToDefault();
    endResetModel();
    emit markAsChanged();
}

}
}

