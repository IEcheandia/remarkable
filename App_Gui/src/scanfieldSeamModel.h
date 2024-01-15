#pragma once

#include <QAbstractListModel>
#include <QMatrix4x4>

class ScanfieldSeamModelTest;

namespace precitec
{
namespace storage
{
class SeamSeries;
class Seam;
}
namespace gui
{

class ScanfieldModule;
class HardwareParametersModule;

/**
 * @brief Model which displays the camera positions of all @link{Seams}s of a @link{SeamSeries} within a scanfield image.
 *
 * This is a small model which provides the spatial information for the camera positions
 * @link{Seams}s of a @link{SeamSeries}. It provides the functionality
 * to display and edit this position, as well as showing the region, which the camera will be able to display.
 **/

class ScanfieldSeamModel : public QAbstractListModel
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
     * Current Seam Series, for whose Seams the model provides information
     **/
    Q_PROPERTY(precitec::storage::SeamSeries* seamSeries READ seamSeries WRITE setSeamSeries NOTIFY seamSeriesChanged)

    /**
     * The currently selected Seam
     **/
    Q_PROPERTY(precitec::storage::Seam* seam READ seam NOTIFY seamChanged)

    /**
     * The last Seam in the Series with a valid camera center, before the currently selected Seam
     **/
    Q_PROPERTY(precitec::storage::Seam* lastValidSeam READ lastValidSeam NOTIFY currentCenterChanged)

    /**
     * The transformation matrix of the scanfield image
     * Provided by the image instance itself
     **/
    Q_PROPERTY(QMatrix4x4 transformation READ transformation WRITE setTransformation NOTIFY transformationChanged)

    /**
     * The camera center of the current @link{seam} in pixel
     **/
    Q_PROPERTY(QPointF currentCenter READ currentCenter NOTIFY currentCenterChanged)

    /**
     * Whether the current @link{seam} uses drive with OCT Reference arm or drive to position
     **/
    Q_PROPERTY(bool currentDriveWithOCTReference READ currentDriveWithOCTReference NOTIFY currentDriveWithOCTReferenceChanged)

    /**
     * The camera center of the @link{lastValidSeam} in pixel
     **/
    Q_PROPERTY(QPointF lastCenter READ lastCenter NOTIFY currentCenterChanged)

    /**
     * The camera center of the current @link{seam} in pixel with the @link{transformation} applied
     **/
    Q_PROPERTY(QPointF currentPaintedCenter READ currentPaintedCenter NOTIFY currentCenterChanged)

    /**
     * Is the value of the camera center of the current @link{seam} valid
     **/
    Q_PROPERTY(bool currentCenterValid READ currentCenterValid NOTIFY currentCenterChanged)

    /**
     * Is the value of the camera center of the @link{lastValidSeam} valid, i.e. is there such a seam
     **/
    Q_PROPERTY(bool lastCenterValid READ lastCenterValid NOTIFY currentCenterChanged)

    /**
     * The camera center (in mm) of the current @link{seam}
     **/
    Q_PROPERTY(QPointF currentCenterInMM READ currentCenterInMM NOTIFY currentCenterChanged)

    /**
     * The camera center (in mm) of the @link{lastValidSeam}
     **/
    Q_PROPERTY(QPointF lastCenterInMM READ lastCenterInMM NOTIFY currentCenterChanged)

    /**
     * The region, observed by the camera at the position of the current @link{seam}
     **/
    Q_PROPERTY(QRectF currentRect READ currentRect NOTIFY currentCenterChanged)

    /**
     * Can the camera be moved further left in the image
     **/
    Q_PROPERTY(bool leftEnabled READ leftEnabled NOTIFY currentCenterChanged)

    /**
     * Can the camera be moved further right in the image
     **/
    Q_PROPERTY(bool rightEnabled READ rightEnabled NOTIFY currentCenterChanged)

    /**
     * Can the camera be moved further upwards in the image
     **/
    Q_PROPERTY(bool topEnabled READ topEnabled NOTIFY currentCenterChanged)

    /**
     * Can the camera be moved further downwards in the image
     **/
    Q_PROPERTY(bool bottomEnabled READ bottomEnabled NOTIFY currentCenterChanged)

    /**
     * Toggle if the camera position of all valid seams or only the current seam is shown in the scanfield image
     **/
    Q_PROPERTY(bool showAllSeams READ showAllSeams WRITE setShowAllSeams NOTIFY showAllSeamsChanged)

    /**
     * System uses OCT with reference arms
     **/
    Q_PROPERTY(bool octWithReferenceArms READ octWithReferenceArms WRITE setOctWithReferenceArms NOTIFY octWithReferenceArmsChanged)

public:
    explicit ScanfieldSeamModel(QObject* parent = nullptr);
    ~ScanfieldSeamModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    ScanfieldModule* scanfieldModule() const
    {
        return m_scanfieldModule;
    }

    HardwareParametersModule* hardwareParametersModule() const
    {
        return m_hardwareParametersModule;
    }

    precitec::storage::SeamSeries* seamSeries() const
    {
        return m_seamSeries;
    }
    void setSeamSeries(precitec::storage::SeamSeries* seamSeries);

    precitec::storage::Seam* seam() const
    {
        return m_seam;
    }

    precitec::storage::Seam* lastValidSeam() const;

    QMatrix4x4 transformation() const
    {
        return m_transformation;
    }
    void setTransformation(const QMatrix4x4& matrix);

    bool showAllSeams() const
    {
        return m_showAllSeams;
    }
    void setShowAllSeams(bool showAllSeams);

    QPointF currentCenter() const;
    QPointF lastCenter() const;
    QPointF currentCenterInMM() const;
    QPointF lastCenterInMM() const;
    QPointF currentPaintedCenter() const;
    bool currentCenterValid() const;
    bool lastCenterValid() const;
    QRectF currentRect() const;
    bool currentDriveWithOCTReference() const;

    bool leftEnabled() const;
    bool rightEnabled() const;
    bool topEnabled() const;
    bool bottomEnabled() const;

    bool octWithReferenceArms() const
    {
        return m_octWithReferenceArms;
    }
    void setOctWithReferenceArms(bool set);

    Q_INVOKABLE QPointF cameraCenter(precitec::storage::Seam* seam) const;
    Q_INVOKABLE void selectSeam(const QUuid& id);
    Q_INVOKABLE QVariantList selectSeams(const QPointF& point);
    Q_INVOKABLE bool pointInCurrentSeam(const QPointF& point) const;
    Q_INVOKABLE void setCameraCenter(const QPointF& point);
    Q_INVOKABLE void setCameraCenterToSeam(int index);
    Q_INVOKABLE void resetCameraCenter();
    Q_INVOKABLE void setDriveWithOCT(bool set);
    Q_INVOKABLE QPointF imageCoordinatesToPaintedPoint(const QPointF& point) const;
    Q_INVOKABLE QPointF imageToScannerCoordinates(const QPointF& point) const;
    Q_INVOKABLE QPointF scannerToImageCoordinates(const QPointF& point) const;

Q_SIGNALS:
    void seamSeriesChanged();
    void transformationChanged();
    void seamChanged();
    void currentCenterChanged();
    void showAllSeamsChanged();
    void currentDriveWithOCTReferenceChanged();
    void octWithReferenceArmsChanged();

private:
    void setSeam(precitec::storage::Seam* seam);
    void updateModel();
    bool pointInSeam(precitec::storage::Seam* seam, const QPointF& point) const;
    precitec::storage::Seam* lastValidCameraCenter(precitec::storage::Seam* seam) const;

    bool m_showAllSeams = true;
    bool m_octWithReferenceArms{false};
    QMatrix4x4 m_transformation;

    std::vector<precitec::storage::Seam*> m_seams;

    precitec::storage::SeamSeries* m_seamSeries = nullptr;
    QMetaObject::Connection m_seamSeriesDestroyed;
    precitec::storage::Seam* m_seam = nullptr;
    QMetaObject::Connection m_seamDestroyed;

    ScanfieldModule* m_scanfieldModule;
    HardwareParametersModule* m_hardwareParametersModule;

    friend ScanfieldSeamModelTest;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ScanfieldSeamModel*)

