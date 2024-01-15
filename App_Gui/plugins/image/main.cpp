#include <QQmlExtensionPlugin>
#include "imageMeasurementController.h"
#include "imageItem.h"
#include "sourceImageItem.h"
#include "imageHistogramModel.h"
#include "intensityProfileModel.h"
#include "lineModel.h"
#include "lineLaserFilterModel.h"

class ImagePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri);
};

void ImagePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(qstrcmp(uri, "precitec.gui.components.image") == 0);
    qmlRegisterType<precitec::gui::components::image::ImageItem>(uri, 1, 0, "ImageItem");
    qmlRegisterType<precitec::gui::components::image::SourceImageItem>(uri, 1, 0, "SourceImageItem");
    qmlRegisterType<precitec::gui::ImageMeasurementController>(uri, 1, 0, "ImageMeasurementController");
    qmlRegisterType<precitec::gui::LineModel>(uri, 1, 0, "LineModel");
    qmlRegisterType<precitec::gui::LineLaserFilterModel>(uri, 1, 0, "LineLaserFilterModel");
    qRegisterMetaType<precitec::CalibrationCoordinatesRequestProxy>();
    qmlRegisterType<precitec::gui::components::image::IntensityProfileModel>(uri,1,0,"IntensityProfileModel");
    qmlRegisterType<precitec::gui::components::image::ImageHistogramModel>(uri,1,0,"ImageHistogramModel");
}

#include "main.moc"
