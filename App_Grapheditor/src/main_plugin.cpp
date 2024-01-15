// stl includes
#include <string>

#include <QDebug>
#include <QString>

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

//Own Stuff
#include "GraphFilterModel.h"
#include "FilterSortModel.h"
#include "GraphModelVisualizer.h"
#include "GraphLoader.h"
#include "FilterGraph.h"
#include "FilterGraphImageSaver.h"
#include "GraphEditor.h"
#include "FilterConnectorSortModel.h"
#include "FilterNodeModel.h"
#include "FilterNodeSortModel.h"
#include "FilterPortModel.h"
#include "FilterPortSortModel.h"
#include "FilterPortPartnerModel.h"
#include "NodeModel.h"
#include "NodeSortModel.h"
#include "FilterGroupModel.h"
#include "FilterGraphFilterModel.h"
#include "DnDConnector.h"
#include "InvalidNodeModel.h"
#include "ResultSettingFilterModel.h"
#include "CopyPasteHandler.h"
#include "DirectoryLoader.h"
#include "DirectoryModel.h"
#include "invalidIDModel.h"
#include "pipeController.h"
#include "selectedFilterNodesFilterModel.h"
#include "groupController.h"
#include "macroController.h"
#include "filterMacro.h"
#include "plausibilityController.h"
#include "filterAttributeGroupModel.h"
#include "ExistingFileValidator.h"

//Stuff from Mod_Storage (integrated in CMakeLists.xml through include_directories(../Mod_Storage/src) and target_link_libraries(App_Grapheditor Mod_Storage)
#include "graphModel.h"
#include "graphSortModel.h"
#include "subGraphModel.h"
#include "graphExporter.h"
#include "fliplib/Fliplib.h"
#include "filterInstanceModel.h"
#include "attribute.h"
#include "attributeModel.h"
#include "filterAttributeModel.h"
#include "filterAttributeSortFilterModel.h"
#include "parameter.h"
#include "resultSettingModel.h"
#include "resultSettingFilterModel.h"
#include "sensorSettingsModel.h"
#include "attributeGroup.h"
#include "attributeGroupItem.h"

#include <QuickQanava.h>

Q_DECLARE_METATYPE(fliplib::GraphContainer*);

class GraphEditorPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
    void initializeEngine ( QQmlEngine * engine, const char * uri ) override;
};

void GraphEditorPlugin::registerTypes ( const char* uri )
{
    Q_UNUSED(uri)
    qRegisterMetaType<fliplib::GraphContainer*>();
    qRegisterMetaType<precitec::gui::components::grapheditor::FilterGroup*>();
    qRegisterMetaType<precitec::storage::AttributeGroup*>();
    qRegisterMetaType<precitec::storage::AttributeGroupItem*>();
    qRegisterMetaType<std::vector<precitec::storage::AttributeGroupItem*>>();
    qRegisterMetaType<precitec::gui::components::grapheditor::FilterMacro*>();
    qRegisterMetaType<precitec::gui::components::grapheditor::GroupController*>();
    qRegisterMetaType<precitec::storage::Attribute*>(); // Q_INVOKABLE mit dem Rückgabewert Attribute* funktioniert jetzt
    //qRegisterMetaType<precitec::storage::Parameter*>();
    qmlRegisterUncreatableType<precitec::storage::Parameter>("Precitec.AppGui", 1, 0, "Parameter", QString());

    qmlRegisterType<precitec::gui::components::grapheditor::GraphFilterModel>("grapheditor.components", 1, 0, "GraphFilterModel");      //Object can be created in the qml file
    qmlRegisterType<precitec::gui::components::grapheditor::FilterSortModel>("grapheditor.components", 1, 0, "FilterSortModel");
    qmlRegisterType<precitec::storage::SubGraphModel>("grapheditor.components", 1, 0, "SubGraphModel");
    qmlRegisterType<precitec::storage::GraphModel>("grapheditor.components", 1, 0, "GraphModel");
    qmlRegisterType<precitec::storage::GraphSortModel>("grapheditor.components", 1, 0, "GraphSortModel");
    qmlRegisterType<precitec::gui::components::grapheditor::GraphLoader>("grapheditor.components", 1, 0, "GraphLoader");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterGraph>("grapheditor.components", 1, 0, "FilterGraph");
    qmlRegisterType<precitec::gui::components::grapheditor::GraphModelVisualizer>("grapheditor.components", 1, 0, "GraphModelVisualizer");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterConnector>("grapheditor.components", 1, 0, "FilterPort");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterComment>("grapheditor.components", 1, 0, "FilterComment");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterGraphImageSaver>("grapheditor.components", 1, 0, "FilterGraphImageSaver");
    qmlRegisterType<precitec::gui::components::grapheditor::GraphEditor>("grapheditor.components", 1, 0, "GraphEditor");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterConnectorSortModel>("grapheditor.components", 1, 0, "FilterConnectorSortModel");
    qmlRegisterType<precitec::storage::FilterInstanceModel>("grapheditor.components", 1, 0, "FilterInstanceModel");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterNodeModel>("grapheditor.components", 1, 0, "FilterNodeModel");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterNodeSortModel>("grapheditor.components", 1, 0, "FilterNodeSortModel");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterPortModel>("grapheditor.components", 1, 0, "FilterPortModel");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterPortSortModel>("grapheditor.components", 1, 0, "FilterPortSortModel");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterPortPartnerModel>("grapheditor.components", 1, 0, "FilterPortPartnerModel");
    qmlRegisterType<precitec::gui::components::grapheditor::NodeModel>("grapheditor.components", 1, 0, "NodeModel");
    qmlRegisterType<precitec::gui::components::grapheditor::NodeSortModel>("grapheditor.components", 1, 0, "NodeSortModel");
    qmlRegisterType<precitec::storage::AttributeModel>("grapheditor.components", 1, 0, "AttributeModel");
    qmlRegisterType<precitec::storage::FilterAttributeModel>("grapheditor.components", 1, 0, "FilterAttributeModel");
    qmlRegisterType<precitec::storage::ResultSettingModel>("grapheditor.components", 1, 0, "ResultSettingModel");
    qmlRegisterType<precitec::storage::SensorSettingsModel>("grapheditor.components", 1, 0, "SensorSettingsModel");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterAttributeGroupModel>("grapheditor.components", 1, 0, "FilterAttributeGroupModel");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterGroupModel>("grapheditor.components", 1, 0, "FilterGroupModel");
    qmlRegisterType<precitec::gui::components::grapheditor::FilterGraphFilterModel>("grapheditor.components", 1, 0, "FilterGraphFilterModel");
    qmlRegisterType<precitec::gui::components::grapheditor::DnDConnector>("grapheditor.components", 1, 0, "DnDConnector");
    qmlRegisterType<precitec::gui::components::grapheditor::InvalidNodeModel>("grapheditor.components", 1, 0, "InvalidNodeModel");
    qmlRegisterType<precitec::gui::ResultSettingFilterModel>("grapheditor.components", 1, 0, "ResultSettingFilterModel");
    qmlRegisterType<precitec::gui::components::grapheditor::CopyPasteHandler>("grapheditor.components", 1, 0, "CopyPasteHandler");
    qmlRegisterType<precitec::gui::components::grapheditor::DirectoryLoader>("grapheditor.components", 1, 0, "DirectoryLoader");
    qmlRegisterType<precitec::gui::components::grapheditor::DirectoryModel>("grapheditor.components", 1, 0, "DirectoryModel");
    qmlRegisterType<precitec::gui::components::grapheditor::InvalidIDModel>("grapheditor.components", 1, 0, "InvalidIDModel");
    qmlRegisterType<precitec::gui::components::grapheditor::PipeController>("grapheditor.components", 1, 0, "PipeController");
    qmlRegisterType<precitec::gui::components::grapheditor::MacroController>("grapheditor.components", 1, 0, "MacroController");
    qmlRegisterType<precitec::gui::components::grapheditor::SelectedFilterNodesFilterModel>("grapheditor.components", 1, 0, "SelectedFilterNodesFilterModel");
    qmlRegisterType<precitec::gui::components::grapheditor::PlausibilityController>("grapheditor.components", 1, 0, "PlausibilityController");
    qmlRegisterType<precitec::gui::components::grapheditor::ExistingFileValidator>("grapheditor.components", 1, 0, "ExistingFileValidator");
}

void GraphEditorPlugin::initializeEngine ( QQmlEngine* engine, const char* uri )
{
    Q_UNUSED(uri)
    QuickQanava::initialize(engine);
}

#include "main_plugin.moc"
