#pragma once

#include "abstractDataExchangeController.h"

class ImportSeamDataControllerTest;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

/**
 * Controller to import the necessary information from the products or seams.
 * The goal is to avoid setting the velocity a lot of times. (In the product and in the figure editor)
 **/
class ImportSeamDataController : public AbstractDataExchangeController
{
    Q_OBJECT
    /**
     * Velocity
     * The imported velocity from the selected seam.
     **/
    Q_PROPERTY(double velocity READ velocity NOTIFY velocityChanged)

public:
    explicit ImportSeamDataController(QObject* parent = nullptr);
    ~ImportSeamDataController() override;

    double velocity() const
    {
        return m_velocity;
    }

    Q_INVOKABLE void importData();

Q_SIGNALS:
    void velocityChanged();

private:
    void reset();
    double umPerSecondToMMPerSecond(double velocityInUmPerSeconds);
    void importVelocityFromSeam();

    double m_velocity{0.0};

    friend ImportSeamDataControllerTest;
};

}
}
}
}

