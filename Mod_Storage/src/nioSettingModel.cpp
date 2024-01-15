#include "nioSettingModel.h"
#include "productModel.h"
#include "product.h"
#include "attributeModel.h"
#include "attribute.h"
#include "parameter.h"
#include "parameterSet.h"
#include "resultSetting.h"
#include "event/resultType.h"

#include <QCoreApplication>

namespace precitec
{
namespace storage
{


NioSettingModel::NioSettingModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &NioSettingModel::productModelChanged, this, &NioSettingModel::createGraphNiosList);
}

NioSettingModel::~NioSettingModel()
{
}

int NioSettingModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_nioItems.size();
}

QVariant NioSettingModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();

    if(row < 0 || row >= (int) m_nioItems.size())
    {
        return {};
    }

    const auto &nioItem = m_nioItems.at(index.row());

    switch (role) {
        case Qt::DisplayRole:
            return nioItem->enumType();
        case Qt::UserRole:
            return nioItem->uuid();
        case Qt::UserRole + 1:
            return nioItem->name();
        case Qt::UserRole + 2:
            return nioItem->plotterNumber();
        case Qt::UserRole + 3:
            return nioItem->plottable();
        case Qt::UserRole + 4:
            return nioItem->min();
        case Qt::UserRole + 5:
            return nioItem->max();
        case Qt::UserRole + 6:
            return nioItem->lineColor();
        case Qt::UserRole + 7:
            return nioItem->visibleItem();
    }
    return {};
}


QHash<int, QByteArray> NioSettingModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, QByteArrayLiteral("enumType")},
        {Qt::UserRole, QByteArrayLiteral("uuid")},
        {Qt::UserRole+1, QByteArrayLiteral("name")},
        {Qt::UserRole+2, QByteArrayLiteral("plotterNumber")},
        {Qt::UserRole+3, QByteArrayLiteral("plottable")},
        {Qt::UserRole+4, QByteArrayLiteral("min")},
        {Qt::UserRole+5, QByteArrayLiteral("max")},
        {Qt::UserRole+6, QByteArrayLiteral("lineColor")},
        {Qt::UserRole+7, QByteArrayLiteral("visibleItem")}
    };
}


void NioSettingModel::setProductModel(ProductModel *productModel)
{
    if (productModel == m_productModel)
    {
        return;
    }
    m_productModel = productModel;
    if (m_productModel)
    {
        connect(m_productModel, &ProductModel::rowsInserted, this, &NioSettingModel::createGraphNiosList);
        connect(m_productModel, &ProductModel::dataChanged, this, &NioSettingModel::createGraphNiosList);
    }
    emit productModelChanged();
}

void NioSettingModel::setAttributeModel(AttributeModel *attributeModel)
{
    if (m_attributeModel == attributeModel)
    {
       return;
    }
    m_attributeModel = attributeModel;
    emit attributeModelChanged();
}

void NioSettingModel::createGraphNiosList()
{
    if (!m_productModel)
    {
        return;
    }
    if (!m_attributeModel)
    {
        return;
    }

    for (auto product : m_productModel->products())
    {
        for (auto parameterSet : product->filterParameterSets())
        {
            for (auto parameter : parameterSet->parameters())
            {
                if (parameter->name().compare(QLatin1String("NioType")) == 0)
                {
                    createAndAddItem(parameter->filterId(), parameter->value().toInt());
                }
            }
        }
    }
}

void NioSettingModel::createAndAddItem(const QUuid &uuid, int enumType)
{
    if (std::any_of(m_nioItems.begin(), m_nioItems.end(), [enumType] (auto nioItem) { return nioItem->enumType() == enumType; }))
    {
        return;
    }

    beginInsertRows(QModelIndex(), m_nioItems.size(), m_nioItems.size());
    auto *newValue = new ResultSetting(uuid, enumType, this);
    newValue->setName(nameForResult(enumType));
    newValue->setLineColor(QStringLiteral("#ee6622"));
    m_nioItems.push_back(newValue);
    endInsertRows();
}

QString NioSettingModel::nameForResult(int enumType)
{
    static const std::map<precitec::interface::ResultType, std::pair<const char*, const char *>> s_resultTypeNames = {
        {precitec::interface::XCoordOutOfLimits, QT_TRANSLATE_NOOP3("", "X position out of limits", "Precitec.Results.XCoordOutOfLimits")},
        {precitec::interface::YCoordOutOfLimits, QT_TRANSLATE_NOOP3("", "Y position out of limits", "Precitec.Results.YCoordOutOfLimits")},
        {precitec::interface::ZCoordOutOfLimits, QT_TRANSLATE_NOOP3("", "Z position out of limits", "Precitec.Results.ZCoordOutOfLimits")},
        {precitec::interface::ValueOutOfLimits, QT_TRANSLATE_NOOP3("", "Value out of limits", "Precitec.Results.ValueOutOfLimits")},
        {precitec::interface::RankViolation, QT_TRANSLATE_NOOP3("", "RankViolation", "Precitec.Results.RankViolation")},
        {precitec::interface::GapPositionError, QT_TRANSLATE_NOOP3("", "GapPositionError", "Precitec.Results.GapPositionError")},
        {precitec::interface::LaserPowerOutOfLimits, QT_TRANSLATE_NOOP3("", "LaserPowerOutOfLimits", "Precitec.Results.LaserPowerOutOfLimits")},
        {precitec::interface::SensorOutOfLimits, QT_TRANSLATE_NOOP3("", "SensorOutOfLimit", "Precitec.Results.SensorOutOfLimits")},
        {precitec::interface::Surveillance01, QT_TRANSLATE_NOOP3("", "Surveillance 1", "Precitec.Results.Surveillance01")},
        {precitec::interface::Surveillance02, QT_TRANSLATE_NOOP3("", "Surveillance 2", "Precitec.Results.Surveillance02")},
        {precitec::interface::Surveillance03, QT_TRANSLATE_NOOP3("", "Surveillance 3", "Precitec.Results.Surveillance03")},
        {precitec::interface::Surveillance04, QT_TRANSLATE_NOOP3("", "Surveillance 4", "Precitec.Results.Surveillance04")},
    };
    const auto it = s_resultTypeNames.find(static_cast<precitec::interface::ResultType>(enumType));
    if (it == s_resultTypeNames.end())
    {
        return QString::number(enumType);
    } else
    {
        return QCoreApplication::instance()->translate("", it->second.first, it->second.second);
    }
}

int NioSettingModel::findIndex(int value)
{
    for (auto i = 0; i < rowCount({}); i++)
    {
        const QModelIndex sourceIndex = index(i, 0);
        auto test = data(sourceIndex, 0);
        if (test.value<int>() == value)
        {
            return i;
        }
    }
    return -1;
}

QString NioSettingModel::findName(int value)
{
    for (int i = 0; i < rowCount({}); i++)
    {
        const QModelIndex sourceIndex = index(i, 0);
        auto test = data(sourceIndex, 0);
        if (test.value<int>() == value)
        {
            return data(sourceIndex, Qt::UserRole + 1).toString();
        }
    }
    return {};
}

}
}
