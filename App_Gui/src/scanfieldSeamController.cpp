#include "scanfieldSeamController.h"
#include "scanfieldModule.h"
#include "seamSeries.h"
#include "seam.h"
#include "hardwareParametersModule.h"
#include "hardwareParameters.h"
#include "parameterSet.h"
#include "parameter.h"

#include <QRectF>

using precitec::storage::Seam;

namespace precitec
{
namespace gui
{

ScanfieldSeamController::ScanfieldSeamController(QObject* parent)
    : QObject(parent)
    , m_scanfieldModule(new ScanfieldModule{this})
    , m_hardwareParametersModule(new HardwareParametersModule{this})
{
    connect(this, &ScanfieldSeamController::seamChanged, this, &ScanfieldSeamController::cameraCenterChanged);
    connect(this, &ScanfieldSeamController::seamChanged, this, &ScanfieldSeamController::roiChanged);
    connect(this, &ScanfieldSeamController::roiChanged, this, &ScanfieldSeamController::paintedRoiChanged);
    connect(this, &ScanfieldSeamController::transformationChanged, this, &ScanfieldSeamController::paintedRoiChanged);

    connect(m_scanfieldModule, &ScanfieldModule::cameraSizeChanged, this, &ScanfieldSeamController::cameraCenterChanged);
    connect(m_scanfieldModule, &ScanfieldModule::imageSizeChanged, this, &ScanfieldSeamController::cameraCenterChanged);
    connect(m_scanfieldModule, &ScanfieldModule::calibrationCoordinatesRequestProxyChanged, this, &ScanfieldSeamController::cameraCenterChanged);
    connect(m_scanfieldModule, &ScanfieldModule::calibrationCoordinatesRequestProxyChanged, this, &ScanfieldSeamController::roiChanged);
    connect(m_scanfieldModule, &ScanfieldModule::configurationValidChanged, this, &ScanfieldSeamController::cameraCenterChanged);
    connect(m_scanfieldModule, &ScanfieldModule::configurationValidChanged, this, &ScanfieldSeamController::roiChanged);

    connect(m_hardwareParametersModule, &HardwareParametersModule::attributeModelChanged, this, &ScanfieldSeamController::cameraCenterChanged);
}

ScanfieldSeamController::~ScanfieldSeamController() = default;

void ScanfieldSeamController::setSeam(Seam* seam)
{
    if (m_seam == seam)
    {
        return;
    }

    if (m_seam)
    {
        disconnect(m_seamDestroyed);
        disconnect(m_seam, &Seam::roiChanged, this, &ScanfieldSeamController::roiChanged);
        disconnect(m_seam, &Seam::hardwareParametersChanged, this, &ScanfieldSeamController::cameraCenterChanged);
    }

    m_seam = seam;

    if (m_seam)
    {
        m_seamDestroyed = connect(m_seam, &Seam::destroyed, this, std::bind(&ScanfieldSeamController::setSeam, this, nullptr));
        connect(m_seam, &Seam::roiChanged, this, &ScanfieldSeamController::roiChanged);
        connect(m_seam, &Seam::hardwareParametersChanged, this, &ScanfieldSeamController::cameraCenterChanged);
        m_scanfieldModule->setSeries(seam->seamSeries() ? seam->seamSeries()->uuid() : QUuid{});
    } else
    {
        m_seamDestroyed = {};
        m_scanfieldModule->setSeries({});
    }

    emit seamChanged();
}

void ScanfieldSeamController::setTransformation(const QMatrix4x4& matrix)
{
    if (m_transformation == matrix)
    {
        return;
    }
    m_transformation = matrix;
    emit transformationChanged();
}

QPointF ScanfieldSeamController::cameraCenter(Seam* seam) const
{
    if (seam)
    {
        if (auto ps = seam->hardwareParameters())
        {
            auto cameraX = m_hardwareParametersModule->findParameter(ps, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewXPosition).uuid);
            auto cameraY = m_hardwareParametersModule->findParameter(ps, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewYPosition).uuid);

            if (cameraX && cameraY)
            {
                return m_scanfieldModule->scannerToImageCoordinates(cameraX->value().toDouble(), cameraY->value().toDouble());
            }
        }
    }
    return {-1, -1};
}

QPointF ScanfieldSeamController::cameraCenter() const
{
    if (!m_seam)
    {
        return {-1, -1};
    }

    const auto& center = cameraCenter(m_seam);
    if (m_scanfieldModule->centerValid(center))
    {
        return center;
    }

    if (!m_seam->seamSeries())
    {
        return {-1, -1};
    }

    const auto& seams = m_seam->seamSeries()->seams();

    auto rit = std::find(seams.rbegin(), seams.rend(), m_seam);

    if (rit == seams.rend())
    {
        return {-1, -1};
    }

    auto valid_it = std::find_if(rit, seams.rend(), [this] (auto s) { return m_scanfieldModule->centerValid(cameraCenter(s)); });

    if (valid_it == seams.rend())
    {
        return {-1, -1};
    }

    return cameraCenter(*valid_it);
}

bool ScanfieldSeamController::cameraCenterValid() const
{
   return m_scanfieldModule->centerValid(cameraCenter());
}

QRectF ScanfieldSeamController::roi() const
{
    if (!m_seam || m_seam->roi().isNull())
    {
        return {};
    }
    //the seam parameters contain the roi for the camera image (not mirrored), we need to recompute the roi so that it will be correct in the scanfield image
    return m_scanfieldModule->mirrorRect(m_seam->roi());
}

QRectF ScanfieldSeamController::cameraRect() const
{
    return m_scanfieldModule->cameraRect(cameraCenter());
}

QRectF ScanfieldSeamController::paintedRoi() const
{
    if (m_transformation.isIdentity())
    {
        return {};
    }
    return m_transformation.mapRect(roi().translated(cameraRect().topLeft()));
}

void ScanfieldSeamController::setPaintedRoi(const QRectF& roi)
{
    if (!m_seam)
    {
        return;
    }

    const auto& intersection = QRectF{QPointF{0, 0}, m_scanfieldModule->cameraSize()}.intersected(roi.normalized());

    //for the seam parameters, we need to recompute the roi so that it will be correct in the image coming from the camera (not mirrored)
    m_seam->setRoi(m_scanfieldModule->mirrorRect(intersection).toRect());
}

void ScanfieldSeamController::resetRoi()
{
    if (!m_seam)
    {
        return;
    }

    m_seam->setRoi({});
}

}
}
