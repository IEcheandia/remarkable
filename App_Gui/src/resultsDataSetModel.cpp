#include "resultsDataSetModel.h"
#include "resultHelper.h"
#include "resultsLoader.h"
#include "product.h"
#include "event/results.h"
#include "resultSetting.h"
#include "errorSettingModel.h"
#include "resultSettingModel.h"
#include <precitec/multicolorSet.h>

#include <QVector2D>

using precitec::storage::ResultSetting;
using precitec::storage::ResultsLoader;
using precitec::gui::components::plotter::MulticolorSet;

namespace precitec
{
namespace gui
{

ResultsDataSetModel::ResultsDataSetModel(QObject *parent)
    : AbstractSingleSeamDataModel(parent)
{
}

ResultsDataSetModel::~ResultsDataSetModel() = default;

void ResultsDataSetModel::setResultsLoader(ResultsLoader *loader)
{
    if (m_resultsLoader == loader)
    {
        return;
    }

    if (m_resultsLoader)
    {
        disconnect(m_resultsLoaderDestroyedConnection);
        disconnect(m_resultsLoader, &ResultsLoader::resultsLoaded, this, &ResultsDataSetModel::update);
    }

    m_resultsLoaderDestroyedConnection = QMetaObject::Connection{};
    m_resultsLoader = loader;

    if (m_resultsLoader)
    {
        m_resultsLoaderDestroyedConnection = connect(m_resultsLoader, &QObject::destroyed, this, std::bind(&ResultsDataSetModel::setResultsLoader, this, nullptr));
        connect(m_resultsLoader, &ResultsLoader::resultsLoaded, this, &ResultsDataSetModel::update);
    } else
    {
        m_resultsLoaderDestroyedConnection = QMetaObject::Connection{};
    }

    emit resultsLoaderChanged();
}

void ResultsDataSetModel::update()
{
    if (!m_resultsLoader)
    {
        return;
    }

    clear();

    const auto results{m_resultsLoader->takeResults()};

    if (currentProduct())
    {
        setSeam(currentProduct()->findSeam(m_resultsLoader->seamSeries(), m_resultsLoader->seam()));
    }

    for (const auto &resultData : results)
    {
        if (resultData.empty())
        {
            continue;
        }

        const auto resultType = resultData.front().resultType();

        ResultSetting* resultConfig = nullptr;
        if (resultData.front().nioPercentage().empty())
        {
            resultConfig = resultsConfigModel() ? resultsConfigModel()->checkAndAddItem(resultType, nameForResult(resultType)) : nullptr;
        } else
        {
            resultConfig = errorConfigModel() ? errorConfigModel()->getItem(resultType) : nullptr;
        }

        std::list<std::pair<QVector2D, float>> signalSamples;
        std::list<QVector2D> upperReferenceSamples;
        std::list<QVector2D> lowerReferenceSamples;
        bool skipNullValues = (resultConfig && resultConfig->visualization() == storage::ResultSetting::Visualization::Binary);

        for (const auto& result : resultData)
        {
            insertImagePosition(std::make_pair(result.context().imageNumber(), result.context().position() / 1000.0));
            collectResults(signalSamples, upperReferenceSamples, lowerReferenceSamples, result, skipNullValues);
        }

        addResults(resultIndex(resultType), resultData.front(), resultConfig, signalSamples, upperReferenceSamples, lowerReferenceSamples);
    }

    setCurrentIndex(0);
    resetImageNumber();

    emit lastPositionChanged();
}

QVariant ResultsDataSetModel::valueAtPosition(const QModelIndex& index, float position)
{
    const auto dataValue = index.data(Qt::UserRole + 2);
    if (!dataValue.isValid())
    {
        // invalid index, cannot return real value
        return {};
    }

    auto *dataSet = dataValue.value<MulticolorSet*>();
    const auto &samples = dataSet->samples();
    for (const auto &sample : samples)
    {
        if (qFuzzyCompare(sample.first.x(), position))
        {
            return sample.first.y();
        }
        if (sample.first.x() > position)
        {
            // we don't need to continue, we won't find it
            break;
        }
    }
    // value not found
    return {};
}

}
}
