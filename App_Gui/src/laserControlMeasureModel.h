#pragma once

#include "abstractLaserControlTaskModel.h"

class LaserControlMeasureModelTest;

namespace precitec
{

namespace storage
{

class ParameterSet;
class AbstractMeasureTask;
class LaserControlPreset;

}

namespace gui
{

class LaserControlMeasureModel : public AbstractLaserControlTaskModel
{
    Q_OBJECT

    /**
     * The Seam for which the HardwareParameter values should be taken
     **/
    Q_PROPERTY(precitec::storage::AbstractMeasureTask *measureTask READ measureTask WRITE setMeasureTask NOTIFY measureTaskChanged)

public:
    explicit LaserControlMeasureModel(QObject *parent = nullptr);
    ~LaserControlMeasureModel() override;

    precitec::storage::AbstractMeasureTask *measureTask() const
    {
        return m_measureTask;
    }
    void setMeasureTask(precitec::storage::AbstractMeasureTask *measureTask);

Q_SIGNALS:
    void measureTaskChanged();

protected:
    void updateHardwareParameters() override;
    precitec::storage::ParameterSet *getParameterSet() const override;
    void init() override;

private:
    precitec::storage::AbstractMeasureTask *m_measureTask = nullptr;
    QMetaObject::Connection m_measureTaskDestroyed;

    friend LaserControlMeasureModelTest;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LaserControlMeasureModel*)
