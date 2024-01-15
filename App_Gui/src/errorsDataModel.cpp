#include "errorsDataModel.h"
#include "abstractPlotterDataModel.h"
#include "seam.h"
#include "precitec/multicolorSet.h"

using precitec::gui::components::plotter::MulticolorSet;

namespace precitec
{
namespace gui
{

ErrorsDataModel::ErrorsDataModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(this, &ErrorsDataModel::plotterModelChanged, this, &ErrorsDataModel::update);
}

ErrorsDataModel::~ErrorsDataModel() = default;

int ErrorsDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_errors.size();
}

QHash<int, QByteArray> ErrorsDataModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("seam")},
        {Qt::UserRole, QByteArrayLiteral("name")},
        {Qt::UserRole + 1, QByteArrayLiteral("color")},
        {Qt::UserRole + 2, QByteArrayLiteral("position")}
    };
}

QVariant ErrorsDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= int(m_errors.size()))
    {
        return {};
    }

    const auto& errorData = m_errors.at(index.row());

    if (role == Qt::DisplayRole)
    {
        return tr("Seam %1").arg(errorData.m_visualSeamNumber);
    }
    if (role == Qt::UserRole)
    {
        return errorData.m_name;
    }
    if (role == Qt::UserRole + 1)
    {
        return errorData.m_color;
    }
    if (role == Qt::UserRole + 2)
    {
        return errorData.m_position;
    }
    return {};
}

void ErrorsDataModel::setPlotterModel(AbstractPlotterDataModel* model)
{
    if (m_plotterModel == model)
    {
        return;
    }

    if (m_plotterModel)
    {
        disconnect(m_destroyedConnection);
        disconnect(m_plotterModel, &AbstractPlotterDataModel::modelReset, this, &ErrorsDataModel::update);
        disconnect(m_plotterModel, &AbstractPlotterDataModel::rowsInserted, this, &ErrorsDataModel::update);
        disconnect(m_plotterModel, &AbstractPlotterDataModel::rowsRemoved, this, &ErrorsDataModel::update);
        disconnect(m_plotterModel, &AbstractPlotterDataModel::dataChanged, this, &ErrorsDataModel::update);
    }

    m_plotterModel = model;

    if (m_plotterModel)
    {
        m_destroyedConnection = connect(m_plotterModel, &QObject::destroyed, this, std::bind(&ErrorsDataModel::setPlotterModel, this, nullptr));
        connect(m_plotterModel, &AbstractPlotterDataModel::modelReset, this, &ErrorsDataModel::update);
        connect(m_plotterModel, &AbstractPlotterDataModel::rowsInserted, this, &ErrorsDataModel::update);
        connect(m_plotterModel, &AbstractPlotterDataModel::rowsRemoved, this, &ErrorsDataModel::update);
        connect(m_plotterModel, &AbstractPlotterDataModel::dataChanged, this, &ErrorsDataModel::update);
    } else
    {
        m_destroyedConnection = {};
    }
    emit plotterModelChanged();
}

void ErrorsDataModel::update()
{
    beginResetModel();
    m_errors.clear();

    if (m_plotterModel)
    {
        for (int i = 0; i < m_plotterModel->rowCount(); ++i)
        {
            const auto idx = m_plotterModel->index(i, 0);
            if (idx.data(Qt::UserRole + 10).toBool())
            {
                const auto name = idx.data(Qt::DisplayRole).toString();
                const auto color = idx.data(Qt::UserRole + 7).value<QColor>();
                const auto dataSetData = idx.data(Qt::UserRole + 2);

                if (dataSetData.canConvert<MulticolorSet*>())
                {
                    if (auto ds = dataSetData.value<MulticolorSet*>())
                    {
                        for (auto it = ds->samples().begin(); it != ds->samples().end(); it++)
                        {
                            if ((*it).second >= 1.0)
                            {
                                auto seam = m_plotterModel->findSeam(0);
                                m_errors.emplace_back(name, color, ds->offset().x() + (*it).first.x(), seam ? seam->visualNumber() : 0);
                            }
                        }
                    }
                }

                if (dataSetData.canConvert<std::vector<MulticolorSet*>>())
                {
                    const auto& dataSets = idx.data(Qt::UserRole + 2).value<std::vector<MulticolorSet*>>();

                    for (std::size_t j = 0u; j < dataSets.size(); j++)
                    {
                        if (auto ds = dataSets.at(j))
                        {
                            for (auto it = ds->samples().begin(); it != ds->samples().end(); it++)
                            {
                                if ((*it).second >= 1.0)
                                {
                                    auto seam = m_plotterModel->findSeam(j);
                                    m_errors.emplace_back(name, color, ds->offset().x() + (*it).first.x(), seam ? seam->visualNumber() : 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //sort by position
    std::sort(m_errors.begin(), m_errors.end(), [](const auto& a, const auto& b)
        {
            if (a.m_visualSeamNumber == b.m_visualSeamNumber)
            {
                return a.m_position < b.m_position;
            }
            return a.m_visualSeamNumber < b.m_visualSeamNumber;
        });

    endResetModel();
}

}
}

