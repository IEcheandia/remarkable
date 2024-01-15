#pragma once

#include <QObject>

namespace precitec
{
namespace gui
{

class LEDChannel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

    Q_PROPERTY(int minCurrent READ minCurrent WRITE setMinCurrent NOTIFY minCurrentChanged)

    Q_PROPERTY(int maxCurrent READ maxCurrent WRITE setMaxCurrent NOTIFY maxCurrentChanged)

    Q_PROPERTY(int referenceBrightness READ referenceBrightness WRITE setReferenceBrightness NOTIFY referenceBrightnessChanged)

    Q_PROPERTY(int measuredBrightness READ measuredBrightness WRITE setMeasuredBrightness NOTIFY measuredBrightnessChanged)

    Q_PROPERTY(int currentValue READ currentValue WRITE setCurrentValue NOTIFY currentValueChanged)

    Q_PROPERTY(int originalValue READ originalValue WRITE setOriginalValue NOTIFY originalValueChanged)

    Q_PROPERTY(int intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)

public:
    explicit LEDChannel(QObject* parent = nullptr);
    ~LEDChannel() override;

    bool enabled() const
    {
        return m_enabled;
    }
    void setEnabled(bool enabled);

    bool visible() const
    {
        return m_visible;
    }
    void setVisible(bool visible);

    int minCurrent() const
    {
        return m_minCurrent;
    }
    void setMinCurrent(int min);

    int maxCurrent() const
    {
        return m_maxCurrent;
    }
    void setMaxCurrent(int max);

    int referenceBrightness() const
    {
        return m_referenceBrightness;
    }
    void setReferenceBrightness(int referenceBrightness);

    int measuredBrightness() const
    {
        return m_measuredBrightness;
    }
    void setMeasuredBrightness(int measuredBrightness);

    int currentValue() const
    {
        return m_currentValue;
    }
    void setCurrentValue(int currentValue);

    int originalValue() const
    {
        return m_originalValue;
    }
    void setOriginalValue(int originalValue);

    int intensity() const
    {
        return m_intensity;
    }
    void setIntensity(int intensity);

    int normalizedIntensity(int currentValue);

    void setIntensityToValue();

Q_SIGNALS:
    void enabledChanged();
    void visibleChanged();
    void minCurrentChanged();
    void maxCurrentChanged();
    void referenceBrightnessChanged();
    void measuredBrightnessChanged();
    void currentValueChanged();
    void offset2Changed();
    void offset3Changed();
    void offset4Changed();
    void originalValueChanged();
    void intensityChanged();

private:
    bool m_enabled = false;
    bool m_visible = true;
    int m_minCurrent = 0;
    int m_maxCurrent = 1500;
    int m_referenceBrightness = 40;
    int m_measuredBrightness = 40;
    int m_currentValue = 500;
    int m_originalValue = 750;
    int m_intensity = 0;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::LEDChannel*)


