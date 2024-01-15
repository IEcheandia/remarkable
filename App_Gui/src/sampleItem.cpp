#include "sampleItem.h"

#include <precitec/dataSet.h>
#include <QVariant>
#include <QPointF>
#include "event/sensor.h"

namespace precitec
{
namespace gui
{

SampleItem::SampleItem(QObject *parent)
    : QObject(parent)
    , m_idmSpectrum(new components::plotter::DataSet(this))
{
    m_idmSpectrum->setEnabled(true);
    m_idmSpectrum->setName(tr("IDM Spectrum"));
    m_idmSpectrum->setColor(m_spectrumColor);
    m_idmSpectrum->setDrawingMode(components::plotter::DataSet::DrawingMode::LineWithPoints);

    connect(this, &SampleItem::spectrumChanged, this, &SampleItem::updateSpectrumData, Qt::QueuedConnection);
    connect(this, &SampleItem::spectrumScaleChanged, this, &SampleItem::updateSpectrumData, Qt::QueuedConnection);
    connect(this, &SampleItem::spectrumColorChanged, this, &SampleItem::updateSpectrumColor, Qt::QueuedConnection);
}

SampleItem::~SampleItem() = default;

void SampleItem::setSampleData(const int& sensorId, const precitec::image::Sample& data)
{
    if (m_sensorId != sensorId)
    {
        m_sensorId = sensorId;
        emit sampleDataChanged();
    }

    if (sensorId == precitec::interface::Sensor::eIDMWeldingDepth)
    {
        m_weldingDepth = data[0];
        emit weldingDepthChanged();
    }

    if (m_sensorId == precitec::interface::Sensor::eIDMSpectrumLine)
    {
        m_spectrum = data;
        emit spectrumChanged();
    } else
    {
        emit spectrumRendered();
    }
}

void SampleItem::setSpectrumColor(const QColor& color)
{
    if (m_spectrumColor == color)
    {
        return;
    }

    m_spectrumColor = color;
    emit spectrumColorChanged();
}

void SampleItem::setSpectrumScale(const int& scale)
{
    if (m_spectrumScale == scale)
    {
        return;
    }

    m_spectrumScale = scale;
    emit spectrumScaleChanged();
}

void SampleItem::updateSpectrumData()
{
    const auto sampleCount = m_spectrum.getSize();
    if (sampleCount != 0)
    {
        std::vector<QVector2D> spectrumVector;
        spectrumVector.reserve(sampleCount);
        float scalingFactor = float (m_spectrumScale)/sampleCount;

        for (auto i = 0u; i < sampleCount; i++)
        {
            spectrumVector.emplace_back(i * scalingFactor, 10.0f * m_spectrum[i]);
        }

        m_idmSpectrum->clear();
        m_idmSpectrum->setProperty("maxElements", int(sampleCount));
        m_idmSpectrum->addSamples(spectrumVector);
    }

    emit spectrumRendered();
}

void SampleItem::updateSpectrumColor()
{
    m_idmSpectrum->setColor(m_spectrumColor);
}

}
}
