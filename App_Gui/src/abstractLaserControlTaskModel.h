#pragma once

#include "abstractLaserControlModel.h"

namespace precitec
{

namespace storage
{

class ParameterSet;
class LaserControlPreset;

}

namespace gui
{

class AbstractLaserControlTaskModel : public AbstractLaserControlModel
{
    Q_OBJECT

    Q_PROPERTY(bool presetEnabled READ presetEnabled WRITE setPresetEnabled NOTIFY presetEnabledChanged)

    Q_PROPERTY(bool delayEnabled READ delayEnabled WRITE setDelayEnabled NOTIFY delayEnabledChanged)

    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)

public:
    explicit AbstractLaserControlTaskModel(QObject *parent = nullptr);
    ~AbstractLaserControlTaskModel() override;

    enum class Key {
        LC_Parameter_No2,
        LC_Send_Data
    };
    Q_ENUM(Key)

    void setAttributeModel(precitec::storage::AttributeModel *model) override;

    void setCurrentPreset(const int index) override;

    bool presetEnabled() const
    {
        return m_presetEnabled;
    }
    void setPresetEnabled(bool enabled);

    bool delayEnabled() const
    {
        return m_delayEnabled;
    }
    void setDelayEnabled(bool enabled);

    int delay() const
    {
        return m_delay;
    }
    void setDelay(int delay);

Q_SIGNALS:
    void loadingFinished();
    void markAsChanged();
    void presetEnabledChanged();
    void delayEnabledChanged();
    void delayChanged();

protected:
    void load() override;
    virtual void updateHardwareParameters() = 0;
    virtual precitec::storage::ParameterSet* getParameterSet() const = 0;
    virtual void init() = 0;
    precitec::storage::Attribute* findAttribute(Key key) const;
    precitec::storage::Parameter* findParameter(precitec::storage::ParameterSet* ps, Key key) const;

    void setPresetEnabledDirect(bool enabled);
    void setDelayEnabledDirect(bool enabled);
    void setDelayDirect(int delay);
    void createCurrentPreset();
    void removeCurrentPreset();

private:
    bool m_presetEnabled = false;
    bool m_delayEnabled = false;
    int m_delay = 0;

    QMetaObject::Connection m_attributeModelReset;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::AbstractLaserControlTaskModel*)

