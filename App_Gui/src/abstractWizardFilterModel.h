#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * The AbstractWizardFilterModel is the abstract base intended to filter the WizardModel::WizardComponent depending on the available hardware.
 **/
class AbstractWizardFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * Whether the yAxis is available, removes WizardComponent::Axis and WizardComponent::SeamAxis if @c false.
     **/
    Q_PROPERTY(bool yAxisAvailable READ isYAxisAvailable WRITE setYAxisAvailable NOTIFY yAxisAvailableChanged)
    /**
     * Whether the system has a sensor grabber (e.g. framegrabber). Removes WizardComponent::Camera, WizardComponent::ToolCenterPoint and WizardComponent::SeamCamera
     **/
    Q_PROPERTY(bool sensorGrabberAvailable READ isSensorGrabberAvailable WRITE setSensorGrabberAvailable NOTIFY sensorGrabberAvailableChanged)
    /**
     * Whether the system has a idm.
     * If @c false, removes WizardComponent::IDM
     * If @link{coaxCameraAvailable} is also @c false, it removes WizardComponent::ScanfieldCalibration.
     * If @link{newsonScannerAvailable} is also @c false, it removes WizardComponent::IDMCalibration, WizardComponent::ToolCenterPointOCT.
     **/
    Q_PROPERTY(bool idmAvailable READ isIDMAvailable WRITE setIDMAvailable NOTIFY idmAvailableChanged)
    /**
     * Whether the system has a coax camera. If together with @link{ledCameraAvailable} are both @c false, removes WizardComponent::CameraCalibration
     **/
    Q_PROPERTY(bool coaxCameraAvailable READ isCoaxCameraAvailable WRITE setCoaxCameraAvailable NOTIFY coaxCameraAvailableChanged)
    /**
     * Whether the system has a Scheimpflug camera.
     **/
    Q_PROPERTY(bool scheimpflugCameraAvailable READ isScheimpflugCameraAvailable WRITE setScheimpflugCameraAvailable NOTIFY scheimpflugCameraAvailableChanged)
    /**
     * Whether the system has a LED camera. If together with @link{coaxCameraAvailable} are both @c false, removes WizardComponent::CameraCalibration
     **/
    Q_PROPERTY(bool ledCameraAvailable READ isLEDCameraAvailable WRITE setLEDCameraAvailable NOTIFY ledCameraAvailableChanged)
    /**
     * Whether the system has a LED panel. Removes WizardComponent::LEDCalibration
     **/
    Q_PROPERTY(bool ledAvailable READ isLEDAvailable WRITE setLEDAvailable NOTIFY ledAvailableChanged)
    /**
     * Whether the system has a Laser Control. Removes WizardComponent::SeamLaserControl
     **/
    Q_PROPERTY(bool laserControlAvailable READ isLaserControlAvailable WRITE setLaserControlAvailable NOTIFY laserControlAvailableChanged)
    /**
     * Whether the system provides WizardComponent::LEDCalibration
     **/
    Q_PROPERTY(bool ledCalibration READ ledCalibration WRITE setLEDCalibration NOTIFY ledCalibrationChanged)
    /**
     * Whether the system provides a scan tracker. If @c false removes WizardComponent::ScanTracker
     **/
    Q_PROPERTY(bool scanTracker READ scanTracker WRITE setScanTracker NOTIFY scanTrackerChanged)
    /**
     * Whether the system provides a z collimator. If @c false removes WizardComponent::FocusPosition
     **/
    Q_PROPERTY(bool zCollimator READ zCollimator WRITE setZCollimator NOTIFY zCollimatorChanged)
    /**
     * Whether the system provides a LWM. If @c false removes WizardComponent::LaserWeldingMonitor
     **/
    Q_PROPERTY(bool lwmAvailable READ isLWMAvailable WRITE setLWMAvailable NOTIFY lwmAvailableChanged)
    /**
     * Whether the system provides a external LWM
     **/
    Q_PROPERTY(bool externalLwmAvailable READ isExternalLWMAvailable WRITE setExternalLWMAvailable NOTIFY externalLwmAvailableChanged)
    /**
     * Whether the system provides the first LineLaser. If all LineLaser are @c false removes WizardComponent::CameraCalibration
     **/
    Q_PROPERTY(bool lineLaser1Available READ isLineLaser1Available WRITE setLineLaser1Available NOTIFY lineLaser1AvailableChanged)
    /**
     * Whether the system provides the second LineLaser. If all LineLaser are @c false removes WizardComponent::CameraCalibration
     **/
    Q_PROPERTY(bool lineLaser2Available READ isLineLaser2Available WRITE setLineLaser2Available NOTIFY lineLaser2AvailableChanged)
    /**
     * Whether the system provides the third LineLaser. If all LineLaser are @c false removes WizardComponent::CameraCalibration
     **/
    Q_PROPERTY(bool lineLaser3Available READ isLineLaser3Available WRITE setLineLaser3Available NOTIFY lineLaser3AvailableChanged)
    /**
     * Whether the system has a Newson Scanner.
     * If @c false, removes WizardComponent::IDMCalibration, WizardComponent::ToolCenterPointOCT.
     **/
    Q_PROPERTY(bool newsonScannerAvailable READ isNewsonScannerAvailable WRITE setNewsonScannerAvailable NOTIFY newsonScannerAvailableChanged)
    /**
     * Whether the system has a Scanlab Scanner.
     * If @c false, removes WizardComponent::ScanfieldCalibration, WizardComponent::SeamScanLabScanner
     **/
    Q_PROPERTY(bool scanlabScannerAvailable READ isScanlabScannerAvailable WRITE setScanlabScannerAvailable NOTIFY scanlabScannerAvailableChanged)
    /**
     * Whether the Scanlab Scanner system is a Scanmaster or a ScanTracker2D.
     * If @c true and @link scanlabScannerAvailable is @c true the ScanTracker2D components are added and the ScanfieldCalibration and Seam/SeamSeries/Product ScanLabScanner pages are removed
     **/
    Q_PROPERTY(bool scanTracker2DAvailable READ isScanTracker2DAvailable WRITE setScanTracker2DAvailable NOTIFY scanTracker2DAvailableChanged)
    /**
     * @li FrameGrabber
     * @li GigE
     **/
    Q_PROPERTY(precitec::gui::AbstractWizardFilterModel::CameraInterfaceType cameraInterfaceType READ cameraInterfaceType WRITE setCameraInterfaceType NOTIFY cameraInterfaceTypeChanged)
    /**
     * Whether the GUI should show elements needed for the production setup (hidden during normal use,
     * like the chessboard calibration page which writes to the camera memory)
     **/
    Q_PROPERTY(bool showProductionSetup READ showProductionSetup WRITE setShowProductionSetup NOTIFY showProductionSetupChanged)
public:
    ~AbstractWizardFilterModel() override;

    bool isYAxisAvailable() const
    {
        return m_yAxisAvailable;
    }
    void setYAxisAvailable(bool set);

    bool isSensorGrabberAvailable() const
    {
        return m_sensorGrabberAvailable;
    }
    void setSensorGrabberAvailable(bool set);

    bool isIDMAvailable() const
    {
        return m_idmAvailable;
    }
    void setIDMAvailable(bool set);

    bool isCoaxCameraAvailable() const
    {
        return m_coaxCameraAvailable;
    }
    void setCoaxCameraAvailable(bool set);

    bool isScheimpflugCameraAvailable() const
    {
        return m_scheimpflugCameraAvailable;
    }
    void setScheimpflugCameraAvailable(bool set);

    bool isLEDCameraAvailable() const
    {
        return m_ledCameraAvailable;
    }
    void setLEDCameraAvailable(bool set);

    bool isLEDAvailable() const
    {
        return m_ledAvailable;
    }
    void setLEDAvailable(bool set);

    bool isLaserControlAvailable() const
    {
        return m_laserControlAvailable;
    }
    void setLaserControlAvailable(bool set);

    bool ledCalibration() const
    {
        return m_ledCalibration;
    }
    void setLEDCalibration(bool set);

    bool scanTracker() const
    {
        return m_scanTracker;
    }
    void setScanTracker(bool set);

    bool zCollimator() const
    {
        return m_zCollimator;
    }
    void setZCollimator(bool set);

    bool isLWMAvailable() const
    {
        return m_lwmAvailable;
    }
    void setLWMAvailable(bool set);

    bool isExternalLWMAvailable() const
    {
        return m_externalLwmAvailable;
    }
    void setExternalLWMAvailable(bool set);

    bool isLineLaser1Available() const
    {
        return m_lineLaser1Available;
    }
    void setLineLaser1Available(bool set);

    bool isLineLaser2Available() const
    {
        return m_lineLaser2Available;
    }
    void setLineLaser2Available(bool set);

    bool isLineLaser3Available() const
    {
        return m_lineLaser3Available;
    }
    void setLineLaser3Available(bool set);

    bool isLineLaserAvailable() const
    {
        return m_lineLaser1Available || m_lineLaser2Available || m_lineLaser3Available;
    }

    bool isNewsonScannerAvailable() const
    {
        return m_newsonScannerAvailable;
    }
    void setNewsonScannerAvailable(bool set);

    bool isScanlabScannerAvailable() const
    {
        return m_scanlabScannerAvailable;
    }
    void setScanlabScannerAvailable(bool set);

    bool isScanTracker2DAvailable() const
    {
        return m_scanTracker2DAvailable;
    }
    void setScanTracker2DAvailable(bool set);

    enum class CameraInterfaceType
    {
        FrameGrabber,
        GigE
    };
    Q_ENUM(CameraInterfaceType)
    CameraInterfaceType cameraInterfaceType() const
    {
        return m_cameraInterfaceType;
    }
    void setCameraInterfaceType(CameraInterfaceType type);

    bool showProductionSetup() const
    {
        return m_showProductionSetup;
    }
    void setShowProductionSetup(bool set);

Q_SIGNALS:
    void yAxisAvailableChanged();
    void sensorGrabberAvailableChanged();
    void idmAvailableChanged();
    void coaxCameraAvailableChanged();
    void scheimpflugCameraAvailableChanged();
    void ledCameraAvailableChanged();
    void ledAvailableChanged();
    void laserControlAvailableChanged();
    void ledCalibrationChanged();
    void scanTrackerChanged();
    void zCollimatorChanged();
    void lwmAvailableChanged();
    void externalLwmAvailableChanged();
    void lineLaser1AvailableChanged();
    void lineLaser2AvailableChanged();
    void lineLaser3AvailableChanged();
    void newsonScannerAvailableChanged();
    void scanlabScannerAvailableChanged();
    void cameraInterfaceTypeChanged();
    void scanTracker2DAvailableChanged();
    void showProductionSetupChanged();

protected:
    AbstractWizardFilterModel(QObject *parent = nullptr);

private:
    bool m_yAxisAvailable = false;
    bool m_sensorGrabberAvailable = false;
    bool m_idmAvailable = false;
    bool m_coaxCameraAvailable = false;
    bool m_scheimpflugCameraAvailable = false;
    bool m_ledCameraAvailable = false;
    bool m_ledAvailable = false;
    bool m_laserControlAvailable = false;
    bool m_ledCalibration = false;
    bool m_scanTracker = false;
    bool m_zCollimator = false;
    bool m_lwmAvailable = false;
    bool m_externalLwmAvailable = false;
    bool m_lineLaser1Available = false;
    bool m_lineLaser2Available = false;
    bool m_lineLaser3Available = false;
    bool m_newsonScannerAvailable = false;
    bool m_scanlabScannerAvailable = false;
    CameraInterfaceType m_cameraInterfaceType = CameraInterfaceType::FrameGrabber;
    bool m_scanTracker2DAvailable = false;
    bool m_showProductionSetup = false;
};

}
}
