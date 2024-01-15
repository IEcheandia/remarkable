#pragma once

#include <QObject>
#include <QMatrix4x4>

namespace precitec
{
namespace storage
{
class Seam;
}
namespace gui
{

class ScanfieldModule;
class HardwareParametersModule;

/**
 * @brief Controller used for setting the @link{Seams} roi within a scanfield image.
 **/

class ScanfieldSeamController : public QObject
{
    Q_OBJECT

    /**
     * Module, responsible for the scanfield management
     **/
    Q_PROPERTY(precitec::gui::ScanfieldModule* scanfieldModule READ scanfieldModule CONSTANT)

    /**
     * Module, responsible for the hardware parameters management
     **/
    Q_PROPERTY(precitec::gui::HardwareParametersModule* hardwareParametersModule READ hardwareParametersModule CONSTANT)

    /**
     * The currently selected Seam
     **/
    Q_PROPERTY(precitec::storage::Seam* seam READ seam WRITE setSeam NOTIFY seamChanged)

    /**
     * The transformation matrix of the scanfield image
     * Provided by the image instance itself
     **/
    Q_PROPERTY(QMatrix4x4 transformation READ transformation WRITE setTransformation NOTIFY transformationChanged)

    /**
     * The camera center of the current @link{seam} in pixel
     **/
    Q_PROPERTY(QPointF cameraCenter READ cameraCenter NOTIFY cameraCenterChanged)

    /**
     * Is the value of the camera center of the current @link{seam} valid
     **/
    Q_PROPERTY(bool cameraCenterValid READ cameraCenterValid NOTIFY cameraCenterChanged)

    /**
     * The roi of the current @link{seam} in pixel
     **/
    Q_PROPERTY(QRectF roi READ roi NOTIFY roiChanged)

    /**
     * The roi of the current @link{seam} in pixel with the @link(transformation) applied
     **/
    Q_PROPERTY(QRectF paintedRoi READ paintedRoi WRITE setPaintedRoi NOTIFY paintedRoiChanged)

public:
    explicit ScanfieldSeamController(QObject* parent = nullptr);
    ~ScanfieldSeamController() override;

    ScanfieldModule* scanfieldModule() const
    {
        return m_scanfieldModule;
    }

    HardwareParametersModule* hardwareParametersModule() const
    {
        return m_hardwareParametersModule;
    }

    precitec::storage::Seam* seam() const
    {
        return m_seam;
    }
    void setSeam(precitec::storage::Seam* seam);

    QMatrix4x4 transformation() const
    {
        return m_transformation;
    }
    void setTransformation(const QMatrix4x4& matrix);

    QPointF cameraCenter() const;

    bool cameraCenterValid() const;

    QRectF roi() const;

    QRectF paintedRoi() const;
    void setPaintedRoi(const QRectF& roi);

    Q_INVOKABLE void resetRoi();

Q_SIGNALS:
    void seamChanged();
    void cameraCenterChanged();
    void transformationChanged();
    void roiChanged();
    void paintedRoiChanged();

private:
    QPointF cameraCenter(precitec::storage::Seam* seam) const;
    QRectF cameraRect() const;

    QMatrix4x4 m_transformation;

    precitec::storage::Seam* m_seam = nullptr;
    QMetaObject::Connection m_seamDestroyed;

    ScanfieldModule* m_scanfieldModule;
    HardwareParametersModule* m_hardwareParametersModule;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ScanfieldSeamController*)


