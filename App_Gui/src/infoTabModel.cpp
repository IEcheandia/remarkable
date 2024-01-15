#include "infoTabModel.h"

#include <QColor>
#include <QUuid>

using namespace precitec::image;

namespace precitec
{
namespace gui
{

InfoTabModel::InfoTabModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(this, &InfoTabModel::dataChanged, this, &InfoTabModel::updateModel);
    connect(this, &InfoTabModel::filterInstancesChanged, this, &InfoTabModel::updateModel);
}

InfoTabModel::~InfoTabModel() = default;

QHash<int, QByteArray> InfoTabModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("text")},
        {Qt::UserRole, QByteArrayLiteral("italic")},
        {Qt::UserRole + 1, QByteArrayLiteral("bold")},
        {Qt::UserRole + 2, QByteArrayLiteral("color")}
    };
}

QVariant InfoTabModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (index.row() < 0 || index.row() >= m_texts.size())
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        return m_texts.at(index.row());
    }
    if (role == Qt::UserRole)
    {
        return m_data.at(index.row() / 3).font_.italic;
    }
    if (role == Qt::UserRole + 1)
    {
        return m_data.at(index.row() / 3).font_.bold;
    }
    if (role == Qt::UserRole + 2)
    {
        const auto color = m_data.at(index.row() / 3).color_;
        return QColor(color.red, color.green, color.blue, color.alpha);
    }
    return {};
}

int InfoTabModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_texts.size();
}

void InfoTabModel::setData(const std::vector<precitec::image::OverlayText> data)
{
    m_data = data;

    emit dataChanged();
}

void InfoTabModel::setFilterInstances(const std::vector<fliplib::InstanceFilter> instances)
{
    m_filterInstances = instances;

    emit filterInstancesChanged();
}

void InfoTabModel::updateModel()
{
    beginResetModel();

    m_texts.clear();

    for (auto data : m_data)
    {
        const auto list = QString::fromStdString(data.text_).split(QStringLiteral(":"));

        if (list.at(1).compare(QStringLiteral("FILTERGUID")) == 0)
        {
            m_texts << filterName(QUuid{list.at(0)}) << QStringLiteral("") << QStringLiteral("");
        } else
        {
            auto unit = list.at(1);
            unit.replace(QStringLiteral("Unit."), QStringLiteral(""));
            unit.replace(QStringLiteral("None"), QStringLiteral(""));
            unit = unit.toLower();
            const ushort s16[] = {
                0x00B2,
                0x0
            };
            unit.replace(QStringLiteral("sqmm"), QStringLiteral("mm").append(QString::fromUtf16(s16)));

            m_texts << list.at(0) << list.at(2) << unit;
        }
    }

    endResetModel();
}

QString InfoTabModel::filterName(const QUuid &id)
{
    const auto it = std::find_if(m_filterInstances.begin(), m_filterInstances.end(), [&id] (auto instance)
    {
        return id == QUuid{QString::fromStdString(instance.id.toString())};
    });

    if (it != m_filterInstances.end())
    {
        return QString::fromStdString((*it).name);
    }

    return id.toString();
}

}
}
