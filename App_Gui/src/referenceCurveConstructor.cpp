#include "referenceCurveConstructor.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "guiConfiguration.h"
#include "precitec/dataSet.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QDir>

#include <numeric>

using precitec::storage::Product;
using precitec::storage::ReferenceCurve;
using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace gui
{

namespace
{

template <class InputIt, class OutputIt, class UnaryOperation1, class UnaryOperation2>
OutputIt transform_if(InputIt first1, InputIt last1, OutputIt d_first, UnaryOperation1 unary_op1, UnaryOperation2 unary_op2)
{
    while (first1 != last1)
    {
        if (unary_op1(*first1))
        {
            *d_first++ = unary_op2(*first1);
        }
        ++first1;
    }
    return d_first;
}

}

ReferenceCurveConstructor::ReferenceCurveConstructor(QObject* parent)
    : InstanceResultModel(parent)
    , m_lower(new DataSet{this})
    , m_middle(new DataSet{this})
    , m_upper(new DataSet{this})
{
    const auto& oversamplingRate = GuiConfiguration::instance()->oversamplingRate();

    m_lower->setDrawingMode(DataSet::DrawingMode::Line);
    m_lower->setColor(Qt::magenta);
    m_lower->setName(QStringLiteral("Lower Boundary"));
    m_lower->setMaxElements(oversamplingRate * m_lower->maxElements());

    m_middle->setDrawingOrder(DataSet::DrawingOrder::OnTop);
    m_middle->setColor(Qt::darkMagenta);
    m_middle->setName(QStringLiteral("Reference Curve"));
    m_middle->setMaxElements(oversamplingRate * m_middle->maxElements());

    m_upper->setDrawingMode(DataSet::DrawingMode::Line);
    m_upper->setColor(Qt::magenta);
    m_upper->setName(QStringLiteral("Upper Boundary"));
    m_upper->setMaxElements(oversamplingRate * m_upper->maxElements());

    connect(this, &ReferenceCurveConstructor::jitterChanged, this, &ReferenceCurveConstructor::changeCurves);
    connect(this, &ReferenceCurveConstructor::referenceTypeChanged, this, &ReferenceCurveConstructor::changeCurves);
    connect(this, &ReferenceCurveConstructor::triggerTypeChanged, this, &ReferenceCurveConstructor::computeCurves);
    connect(this, &ReferenceCurveConstructor::thresholdChanged, this, &ReferenceCurveConstructor::computeCurves);
    connect(this, &ReferenceCurveConstructor::resultTypeChanged, this, &ReferenceCurveConstructor::computeCurves);
    connect(this, &ReferenceCurveConstructor::modelReset, this, &ReferenceCurveConstructor::computeCurves);
    connect(this, &ReferenceCurveConstructor::modelReset, this, &ReferenceCurveConstructor::updateReferences);
    connect(this, &ReferenceCurveConstructor::referenceCurveChanged, this, &ReferenceCurveConstructor::updateReferences);

    connect(this, &ReferenceCurveConstructor::dataChanged, this, [this] (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
    {
        Q_UNUSED(topLeft)
        Q_UNUSED(bottomRight)

        // selected role (Qt::UserRole + 5)
        if (roles.empty() || std::any_of(roles.begin(), roles.end(), [] (const int entry) { return entry == Qt::UserRole + 5; }))
        {
            changeCurves();
        }
    });
}

ReferenceCurveConstructor::~ReferenceCurveConstructor() = default;

void ReferenceCurveConstructor::setJitter(float jitter)
{
    if (m_jitter == jitter)
    {
        return;
    }
    m_jitter = jitter;
    emit jitterChanged();
}

void ReferenceCurveConstructor::setReferenceType(ReferenceCurve::ReferenceType type)
{
    if (m_referenceType == type)
    {
        return;
    }
    m_referenceType = type;
    emit referenceTypeChanged();
}

void ReferenceCurveConstructor::setCurrentProduct(Product* product)
{
    if (m_currentProduct == product)
    {
        return;
    }

    disconnect(m_productDestroyConnection);

    m_currentProduct = product;

    if (m_currentProduct)
    {
        m_productDestroyConnection = connect(m_currentProduct, &QObject::destroyed, this, std::bind(&ReferenceCurveConstructor::setCurrentProduct, this, nullptr));
    } else
    {
        m_productDestroyConnection = {};
    }

    emit currentProductChanged();
}

void ReferenceCurveConstructor::setReferenceCurve(ReferenceCurve* referenceCurve)
{
    if (m_referenceCurve == referenceCurve)
    {
        return;
    }

    if (m_referenceCurve)
    {
        disconnect(m_curveDestroyConnection);
    }

    m_referenceCurve = referenceCurve;

    if (m_referenceCurve)
    {
        m_curveDestroyConnection = connect(m_referenceCurve, &QObject::destroyed, this, std::bind(&ReferenceCurveConstructor::setReferenceCurve, this, nullptr));

        setJitter(m_referenceCurve->jitter());
        setReferenceType(m_referenceCurve->referenceType());

        // initial set of values from curve should not be considered a change
        resetChanges();
    } else
    {
        m_curveDestroyConnection = {};
    }

    emit referenceCurveChanged();
}

void ReferenceCurveConstructor::markAsChanged()
{
    if (m_hasChanges)
    {
        return;
    }
    m_hasChanges = true;
    emit hasChanged();
}

void ReferenceCurveConstructor::resetChanges()
{
    if (!m_hasChanges)
    {
        return;
    }
    m_hasChanges = false;
    emit hasChanged();
}

void ReferenceCurveConstructor::refreshUpdating()
{
    const auto updating = m_upperUpdating || m_middleUpdating || m_lowerUpdating;
    if (m_updating == updating)
    {
        return;
    }
    m_updating = updating;
    if (!m_updating && m_updatePending)
    {
        // repeat compute
        // don't send unnecessary signal, wait until compute is truly over
        // don't call changeCurves(), if the original call was to change, it is already marked so. If not, then it sould remain unmarked
        computeCurves();
    } else
    {
        emit updatingChanged();
    }
}

void ReferenceCurveConstructor::computeCurves()
{
    if (!m_referenceCurve)
    {
        return;
    }

    if (m_updating)
    {
        m_updatePending = true;
        return;
    }

    m_upperUpdating = true;
    m_middleUpdating = true;
    m_lowerUpdating = true;

    if (m_updatePending)
    {
        m_updating = true;
        m_updatePending = false;
    } else
    {
        refreshUpdating();
    }

    const auto& data = this->data();

    m_upper->clear();
    m_middle->clear();
    m_lower->clear();

    std::vector<Instance> results;

    transform_if(data.begin(), data.end(), std::back_inserter(results),
        [] (const auto& info)
        {
            return info.selected;
        },
        [](const auto& info)
        {
            return Instance{info.path, info.seamNumber};
        }
    );

    if (results.empty())
    {
        m_upperUpdating = false;
        m_middleUpdating = false;
        m_lowerUpdating = false;
        refreshUpdating();

        return;
    }

    if (m_referenceType == ReferenceCurve::ReferenceType::MinMax)
    {
        auto minMaxWatcher = new QFutureWatcher<void>{this};

        connect(minMaxWatcher, &QFutureWatcher<void>::finished, this,
        [this, minMaxWatcher]
            {
                minMaxWatcher->deleteLater();

                m_lowerUpdating = false;
                m_middleUpdating = false;
                m_upperUpdating = false;

                refreshUpdating();
            }
        );

        minMaxWatcher->setFuture(QtConcurrent::run(this, &ReferenceCurveConstructor::minMaxCurve, results));
    } else
    {
        auto lowerWatcher = new QFutureWatcher<void>{this};
        auto upperWatcher = new QFutureWatcher<void>{this};
        auto middleWatcher = new QFutureWatcher<void>{this};

        connect(middleWatcher, &QFutureWatcher<void>::finished, this,
        [this, middleWatcher, upperWatcher, lowerWatcher]
            {
                middleWatcher->deleteLater();

                m_middleUpdating = false;

                lowerWatcher->setFuture(QtConcurrent::run(this, &ReferenceCurveConstructor::lowerBound));
                upperWatcher->setFuture(QtConcurrent::run(this, &ReferenceCurveConstructor::upperBound));
            }
        );

        connect(upperWatcher, &QFutureWatcher<void>::finished, this,
        [this, upperWatcher]
            {
                upperWatcher->deleteLater();

                m_upperUpdating = false;

                refreshUpdating();
            }
        );

        connect(lowerWatcher, &QFutureWatcher<void>::finished, this,
        [this, lowerWatcher]
            {
                lowerWatcher->deleteLater();

                m_lowerUpdating = false;

                refreshUpdating();
            }
        );

        middleWatcher->setFuture(QtConcurrent::run(this, &ReferenceCurveConstructor::middleCurve, results, m_referenceType));
    }
}

void ReferenceCurveConstructor::middleCurve(std::vector<Instance> instances, ReferenceCurve::ReferenceType referenceType)
{
    m_middle->clear();

    const auto resultType = this->resultType();
    const auto triggerType = this->triggerType();
    const auto threshold = this->threshold();

    std::vector<float> x;
    std::vector<std::vector<float>> y_values;

    auto counter = 1;
    for (const auto& info : instances)
    {
        const auto directory = QDir{QStringLiteral("%1/seam_series%2/seam%3/").arg(info.path)
            .arg(seam()->seamSeries()->number(), 4, 10, QLatin1Char('0'))
            .arg(info.seamNumber, 4, 10, QLatin1Char('0'))};

        std::vector<QVector2D> samples;

        if (triggerType == -1)
        {
            samples = loadResult(directory, resultType);
        } else
        {
            samples = loadResultRange(directory, resultType, triggerType, threshold);
        }

        if (samples.empty())
        {
            continue;
        }

        if (samples.size() > x.size())
        {
            // acquire x values from the longest result
            x.clear();
            const auto& offset = samples.front().x();
            std::transform(samples.begin(), samples.end(), std::back_inserter(x), [offset] (const auto& sample) { return sample.x() - offset; });
        }

        y_values.reserve(samples.size());

        for (std::size_t i = 0; i < samples.size(); i++)
        {
            if (i >= y_values.size())
            {
                y_values.emplace_back(std::vector<float>{samples.at(i).y()});
            } else
            {
                y_values.at(i).emplace_back(samples.at(i).y());
            }
        }

        emit progressChanged(counter / static_cast<float> (instances.size()));
        counter++;
    }

    std::vector<QVector2D> middleCurve;
    middleCurve.reserve(x.size());

    switch (referenceType)
    {
        case ReferenceCurve::ReferenceType::Average:
        {
            computeAverage(x, y_values, middleCurve);
            break;
        }
        case ReferenceCurve::ReferenceType::Median:
        {
            computeMedian(x, y_values, middleCurve);
            break;
        }
        default:
            Q_UNREACHABLE();
    }

    m_middle->addSamples(middleCurve);

    x.clear();
    y_values.clear();
    middleCurve.clear();
}

void ReferenceCurveConstructor::computeAverage(const std::vector<float>& x_values, std::vector<std::vector<float>>& y_values, std::vector<QVector2D>& output)
{
    for (std::size_t i = 0; i < x_values.size(); i++)
    {
        auto& y = y_values.at(i);
        output.emplace_back(x_values.at(i), std::accumulate(y.begin(), y.end(), 0.0) / static_cast<float>(y.size()));
        y.clear();
    }
}

void ReferenceCurveConstructor::computeMedian(const std::vector<float>& x_values, std::vector<std::vector<float>>& y_values, std::vector<QVector2D>& output)
{
    for (std::size_t i = 0; i < x_values.size(); i++)
    {
        auto& y = y_values.at(i);

        std::sort(y.begin(), y.end());
        std::size_t middle = 0.5 * y.size();

        output.emplace_back(x_values.at(i), y.at(middle));
        y.clear();
    }
}

void ReferenceCurveConstructor::upperBound()
{
    const auto& curve = m_middle->samples();

    if (curve.empty())
    {
        return;
    }

    std::vector<QVector2D> upperBound;
    upperBound.reserve(m_middle->sampleCount());

    const auto firstPosition = curve.front().x();
    const auto lastPosition = m_middle->lastSamplePosition();

    for (const auto& sample : curve)
    {
        auto jitterIntervalMin = std::max(firstPosition, sample.x() - m_jitter);
        auto jitterIntervalMax = std::min(lastPosition, jitterIntervalMin + 2 * m_jitter);
        auto value = std::numeric_limits<float>::lowest();
        std::for_each(curve.begin(), curve.end(), [jitterIntervalMin, jitterIntervalMax, &value] (const auto& s) {
            if (s.x() >= jitterIntervalMin && s.x() <= jitterIntervalMax)
            {
                value = std::max(value, s.y());
            }
        });
        upperBound.emplace_back(sample.x(), value);
    }

    m_upper->addSamples(upperBound);
}

void ReferenceCurveConstructor::lowerBound()
{
    const auto& curve = m_middle->samples();

    if (curve.empty())
    {
        return;
    }

    std::vector<QVector2D> lowerBound;
    lowerBound.reserve(m_middle->sampleCount());

    const auto firstPosition = curve.front().x();
    const auto lastPosition = m_middle->lastSamplePosition();

    for (const auto& sample : curve)
    {
        auto jitterIntervalMin = std::max(firstPosition, sample.x() - m_jitter);
        auto jitterIntervalMax = std::min(lastPosition, jitterIntervalMin + 2 * m_jitter);
        auto value = std::numeric_limits<float>::max();
        std::for_each(curve.begin(), curve.end(), [jitterIntervalMin, jitterIntervalMax, &value] (const auto& s) {
            if (s.x() >= jitterIntervalMin && s.x() <= jitterIntervalMax)
            {
                value = std::min(value, s.y());
            }
        });
        lowerBound.emplace_back(sample.x(), value);
    }

    m_lower->addSamples(lowerBound);
}

void ReferenceCurveConstructor::minMaxCurve(std::vector<Instance> instances)
{
    m_lower->clear();
    m_middle->clear();
    m_upper->clear();

    std::vector<QVector2D> lowerBound;
    std::vector<QVector2D> middleCurve;
    std::vector<QVector2D> upperBound;

    const auto resultType = this->resultType();
    const auto triggerType = this->triggerType();
    const auto threshold = this->threshold();

    auto counter = 1;
    for (const auto& info : instances)
    {
        const auto directory = QDir{QStringLiteral("%1/seam_series%2/seam%3/").arg(info.path)
            .arg(seam()->seamSeries()->number(), 4, 10, QLatin1Char('0'))
            .arg(info.seamNumber, 4, 10, QLatin1Char('0'))};

        std::vector<QVector2D> samples;

        if (triggerType == -1)
        {
            samples = loadResult(directory, resultType);
        } else
        {
            samples = loadResultRange(directory, resultType, triggerType, threshold);
        }

        if (samples.empty())
        {
            continue;
        }

        const auto& offset = samples.front().x();
        std::for_each(samples.begin(), samples.end(), [offset] (auto& sample) { sample.setX(sample.x() - offset); });

        if (lowerBound.empty())
        {
            lowerBound = samples;
        } else
        {
            lowerBound.reserve(samples.size());

            for (std::size_t i = 0;  i < samples.size(); i++)
            {
                const auto& sample = samples.at(i);

                if (i < lowerBound.size())
                {
                    auto& current = lowerBound.at(i);
                    current.setY(std::min(current.y(), sample.y()));
                } else
                {
                    lowerBound.emplace_back(sample);
                }
            }
        }

        if (upperBound.empty())
        {
            upperBound = samples;
        } else
        {
            upperBound.reserve(samples.size());

            for (std::size_t i = 0;  i < samples.size(); i++)
            {
                const auto& sample = samples.at(i);

                if (i < upperBound.size())
                {
                    auto& current = upperBound.at(i);
                    current.setY(std::max(current.y(), sample.y()));
                } else
                {
                    upperBound.emplace_back(sample);
                }
            }
        }

        emit progressChanged(counter / static_cast<float> (instances.size()));
        counter++;
    }

    middleCurve.reserve(lowerBound.size());
    for (std::size_t i = 0;  i < lowerBound.size(); i++)
    {
        const auto& minElement = lowerBound.at(i);
        const auto& maxElement = upperBound.at(i);
        middleCurve.emplace_back(minElement + 0.5f * (maxElement - minElement));
    }

    m_lower->addSamples(lowerBound);
    m_middle->addSamples(middleCurve);
    m_upper->addSamples(upperBound);
}

void ReferenceCurveConstructor::changeCurves()
{
    computeCurves();
    markAsChanged();
}

void ReferenceCurveConstructor::save()
{
    if (!m_currentProduct || !m_referenceCurve)
    {
        return;
    }

    std::vector<QString> serialNumbers;

    const auto& data = this->data();
    transform_if(data.begin(), data.end(), std::back_inserter(serialNumbers),
        [] (const auto& info)
        {
            return info.selected;
        },
        [](const auto& info)
        {
            return info.serialNumber;
        }
    );

    m_referenceCurve->setSourceOfCurve(serialNumbers);
    m_referenceCurve->setReferenceType(m_referenceType);
    m_referenceCurve->setJitter(m_jitter);

    m_currentProduct->setReferenceCurveData(m_referenceCurve->lower(), m_lower->samples(), m_lower->sampleCount());
    m_currentProduct->setReferenceCurveData(m_referenceCurve->middle(), m_middle->samples(), m_lower->sampleCount());
    m_currentProduct->setReferenceCurveData(m_referenceCurve->upper(), m_upper->samples(), m_lower->sampleCount());

    resetChanges();
}

void ReferenceCurveConstructor::updateReferences()
{
    if (!m_referenceCurve || !m_referenceCurve->hasSource())
    {
        return;
    }

    auto& data = this->data();

    for (auto& info : data)
    {
        info.selected = m_referenceCurve->isSource(info.serialNumber);
    }

    loadLastMaxResults();

    emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole + 5, Qt::UserRole + 6});
}

}
}
