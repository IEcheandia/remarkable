#include <QQmlExtensionPlugin>
#include <QQmlEngine>
#include "languageSupport.h"
#include "fileSystemNameValidator.h"
#include "guiConfiguration.h"
#include "weldmasterPaths.h"
#include "removableDevicePaths.h"
#include "systemConfigurationQml.h"

class GeneralPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

void GeneralPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<precitec::gui::FileSystemNameValidator>(uri, 1, 0, "FileSystemNameValidator");
    qmlRegisterSingletonType<precitec::gui::LanguageSupport>(uri, 1, 0, "LanguageSupport",
        [](QQmlEngine *, QJSEngine *) -> QObject *
        {
            auto languageSupport = precitec::gui::LanguageSupport::instance();
            QQmlEngine::setObjectOwnership(languageSupport, QQmlEngine::CppOwnership);
            return languageSupport;
        });

    qmlRegisterSingletonType<precitec::gui::GuiConfiguration>(uri, 1, 0, "GuiConfiguration",
        [](QQmlEngine *, QJSEngine *) -> QObject *
        {
            auto guiConfiguration = precitec::gui::GuiConfiguration::instance();
            QQmlEngine::setObjectOwnership(guiConfiguration, QQmlEngine::CppOwnership);
            return guiConfiguration;
        });
    qmlRegisterSingletonType<precitec::gui::WeldmasterPaths>(uri, 1, 0, "WeldmasterPaths",
        [](QQmlEngine *, QJSEngine *) -> QObject *
        {
            auto paths = precitec::gui::WeldmasterPaths::instance();
            QQmlEngine::setObjectOwnership(paths, QQmlEngine::CppOwnership);
            return paths;
        });

    qmlRegisterSingletonType<precitec::gui::RemovableDevicePaths>(uri, 1, 0, "RemovableDevicePaths",
        [](QQmlEngine *, QJSEngine *) -> QObject *
        {
            auto paths = precitec::gui::RemovableDevicePaths::instance();
            QQmlEngine::setObjectOwnership(paths, QQmlEngine::CppOwnership);
            return paths;
        });

    qmlRegisterSingletonType<precitec::gui::SystemConfigurationQml>(uri, 1, 0, "SystemConfiguration",
        [](QQmlEngine *, QJSEngine *) -> QObject *
        {
            auto config = precitec::gui::SystemConfigurationQml::instance();
            QQmlEngine::setObjectOwnership(config, QQmlEngine::CppOwnership);
            return config;
        });
}

#include "main.moc"
