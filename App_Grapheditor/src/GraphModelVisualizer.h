#pragma once

#include <array>

#include <QSize>
#include <QQuickPaintedItem>
#include <QTimer>

#include "GraphLoader.h"
#include "FilterGraph.h"
#include "GraphFilterModel.h"
#include "attributeModel.h"

#include <Poco/UUID.h>

#include <QuickQanava.h>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

using namespace precitec::gui::components::grapheditor;

class GroupController;
class MacroController;
class PipeController;
class FilterMacro;

class GraphModelVisualizer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::GraphLoader *graphLoader READ graphLoader WRITE setGraphLoader NOTIFY graphLoaderChanged)
    Q_PROPERTY(precitec::gui::components::grapheditor::FilterGraph *filterGraph READ filterGraph WRITE setFilterGraph NOTIFY filterGraphChanged)
    Q_PROPERTY(precitec::gui::components::grapheditor::GraphFilterModel *graphFilterModel READ graphFilterModel WRITE setGraphFilterModel NOTIFY graphFilterModelChanged)
    Q_PROPERTY(precitec::storage::AttributeModel* attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)
    Q_PROPERTY(qan::GraphView* graphView READ graphView WRITE setGraphView NOTIFY graphViewChanged)     //FOCUS object after insertion (Filternode and filterGroup)

    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(bool useGridSizeAutomatically READ useGridSizeAutomatically WRITE setUseGridSizeAutomatically NOTIFY useGridSizeAutomaticallyChanged)

    Q_PROPERTY(bool graphEdited READ graphEdited WRITE setGraphEdited NOTIFY graphEditedChanged)
    Q_PROPERTY(bool graphNameEdited READ graphNameEdited NOTIFY graphNameEditedChanged)

    Q_PROPERTY(precitec::gui::components::grapheditor::GroupController *groupController READ groupController CONSTANT)
    Q_PROPERTY(precitec::gui::components::grapheditor::MacroController *macroController READ macroController WRITE setMacroController NOTIFY macroControllerChanged)

    Q_PROPERTY(fliplib::GraphContainer *graph READ graph NOTIFY graphChanged)

    Q_PROPERTY(precitec::gui::components::grapheditor::PipeController *pipeController READ pipeController WRITE setPipeController NOTIFY pipeControllerChanged)

public:
    explicit GraphModelVisualizer(QObject *parent = nullptr);
    ~GraphModelVisualizer() override;

    GraphLoader *graphLoader() const;
    void setGraphLoader(GraphLoader *graphLoader);
    FilterGraph *filterGraph() const;
    void setFilterGraph(FilterGraph *filterGraph);
    GraphFilterModel *graphFilterModel() const;
    void setGraphFilterModel(GraphFilterModel* graphFilterModel);
    precitec::storage::AttributeModel* attributeModel() const;
    void setAttributeModel(precitec::storage::AttributeModel* attributeModel);
    qan::GraphView* graphView() const;
    void setGraphView(qan::GraphView* view);

    fliplib::GraphContainer* graph();
    void setActualGraph( const fliplib::GraphContainer& actualGraph);
    void setFilterContainer( const std::vector<GraphFilterModel::FilterInfoContainer>& filterInfoContainer);

    bool graphEdited() const;
    void setGraphEdited(bool edited);

    bool graphNameEdited() const
    {
        return m_graphNameEdited;
    }

    int gridSize() const;
    void setGridSize(const int newSize);
    bool useGridSizeAutomatically() const;
    void setUseGridSizeAutomatically(bool useGridSize);

    MacroController *macroController() const
    {
        return m_macroController;
    }
    void setMacroController(MacroController *macroController);

    void setPipeController(PipeController *controller);
    PipeController *pipeController() const
    {
        return m_pipeController;
    }

//Start, init and show functions for the grapheditor
    Q_INVOKABLE void startUp();         //Loading the filter classes
    Q_INVOKABLE void initGraph();
    Q_INVOKABLE void showGraph();
    enum class Type {
        Graph,
        SubGraph,
    };
    Q_ENUM(Type)
    Q_INVOKABLE void createNewGraph(Type type);
    Q_INVOKABLE void clearFilterGraph();

    Q_INVOKABLE QString getFilePath() const;
    Q_INVOKABLE QString getFileName() const;
    Q_INVOKABLE void setGraphName(const QString &graphName);
    Q_INVOKABLE void setFileInfo(QString path, QString name);

//FILTERGRAPH functions --> interaction with FilterGraph.h and graphContainer.h
    //Get FILTERGRAPH properties from struct graphContainer.h
    QString getFilterGraphName() const;
    Q_INVOKABLE QUuid getFilterGraphID() const;
    QString getFilterGraphComment() const;
    QString getFilterGraphPath() const;
    //Set FILTERGRAPH properties after opening the graphProperties.qml dialog
    void setFilterGraphName(const QString &name);
    void setFilterGraphComment(const QString &comment);
    Q_INVOKABLE void setFilterProperties(const QString &name, const QString &comment, const QString &group);
    //Other FILTERGRAPH functions
    void createNewFilterGraph(Type type);
    void initExistingFilterGraph();
//

//FilterContainer functions --> get filter type information
    //Get information by position
    /*QString getFilterLibName (int actualFilterType) const;
    QString getLibPath (int actualFilterType) const;
    QString getFilterTypeName (int actualFilterType) const;
    QUuid getFilterTypeID (int actualFilterType) const;
    QString getFilterImagePath (int actualFilterType) const;
    int getFilterType (int actualFilterType) const;
    std::vector<fliplib::PipeConnector> getInPipes(int actualFilterType) const;
    std::vector<fliplib::PipeConnector> getOutPipes(int actualFilterType) const;
    std::vector<QUuid> getVariantIDs(int actualFilterType) const;*/
    //Search information
//

//FILTERNODE functions
    QString getFilterLabel( int actualFilter ) const;
    QPointF getFilterPosition( int actualFilter ) const;
    QSizeF getFilterSize( int actualFilter ) const;
    QUuid getFilterInstanceID( int actualFilter) const;
    QUuid getFilterTypeID( int actualFilter) const;                  // Needed to get the right image to display in the graph and to
    int searchFilterType( const QUuid &filterTypeID) const;                 //?
    QUuid getFilterDescriptionID( int actualFilter) const;
    QString getFilterDescriptionName( int actualFilter) const;
    //int searchFilterTypeDescription( const QUuid &filterTypeID) const;    //Da actualFilter = FilterInstance und FilterDescription auf selber position
    QString getFilterType( int filterTypePosition) const;
    QUuid getFilterUUID( int filterTypePosition) const;
    QUrl getFilterImagePathID( const QUuid &filterTypeID);           //Changes: Only filterTypeID as parameter, need to rename the images to UUID
    int getFilterGroup( int actualFilter) const;                           //Name: or getGroup? Needed to get the right group to group the node in
    std::vector<QUuid> getVariantIDs(const int filterTypePosition) const;
    Q_INVOKABLE int getNumberOfFilters() const;

    std::vector<fliplib::InstanceFilter::Attribute> fillAttribute(const std::vector<QUuid> &variantIDs, const Poco::UUID &instanceVariantID);

    //GraphEditor --> insert objects
    void insertNewPort(const fliplib::Port& newPort);

//Search cpp functions
    std::pair<fliplib::InstanceFilter*, unsigned int> searchInstanceFilter(const FilterNode& filter);

//Delete functions
    //Filter delete
    void eraseSingleFilter(const QUuid& ID);

    //Port delete
    void erasePort(const QUuid& ID);

    //Comment delete
    void eraseComment(const QUuid& ID);

    //FilterType functions, access to the GraphFilterModel::FilterInfoContainer
    QString getFilterLibName( int index) const;
    QUuid getFilterID( int index) const;
    QString getFilterName( int index) const;
    QUrl getFilterImagePath( int index) const;

    //Connector functions
    void insertOneInputConnector(FilterNode* node, int connectorInfoPosition, int actualConnector);
    void insertOneOutputConnector(FilterNode* node, int connectorInfoPosition, int actualConnector);
    void insertAllInputConnectors(FilterNode* node, int connectorInfoPosition);
    void insertAllOutputConnectors(FilterNode* node, int connectorInfoPosition);
    void insertAllConnectors(FilterNode* node, int connectorInfoPosition);

    //Visualization of the nodes
    FilterNode* insertOneFilterNode(fliplib::InstanceFilter filterInstance, fliplib::FilterDescription filterDescription);
    Q_INVOKABLE void insertOneFilterNode( int actualFilter);
    Q_INVOKABLE void insertAllFilterNodes();

//PORTNODES functions
    int getPortType( int actualPort);
    QString getPortName( int actualPort);
    QUuid getPortID( int actualPort);
    QPointF getPortPosition( int actualPort);
    QSize getPortSize( int actualPort);
    int getPortGroup( int actualPort);
    QUuid getSenderFilterID( int actualPort);
    QUuid getSenderConnectorID( int actualPort);
    QUuid getReceiverFilterID( int actualPort);
    QUuid getReceiverConnectorID( int actualPort);
    QString getPortText( int actualPort);
    std::vector<fliplib::GraphItemExtension> getGraphItemExtensions( int actualPort);
    FilterPort* searchPortPartner(const QUuid &partnerID);

    Q_INVOKABLE int getNumberOfPorts();

    //Visualization portnodes
    void getPortPartners();
    FilterPort* insertOnePort(fliplib::Port port);
    void insertOneFilterPort( int actualPort);
    void insertOneComment( int actualPort);
    Q_INVOKABLE void insertAllFilterPorts();

    //Visualization of the pipes
    void insertPipes(fliplib::Pipe newPipe);
    void getInternalNodeList(std::vector<FilterNode*> &nodes);
    void getInternalNodeList(std::vector<FilterMacro*>& nodes);
    void getInternalPortList(std::vector<FilterPort*> &ports);
    void insertOnePipePortWithConnector(const fliplib::Pipe &pipe, const std::vector<FilterPort*> &ports);
    void insertAllPipesWithPorts(const std::vector<FilterNode*> &copiedNodes, const std::vector<FilterPort*> &copiedPorts);
    void insertAllPipesWithPorts(const std::vector<FilterMacro*>& copiedNodes, const std::vector<FilterPort*>& copiedPorts);
    Q_INVOKABLE void insertAllPipesWithPorts();
    //port and pipe functions

    //Save modified graph
    Q_INVOKABLE void updateGraphLoaderWithModifiedGraph(bool changeUUID = 0);
    Q_INVOKABLE QUuid getActualGraphUUID();

    //fliplib::InstanceFilter* getFilterActualGraph( const int index);
    FilterNode* getFilterGraphNode( int index);
    FilterPort* getFilterGraphPort( int index);

//Attributes changing functions --> Changing on the cpp side
    //QAN::NODE
    Q_INVOKABLE void updatePosition(QObject* node);         //Used when a node was moved -> FilterGraphView.qml
    Q_INVOKABLE void updateFilterLabel(FilterNode *node, const QString &label);
    Q_INVOKABLE void updateFilterPosition(FilterNode *node, const QPointF &point);
    Q_INVOKABLE void updateCommentPosition(FilterComment* comment, const QPointF &point);
    Q_INVOKABLE void updateCommentSize(FilterComment* comment, const QSizeF &size);
    Q_INVOKABLE void updatePortPosition(FilterPort *port, const QPointF &point);
    Q_INVOKABLE void updatePortLabel(FilterPort *port, const QString &label);
    //FILTERNODE specific Attributes
    Q_INVOKABLE void updateFilterAttributes(FilterNode* node, const QUuid &parameterUuid, int parameterValue);
    //FILTERPORT specific Attributes, but would function so too, works with bool too
    Q_INVOKABLE void updatePortAttributes(FilterPort* port, FilterPort* portPartner);
    //FILTERCOMMENT specific Attributes
    Q_INVOKABLE void updateCommentAttributes(FilterComment* comment, const QString &text);

    GroupController *groupController() const
    {
        return m_groupController;
    }
//

//Insertion of new elements CPP functions
    //Insertion FILTERNODE
    int getComponentNumber(const QString& component);
    Poco::UUID getComponentID(int component);
    std::string getComponentVersion(int component);
    void insertNewCppFilter(FilterNode* newFilter, const int actualFilterTypePos);
    //Insertion FILTERPORT
    void insertNewCppFilterPort(FilterPort* newPort);
    //Insertion FILTERCOMMENT
    void insertNewCppFilterComment(FilterComment* newComment);
//

//Copy-Paste-Stuff TODO
    fliplib::InstanceFilter searchInstanceFilter(const QUuid &id);
    fliplib::FilterDescription searchFilterDescription(const QUuid &filterID);
    fliplib::Port searchPortComment(const QUuid &id);
    fliplib::Pipe searchPipe(const QUuid &senderID, const QUuid &senderConnectorID, const QUuid &receiverID, const QUuid &receiverConnectorID);
    void checkOtherPortConnections(const Poco::UUID &ID);

//Grid stuff!
    Q_INVOKABLE void alignSelectionHorizontal();
    Q_INVOKABLE void alignSelectionVertical();

Q_SIGNALS:
        void graphLoaderChanged();
        void filterGraphChanged();
        void graphFilterModelChanged();
        void attributeModelChanged();
        void graphViewChanged();

        void newObjectInserted(qan::Node* object);

        void graphChanged();
        void graphEditedChanged();
        void graphNameEditedChanged();

        void gridSizeChanged();
        void useGridSizeAutomaticallyChanged();
        void macroControllerChanged();

        void pipeControllerChanged();

private:
    template <typename T>
    void updatePositionImpl(QObject* node);
    template <typename T>
    void alignSelectionHorizontal(qan::Node *node);
    template <typename T>
    void alignSelectionVertical(qan::Node *node);
    void alignToGridHorizontal(QPointF &point);
    void alignToGridVertical(QPointF &point);

    void insertMacros();

    void setGraphNameEdited(bool set);

    enum Component
    {
        bridges,
        calibration,
        gapTracking,
        imageProcessing,
        imageSource,
        lineGeometry,
        lineTracking,
        poreAnalysis,
        results,
        sampleSource,
        seamSearch,
        utility
    };
    GraphLoader *m_graphLoader = nullptr;
    QMetaObject::Connection m_graphLoaderDestroyedConnection;
    FilterGraph *m_filterGraph = nullptr;
    QMetaObject::Connection m_filterGraphDestroyedConnection;
    GraphFilterModel *m_graphFilterModel = nullptr;
    QMetaObject::Connection m_graphFilterModelDestroyedConnection;
    precitec::storage::AttributeModel* m_attributeModel = nullptr;
    qan::GraphView* m_graphView = nullptr;
    QMetaObject::Connection m_graphViewDestroyedConnection;
    fliplib::GraphContainer m_actualGraph;
    std::vector<GraphFilterModel::FilterInfoContainer> m_filterContainer;
    std::vector<FilterPort*> m_filterPorts;
    int m_gridSize = 1;
    bool m_useGridSizeAutomatically = false;

    bool m_graphEdited = false;
    bool m_graphNameEdited = false;

    GroupController *m_groupController;
    MacroController *m_macroController = nullptr;
    QMetaObject::Connection m_macroControllerDestroyed;

    PipeController *m_pipeController = nullptr;
    QMetaObject::Connection m_pipeControllerDestroyed;
    bool m_initializingGraph{false};
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::GraphModelVisualizer*)
