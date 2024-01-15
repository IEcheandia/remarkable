#include "resultsStatisticsAbstractErrorModel.h"
#include "resultsStatisticsController.h"
#include "errorSettingModel.h"

#include <QColor>

namespace precitec
{
namespace storage
{

ResultsStatisticsAbstractErrorModel::ResultsStatisticsAbstractErrorModel(QObject* parent)
: QAbstractListModel(parent)
{
    connect(this, &ResultsStatisticsAbstractErrorModel::resultsStatisticsControllerChanged, this, &ResultsStatisticsAbstractErrorModel::updateModel);

    connect(this, &ResultsStatisticsAbstractErrorModel::errorSettingModelChanged, this, [this] {
        emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole, Qt::UserRole + 3});
    });

    connect(this, &ResultsStatisticsAbstractErrorModel::modelReset, this, &ResultsStatisticsAbstractErrorModel::emptyChanged);
}

ResultsStatisticsAbstractErrorModel::~ResultsStatisticsAbstractErrorModel() = default;

QVariant ResultsStatisticsAbstractErrorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= int(m_nios.size()))
    {
        return {};
    }

    auto it = m_nios.begin();
    std::advance(it, index.row());

    auto errorSettings = m_errorSettingModel ? m_errorSettingModel->getItem(it->first) : nullptr;

    switch(role)
    {
        case Qt::DisplayRole:
            return errorSettings ? errorSettings->name() : QLatin1String("");
        case Qt::UserRole:
            return it->second;
        case Qt::UserRole + 1:
            return m_nioCount != 0 ? it->second / double(m_nioCount) : 0.0;
        case Qt::UserRole + 2:
            return errorSettings ? errorSettings->lineColor() : QColor{};
    }
    return {};
}

int ResultsStatisticsAbstractErrorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_nios.size();
}

QHash<int, QByteArray> ResultsStatisticsAbstractErrorModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("count")},
        {Qt::UserRole + 1, QByteArrayLiteral("countInPercent")},
        {Qt::UserRole + 2, QByteArrayLiteral("color")}
    };
}

void ResultsStatisticsAbstractErrorModel::setResultsStatisticsController(ResultsStatisticsController* controller)
{
    if (m_resultsStatisticsController == controller)
    {
        return;
    }

    if (m_resultsStatisticsController)
    {
        disconnect(m_resultsStatisticsControllerDestroyed);
        disconnect(m_resultsStatisticsController, &ResultsStatisticsController::update, this, &ResultsStatisticsAbstractErrorModel::updateModel);
    }

    m_resultsStatisticsController = controller;

    if (m_resultsStatisticsController)
    {
        m_resultsStatisticsControllerDestroyed = connect(m_resultsStatisticsController, &ResultsStatisticsController::destroyed, this, std::bind(&ResultsStatisticsAbstractErrorModel::setResultsStatisticsController, this, nullptr));
        connect(m_resultsStatisticsController, &ResultsStatisticsController::update, this, &ResultsStatisticsAbstractErrorModel::updateModel);
    } else
    {
        m_resultsStatisticsControllerDestroyed = {};
    }

    emit resultsStatisticsControllerChanged();
}

void ResultsStatisticsAbstractErrorModel::setErrorSettingModel(ErrorSettingModel* errorSettingModel)
{
    if (m_errorSettingModel == errorSettingModel)
    {
        return;
    }

    disconnect(m_errorSettingModelDestroyed);

    m_errorSettingModel = errorSettingModel;

    if (m_errorSettingModel)
    {
        m_errorSettingModelDestroyed = connect(m_errorSettingModel, &ErrorSettingModel::destroyed, this, std::bind(&ResultsStatisticsAbstractErrorModel::setErrorSettingModel, this, nullptr));
    } else
    {
        m_resultsStatisticsControllerDestroyed = {};
    }

    emit errorSettingModelChanged();
}

void ResultsStatisticsAbstractErrorModel::clear()
{
    // use only within begin-/endResetModel()

    m_nios.clear();
    m_nioCount = 0;
}

void ResultsStatisticsAbstractErrorModel::setNios(const std::map<precitec::interface::ResultType, unsigned int>& nios)
{
    // use only within begin-/endResetModel()

    m_nios = nios;

    m_nioCount = 0;
    for (const auto& nio : m_nios)
    {
        m_nioCount += nio.second;
    }
}

}
}

