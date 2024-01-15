#include "hardwareParametersOverviewModel.h"

#include "abstractHardwareParameterModel.h"
#include "attribute.h"
#include "attributeModel.h"
#include "guiConfiguration.h"
#include "linkedSeam.h"
#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include "seam.h"
#include "seamSeries.h"
#include "languageSupport.h"

#include <QGuiApplication>
#include <QFontMetrics>

#include <algorithm>

namespace precitec
{

using storage::AttributeModel;
using storage::Product;
using storage::Parameter;
using storage::ParameterSet;
using storage::Seam;
using storage::SeamSeries;

namespace gui
{

HardwareParametersOverviewModel::HardwareParametersOverviewModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(this, &HardwareParametersOverviewModel::productChanged, this, &HardwareParametersOverviewModel::init);
    connect(GuiConfiguration::instance(), &GuiConfiguration::seamSeriesOnProductStructureChanged, this, &HardwareParametersOverviewModel::init);
}

HardwareParametersOverviewModel::~HardwareParametersOverviewModel() = default;

QVariant HardwareParametersOverviewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        auto parameter = dataForParameterSet(index, index.column() == 0 ? m_product->hardwareParameters() : m_measureTasks.at(index.column() - 1)->hardwareParameters());

        if (!parameter)
        {
            return {};
        }

        if (parameter->type() == Parameter::DataType::Double || parameter->type() == Parameter::DataType::Float)
        {
            return QLocale{}.toString(parameter->value().toDouble(), 'f', 3);
        }

        return parameter->value();
    }
    return {};
}

Parameter* HardwareParametersOverviewModel::dataForParameterSet(const QModelIndex &index, ParameterSet *set) const
{
    if (!set)
    {
        return nullptr;
    }
    auto it = m_hardwareParameterKeys.begin();
    std::advance(it, index.row());
    const auto &name = *it;
    auto findIt = std::find_if(set->parameters().begin(), set->parameters().end(), [name] (auto *p) { return p->name() == name;});
    if (findIt == set->parameters().end())
    {
        return nullptr;
    }
    return *findIt;
}

int HardwareParametersOverviewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_hardwareParameterKeys.size();
}

int HardwareParametersOverviewModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_measureTasks.size() + (m_product ? 1 : 0);
}

void HardwareParametersOverviewModel::setProduct(Product *product)
{
    if (m_product == product)
    {
        return;
    }
    // TODO: detect changes
    m_product = product;
    disconnect(m_productDestroyed);
    if (m_product)
    {
        m_productDestroyed = connect(m_product, &Product::destroyed, this, std::bind(&HardwareParametersOverviewModel::setProduct, this, nullptr));
    }
    else
    {
        m_productDestroyed = {};
    }
    emit productChanged();
}

void HardwareParametersOverviewModel::setKeyValueAttributeModel(AttributeModel *model)
{
    if (m_keyValueAttributeModel == model)
    {
        return;
    }
    disconnect(m_keyValueAttributeModelDestroyed);
    m_keyValueAttributeModel = model;
    if (m_keyValueAttributeModel)
    {
        m_keyValueAttributeModelDestroyed = connect(m_keyValueAttributeModel, &AttributeModel::destroyed, this, std::bind(&HardwareParametersOverviewModel::setKeyValueAttributeModel, this, nullptr));
    }
    else
    {
        m_keyValueAttributeModelDestroyed = {};
    }
    emit keyValueAttributeModelChanged();
}

void HardwareParametersOverviewModel::init()
{
    beginResetModel();
    m_hardwareParameterKeys.clear();
    m_measureTasks.clear();
    m_columnWidth.clear();
    initForProduct();
    endResetModel();
}

void HardwareParametersOverviewModel::initForProduct()
{
    if (!m_product)
    {
        return;
    }
    initParameterSet(m_product->hardwareParameters());
    for (auto *series : m_product->seamSeries())
    {
        initForSeamSeries(series);
    }
}

void HardwareParametersOverviewModel::initForSeamSeries(SeamSeries *seamSeries)
{
    if (GuiConfiguration::instance()->seamSeriesOnProductStructure())
    {
        m_measureTasks.push_back(seamSeries);
        initParameterSet(seamSeries->hardwareParameters());
    }
    for (auto *seam : seamSeries->seams())
    {
        initForSeam(seam);
    }
}

void HardwareParametersOverviewModel::initForSeam(Seam *seam)
{
    if (seam->metaObject()->inherits(&storage::LinkedSeam::staticMetaObject))
    {
        // ignore linked seams
        return;
    }

    m_measureTasks.push_back(seam);
    initParameterSet(seam->hardwareParameters());
}

void HardwareParametersOverviewModel::initParameterSet(ParameterSet *set)
{
    if (!set)
    {
        return;
    }
    for (auto *parameter : set->parameters())
    {
        m_hardwareParameterKeys.insert(parameter->name());
    }
}

QVariant HardwareParametersOverviewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return {};
    }
    switch (orientation)
    {
    case Qt::Horizontal:
    {
        if (section >= columnCount())
        {
            return {};
        }
        if (section == 0)
        {
            return QStringLiteral("P");
        }
        auto *measureTask = m_measureTasks.at(section - 1);
        if (auto *ss = qobject_cast<SeamSeries*>(measureTask))
        {
            return QStringLiteral("%1").arg(ss->visualNumber());
        }
        if (auto *s = qobject_cast<Seam*>(measureTask))
        {
            if (GuiConfiguration::instance()->seamSeriesOnProductStructure())
            {
                return QStringLiteral("%1\n%2").arg(s->seamSeries()->visualNumber()).arg(s->visualNumber());
            }
            return QStringLiteral("%1").arg(s->visualNumber());
        }
        return {};
    }
    case Qt::Vertical:
    {
        if (section >= rowCount())
        {
            return {};
        }
        auto it = m_hardwareParameterKeys.begin();
        std::advance(it, section);
        if (m_keyValueAttributeModel)
        {
            if (auto *attribute = m_keyValueAttributeModel->findAttributeByName(*it))
            {
                const auto& name = AbstractHardwareParameterModel::nameForAttribute(attribute);
                if (const auto& unit = LanguageSupport::instance()->getStringWithFallback(attribute->unit(), attribute->unit()); !unit.isEmpty())
                {
                    return QStringLiteral("%1 [%2]").arg(name).arg(unit);
                }
                return name;
            }
        }
        return *it;
    }
    default:
        Q_UNREACHABLE();
    }
}

int HardwareParametersOverviewModel::columnWidth(int column)
{
    if (!m_product || column < 0 || column > int(m_measureTasks.size()))
    {
        return 1;
    }
    const auto it = m_columnWidth.find(column);
    if (it != m_columnWidth.end())
    {
        return it->second;
    }
    const auto width = calculateColumnWidth(column);
    m_columnWidth.emplace(column, width);

    return width;
}

int HardwareParametersOverviewModel::calculateColumnWidth(int column) const
{
    int width = 1;
    auto boldFont = QGuiApplication::font();
    boldFont.setBold(true);
    QFontMetrics headerMetrics{boldFont};
    width = headerMetrics.horizontalAdvance(headerData(column, Qt::Horizontal).toString());

    QFontMetrics dataMetrics{QGuiApplication::font()};
    storage::ParameterSet *parameters = nullptr;
    if (column == 0)
    {
        parameters = m_product->hardwareParameters();
    }
    else
    {
        parameters = m_measureTasks.at(column - 1)->hardwareParameters();
    }
    if (parameters)
    {
        for (auto *parameter : parameters->parameters())
        {
            if (parameter->type() == Parameter::DataType::Double || parameter->type() == Parameter::DataType::Float)
            {
                width = std::max(width, dataMetrics.horizontalAdvance(QLocale{}.toString(parameter->value().toDouble(), 'f', 3)));
            } else
            {
                width = std::max(width, dataMetrics.horizontalAdvance(parameter->value().toString()));
            }
        }
    }

    return width;
}

}
}
