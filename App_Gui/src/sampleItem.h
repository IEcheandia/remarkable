#pragma once

#include <QObject>
#include <QColor>

#include "image/ipSignal.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace plotter
{
class DataSet;
}
}

class SampleItem : public QObject
{
    Q_OBJECT

    /**
     * Holds the IDM spectrum signal values.
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet* idmSpectrum READ idmSpectrum CONSTANT)

    /**
     * Spectrum color
     **/
    Q_PROPERTY(QColor spectrumColor READ spectrumColor WRITE setSpectrumColor NOTIFY spectrumColorChanged)

    /**
     * Spectrum scale
     **/
    Q_PROPERTY(int spectrumScale READ spectrumScale WRITE setSpectrumScale NOTIFY spectrumScaleChanged)

    Q_PROPERTY(qreal weldingDepth READ weldingDepth NOTIFY weldingDepthChanged)

public:
    explicit SampleItem(QObject* parent = nullptr);
    ~SampleItem() override;

    precitec::gui::components::plotter::DataSet* idmSpectrum() const
    {
        return m_idmSpectrum;
    }

    QColor spectrumColor() const
    {
        return m_spectrumColor;
    }
    void setSpectrumColor(const QColor& color);

    int spectrumScale() const
    {
        return m_spectrumScale;
    }
    void setSpectrumScale(const int& scale);

    qreal weldingDepth() const
    {
        return m_weldingDepth;
    }

    Q_INVOKABLE void setSampleData(const int& sensorId, const precitec::image::Sample& data);

Q_SIGNALS:
    void sampleDataChanged();
    void spectrumChanged();
    void spectrumColorChanged();
    void spectrumScaleChanged();
    void spectrumRendered();
    void weldingDepthChanged();

private:
    void updateSpectrumData();
    void updateSpectrumColor();

    int m_sensorId;
    image::Sample m_spectrum;
    QColor m_spectrumColor;
    int m_spectrumScale;
    qreal m_weldingDepth{0.0};

    precitec::gui::components::plotter::DataSet* m_idmSpectrum = nullptr;
};
}
}

Q_DECLARE_METATYPE(precitec::image::Sample)
