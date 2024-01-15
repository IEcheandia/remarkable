#include <QString>

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

//Register cpp types for qml
#include "WobbleFigure.h"
#include "WobbleFigureEditor.h"
#include "FileModel.h"
#include "FigureCreator.h"
#include "AttributeController.h"
#include "FigureAnalyzer.h"
#include "PlotHandler.h"
#include "fileSortModel.h"
#include "fileSaveHandler.h"
#include "laserPowerController.h"
#include "selectionHandler.h"
#include "powerRampModel.h"
#include "figureEditorSettings.h"
#include "gridController.h"
#include "fileType.h"
#include "powerLimits.h"
#include "powerModulationMode.h"
#include "velocityLimits.h"
#include "wobbleFigureModel.h"
#include "abstractDataExchangeController.h"
#include "importSeamDataController.h"
#include "laserPointController.h"
#include "simulationController.h"
#include "wobbleFigureDataModel.h"
#include "plausibilityChecker.h"
#include "previewController.h"
#include "dxfImportController.h"
#include "rampModel.h"
#include "rampValidator.h"
#include "valueConverter.h"
#include "commandManager.h"
#include "requestChangesManager.h"

#include <QuickQanava.h>

class FigureEditorPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char* uri) override;
    void initializeEngine(QQmlEngine* engine, const char* uri) override;
};

void FigureEditorPlugin::registerTypes(const char* uri)
{
    //Register cpp types for qml
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::WobbleFigure>("wobbleFigureEditor.components", 1, 0, "WobbleFigure");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor>("wobbleFigureEditor.components", 1, 0, "WobbleFigureEditor");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::FileModel>("wobbleFigureEditor.components", 1, 0, "FileModel");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::FigureCreator>("wobbleFigureEditor.components", 1, 0, "FigureCreator");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::AttributeController>("wobbleFigureEditor.components", 1, 0, "AttributeController");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::FigureAnalyzer>("wobbleFigureEditor.components", 1, 0, "FigureAnalyzer");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::PlotHandler>("wobbleFigureEditor.components", 1, 0, "PlotHandler");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::FileSortModel>("wobbleFigureEditor.components", 1, 0, "FileSortModel");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::FileSaveHandler>("wobbleFigureEditor.components", 1, 0, "FileSaveHandler");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::LaserPowerController>("wobbleFigureEditor.components", 1, 0, "LaserPowerController");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::SelectionHandler>("wobbleFigureEditor.components", 1, 0, "SelectionHandler");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::PowerRampModel>("wobbleFigureEditor.components", 1, 0, "PowerRampModel");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::GridController>("wobbleFigureEditor.components", 1, 0, "GridController");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::WobbleFigureModel>("wobbleFigureEditor.components", 1, 0, "WobbleFigureModel");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::AbstractDataExchangeController>("wobbleFigureEditor.components", 1, 0, "AbstractDataExchangeController");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::ImportSeamDataController>("wobbleFigureEditor.components", 1, 0, "ImportSeamDataController");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::LaserPointController>("wobbleFigureEditor.components", 1, 0, "LaserPointController");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::SimulationController>("wobbleFigureEditor.components", 1, 0, "SimulationController");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::WobbleFigureDataModel>("wobbleFigureEditor.components", 1, 0, "WobbleFigureDataModel");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::CommandManager>("wobbleFigureEditor.components", 1, 0, "CommandManager");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::RequestChangesManager>("wobbleFigureEditor.components", 1, 0, "RequestChangesManager");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::PlausibilityChecker>("wobbleFigureEditor.components", 1, 0, "PlausibilityChecker");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::PreviewController>("wobbleFigureEditor.components", 1, 0, "PreviewController");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::DxfImportController>("wobbleFigureEditor.components", 1, 0, "DxfImportController");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::RampModel>("wobbleFigureEditor.components", 1, 0, "RampModel");
    qmlRegisterType<precitec::scanmaster::components::wobbleFigureEditor::RampValidator>("wobbleFigureEditor.components", 1, 0, "RampValidator");

    qmlRegisterSingletonType<precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings>(uri, 1, 0, "FigureEditorSettings",
                                                                                                         [](QQmlEngine*, QJSEngine*) -> QObject*
                                                                                                         {
                                                                                                             auto figureEditorSettings = precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings::instance();
                                                                                                             QQmlEngine::setObjectOwnership(figureEditorSettings, QQmlEngine::CppOwnership);
                                                                                                             return figureEditorSettings;
                                                                                                         });
    qmlRegisterSingletonType<precitec::scanmaster::components::wobbleFigureEditor::ValueConverter>(uri, 1, 0, "ValueConverter",
                                                                                                   [](QQmlEngine*, QJSEngine*) -> QObject*
                                                                                                   {
                                                                                                       auto valueConverter = precitec::scanmaster::components::wobbleFigureEditor::ValueConverter::instance();
                                                                                                       QQmlEngine::setObjectOwnership(valueConverter, QQmlEngine::CppOwnership);
                                                                                                       return valueConverter;
                                                                                                   });
    qmlRegisterSingletonType<precitec::scantracker::components::wobbleFigureEditor::ImportFileModelFactory>(uri, 1, 0, "ImportFileModelFactory",
                                                                                                            [](QQmlEngine*, QJSEngine*) -> QObject*
                                                                                                            {
                                                                                                                auto inst = precitec::scantracker::components::wobbleFigureEditor::ImportFileModelFactory::instance();
                                                                                                                QQmlEngine::setObjectOwnership(inst, QQmlEngine::CppOwnership);
                                                                                                                return inst;
                                                                                                            });

    qRegisterMetaType<precitec::scantracker::components::wobbleFigureEditor::FileType>();
    qmlRegisterUncreatableMetaObject(precitec::scantracker::components::wobbleFigureEditor::staticMetaObject, "wobbleFigureEditor.components", 1, 0, "FileType", QString());
    qRegisterMetaType<precitec::scanmaster::components::wobbleFigureEditor::PowerLimits>();
    qmlRegisterUncreatableMetaObject(precitec::scanmaster::components::wobbleFigureEditor::staticMetaObject, "wobbleFigureEditor.components", 1, 0, "PowerLimits", QString());
    qRegisterMetaType<precitec::scanmaster::components::wobbleFigureEditor::powerModulationMode::PowerModulationMode>();
    qmlRegisterUncreatableMetaObject(precitec::scanmaster::components::wobbleFigureEditor::powerModulationMode::staticMetaObject, "wobbleFigureEditor.components", 1, 0, "PowerModulationMode", QString());
    qRegisterMetaType<precitec::scanmaster::components::wobbleFigureEditor::velocityLimits::VelocityLimits>();
    qmlRegisterUncreatableMetaObject(precitec::scanmaster::components::wobbleFigureEditor::velocityLimits::staticMetaObject, "wobbleFigureEditor.components", 1, 0, "VelocityLimits", QString());
}

void FigureEditorPlugin::initializeEngine(QQmlEngine* engine, const char* uri)
{
    Q_UNUSED(uri)
    QuickQanava::initialize(engine);
}

#include "main_plugin.moc"
#include "moc_fileType.cpp"
#include "moc_powerLimits.cpp"
#include "moc_powerModulationMode.cpp"
#include "moc_velocityLimits.cpp"
