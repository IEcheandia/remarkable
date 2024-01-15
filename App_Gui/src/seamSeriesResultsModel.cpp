#include "seamSeriesResultsModel.h"
#include "resultsSeriesLoader.h"
#include "resultSetting.h"
#include "resultSettingModel.h"
#include "errorSettingModel.h"
#include "sensorSettingsModel.h"
#include "seam.h"
#include "product.h"
#include "resultHelper.h"
#include "event/results.h"

#include <multicolorSet.h>

#include <QCoreApplication>
#include <QVector2D>

using precitec::storage::ResultsSeriesLoader;
using precitec::storage::ResultSetting;
using precitec::interface::ResultArgs;

namespace precitec
{
namespace gui
{

static constexpr std::size_t s_maxElementsInPlotter{10000u};

SeamSeriesResultsModel::SeamSeriesResultsModel(QObject* parent)
    : AbstractMultiSeamDataModel(parent)
{
}

SeamSeriesResultsModel::~SeamSeriesResultsModel() = default;

void SeamSeriesResultsModel::setResultsSeriesLoader(ResultsSeriesLoader* loader)
{
    if (m_resultsSeriesLoader == loader)
    {
        return;
    }

    if (m_resultsSeriesLoader)
    {
        disconnect(m_resultsLoaderDestroyedConnection);
        disconnect(m_resultsSeriesLoader, &ResultsSeriesLoader::resultsLoaded, this, &SeamSeriesResultsModel::update);
    }

    m_resultsLoaderDestroyedConnection = QMetaObject::Connection{};
    m_resultsSeriesLoader = loader;

    if (m_resultsSeriesLoader)
    {
        m_resultsLoaderDestroyedConnection = connect(m_resultsSeriesLoader, &QObject::destroyed, this, std::bind(&SeamSeriesResultsModel::setResultsSeriesLoader, this, nullptr));
        connect(m_resultsSeriesLoader, &ResultsSeriesLoader::resultsLoaded, this, &SeamSeriesResultsModel::update);
    } else
    {
        m_resultsLoaderDestroyedConnection = QMetaObject::Connection{};
    }

    emit resultsSeriesLoaderChanged();
}

void SeamSeriesResultsModel::update()
{
    if (!m_resultsSeriesLoader)
    {
        return;
    }

    clear();

    const auto results{m_resultsSeriesLoader->takeResults()};

    if (!currentProduct())
    {
        return;
    }
    setLoading(true);

    for (const auto& resultSet : results)
    {
        const auto seamNumber = resultSet.first;
        const auto& resultData = resultSet.second;

        if (resultData.empty())
        {
            continue;
        }

        auto seam = currentProduct()->findSeam(m_resultsSeriesLoader->seamSeries(), seamNumber.number);

        const auto resultType = resultData.front().resultType();

        ResultSetting* resultConfig = nullptr;
        if (resultData.front().nioPercentage().empty())
        {
            resultConfig = resultsConfigModel() ? resultsConfigModel()->checkAndAddItem(resultType, nameForResult(resultType)) : nullptr;
        } else
        {
            resultConfig = errorConfigModel() ? errorConfigModel()->getItem(resultType) : nullptr;
        }

        auto seamIdx = seamIndex(seam);

        if (seamIdx == -1)
        {
            addSeam(seam);
            seamIdx = seamIndex(seam);
        }

        std::list<std::pair<QVector2D, float>> signalSamples;
        std::list<QVector2D> upperReferenceSamples;
        std::list<QVector2D> lowerReferenceSamples;
        bool skipNullValues = (resultConfig && resultConfig->visualization() == storage::ResultSetting::Visualization::Binary);

        for (const auto& result : resultData)
        {
            collectResults(signalSamples, upperReferenceSamples, lowerReferenceSamples, result, skipNullValues);
            // loading the results can be expensive, ensure the UI keeps updating
            QCoreApplication::instance()->processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        addResults(seamIdx, resultIndex(resultType), resultData.front(), resultConfig, signalSamples, upperReferenceSamples, lowerReferenceSamples);

        disableTooManyPoints(resultIndex(resultType), seamIdx);

        // loading the results can be expensive, ensure the UI keeps updating
        QCoreApplication::instance()->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    updateSeamLength();

    setCurrentIndex(0);
    setNumberOfSeamsInPlotter(maxIndex() + 1);

    emit maxIndexChanged();
    setLoading(false);
}

void SeamSeriesResultsModel::disableTooManyPoints(int resultIndex, int seamIndex)
{
    const auto& allResults = results();
    if (resultIndex >= int(allResults.size()))
    {
        return;
    }
    auto& data = allResults.at(resultIndex).m_data;

    if (seamIndex >= int(data.size()))
    {
        return;
    }
    if (auto dataSet = data.at(seamIndex).m_signal; dataSet && dataSet->sampleCount() > s_maxElementsInPlotter)
    {
        dataSet->setEnabled(false);
        internalDisable(resultIndex);
    }
}

void SeamSeriesResultsModel::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }
    m_loading = loading;
    emit loadingChanged();
}

}
}

