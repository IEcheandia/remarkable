#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace gui
{

/**
 * The WizardModel contains all elements which can be shown in the Wizard.
 * It is recommended to combine it with the WizardFilterModel to restrict on the actual available hardware at runtime.
 *
 * This model provides the following roles:
 * @li @c Qt::DisplayRole exposed as "display" (QString)
 * @li @c Qt::DecorationRole exposed as "icon" (QString)
 * @li @c Qt::UserRole + 1 exposed as "subitem" (bool)
 * @li @c Qt::UserRole + 2 exposed as "component" (WizardModel::WizardComponent)
 *
 * The WizardModel provides an enum @c WizardComponent which describe all the components available for the Wizard.
 **/
class WizardModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class WizardComponent {
        Axis,
        Camera,
        IDM,
        LWM,
        ScanLabScanner,
        ToolCenterPointOCT,
        IDMCalibration,
        ScanfieldCalibration,
        CameraCalibration,
        CameraChessboardCalibration,
        LEDCalibration,
        FocusPosition,
        ToolCenterPoint,
        LaserControl,
        LaserControlDelay,
        ScanTracker,
        FigureEditor,
        ProductStructure,
        ProductColorMaps,
        ProductHardwareParametersOverview,
        ProductDetectionOverview,
        ProductCamera,
        ProductError,
        ProductLaserControl,
        ProductScanTracker,
        ProductLaserWeldingMonitor,
        ProductScanLabScanner,
        ProductIDM,
        ProductZCollimator,
        ProductScanTracker2D,
        SeamSeriesAcquireScanField,
        SeamSeriesError,
        SeamSeriesLaserControl,
        SeamSeriesScanTracker,
        SeamSeriesLaserWeldingMonitor,
        SeamSeriesScanLabScanner,
        SeamSeriesIDM,
        SeamSeriesZCollimator,
        SeamSeriesScanTracker2D,
        SeamAssemblyImage,
        SeamCamera,
        SeamAxis,
        SeamDetection,
        SeamError,
        SeamReferenceCurves,
        SeamLaserControl,
        SeamScanTracker,
        SeamLaserWeldingMonitor,
        SeamScanLabScanner,
        SeamIDM,
        SeamZCollimator,
        SeamIntervalError,
        SeamScanTracker2D,
        ExternalLWM,
        SeamExternalLWM
    };
    Q_ENUM(WizardComponent)
    WizardModel(QObject *parent = nullptr);
    ~WizardModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    /**
     * @returns the index for the @p component
     **/
    Q_INVOKABLE QModelIndex indexForComponent(precitec::gui::WizardModel::WizardComponent component) const;

    QHash<int, QByteArray> roleNames() const override;

private:
    QString name(WizardComponent component) const;
    QString iconName(WizardComponent component) const;
    bool subItem(WizardComponent component) const;
    bool productItem(WizardComponent component) const;
    bool seamSeriesItem(WizardComponent component) const;
};

}
}
