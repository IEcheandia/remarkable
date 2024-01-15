#include "abstractSingleSeamDataModel.h"
#include "resultSetting.h"
#include "resultSettingModel.h"
#include "errorSettingModel.h"
#include "event/results.h"
#include "resultHelper.h"
#include "precitec/multicolorSet.h"
#include "precitec/dataSet.h"
#include "seam.h"

using precitec::storage::Seam;
using precitec::storage::ResultSetting;
using precitec::storage::ResultSettingModel;
using precitec::storage::ErrorSettingModel;
using precitec::interface::ResultArgs;
using precitec::gui::components::plotter::MulticolorSet;
using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace gui
{

AbstractSingleSeamDataModel::AbstractSingleSeamDataModel(QObject* parent)
    : AbstractPlotterDataModel(parent)
{
    setCurrentIndex(0);
}

AbstractSingleSeamDataModel::~AbstractSingleSeamDataModel() = default;

QVariant AbstractSingleSeamDataModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto& resultsInfo = resultAt(index.row());
    const auto& data = resultsInfo.m_data;

    switch (role)
    {
        case Qt::DisplayRole:
            return resultsInfo.m_resultName;
        case Qt::UserRole:
        {
            if (!data.empty())
            {
                return data.front().m_signal->maxX();
            }
            return {};
        }
        case Qt::UserRole + 1:
        {
            if (!data.empty() && !data.front().m_signal->isEmpty())
            {
                return data.front().m_signal->lastSampleValue();
            }
            return {};
        }
        case Qt::UserRole + 2:
        {
            if (!data.empty())
            {
                return QVariant::fromValue(data.front().m_signal);
            }
            return {};
        }
        case Qt::UserRole + 3:
            return resultsInfo.m_plotterNumber;
        case Qt::UserRole + 4:
            return resultsInfo.m_isBinary;
        case Qt::UserRole + 5:
            return resultsInfo.m_resultType;
        case Qt::UserRole + 6:
            return resultsInfo.m_isVisible;
        case Qt::UserRole + 7:
            return resultsInfo.m_color;
        case Qt::UserRole + 8:
            return resultsInfo.m_isEnabled;
        case Qt::UserRole + 9:
            return {};
        case Qt::UserRole + 10:
            return resultsInfo.m_isNioPercentageSet;
        case Qt::UserRole + 11:
        {
            if (!data.empty())
            {
                std::vector<DataSet*> boundarySets;
                boundarySets.reserve(2);

                if (auto top = data.front().m_top)
                {
                    boundarySets.push_back(top);
                }
                if (auto bottom = data.front().m_bottom)
                {
                    boundarySets.push_back(bottom);
                }
                return QVariant::fromValue(boundarySets);
            }
            return {};
        }
    }

    return {};
}

void AbstractSingleSeamDataModel::insertNewResult(const ResultArgs& result, ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples)
{
    AbstractPlotterDataModel::insertNewResult(result, resultConfig, 0, signalSamples, upperReferenceSamples, lowerReferenceSamples, 1);
}

void AbstractSingleSeamDataModel::insertNewSensor(int sensorId, ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples)
{
    AbstractPlotterDataModel::insertNewSensor(sensorId, resultConfig, 0, signalSamples, 1);
}

void AbstractSingleSeamDataModel::addResults(int resultIndex, const ResultArgs& result, ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples)
{
    AbstractPlotterDataModel::addResults(0, resultIndex, result, resultConfig, signalSamples, upperReferenceSamples, lowerReferenceSamples, 1);
}

void AbstractSingleSeamDataModel::addSignalSamples(int resultIndex, ResultSetting* resultConfig, int sensorId, const std::list<std::pair<QVector2D, float>>& signalSamples)
{
    AbstractPlotterDataModel::addSignalSamples(resultIndex, 0, resultConfig, sensorId, signalSamples);
}

void AbstractSingleSeamDataModel::clear()
{
    beginResetModel();

    AbstractPlotterDataModel::clear();

    m_imageNumberToPosition.clear();
    m_selectedImageNumber = m_imageNumberToPosition.end();
    m_selectedImagePosition = 0;

    endResetModel();

    emit lastPositionChanged();
}

quint32 AbstractSingleSeamDataModel::selectedImageNumber() const
{
    if (m_selectedImageNumber == m_imageNumberToPosition.end())
    {
        return 0;
    }
    return m_selectedImageNumber->first;
}

bool AbstractSingleSeamDataModel::foundPosition(qreal position, const ImageNumberToPosition::const_iterator& it, const ImageNumberToPosition::const_iterator& last)
{
    if (it == last || it->second > position)
    {
        return false;
    }
    auto next = std::next(it);
    return (next == last || next->second > position);
}

qreal AbstractSingleSeamDataModel::selectedImagePosition() const
{
    if (m_selectedImageNumber == m_imageNumberToPosition.end())
    {
        return 0.0;
    }
    return m_selectedImagePosition;
}

quint32 AbstractSingleSeamDataModel::lastImageNumber() const
{
    if (m_imageNumberToPosition.empty())
    {
        return 0;
    }

    return m_imageNumberToPosition.rbegin()->first;
}

quint32 AbstractSingleSeamDataModel::firstImageNumber() const
{
    if (m_imageNumberToPosition.empty())
    {
        return 0;
    }
    return m_imageNumberToPosition.begin()->first;
}

void AbstractSingleSeamDataModel::selectImageNumber(quint32 hint)
{
    auto it = m_imageNumberToPosition.find(hint);
    if (it == m_selectedImageNumber)
    {
        return;
    }
    if (it == m_imageNumberToPosition.end())
    {
        auto upperBound = m_imageNumberToPosition.upper_bound(hint);
        auto lowerBound = m_imageNumberToPosition.lower_bound(hint);
        if (lowerBound != m_imageNumberToPosition.end())
        {
            lowerBound--;
        }
        if (upperBound == m_imageNumberToPosition.end() && lowerBound == m_imageNumberToPosition.end())
        {
            return;
        }
        if (upperBound == m_imageNumberToPosition.end())
        {
            it = lowerBound;
        }
        else if (lowerBound == m_imageNumberToPosition.end())
        {
            it = upperBound;
        }
        else
        {
            if (selectedImageNumber() >= hint)
            {
                it = lowerBound;
            }
            else
            {
                it = upperBound;
            }
        }
    }
    if (it != m_imageNumberToPosition.end() && it != m_selectedImageNumber)
    {
        m_selectedImageNumber = it;
        m_selectedImagePosition = m_selectedImageNumber->second;
        emit selectedImageNumberChanged();
        emit selectedImagePositionChanged();
    }
}

void AbstractSingleSeamDataModel::setSeam(Seam* seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }
    m_currentSeam = seam;
    emit currentSeamChanged();
}

void AbstractSingleSeamDataModel::insertImagePosition(std::pair<qint32, qreal> position)
{
    m_imageNumberToPosition.insert(position);
}

void AbstractSingleSeamDataModel::resetImageNumber()
{
    m_selectedImageNumber = m_imageNumberToPosition.begin();
    m_selectedImagePosition = m_selectedImageNumber!= m_imageNumberToPosition.end() ? m_selectedImageNumber->second : 0;

    emit selectedImageNumberChanged();
    emit selectedImagePositionChanged();
}

void AbstractSingleSeamDataModel::selectImagePosition(qreal hint)
{
    if (foundPosition(hint, m_selectedImageNumber, m_imageNumberToPosition.end()))
    {
        if (m_selectedImagePosition != hint)
        {
            m_selectedImagePosition = hint;
            emit selectedImagePositionChanged();
        }
        return;
    }

    for (auto it = m_imageNumberToPosition.cbegin(), end = m_imageNumberToPosition.cend() ; it != end; it++)
    {
        if (foundPosition(hint, it, end))
        {
            m_selectedImageNumber = it;
            m_selectedImagePosition = hint;
            emit selectedImageNumberChanged();
            emit selectedImagePositionChanged();
            return;
        }
    }
    //position not found, select the first image
    resetImageNumber();
}

}
}
