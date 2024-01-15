#include "ledChannel.h"

namespace precitec
{
namespace gui
{

LEDChannel::LEDChannel(QObject *parent)
    : QObject(parent)
{
}

LEDChannel::~LEDChannel() = default;

void LEDChannel::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
    {
        return;
    }
    m_enabled = enabled;
    emit enabledChanged();
}

void LEDChannel::setVisible(bool visible)
{
    if (m_visible == visible)
    {
        return;
    }
    m_visible = visible;
    emit visibleChanged();
}


void LEDChannel::setMinCurrent(int min)
{
    if (m_minCurrent == min)
    {
        return;
    }
    m_minCurrent = min;
    emit minCurrentChanged();
}

void LEDChannel::setMaxCurrent(int max)
{
    if (m_maxCurrent == max)
    {
        return;
    }
    m_maxCurrent = max;
    emit maxCurrentChanged();
}

void LEDChannel::setReferenceBrightness(int referenceBrightness)
{
    if (m_referenceBrightness == referenceBrightness)
    {
        return;
    }
    m_referenceBrightness = referenceBrightness;
    emit referenceBrightnessChanged();
}

void LEDChannel::setMeasuredBrightness(int measuredBrightness)
{
    if (m_measuredBrightness == measuredBrightness)
    {
        return;
    }
    m_measuredBrightness = measuredBrightness;
    emit measuredBrightnessChanged();
}

void LEDChannel::setCurrentValue(int currentValue)
{
    if (m_currentValue == currentValue)
    {
        return;
    }
    m_currentValue = currentValue;
    emit currentValueChanged();
}

void LEDChannel::setOriginalValue(int originalValue)
{
    if (m_originalValue == originalValue)
    {
        return;
    }
    m_originalValue = originalValue;
    emit originalValueChanged();
}

void LEDChannel::setIntensity(int intensity)
{
    if (m_intensity == intensity)
    {
        return;
    }
    m_intensity = intensity;
    emit intensityChanged();
}

int LEDChannel::normalizedIntensity(int currentValue)
{
    return int(100 * float(currentValue) / m_maxCurrent);
}

void LEDChannel::setIntensityToValue()
{
    setCurrentValue(int(float(m_maxCurrent) * m_intensity / 100.0f));
}

}
}



