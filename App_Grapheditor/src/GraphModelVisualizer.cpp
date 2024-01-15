#include "GraphModelVisualizer.h"
#include "graphHelper.h"
#include "groupController.h"
#include "filterMacro.h"
#include "filterMacroConnector.h"
#include "macroController.h"
#include "pipeController.h"

#include <QDebug>
#include <QDir>
#include <algorithm>

#include "fliplib/graphContainer.h"                 //showAllFilters
#include "../App_Storage/src/compatibility.h"

#include "attribute.h"
#include "Poco/UUIDGenerator.h"

using namespace precitec::gui::components::grapheditor;

GraphModelVisualizer::GraphModelVisualizer(QObject* parent)
    : QObject(parent)
    , m_groupController{new GroupController{this}}
{
    connect(this, &GraphModelVisualizer::graphChanged, this, [this] { m_groupController->setActualGraph(&m_actualGraph); });
    connect(this, &GraphModelVisualizer::filterGraphChanged, this, [this] { m_groupController->setFilterGraph(m_filterGraph); });
    connect(this, &GraphModelVisualizer::gridSizeChanged, this, [this] { m_groupController->setGridSize(m_gridSize); });
    connect(this, &GraphModelVisualizer::useGridSizeAutomaticallyChanged, this, [this] { m_groupController->setUseGridSizeAutomatically(m_useGridSizeAutomatically); });
    connect(m_groupController, &GroupController::changed, this, std::bind(&GraphModelVisualizer::setGraphEdited, this, true));
    m_groupController->setActualGraph(&m_actualGraph);
}

GraphModelVisualizer::~GraphModelVisualizer() = default;

GraphLoader * GraphModelVisualizer::graphLoader() const
{
    return m_graphLoader;
}

void GraphModelVisualizer::setGraphLoader(GraphLoader* graphLoader)
{
    if (m_graphLoader == graphLoader)
    {
        return;
    }
    disconnect(m_graphLoaderDestroyedConnection);
    m_graphLoader = graphLoader;
    if (m_graphLoader)
    {
        m_graphLoaderDestroyedConnection = connect(m_graphLoader, &QObject::destroyed, this, std::bind(&GraphModelVisualizer::setGraphLoader, this, nullptr));
    }
    else
    {
        m_graphLoaderDestroyedConnection = {};
    }
    emit graphLoaderChanged();
}

FilterGraph * GraphModelVisualizer::filterGraph() const
{
    return m_filterGraph;
}

void GraphModelVisualizer::setFilterGraph(FilterGraph* filterGraph)
{
    if (m_filterGraph == filterGraph)
    {
        return;
    }
    disconnect(m_filterGraphDestroyedConnection);
    m_filterGraph = filterGraph;
    if (m_filterGraph)
    {
        m_filterGraphDestroyedConnection = connect(m_filterGraph, &QObject::destroyed, this, std::bind(&GraphModelVisualizer::setFilterGraph, this, nullptr));
    }
    else
    {
        m_filterGraphDestroyedConnection = {};
    }
    emit filterGraphChanged();
}

GraphFilterModel * GraphModelVisualizer::graphFilterModel() const
{
    return m_graphFilterModel;
}

void GraphModelVisualizer::setGraphFilterModel(GraphFilterModel* graphFilterModel)
{
    if (m_graphFilterModel == graphFilterModel)
    {
        return;
    }
    disconnect(m_filterGraphDestroyedConnection);
    m_graphFilterModel = graphFilterModel;
    if (m_graphFilterModel)
    {
        m_graphFilterModelDestroyedConnection = connect(m_graphFilterModel, &QObject::destroyed, this, std::bind(&GraphModelVisualizer::setGraphFilterModel, this, nullptr));
    }
    else
    {
        m_graphFilterModelDestroyedConnection = {};
    }
    emit graphFilterModelChanged();
}

precitec::storage::AttributeModel * GraphModelVisualizer::attributeModel() const
{
    return m_attributeModel;
}

void GraphModelVisualizer::setAttributeModel(precitec::storage::AttributeModel* attributeModel)
{
    if (m_attributeModel != attributeModel)
    {
        m_attributeModel = attributeModel;
        emit attributeModelChanged();
    }
}

qan::GraphView * GraphModelVisualizer::graphView() const
{
    return m_graphView;
}

void GraphModelVisualizer::setGraphView(qan::GraphView* view)
{
    if (m_graphView == view)
    {
        return;
    }
    m_graphView = view;
    if (m_graphView)
    {
        m_graphViewDestroyedConnection = connect(m_graphView, &QObject::destroyed, this, std::bind(&GraphModelVisualizer::setGraphView, this, nullptr));
    }
    else
    {
        m_graphViewDestroyedConnection = {};
    }
    emit graphViewChanged();
}

fliplib::GraphContainer * GraphModelVisualizer::graph()
{
    return &m_actualGraph;
}

void GraphModelVisualizer::setActualGraph(const fliplib::GraphContainer& actualGraph)
{
    m_actualGraph = actualGraph;
    emit graphChanged();
}

void GraphModelVisualizer::setFilterContainer(const std::vector<GraphFilterModel::FilterInfoContainer>& filterInfoContainer)
{
    if (m_filterContainer.size() == 0)
    {
        m_filterContainer = filterInfoContainer;
    }
}

bool GraphModelVisualizer::graphEdited() const
{
    return m_graphEdited;
}

void GraphModelVisualizer::setGraphEdited(bool edited)
{
    m_graphEdited = edited;
    emit graphEditedChanged();
}

int GraphModelVisualizer::gridSize() const
{
    return m_gridSize;
}

void GraphModelVisualizer::setGridSize(const int newSize)
{
    if (m_gridSize != newSize)
    {
        m_gridSize = newSize;
        emit gridSizeChanged();
    }
}

bool GraphModelVisualizer::useGridSizeAutomatically() const
{
    return m_useGridSizeAutomatically;
}

void GraphModelVisualizer::setUseGridSizeAutomatically(bool useGridSize)
{
    if (m_useGridSizeAutomatically != useGridSize)
    {
        m_useGridSizeAutomatically = useGridSize;
        emit useGridSizeAutomaticallyChanged();
    }
}

void GraphModelVisualizer::startUp()
{
    setFilterContainer(m_graphFilterModel->getFilterInformationModel());
    if (m_filterContainer.size() != 0)
    {
        qDebug() << "Filtercontainer loaded successful!";
    }
    createNewFilterGraph(Type::Graph);
}

void GraphModelVisualizer::initGraph()
{
    if (m_graphEdited == true)
    {
        qDebug() << "The graph was edited. Please save the changes first!";
        return;
    }
    m_initializingGraph = true;
    setActualGraph(m_graphLoader->actualGraphContainer());
    if (!m_actualGraph.id.isNull())
    {
        //Reorganize/swap the order of the pipe extensions for easier save as operation
        //Order is first extension with type = 7 and second if available type = 1
        for (auto &pipe : m_actualGraph.pipes)
        {
            if (pipe.extensions.size() == 2 && pipe.extensions.at(0).type == 1)
            {
                //Swap the content
                std::swap(pipe.extensions.at(0), pipe.extensions.at(1));
            }
        }
    }
    //Insert graph properties to filter graph
    initExistingFilterGraph();
    //Show the graph in the graph view
    insertAllFilterNodes();
    m_groupController->insertAllFilterGroups();
    insertAllFilterPorts();
    m_groupController->connectNodesToFilterGroups();

    insertMacros();

    insertAllPipesWithPorts();
    m_initializingGraph = false;

    // force an updateItem for each edge. Needs to be slightly delayed, at end of loop is not sufficient
    QTimer::singleShot(1,
        [this]{
            for (auto edge : m_filterGraph->get_edges())
            {
                edge->getItem()->updateItem();
            }
        });
}

QString GraphModelVisualizer::getFilterGraphName() const
{
    return QString::fromStdString(m_actualGraph.name);
}

QUuid GraphModelVisualizer::getFilterGraphID() const
{
    return precitec::storage::compatibility::toQt(m_actualGraph.id);
}

QString GraphModelVisualizer::getFilterGraphComment() const
{
    return QString::fromStdString(m_actualGraph.comment);
}

QString GraphModelVisualizer::getFilterGraphPath() const
{
    return QString::fromStdString(m_actualGraph.path);
}

void GraphModelVisualizer::setFilterGraphName(const QString &name)
{
    const auto graphName = name.toStdString();

    if (m_actualGraph.name != graphName)
    {
        m_actualGraph.name = graphName;
        m_actualGraph.fileName = graphName;
        setGraphEdited(true);
        setGraphNameEdited(true);
    }
}

void GraphModelVisualizer::setFilterGraphComment(const QString &comment)
{
    const auto graphComment = comment.toStdString();

    if (m_actualGraph.comment != graphComment)
    {
        m_actualGraph.comment = graphComment;
        setGraphEdited(true);
    }
}

void GraphModelVisualizer::setFilterProperties(const QString& name, const QString& comment, const QString &group)
{
    if (!name.isEmpty())
    {
        setFilterGraphName(name);
    }
    if (!comment.isEmpty())
    {
        setFilterGraphComment(comment);
    }
    if (const auto groupString = group.toStdString(); m_actualGraph.group != groupString)
    {
        m_actualGraph.group = groupString;
        m_filterGraph->setGroupName(group);
        setGraphEdited(true);
    }
}

void GraphModelVisualizer::createNewFilterGraph(Type type)
{
    const QString graphName{"New_Graph"};
    const QUuid graphID = QUuid::createUuid();
    const QString graphComment{"New graph comment!"};
    //const QString graphDate{QDateTime::currentDateTime().toString(QStringLiteral("MM/dd/yyyy HH:mm:ss"))};

    m_actualGraph.id = precitec::storage::compatibility::toPoco(graphID);
    m_actualGraph.name = graphName.toStdString();
    m_actualGraph.comment = graphComment.toStdString();
    m_actualGraph.fileName = graphName.toStdString();

    const auto userPath = QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QStringLiteral("/config/") + (type == Type::Graph ? QStringLiteral("graphs/") : QStringLiteral("sub_graphs/"));
    m_actualGraph.path = userPath.toStdString();                    //Graph dir for user graphs

    m_filterGraph->setPortDelegateToFilterPort();
    initExistingFilterGraph();
    setGraphNameEdited(true);
}

void GraphModelVisualizer::createNewGraph(Type type)
{
    //Delete cpp side
    m_actualGraph = {};
    //Delete qml side and get Properties from new graph
    createNewFilterGraph(type);

}

void GraphModelVisualizer::initExistingFilterGraph()
{
    //Only on qml side, because the cpp side was already filled with the infos with m_actualGraph
    m_filterGraph->clear();
    //m_filterGraph->setPortDelegateToFilterPort();
    m_filterGraph->setGraphName(getFilterGraphName());
    m_filterGraph->setGraphID(getFilterGraphID());
    m_filterGraph->setGraphComment(getFilterGraphComment());
    m_filterGraph->setGraphPath(getFilterGraphPath());
    m_filterGraph->setGroupName(QString::fromStdString(m_actualGraph.group));

    setGraphNameEdited(false);
}

QString GraphModelVisualizer::getFilterLabel( int actualFilter) const
{
    return QString::fromStdString(m_actualGraph.instanceFilters.at(actualFilter).name);
}

QPointF GraphModelVisualizer::getFilterPosition( int actualFilter) const
{
    return QPoint{m_actualGraph.instanceFilters.at(actualFilter).position.x, m_actualGraph.instanceFilters.at(actualFilter).position.y};
}

QSizeF GraphModelVisualizer::getFilterSize( int actualFilter) const
{
    return QSize{m_actualGraph.instanceFilters.at(actualFilter).position.width, m_actualGraph.instanceFilters.at(actualFilter).position.height};
}

QUuid GraphModelVisualizer::getFilterInstanceID(int actualFilter) const
{
    return precitec::storage::compatibility::toQt(m_actualGraph.instanceFilters.at(actualFilter).id);
}

QUuid GraphModelVisualizer::getFilterTypeID(int actualFilter) const
{
    return precitec::storage::compatibility::toQt(m_actualGraph.instanceFilters.at(actualFilter).filterId);
}

int GraphModelVisualizer::searchFilterType(const QUuid& filterTypeID) const
{
    auto filterType = std::find_if(m_filterContainer.begin(), m_filterContainer.end(), [filterTypeID](GraphFilterModel::FilterInfoContainer const &actualFilterType) {return actualFilterType.filterID == filterTypeID;});
    if (filterType != m_filterContainer.end())
    {
        return std::distance(m_filterContainer.begin(), filterType);
    }
    else
    {
        return -1;
    }
}

QUuid GraphModelVisualizer::getFilterDescriptionID( int actualFilter) const
{
    return precitec::storage::compatibility::toQt(m_actualGraph.instanceFilters.at(actualFilter).filterId);
}

QString GraphModelVisualizer::getFilterDescriptionName( int actualFilter) const
{
    const auto descriptionId = m_actualGraph.instanceFilters.at(actualFilter).filterId;
    const auto it = std::find_if(m_actualGraph.filterDescriptions.begin(), m_actualGraph.filterDescriptions.end(), [&descriptionId] (const auto &description) { return description.id == descriptionId; });
    if (it == m_actualGraph.filterDescriptions.end())
    {
        return {};
    }
    return QString::fromStdString(it->name);
}

QString GraphModelVisualizer::getFilterType ( int filterTypePosition ) const
{
    if (filterTypePosition != -1)
    {
        return m_filterContainer.at(filterTypePosition).filterName;
    }
    else
    {
        return {};
    }
}

QUuid GraphModelVisualizer::getFilterUUID(int filterTypePosition) const
{
    return m_filterContainer.at(filterTypePosition).filterID;
}

QUrl GraphModelVisualizer::getFilterImagePathID(const QUuid& filterTypeID)
{
    auto foundFilterType = std::find_if(m_filterContainer.begin(), m_filterContainer.end(), [filterTypeID](const auto &actualFilterType){return actualFilterType.filterID == filterTypeID;});
    if (foundFilterType != m_filterContainer.end())
    {
        QDir dir = m_graphFilterModel->filterImagePath();
        if (dir.exists(foundFilterType->filterImagePath.toString().right(40)))
        {
            return foundFilterType->filterImagePath;
        }
    }
    return QUrl::fromLocalFile(m_graphFilterModel->filterImagePath() + "dummyFilter.png");
}

int GraphModelVisualizer::getFilterGroup( int actualFilter) const
{
    return m_actualGraph.instanceFilters.at(actualFilter).group;
}

std::vector<QUuid> GraphModelVisualizer::getVariantIDs(const int filterTypePosition) const
{
    return m_filterContainer.at(filterTypePosition).variantID;
}

inline int GraphModelVisualizer::getNumberOfFilters() const
{
    return m_actualGraph.instanceFilters.size();
}

std::vector<fliplib::InstanceFilter::Attribute> GraphModelVisualizer::fillAttribute(const std::vector<QUuid> &variantIDs, const Poco::UUID &instanceVariantID)
{
    if (variantIDs.size() == 0)
    {
        qDebug() << "Something wrong, no variantIDs (GraphModelVisualizer::fillAttribute())";
        return {};
    }

    std::vector<fliplib::InstanceFilter::Attribute> fliplibAttributes;
    for (const auto variantID : variantIDs)
    {
        auto attributes = m_attributeModel->findAttributesByVariantId(variantID);
        if (attributes.size() == 0)
        {
            qDebug() << "Something wrong, no attributes (GraphModelVisualizer::fillAttribute())";
        }

        for (const auto &attribute : attributes)
        {
            fliplib::InstanceFilter::Attribute fliplibAttribute;
            fliplibAttribute.attributeId = precitec::storage::compatibility::toPoco(attribute->uuid());
            fliplibAttribute.instanceAttributeId = Poco::UUIDGenerator().createRandom();
            fliplibAttribute.name = attribute->name().toStdString();      //or contentName
            fliplibAttribute.value = attribute->defaultValue().toString().toStdString();     //FIXME Add type comparison and change QVariant to right value
            fliplibAttribute.userLevel = attribute->userLevel();
            fliplibAttribute.visible = attribute->visible();
            fliplibAttribute.publicity = attribute->publicity();
            //fliplibAttribute.blob = attribute->
            //fliplibAttribute.hasBlob = attribute->
            fliplibAttribute.instanceVariantId = instanceVariantID;
            fliplibAttribute.variantId = precitec::storage::compatibility::toPoco(attribute->variantId());
            //fliplibAttribute.helpFile = QString::toStdString();
            fliplibAttributes.push_back(fliplibAttribute);
        }
    }

    return fliplibAttributes;
}

void GraphModelVisualizer::insertNewPort(const fliplib::Port& newPort)
{
    m_actualGraph.ports.push_back(newPort);

    m_graphEdited = true;
    emit graphEditedChanged();
}

void GraphModelVisualizer::eraseComment(const QUuid& ID)
{
    auto commentID = precitec::storage::compatibility::toPoco(ID);
    auto comment = std::find_if(m_actualGraph.ports.begin(), m_actualGraph.ports.end(), [commentID](fliplib::Port const &actualPort){return actualPort.id == commentID;});
    if (comment != m_actualGraph.ports.end())
    {
        m_actualGraph.ports.erase(comment);
        m_graphEdited = true;
        emit graphEditedChanged();
        return;
    }
    qDebug() << "File must be corrupted, no element found! (GraphModelVisualizer::eraseComment)";
}

std::pair<fliplib::InstanceFilter*, unsigned int> GraphModelVisualizer::searchInstanceFilter(const FilterNode& filter)
{
    auto ID = precitec::storage::compatibility::toPoco(filter.ID());
    auto foundFilter = std::find_if(m_actualGraph.instanceFilters.begin(), m_actualGraph.instanceFilters.end(), [ID](const fliplib::InstanceFilter &actualFilter){return actualFilter.id == ID;});
    if (foundFilter != m_actualGraph.instanceFilters.end())
    {
        auto pos = std::distance(m_actualGraph.instanceFilters.begin(), foundFilter);
        return {&m_actualGraph.instanceFilters.at(std::distance(m_actualGraph.instanceFilters.begin(),foundFilter)), pos};
    }
    return {};
}

void GraphModelVisualizer::eraseSingleFilter(const QUuid& ID)
{
    auto id = precitec::storage::compatibility::toPoco(ID);
    auto foundFilter = std::find_if(m_actualGraph.instanceFilters.begin(), m_actualGraph.instanceFilters.end(), [id](const fliplib::InstanceFilter &actualFilter){return actualFilter.id == id;});
    if (foundFilter != m_actualGraph.instanceFilters.end())
    {
        // check whether it is the last filter of that type
        if (std::none_of(m_actualGraph.instanceFilters.begin(), m_actualGraph.instanceFilters.end(), [foundFilter] (const auto &filter) { return foundFilter->id != filter.id && foundFilter->filterId == filter.filterId; }))
        {
            auto it = std::find_if(m_actualGraph.filterDescriptions.begin(), m_actualGraph.filterDescriptions.end(), [foundFilter] (const auto &filterDescription) { return filterDescription.id == foundFilter->filterId; });
            if (it != m_actualGraph.filterDescriptions.end())
            {
                m_actualGraph.filterDescriptions.erase(it);
            }
        }

        m_actualGraph.instanceFilters.erase(foundFilter);
        m_graphEdited = true;
        emit graphEditedChanged();
        return;
    }
    qDebug() << "File must be corrupted, no element found! (GraphModelVisualizer::eraseSingleFilter)";
}

void GraphModelVisualizer::erasePort(const QUuid& ID)
{
    auto id = precitec::storage::compatibility::toPoco(ID);
    auto foundPort = std::find_if(m_actualGraph.ports.begin(), m_actualGraph.ports.end(), [id](fliplib::Port const &actualPort) {return actualPort.id == id;});
    if (foundPort != m_actualGraph.ports.end())
    {
        m_actualGraph.ports.erase(foundPort);
        m_graphEdited = true;
        emit graphEditedChanged();
    }
    else
    {
        qDebug() << "File must be corrupted, no element found! (GraphModelVisualizer::erasePort)";
    }
}

QString GraphModelVisualizer::getFilterLibName(int index) const
{
    return m_filterContainer.at(index).filterLibName;
}

QUuid GraphModelVisualizer::getFilterID(int index) const
{
    return m_filterContainer.at(index).filterID;
}

QString GraphModelVisualizer::getFilterName(int index) const
{
    return m_filterContainer.at(index).filterName;
}

QUrl GraphModelVisualizer::getFilterImagePath(int index) const
{
    return m_filterContainer.at(index).filterImagePath;
}

void GraphModelVisualizer::insertOneInputConnector(FilterNode* node, int connectorInfoPosition, int actualConnector)
{
    auto newInputConnector = dynamic_cast<FilterConnector*> (m_filterGraph->insertFilterConnector(node, qan::NodeItem::Dock::Left, qan::PortItem::Type::In, QString::fromStdString(m_filterContainer.at(connectorInfoPosition).inPipes.at(actualConnector).name())));
    newInputConnector->setID(precitec::storage::compatibility::toQt(m_filterContainer.at(connectorInfoPosition).inPipes.at(actualConnector).id()));
    newInputConnector->setTag(QString::fromStdString(m_filterContainer.at(connectorInfoPosition).inPipes.at(actualConnector).tag()));
    newInputConnector->setGroup(m_filterContainer.at(connectorInfoPosition).inPipes.at(actualConnector).group());
    newInputConnector->setConnectorType(static_cast<int>(m_filterContainer.at(connectorInfoPosition).inPipes.at(actualConnector).dataType()));
    newInputConnector->setColorValue(newInputConnector->connectorType());
    newInputConnector->setMultiplicity(qan::PortItem::Multiplicity::Single);
    newInputConnector->setConnectionType(m_filterContainer.at(connectorInfoPosition).inPipes.at(actualConnector).connectionType());
    newInputConnector->setGeometryInputConnector(node->getItemGeometry(), actualConnector,m_filterContainer.at(connectorInfoPosition).inPipes.size());
}

void GraphModelVisualizer::insertOneOutputConnector(FilterNode* node, int connectorInfoPosition, int actualConnector)
{
    auto newOutputConnector = dynamic_cast<FilterConnector*> (m_filterGraph->insertFilterConnector(node, qan::NodeItem::Dock::Right, qan::PortItem::Type::Out, QString::fromStdString(m_filterContainer.at(connectorInfoPosition).outPipes.at(actualConnector).name())));
    newOutputConnector->setID(precitec::storage::compatibility::toQt(m_filterContainer.at(connectorInfoPosition).outPipes.at(actualConnector).id()));
    newOutputConnector->setTag(QString::fromStdString(m_filterContainer.at(connectorInfoPosition).outPipes.at(actualConnector).tag()));
    newOutputConnector->setGroup(m_filterContainer.at(connectorInfoPosition).outPipes.at(actualConnector).group());
    newOutputConnector->setConnectorType(static_cast<int>(m_filterContainer.at(connectorInfoPosition).outPipes.at(actualConnector).dataType()));
    newOutputConnector->setColorValue(newOutputConnector->connectorType());
    newOutputConnector->setMultiplicity(qan::PortItem::Multiplicity::Multiple);
    newOutputConnector->setConnectionType(m_filterContainer.at(connectorInfoPosition).outPipes.at(actualConnector).connectionType());
    newOutputConnector->setGeometryOutputConnector(node->getItemGeometry(), actualConnector,m_filterContainer.at(connectorInfoPosition).outPipes.size());
}

void GraphModelVisualizer::insertAllInputConnectors( FilterNode* node, int connectorInfoPosition )
{
    for (long unsigned int actualConnector = 0; actualConnector < m_filterContainer.at(connectorInfoPosition).inPipes.size(); actualConnector++)
    {
        insertOneInputConnector(node, connectorInfoPosition, actualConnector);
    }
}

void GraphModelVisualizer::insertAllOutputConnectors( FilterNode* node, int connectorInfoPosition  )
{
    for (long unsigned int actualConnector = 0; actualConnector < m_filterContainer.at(connectorInfoPosition).outPipes.size(); actualConnector++)
    {
        insertOneOutputConnector(node,connectorInfoPosition, actualConnector);
    }
}

void GraphModelVisualizer::insertAllConnectors( FilterNode* node, int connectorInfoPosition )
{
    if (connectorInfoPosition != -1)
    {
        insertAllInputConnectors(node, connectorInfoPosition);
        insertAllOutputConnectors(node, connectorInfoPosition);
    }
}

namespace
{

std::vector<FilterConnector*> getConnectors(qan::Node* node)
{
    if (!node)
    {
        return {};
    }
    //Stolen from qanNodeItem.cpp -> findPort(const QString& portID); Method 1
    std::vector<FilterConnector*> connectors;
    connectors.reserve(static_cast<unsigned int>(node->getItem()->getPorts().size()));
    for ( const auto &connector : qAsConst(node->getItem()->getPorts()))
    {
        const auto connectorItem = qobject_cast<FilterConnector*>(connector);
        if (connectorItem && connectors.size() < static_cast<unsigned int>(node->getItem()->getPorts().size()))
        {
            connectors.push_back(connectorItem);
        }
    }
    return connectors;
}

FilterConnector* searchConnector( qan::Node* node, const QUuid &connectorID)
{
    auto connectors = getConnectors(node);
    auto connector = std::find_if(connectors.begin(), connectors.end(), [connectorID] (FilterConnector* const &actualConnector){return actualConnector->ID() == connectorID;});
    if (connector != connectors.end())
    {
        return connectors.at(std::distance(connectors.begin(), connector));
    }
    else
        return {};
}

}

FilterNode* GraphModelVisualizer::insertOneFilterNode(fliplib::InstanceFilter filterInstance, fliplib::FilterDescription filterDescription)
{
    //Create filter in qml
    auto newNode = dynamic_cast<FilterNode*> (m_filterGraph->insertFilterNode());
    newNode->setLabel(QString::fromStdString(filterInstance.name));
    newNode->setID(precitec::storage::compatibility::toQt(filterInstance.id));  //TODO Change
    newNode->setType(QString::fromStdString(filterDescription.name));
    newNode->setTypeID(precitec::storage::compatibility::toQt(filterDescription.id));
    newNode->setImage(getFilterImagePathID(newNode->typeID()));
    newNode->setGroupID(filterInstance.group);  //TODO Change
    QString contentName{"Precitec.Filter."};
    contentName += newNode->type().remove("precitec::filter::");
    newNode->setContentName(contentName);
    newNode->getItem()->setRect({static_cast<qreal>(filterInstance.position.x), static_cast<qreal>(filterInstance.position.y), static_cast<qreal>(filterInstance.position.width), static_cast<qreal>(filterInstance.position.height)});
    newNode->getItem()->setResizable(false);
    insertAllConnectors(newNode, searchFilterType(newNode->typeID()));
    //Copy structs to the graph (cpp)
    m_actualGraph.instanceFilters.push_back(filterInstance);
    m_actualGraph.filterDescriptions.push_back(filterDescription);
    return newNode;
}

void GraphModelVisualizer::insertOneFilterNode( int actualFilter)
{
    auto newNode = dynamic_cast<FilterNode*> (m_filterGraph->insertFilterNode());
    newNode->setLabel(getFilterLabel(actualFilter));
    newNode->setID(getFilterInstanceID(actualFilter));
    newNode->setType(getFilterDescriptionName(actualFilter));
    newNode->setTypeID(getFilterTypeID(actualFilter));
    newNode->setImage(getFilterImagePathID(newNode->typeID()));
    newNode->setGroupID(getFilterGroup(actualFilter));
    QString contentName{"Precitec.Filter."};
    contentName += getFilterDescriptionName(actualFilter).remove("precitec::filter::");
    newNode->setContentName(contentName);
    newNode->getItem()->setRect({getFilterPosition(actualFilter).x(), getFilterPosition(actualFilter).y(), getFilterSize(actualFilter).width(), getFilterSize(actualFilter).height()});
    newNode->getItem()->setResizable(false);
    insertAllConnectors(newNode, searchFilterType(getFilterTypeID(actualFilter)));
}

void GraphModelVisualizer::insertAllFilterNodes()
{
    for (int actualFilter = 0; actualFilter < getNumberOfFilters(); actualFilter++)
    {
        insertOneFilterNode(actualFilter);
    }
}

int GraphModelVisualizer::getPortType( int actualPort)
{
    return m_actualGraph.ports.at(actualPort).type;
}

QString GraphModelVisualizer::getPortName( int actualPort)
{
    return QString::fromStdString(m_actualGraph.ports.at(actualPort).receiverName);
}

QUuid GraphModelVisualizer::getPortID( int actualPort)
{
    return precitec::storage::compatibility::toQt(m_actualGraph.ports.at(actualPort).id);
}
QPointF GraphModelVisualizer::getPortPosition( int actualPort)
{
    return {static_cast<qreal>(m_actualGraph.ports.at(actualPort).position.x),static_cast<qreal>(m_actualGraph.ports.at(actualPort).position.y)};
}
QSize GraphModelVisualizer::getPortSize( int actualPort)
{
    return {m_actualGraph.ports.at(actualPort).position.width,m_actualGraph.ports.at(actualPort).position.height};          //Use of fliplib::Position like in the graphContainer.h?
}
int GraphModelVisualizer::getPortGroup( int actualPort)
{
    return m_actualGraph.ports.at(actualPort).group;
}

QUuid GraphModelVisualizer::getSenderFilterID( int actualPort)
{
    return precitec::storage::compatibility::toQt(m_actualGraph.ports.at(actualPort).senderInstanceFilterId);
}

QUuid GraphModelVisualizer::getSenderConnectorID( int actualPort)
{
    return precitec::storage::compatibility::toQt(m_actualGraph.ports.at(actualPort).senderConnectorId);
}

QUuid GraphModelVisualizer::getReceiverFilterID( int actualPort)
{
    return precitec::storage::compatibility::toQt(m_actualGraph.ports.at(actualPort).receiverInstanceFilterId);
}

QUuid GraphModelVisualizer::getReceiverConnectorID( int actualPort)
{
    return precitec::storage::compatibility::toQt(m_actualGraph.ports.at(actualPort).receiverConnectorId);
}

QString GraphModelVisualizer::getPortText(int actualPort)
{
    return QString::fromStdString(m_actualGraph.ports.at(actualPort).text);
}

std::vector<fliplib::GraphItemExtension> GraphModelVisualizer::getGraphItemExtensions( int actualPort)
{
    return m_actualGraph.ports.at(actualPort).extensions;
}

FilterPort * GraphModelVisualizer::searchPortPartner(const QUuid& partnerID)
{
    return {};
}

inline int GraphModelVisualizer::getNumberOfPorts()
{
    return m_actualGraph.ports.size();
}

void GraphModelVisualizer::getPortPartners()
{
    getInternalPortList(m_filterPorts);
    for ( auto filterPort : m_filterPorts)
    {
        if ( filterPort->type() == 3 && !filterPort->gotPartner())
        {
            for (const auto filterPortPartner : m_filterPorts)
            {
                if ( filterPort->ID() != filterPortPartner->ID())
                {
                    if ( filterPort->getLabel() == filterPortPartner->getLabel())
                    {
                        if (filterPort->gotPartner() == false)
                        {
                            filterPort->setGotPartner(true);
                        }
                        filterPort->insertPartner(filterPortPartner);
                    }
                }
            }
        }
        if ( filterPort->type() == 2 && !filterPort->gotPartner())
        {
            auto filterPortPartner = std::find_if(m_filterPorts.begin(), m_filterPorts.end(), [filterPort](FilterPort* const &actualFilterPort) {return (actualFilterPort->getLabel() == filterPort->getLabel() && actualFilterPort->ID() != filterPort->ID() && actualFilterPort->type() != filterPort->type());});
            if (filterPortPartner != m_filterPorts.end())
            {
                filterPort->setGotPartner(true);
                filterPort->insertPartner(m_filterPorts.at(std::distance(m_filterPorts.begin(), filterPortPartner)));
            }
        }
    }
}

FilterPort* GraphModelVisualizer::insertOnePort ( fliplib::Port port )
{
    if (port.type == 5)
    {
        auto newComment = dynamic_cast<FilterComment*> (m_filterGraph->insertFilterComment());
        newComment->setID(precitec::storage::compatibility::toQt(port.id));
        newComment->setGroupID(port.group);
        newComment->setText(QString::fromStdString(port.text));
        newComment->getItem()->setMinimumSize(QSizeF{25.0,25.0});
        newComment->getItem()->setRect({static_cast<qreal>(port.position.x), static_cast<qreal>(port.position.y), static_cast<qreal>(port.position.width), static_cast<qreal>(port.position.height)});
        m_actualGraph.ports.push_back(port);
        return {};
    }
    auto newFilterPort = dynamic_cast<FilterPort*> (m_filterGraph->insertFilterPort());
    newFilterPort->setID(precitec::storage::compatibility::toQt(port.id));
    newFilterPort->setLabel(QString::fromStdString(port.receiverName));
    newFilterPort->setType(port.type);
    newFilterPort->setGroupID(port.group);
    newFilterPort->setUuid(UUIDs::senderFilterInstanceUUID, precitec::storage::compatibility::toQt(port.senderInstanceFilterId));
    newFilterPort->setUuid(UUIDs::senderFilterConnectorUUID, precitec::storage::compatibility::toQt(port.senderConnectorId));
    newFilterPort->setUuid(UUIDs::receiverFilterInstanceUUID, precitec::storage::compatibility::toQt(port.receiverInstanceFilterId));
    newFilterPort->setUuid(UUIDs::receiverFilterConnectorUUID, precitec::storage::compatibility::toQt(port.receiverConnectorId));
    newFilterPort->getItem()->setMinimumSize(QSizeF{25,25});
    newFilterPort->getItem()->setRect({static_cast<qreal>(port.position.x), static_cast<qreal>(port.position.y), static_cast<qreal>(25), static_cast<qreal>(25)});
    newFilterPort->getItem()->setResizable(false);
    m_actualGraph.ports.push_back(port);
    return newFilterPort;
}

void GraphModelVisualizer::insertOneFilterPort( int actualPort)
{
    auto newFilterPort = dynamic_cast<FilterPort*> (m_filterGraph->insertFilterPort());
    newFilterPort->setID(getPortID(actualPort));
    newFilterPort->setLabel(getPortName(actualPort));
    newFilterPort->setType(getPortType(actualPort));
    newFilterPort->setGroupID(getPortGroup(actualPort));
    newFilterPort->setUuid(UUIDs::senderFilterInstanceUUID, getSenderFilterID(actualPort));
    newFilterPort->setUuid(UUIDs::senderFilterConnectorUUID, getSenderConnectorID(actualPort));
    newFilterPort->setUuid(UUIDs::receiverFilterInstanceUUID, getReceiverFilterID(actualPort));
    newFilterPort->setUuid(UUIDs::receiverFilterConnectorUUID, getReceiverConnectorID(actualPort));
    newFilterPort->getItem()->setMinimumSize(QSizeF{25,25});
    newFilterPort->getItem()->setRect({getPortPosition(actualPort).x(),getPortPosition(actualPort).y(),25,25});
    newFilterPort->getItem()->setResizable(false);
}

void GraphModelVisualizer::insertOneComment(int actualPort)
{
    auto newComment = dynamic_cast<FilterComment*> (m_filterGraph->insertFilterComment());
    newComment->setID(getPortID(actualPort));
    newComment->setGroupID(getPortGroup(actualPort));
    newComment->setText(getPortText(actualPort));
    newComment->getItem()->setMinimumSize(QSizeF{static_cast<qreal>(getPortSize(actualPort).width()), static_cast<qreal>(getPortSize(actualPort).height())});
    newComment->getItem()->setRect({getPortPosition(actualPort).x(), getPortPosition(actualPort).y(), static_cast<qreal>(getPortSize(actualPort).width()), static_cast<qreal>(getPortSize(actualPort).height())});
}

void GraphModelVisualizer::insertAllFilterPorts()
{
    for (int actualPort = 0; actualPort < getNumberOfPorts(); actualPort++)
    {
        if (m_actualGraph.ports.at(actualPort).type == 5)
        {
            insertOneComment(actualPort);
        }
        else
        {
            insertOneFilterPort(actualPort);
        }
    }
    getInternalPortList(m_filterPorts);
    // Insert all filterPortPartner, if type = 2 --> a lot of partner, if type = 3 --> one partner
    for ( auto filterPort : m_filterPorts)
    {
        if ( filterPort->type() == 3)
        {
            for (const auto filterPortPartner : m_filterPorts)
            {
                if ( filterPort->ID() != filterPortPartner->ID())
                {
                    if ( filterPort->getLabel() == filterPortPartner->getLabel())
                    {
                        if (filterPort->gotPartner() == false)
                        {
                            filterPort->setGotPartner(true);
                        }
                        filterPort->insertPartner(filterPortPartner);
                    }
                }
            }
        }
        if ( filterPort->type() == 2)
        {
            auto filterPortPartner = std::find_if(m_filterPorts.begin(), m_filterPorts.end(), [filterPort](FilterPort* const &actualFilterPort) {return (actualFilterPort->getLabel() == filterPort->getLabel() && actualFilterPort->ID() != filterPort->ID() && actualFilterPort->type() != filterPort->type());});
            if (filterPortPartner != m_filterPorts.end())
            {
                filterPort->setGotPartner(true);
                filterPort->insertPartner(m_filterPorts.at(std::distance(m_filterPorts.begin(), filterPortPartner)));
            }
        }
    }
}

void GraphModelVisualizer::insertMacros()
{
    if (!m_macroController)
    {
        return;
    }
    for (const auto &macro : m_actualGraph.macros)
    {
        m_macroController->insertMacro(macro);
    }
    for (const auto &connector : m_actualGraph.inConnectors)
    {
        m_macroController->insertMacroConnector(connector, qan::NodeItem::Dock::Right, qan::PortItem::Type::Out);
    }
    for (const auto &connector : m_actualGraph.outConnectors)
    {
        m_macroController->insertMacroConnector(connector, qan::NodeItem::Dock::Left, qan::PortItem::Type::In);
    }
}

namespace
{

template <typename T>
typename std::vector<T*>::const_iterator findPosition(const QUuid &id, const std::vector<T*> &nodes)
{
    return std::find_if(nodes.begin(), nodes.end(), [id](T* const &node){ return node->ID() == id; });
}

qan::Node *findNode(FilterGraph *filterGraph, const QUuid &id)
{
    for (const auto &nodePtr : qAsConst(filterGraph->get_nodes()))
    {
        if (auto filter = dynamic_cast<FilterNode*>(nodePtr); filter && filter->ID() == id)
        {
            return nodePtr;
        }
        if (auto macro = dynamic_cast<FilterMacro*>(nodePtr); macro && macro->ID() == id)
        {
            return nodePtr;
        }
        if (auto macroConnector = dynamic_cast<FilterMacroConnector*>(nodePtr); macroConnector && macroConnector->ID() == id)
        {
            return nodePtr;
        }
    }
    return nullptr;
}

}

void GraphModelVisualizer::insertOnePipePortWithConnector(const fliplib::Pipe& pipe, const std::vector<FilterPort *>& ports)
{
    const auto &pipeID = precitec::storage::compatibility::toQt(pipe.id);
    const auto &pipeSourceID = precitec::storage::compatibility::toQt(pipe.sender);
    const auto &pipeSourceConnectorID = precitec::storage::compatibility::toQt(pipe.senderConnectorId);
    const auto &pipeReceiverID = precitec::storage::compatibility::toQt(pipe.receiver);
    const auto &pipeReceiverConnectorID = precitec::storage::compatibility::toQt(pipe.receiverConnectorId);

    auto senderNode = findNode(m_filterGraph, pipeSourceID);
    auto receiverNode = findNode(m_filterGraph, pipeReceiverID);

    if (senderNode && receiverNode)
    {
        auto edge = m_filterGraph->insertEdge(senderNode, receiverNode);
        edge->setLabel(pipeID.toString());
        auto senderConnector = searchConnector(senderNode, pipeSourceConnectorID);
        auto receiverConnector = searchConnector(receiverNode, pipeReceiverConnectorID);
        if ( senderConnector && receiverConnector)
        {
            m_filterGraph->bindEdge(edge, senderConnector, receiverConnector);
        }
        else if (senderConnector)
        {
            m_filterGraph->bindEdgeSource(edge, senderConnector);
        }
        else if (receiverConnector)
        {
            m_filterGraph->bindEdgeDestination(edge, receiverConnector);
        }
        if (pipe.extensions.size() > 1)
        {
            m_filterGraph->removeEdge(edge);
            //pipe->getItem()->setVisible(false);
            for (const auto &extension : pipe.extensions)
            {
                if (extension.type == 1)
                {
                    if (auto it = findPosition(precitec::storage::compatibility::toQt(extension.localScope), ports); it != ports.end())
                    {
                        auto port = *it;
                        if (port && !(port->getUuid(UUIDs::receiverFilterConnectorUUID) == pipeReceiverConnectorID && port->getUuid(UUIDs::receiverFilterInstanceUUID) == pipeReceiverID))
                        {
                            auto portPipe = m_filterGraph->insertEdge(port, receiverNode);
                            if (receiverConnector)
                            {
                                receiverConnector->getInEdgeItems().clear();            //FIXME Results in a crash? (New filters have no ports, so check if a filter without ports) Clear inEdgeItems because pipe was bind to connector, after removing the pipe, clear the getInEdgeItems
                            }
                            if (receiverConnector)
                            {
                                m_filterGraph->bindEdgeDestination(portPipe, receiverConnector);
                            }
                        }
                        else
                        {
                            //qDebug() << "Port not found!";
                        }
                    }
                }
            }
        }
    }
    else
    {
        qDebug() << "!Warning: Pipe doesn't have valid sender or receiver node!";
    }
}

namespace
{
template <typename T>
void insertAllPipesWithPortsImpl(const std::vector<T*>& copiedNodes, const std::vector<FilterPort*>& copiedPorts, FilterGraph* filterGraph)
{
    //copiedNodes contains all new ports and filters for connecting the pipes!
    for (const auto &port : copiedPorts)
    {
        auto senderIt = findPosition(port->getUuid(UUIDs::senderFilterInstanceUUID), copiedNodes);
        auto receiverIt = findPosition(port->getUuid(UUIDs::receiverFilterInstanceUUID), copiedNodes);
        if (port->type() == 3 && senderIt != copiedNodes.end())
        {
            auto senderNode = *senderIt;    //FIXME do it like searchConnector
            auto pipe = filterGraph->insertEdge(senderNode, port);
            auto senderConnector = searchConnector(senderNode, port->getUuid(UUIDs::senderFilterConnectorUUID));
            if (senderConnector)
            {
                port->setDataType(senderConnector->connectorType());
                filterGraph->bindEdgeSource(pipe, senderConnector);
            }
        }
        else if (port->type() == 2 && receiverIt != copiedNodes.end())
        {
            auto receiverNode = *receiverIt;    //FIXME do it like searchConnector
            auto pipe = filterGraph->insertEdge(port, receiverNode);
            auto receiverConnector = searchConnector(receiverNode, port->getUuid(UUIDs::receiverFilterConnectorUUID));
            if (receiverConnector)
            {
                filterGraph->bindEdgeDestination(pipe, receiverConnector);
            }
        }
    }
    for (const auto port : copiedPorts)
    {
        if (port->type() == 3)
        {
            port->setDataTypeOfFilterPortPartner();
        }
        else if (port->type() == 2 && port->gotPartner() && port->getUuid(UUIDs::senderFilterInstanceUUID).isNull())    //Check if port with type == 2 (Output port) has port partner and check if sender IDs are set -> fix for all graphs to avoid a corrupted graph after deleting a connection which is defined by a port.
        {
            auto partnerPort = port->getOneFilterPortPartner();
            port->setUuid(UUIDs::senderFilterInstanceUUID, partnerPort->getUuid(UUIDs::senderFilterInstanceUUID));
            port->setUuid(UUIDs::senderFilterConnectorUUID, partnerPort->getUuid(UUIDs::senderFilterConnectorUUID));
        }
    }
}
}

void GraphModelVisualizer::insertAllPipesWithPorts(const std::vector<FilterNode*> &copiedNodes, const std::vector<FilterPort*> &copiedPorts)
{
    insertAllPipesWithPortsImpl(copiedNodes, copiedPorts, m_filterGraph);
}

void GraphModelVisualizer::insertAllPipesWithPorts(const std::vector<FilterMacro*>& copiedNodes, const std::vector<FilterPort*>& copiedPorts)
{
    insertAllPipesWithPortsImpl(copiedNodes, copiedPorts, m_filterGraph);
}

void GraphModelVisualizer::insertAllPipesWithPorts()
{
    //TODO if a graph is not complete and workable there must be a tag or any info and then use the information to skip some uncomplete things.
    std::vector<FilterNode*> nodes;
    getInternalNodeList(nodes);
    std::vector<FilterPort*> ports;
    getInternalPortList(ports);

    insertAllPipesWithPorts(nodes, ports);

    std::vector<FilterMacro*> macros;
    getInternalNodeList(macros);
    insertAllPipesWithPorts(macros, ports);

    //Pipes
    for (const auto &pipe : m_actualGraph.pipes)
    {
        insertOnePipePortWithConnector(pipe, ports);
    }
}

void GraphModelVisualizer::insertPipes ( fliplib::Pipe newPipe )
{
    m_actualGraph.pipes.push_back(newPipe);
}

namespace
{
template <typename T>
void getInternalNodeListImpl(std::vector<T*>& nodes, FilterGraph* filterGraph)
{
    //Stolen from qanGraph function groupAt() and adapted
    nodes.clear();
    nodes.reserve(static_cast<unsigned int>(filterGraph->get_nodes().size()));
    for (const auto &nodesPtr : qAsConst(filterGraph->get_nodes().getContainer()))
    {
        if (auto node = dynamic_cast<T*>(nodesPtr))          //gives you Null (!=false) if cast is impossible
        {
            nodes.push_back(node);
        }
    }
}

}

void GraphModelVisualizer::getInternalNodeList(std::vector<FilterNode *> &nodes)
{
    getInternalNodeListImpl(nodes, m_filterGraph);
}

void GraphModelVisualizer::getInternalNodeList(std::vector<FilterMacro*>& nodes)
{
    getInternalNodeListImpl(nodes, m_filterGraph);
}

void GraphModelVisualizer::getInternalPortList(std::vector<FilterPort *> &ports)
{
    getInternalNodeListImpl(ports, m_filterGraph);
}

void GraphModelVisualizer::updateGraphLoaderWithModifiedGraph(bool changeUUID)
{
    if (changeUUID)
    {
        Poco::UUIDGenerator uuidGenerator;
        m_actualGraph.id = uuidGenerator.createRandom();
        m_actualGraph.parameterSet = uuidGenerator.createRandom();
        bool pipeIDsChanged = false;
        for (auto &instanceFilter : m_actualGraph.instanceFilters)
        {
            auto instanceFilterID = instanceFilter.id;
            instanceFilter.id = uuidGenerator.createRandom();
            //Check if there is a port, if not then change the pipes!
            if (m_actualGraph.ports.size() == 0)
            {
                for (auto &pipe : m_actualGraph.pipes)
                {
                    if (pipe.sender == instanceFilterID)
                    {
                        pipe.sender = instanceFilter.id;
                    }
                    else if (pipe.receiver == instanceFilterID)
                    {
                        pipe.receiver = instanceFilter.id;
                    }
                    pipe.id = uuidGenerator.createRandom();
                }
            }
            else
            {
                //Find the old id in the ports and replace them with the new id
                for (auto &port : m_actualGraph.ports)
                {
                    auto portID = port.id;
                    if (port.senderInstanceFilterId == instanceFilterID)
                    {
                        port.senderInstanceFilterId = instanceFilter.id;
                    }
                    else if (port.receiverInstanceFilterId == instanceFilterID)
                    {
                        port.receiverInstanceFilterId = instanceFilter.id;
                    }
                    port.id = uuidGenerator.createRandom();
                    //Check pipes to change the localScope of GraphItemExtensions with the type 1
                    for (auto &pipe : m_actualGraph.pipes)
                    {
                        if (pipe.extensions.size() == 2)
                        {
                            if (pipe.extensions.at(1).localScope == portID)
                            {
                                pipe.extensions.at(1).localScope = port.id;
                                pipe.extensions.at(1).id = uuidGenerator.createRandom();
                            }
                        }
                        //Find the old id in the pipes and replace them with the new id
                        if (pipe.sender == instanceFilterID)
                        {
                            pipe.sender = instanceFilter.id;
                        }
                        else if (pipe.receiver == instanceFilterID)
                        {
                            pipe.receiver = instanceFilter.id;
                        }
                        if (!pipeIDsChanged)
                        {
                            pipe.id = uuidGenerator.createRandom();
                            if (!pipe.extensions.empty())
                            {
                                pipe.extensions.at(0).id = uuidGenerator.createRandom();
                            }
                        }
                    }
                    pipeIDsChanged = true;          //Avoid of changing pipeID and PipeExtensiondID every cycle if the type is 7
                }
            }
            //Change the ids of attribute and variant
            auto instanceVariantID = uuidGenerator.createRandom();
            for (auto &attribute : instanceFilter.attributes)
            {
                attribute.instanceAttributeId = uuidGenerator.createRandom();
                attribute.instanceVariantId = instanceVariantID;
            }
        }
    }
    m_graphLoader->setActualGraphContainer(m_actualGraph);
    m_graphLoader->setGraphID(getActualGraphUUID());
}

QUuid GraphModelVisualizer::getActualGraphUUID()
{
    return precitec::storage::compatibility::toQt(m_actualGraph.id);
}

void GraphModelVisualizer::showGraph()
{
    QTimer::singleShot(0, this, &GraphModelVisualizer::initGraph);
}

FilterNode* GraphModelVisualizer::getFilterGraphNode( int index)
{
    std::vector<FilterNode*> filterNodes;
    getInternalNodeList(filterNodes);
    return filterNodes.at(index);
}

FilterPort * GraphModelVisualizer::getFilterGraphPort(int index)
{
    return m_filterPorts.at(index);
}

void GraphModelVisualizer::clearFilterGraph()
{
    m_filterGraph->clear();
}

QString GraphModelVisualizer::getFileName() const
{
    return QString::fromStdString(m_actualGraph.fileName).remove(".xml");
}

void GraphModelVisualizer::setGraphName(const QString& graphName)
{
    const auto graphNameStd = graphName.toStdString();
    if (m_actualGraph.name != graphNameStd)
    {
        m_actualGraph.name = graphNameStd;
        filterGraph()->setGraphName(graphName);
        setGraphEdited(true);
    }
}

QString GraphModelVisualizer::getFilePath() const
{
    return QFileInfo{QString::fromStdString(m_actualGraph.path)}.absolutePath();
}

void GraphModelVisualizer::setFileInfo(QString path, QString name)
{
    m_actualGraph.path = path.remove(name).toStdString();
    m_actualGraph.fileName = name.toStdString();
}

template <typename T>
void GraphModelVisualizer::updatePositionImpl(QObject* node)
{
    GraphHelper helper{&m_actualGraph};
    if (auto filterNode = qobject_cast<T*>(node))
    {
        if (auto nodeCpp = helper.find(filterNode))
        {
            QPointF point = helper.positionToGroup(filterNode, {filterNode->getItemGeometry().x(),filterNode->getItemGeometry().y()});
            if (m_gridSize != 1 && m_useGridSizeAutomatically)
            {
                alignToGridHorizontal(point);
                alignToGridVertical(point);
                filterNode->getItem()->setRect({point.x(), point.y(), filterNode->getItemGeometry().width(), filterNode->getItemGeometry().height()});
            }
            nodeCpp->position.x = point.x();
            nodeCpp->position.y = point.y();
        }
    }
}

void GraphModelVisualizer::updatePosition(QObject* node)
{
    if ( m_initializingGraph )
    {
        return;
    }

    updatePositionImpl<FilterNode>(node);
    updatePositionImpl<FilterPort>(node);
    updatePositionImpl<FilterComment>(node);
    updatePositionImpl<FilterMacro>(node);
    if (auto filterGroup = qobject_cast<FilterGroup*>(node))
    {
        QPointF point = {filterGroup->getItemGeometry().x(), filterGroup->getItemGeometry().y()};
        if (m_gridSize != 1 && m_useGridSizeAutomatically)
        {
            alignToGridHorizontal(point);
            alignToGridVertical(point);
            filterGroup->setItemGeometry({point.x(), point.y(), filterGroup->getItemGeometry().width(), filterGroup->getItemGeometry().height()});
        }
        m_groupController->updateContentPosition(filterGroup);
    }

    setGraphEdited(true);
}

void GraphModelVisualizer::updateFilterLabel(FilterNode *node, const QString &label)
{
    if (auto filter = GraphHelper{&m_actualGraph}.find(node))
    {
        filter->name = label.toStdString();
    }
    node->setLabel(label);
    setGraphEdited(true);
}

void GraphModelVisualizer::updateFilterPosition(FilterNode *node, const QPointF &point)
{
    GraphHelper{&m_actualGraph}.updatePosition(node, point);
    setGraphEdited(true);
}

void GraphModelVisualizer::updateCommentPosition(FilterComment* comment, const QPointF &point)
{
    GraphHelper{&m_actualGraph}.updatePosition(comment, point);
    setGraphEdited(true);
}

void GraphModelVisualizer::updateCommentSize(FilterComment* comment, const QSizeF &size)
{
    if (auto c = GraphHelper{&m_actualGraph}.find(comment))
    {
        c->position.width = size.width();
        c->position.y = size.height();
    }
    comment->setItemGeometry(comment->getItemGeometry().x(), comment->getItemGeometry().y(), size.width(), size.height());
    setGraphEdited(true);
}

void GraphModelVisualizer::updatePortPosition(FilterPort *port, const QPointF &point)
{
    GraphHelper{&m_actualGraph}.updatePosition(port, point);
    setGraphEdited(true);
}

void GraphModelVisualizer::updatePortLabel(FilterPort *port, const QString &label)
{
    if (port->type() != 3)
    {
        return;
    }
    GraphHelper helper{&m_actualGraph};
    if (auto p = helper.find(port))
    {
        p->receiverName = label.toStdString();
    }
    port->setLabel(label);
    if (port->gotPartner())
    {
        for (auto portPartner : port->getFilterPortPartner())
        {
            portPartner->setLabel(label);
            if (auto partner = helper.find(portPartner))
            {
                partner->receiverName = label.toStdString();
            }
        }
    }
    setGraphEdited(true);
}

void GraphModelVisualizer::updateFilterAttributes(FilterNode* node, const QUuid& parameterUuid, int parameterValue)
{
    const auto attributeUuid = precitec::storage::compatibility::toPoco(parameterUuid);
    if (auto nodeCpp = GraphHelper{&m_actualGraph}.find(node))
    {
        auto attribute = std::find_if(nodeCpp->attributes.begin(), nodeCpp->attributes.end(), [attributeUuid](fliplib::InstanceFilter::Attribute const &actualAttribute){return actualAttribute.instanceAttributeId == attributeUuid;});
        if (attribute != nodeCpp->attributes.end())
        {
            attribute->value = parameterValue;
        }
    }
}

void GraphModelVisualizer::updatePortAttributes(FilterPort* port, FilterPort* portPartner)
{
    if (port->type() != 2 || !m_pipeController)
    {
        return;
    }
    if (auto p = GraphHelper{&m_actualGraph}.find(port))
    {
        p->receiverName = portPartner->getLabel().toStdString();     //Name of the port
        //nodeCpp->senderName = "";      //FIXME Unimportant because this parameter character is undefined
        p->senderConnectorId = precitec::storage::compatibility::toPoco(portPartner->getUuid(UUIDs::senderFilterConnectorUUID));
        p->senderInstanceFilterId = precitec::storage::compatibility::toPoco(portPartner->getUuid(UUIDs::senderFilterInstanceUUID));
        //qDebug() << "Found Portpartner to insert changes!";
    }
    if (port->gotPartner())
    {
        if (port->getOneFilterPortPartner()->dataType() != portPartner->dataType())
        {
            m_pipeController->deleteEdges(port->get_out_edges());
        }
        port->getOneFilterPortPartner()->removeFilterPortPartner(port->ID());
    }
    port->insertPartner(portPartner);
    portPartner->insertPartner(port);
    port->setLabel(portPartner->getLabel());
    port->setUuid(UUIDs::senderFilterInstanceUUID, portPartner->getUuid(UUIDs::senderFilterInstanceUUID));
    port->setUuid(UUIDs::senderFilterConnectorUUID, portPartner->getUuid(UUIDs::senderFilterConnectorUUID));

    // update pipes
    if (portPartner->get_in_edges().size() != 0)
    {
        const auto portId = storage::compatibility::toPoco(port->ID());
        auto sourceNode = portPartner->get_in_edges().at(0)->getSource();
        auto sourceFilter = qobject_cast<FilterNode*>(sourceNode);
        auto sourceMacro = qobject_cast<FilterMacro*>(sourceNode);
        auto sourceConnectorItem = portPartner->get_in_edges().at(0)->getItem()->getSourceItem();
        auto sourceConnectorRight = qobject_cast<FilterConnector*> (sourceConnectorItem);
        for (auto &pipe : m_actualGraph.pipes)
        {
            if (pipe.extensions.empty())
            {
                continue;
            }
            for (auto extension : pipe.extensions)
            {
                if (extension.type != 1)
                {
                    continue;
                }
                if (extension.localScope != portId)
                {
                    continue;
                }
                pipe.sender = precitec::storage::compatibility::toPoco(sourceFilter ? sourceFilter->ID() : sourceMacro->ID());
                pipe.senderConnectorId = precitec::storage::compatibility::toPoco(sourceConnectorRight->ID());
                pipe.senderConnectorName = sourceConnectorRight->getLabel().toStdString();
                break;
            }
        }
    }
    setGraphEdited(true);
}

void GraphModelVisualizer::updateCommentAttributes(FilterComment* comment, const QString& text)
{
    if (auto nodeCpp = GraphHelper{&m_actualGraph}.find(comment))
    {
        nodeCpp->text = text.toStdString();
        comment->setText(text);
        setGraphEdited(true);
    }
}

int GraphModelVisualizer::getComponentNumber(const QString& component)
{
    if (component == "Filter_Bridges")
    {
        return 0;
    }
    else if (component == "Filter_Calibration")
    {
        return 1;
    }
    else if (component == "Filter_GapTracking")
    {
        return 2;
    }
    else if (component == "Filter_ImageProcessing")
    {
        return 3;
    }
    else if (component == "Filter_ImageSource")
    {
        return 4;
    }
    else if (component == "Filter_LineGeometry")
    {
        return 5;
    }
    else if (component == "Filter_LineTracking")
    {
        return 6;
    }
    else if (component == "Filter_PoreAnalysis")
    {
        return 7;
    }
    else if (component == "Filter_Results")
    {
        return 8;
    }
    else if (component == "Filter_SampleSource")
    {
        return 9;
    }
    else if (component == "Filter_SeamSearch")
    {
        return 10;
    }
    else if (component == "Filter_Utility")
    {
        return 11;
    }
    else
    {
        return 12;
    }
}

Poco::UUID GraphModelVisualizer::getComponentID(int component)
{
    switch (component)
    {
        case bridges : return Poco::UUID{"3899ECCE-4F8D-4022-8A5F-641862372EA8"};
        case calibration : return Poco::UUID{"DA1B290A-47D9-4586-994E-57C57BD17788"};
        case gapTracking : return Poco::UUID{"1E017E61-6691-42FE-895B-E1EE84E9CE32"};
        case imageProcessing : return Poco::UUID{"8F9FEBD0-B0AC-451F-ACE1-F694225689EE"};
        case imageSource : return Poco::UUID{"609D64FF-0F63-4191-B5F2-B67E13116582"};
        case lineGeometry : return Poco::UUID{"605E3F32-D5C5-4325-A39D-2B2A47ECC6D8"};
        case lineTracking : return Poco::UUID{"E4787B8F-9394-4F9D-8518-1F4FA99FD673"};
        case poreAnalysis : return Poco::UUID{"AC710F18-4E1C-4F86-A76F-15D8CC21AAA8"};
        case results : return Poco::UUID{"4462301F-7ABD-46FD-AC5B-C6DAB54FAA94"};
        case sampleSource : return Poco::UUID{"5229D321-9BD9-4BB5-89BA-02BC1313B038"};
        case seamSearch : return Poco::UUID{"196716A1-0152-4736-824B-D30E99A48BFC"};
        case utility : return Poco::UUID{"5E6DF79F-39E2-4A9F-A76F-82E884AFE0E9"};
        default: return Poco::UUID{"00000000-0000-0000-0000-000000000000"};
    }
}

std::string GraphModelVisualizer::getComponentVersion(int component)
{
    switch (component)
    {
        case 0 : return {"Version 0.0.1"};
        case 1 : return {"Calibration v1"};
        case 2 : return {"Version 0.0.1"};
        case 3 : return {"Version 0.0.1"};
        case 4 : return {"Version 0.0.1"};
        case 5 : return {"Version 0.1.0"};
        case 6 : return {"Version 0.0.1"};
        case 7 : return {"Version 0.1.0"};
        case 8 : return {"Version 0.1.0"};
        case 9 : return {"Version 0.0.1"};
        case 10 : return {"Version 0.1.0"};
        case 11 : return {"Version 0.0.1"};
        default: return {"Version 0.0.0"};
    }
}

void GraphModelVisualizer::insertNewCppFilter(FilterNode* newFilter, const int actualFilterTypePos)
{
    //FilterDescription
    fliplib::FilterDescription newFilterDescription;
    newFilterDescription.id = precitec::storage::compatibility::toPoco(newFilter->typeID());              //Equal to the filterTypeID
    newFilterDescription.name = newFilter->type().toStdString();           //Equal to the filterType
    auto componentNumber = getComponentNumber(getFilterLibName(actualFilterTypePos));
    newFilterDescription.version = getComponentVersion(componentNumber);
    newFilterDescription.component = getFilterLibName(actualFilterTypePos).toStdString();
    newFilterDescription.componentId = getComponentID(componentNumber);

    //InstanceFilter
    fliplib::InstanceFilter newFilterInstance;
    newFilterInstance.filterId = precitec::storage::compatibility::toPoco(newFilter->typeID());
    newFilterInstance.id = precitec::storage::compatibility::toPoco(newFilter->ID());
    newFilterInstance.name = newFilter->getLabel().toStdString();
    newFilterInstance.group = newFilter->groupID();
    newFilterInstance.position = fliplib::Position{static_cast<int>(newFilter->getItemGeometry().x()), static_cast<int>(newFilter->getItemGeometry().y()), static_cast<int>(newFilter->getItemGeometry().width()), static_cast<int>(newFilter->getItemGeometry().height())};
    //Get attributes from filtertypes
    auto variantID = getVariantIDs(actualFilterTypePos);
    const Poco::UUID instanceVariantID = Poco::UUIDGenerator().createRandom();
    newFilterInstance.attributes = fillAttribute(variantID, instanceVariantID);
    //Extensions are empty, because the extension just say its a filter

    //Insert in the actual graph
    m_actualGraph.filterDescriptions.push_back(newFilterDescription);
    m_actualGraph.instanceFilters.push_back(newFilterInstance);

    m_graphEdited = true;
    emit graphEditedChanged();
}

void GraphModelVisualizer::insertNewCppFilterPort(FilterPort* newPort)
{
    //Cpp structure port
    fliplib::Port newPortInstance;
    newPortInstance.id = precitec::storage::compatibility::toPoco(newPort->ID());
    newPortInstance.type = newPort->type();
    newPortInstance.receiverConnectorId = Poco::UUID("00000000-0000-0000-0000-000000000000");
    newPortInstance.receiverInstanceFilterId = Poco::UUID("00000000-0000-0000-0000-000000000000");
    newPortInstance.receiverName = newPort->getLabel().toStdString();
    newPortInstance.senderConnectorId = Poco::UUID("00000000-0000-0000-0000-000000000000");
    newPortInstance.senderInstanceFilterId = Poco::UUID("00000000-0000-0000-0000-000000000000");;
    newPortInstance.senderName = std::string{""};
    newPortInstance.text = std::string{""};
    newPortInstance.group = newPort->groupID();
    newPortInstance.position = fliplib::Position{static_cast<int>(newPort->getItemGeometry().x()), static_cast<int>(newPort->getItemGeometry().y()), static_cast<int>(newPort->getItemGeometry().width()), static_cast<int>(newPort->getItemGeometry().height())};
    //Extensions are empty, because the extension just say its a port

    //Insert in the actual graph
    m_actualGraph.ports.push_back(newPortInstance);

    m_graphEdited = true;
    emit graphEditedChanged();
}

void GraphModelVisualizer::insertNewCppFilterComment(FilterComment* newComment)
{
    //Cpp structure comment
    fliplib::Port newCommentInstance;
    newCommentInstance.id = precitec::storage::compatibility::toPoco(newComment->ID());
    newCommentInstance.type = 5;
    newCommentInstance.receiverConnectorId = Poco::UUID("00000000-0000-0000-0000-000000000000");;
    newCommentInstance.receiverInstanceFilterId = Poco::UUID("00000000-0000-0000-0000-000000000000");;
    newCommentInstance.receiverName = newComment->getLabel().toStdString();
    newCommentInstance.senderConnectorId = Poco::UUID("00000000-0000-0000-0000-000000000000");;
    newCommentInstance.senderInstanceFilterId = Poco::UUID("00000000-0000-0000-0000-000000000000");;
    newCommentInstance.senderName = std::string{};
    newCommentInstance.text = newComment->text().toStdString();
    newCommentInstance.group = newComment->groupID();
    newCommentInstance.position = fliplib::Position{static_cast<int>(newComment->getItemGeometry().x()), static_cast<int>(newComment->getItemGeometry().y()), static_cast<int>(newComment->getItemGeometry().width()), static_cast<int>(newComment->getItemGeometry().height())};;
    //Extensions are empty, because the extension just say its a comment

    //Insert in the actual graph
    m_actualGraph.ports.push_back(newCommentInstance);

    m_graphEdited = true;
    emit graphEditedChanged();
}

namespace
{
template <typename T>
T searchImpl(const QUuid &id, const std::vector<T> &elements)
{
    const auto pocoID = precitec::storage::compatibility::toPoco(id);
    auto it = std::find_if(elements.begin(), elements.end(), [pocoID] (const auto &element){return element.id == pocoID;});
    if (it != elements.end())
    {
        return *it;
    }
    return {};
}

}

fliplib::InstanceFilter GraphModelVisualizer::searchInstanceFilter(const QUuid& id)
{
    return searchImpl(id, m_actualGraph.instanceFilters);
}

fliplib::FilterDescription GraphModelVisualizer::searchFilterDescription(const QUuid& filterID)
{
    return searchImpl(filterID, m_actualGraph.filterDescriptions);
}

fliplib::Port GraphModelVisualizer::searchPortComment(const QUuid& id)
{
    return searchImpl(id, m_actualGraph.ports);
}

fliplib::Pipe GraphModelVisualizer::searchPipe(const QUuid& senderID, const QUuid& senderConnectorID, const QUuid& receiverID, const QUuid& receiverConnectorID)
{
    auto pocoSenderID = precitec::storage::compatibility::toPoco(senderID);
    auto pocoSenderConnectorID = precitec::storage::compatibility::toPoco(senderConnectorID);
    auto pocoReceiverID = precitec::storage::compatibility::toPoco(receiverID);
    auto pocoReceiverConnectorID = precitec::storage::compatibility::toPoco(receiverConnectorID);
    auto foundPipe = std::find_if(m_actualGraph.pipes.begin(), m_actualGraph.pipes.end(), [pocoSenderID, pocoSenderConnectorID, pocoReceiverID, pocoReceiverConnectorID](const auto &actualPipe){return actualPipe.sender == pocoSenderID && actualPipe.receiver == pocoReceiverID && actualPipe.senderConnectorId == pocoSenderConnectorID && actualPipe.receiverConnectorId == pocoReceiverConnectorID;});
    if (foundPipe != m_actualGraph.pipes.end())
    {
        return *foundPipe;
    }
    return {};
}

void GraphModelVisualizer::checkOtherPortConnections(const Poco::UUID& ID)
{
    const auto &portID = precitec::storage::compatibility::toQt(ID);
    getInternalPortList(m_filterPorts);

    auto port = std::find_if(m_filterPorts.begin(), m_filterPorts.end(), [portID](const auto &actualPort){return actualPort->ID() == portID;});
    if (port == m_filterPorts.end())
    {
        qDebug() << "QML Port doesn't exist!";
        return;
    }
    auto portInstance = *port;

    if (portInstance->get_in_edges().size() == 0)           //Sender or input filter should always be set.
    {
        portInstance->setUuid(UUIDs::senderFilterInstanceUUID, {});
        portInstance->setUuid(UUIDs::senderFilterConnectorUUID, {});
    }
    if (portInstance->get_out_edges().size() == 0)
    {
        portInstance->setUuid(UUIDs::receiverFilterInstanceUUID, {});
        portInstance->setUuid(UUIDs::receiverFilterConnectorUUID, {});
        qDebug() << "Out going edges doesn't exist!";
        return;
    }
    auto outGoingEdge = portInstance->get_out_edges().at(0);
    auto receiverFilter = outGoingEdge->getDestination();
    if (!receiverFilter)
    {
        qDebug() << "Receiver filter doesn't exist!";
        return;
    }
    auto receiverFilterInstance = dynamic_cast<FilterNode*>(receiverFilter);
    auto receiverMacro = dynamic_cast<FilterMacro*>(receiverFilter);
    if (!receiverFilterInstance && !receiverMacro)
    {
        qDebug() << "Receiver filter instance doesn't exist!";
        return;
    }
    auto receiverConnector = outGoingEdge->getItem()->getDestinationItem();
    if (!receiverConnector)
    {
        qDebug() << "Receiver connector doesn't exist!";
        return;
    }
    auto receiverConnectorInstance = dynamic_cast<FilterConnector*> (receiverConnector);
    if (!receiverConnectorInstance)
    {
        qDebug() << "Receiver connector instance doesn't exist!";
        return;
    }
    const auto receiverId = receiverFilterInstance ? receiverFilterInstance->ID() : receiverMacro->ID();
    portInstance->setUuid(UUIDs::receiverFilterInstanceUUID, receiverId);
    portInstance->setUuid(UUIDs::receiverFilterConnectorUUID, receiverConnectorInstance->ID());
    auto pocoFilterID = precitec::storage::compatibility::toPoco(receiverId);
    auto pocoConnectorID = precitec::storage::compatibility::toPoco(receiverConnectorInstance->ID());
    auto foundPort = std::find_if(m_actualGraph.ports.begin(), m_actualGraph.ports.end(), [ID](const auto &actualPort){return actualPort.id == ID;});
    if (foundPort != m_actualGraph.ports.end())
    {
        foundPort->receiverInstanceFilterId = pocoFilterID;
        foundPort->receiverConnectorId = pocoConnectorID;
    }
}

void GraphModelVisualizer::alignToGridHorizontal(QPointF &point)
{
    auto deltaToLastX = (static_cast<int>(point.x())%m_gridSize);
    auto deltaToNextX = (m_gridSize - deltaToLastX);

    if (deltaToLastX < deltaToNextX)
    {
        point.setX(point.x() - deltaToLastX);
    }
    else
    {
        point.setX(point.x() + deltaToNextX);
    }
}

template <typename T>
void GraphModelVisualizer::alignSelectionHorizontal(qan::Node *node)
{
    GraphHelper helper{&m_actualGraph};
    if (auto filter = qobject_cast<T*>(node))
    {
        if (auto nodeCpp = helper.find(filter))
        {
            QPointF point = helper.positionToGroup(filter, {filter->getItemGeometry().x(), filter->getItemGeometry().y()});
            alignToGridHorizontal(point);
            filter->getItem()->setRect({point.x(), point.y(), filter->getItemGeometry().width(), filter->getItemGeometry().height()});
            nodeCpp->position.x = point.x();
        }
    }
}

void GraphModelVisualizer::alignSelectionHorizontal()
{
    if (m_gridSize == 1)
    {
        return;
    }
    const auto &selectedNodes = m_filterGraph->getSelectedNodes();
    const auto &selectedGroups = m_filterGraph->getSelectedGroups();

    if (selectedNodes.size())
    {
        for (const auto &element: selectedNodes.getContainer())
        {
            alignSelectionHorizontal<FilterNode>(element);
            alignSelectionHorizontal<FilterPort>(element);
            alignSelectionHorizontal<FilterComment>(element);
            alignSelectionHorizontal<FilterMacro>(element);
            setGraphEdited(true);
        }
    }
    if (selectedGroups.size())
    {
        for (const auto &element : selectedGroups.getContainer())
        {
            if (auto filterGroup = qobject_cast<FilterGroup*>(element))
            {
                QPointF point = {filterGroup->getItemGeometry().x(), filterGroup->getItemGeometry().y()};
                alignToGridHorizontal(point);
                filterGroup->setItemGeometry({point.x(), point.y(), filterGroup->getItemGeometry().width(), filterGroup->getItemGeometry().height()});
                setGraphEdited(true);
            }
        }
    }
}

void GraphModelVisualizer::alignToGridVertical(QPointF &point)
{
    auto deltaToLastY = (static_cast<int>(point.y())%m_gridSize);
    auto deltaToNextY = (m_gridSize - deltaToLastY);

    if (deltaToLastY < deltaToNextY)
    {
        point.setY(point.y() - deltaToLastY);
    }
    else
    {
        point.setY(point.y() + deltaToNextY);
    }
}

template <typename T>
void GraphModelVisualizer::alignSelectionVertical(qan::Node *node)
{
    GraphHelper helper{&m_actualGraph};
    if (auto filter = qobject_cast<T*>(node))
    {
        if (auto nodeCpp = helper.find(filter))
        {
            QPointF point = helper.positionToGroup(filter, {filter->getItemGeometry().x(), filter->getItemGeometry().y()});
            alignToGridVertical(point);
            filter->getItem()->setRect({point.x(), point.y(), filter->getItemGeometry().width(), filter->getItemGeometry().height()});
            nodeCpp->position.y = point.y();
        }
    }
}

void GraphModelVisualizer::alignSelectionVertical()
{
    if (m_gridSize == 1)
    {
        return;
    }
    const auto &selectedNodes = m_filterGraph->getSelectedNodes();
    const auto &selectedGroups = m_filterGraph->getSelectedGroups();

    if (selectedNodes.size())
    {
        for (const auto &element : selectedNodes.getContainer())
        {
            alignSelectionVertical<FilterNode>(element);
            alignSelectionVertical<FilterPort>(element);
            alignSelectionVertical<FilterComment>(element);
            alignSelectionVertical<FilterMacro>(element);
            setGraphEdited(true);
        }
    }
    if (selectedGroups.size())
    {
        for (const auto &element : selectedGroups.getContainer())
        {
            if (auto filterGroup = qobject_cast<FilterGroup*>(element))
            {
                QPointF point = {filterGroup->getItemGeometry().x(), filterGroup->getItemGeometry().y()};
                alignToGridVertical(point);
                filterGroup->setItemGeometry({point.x(), point.y(), filterGroup->getItemGeometry().width(), filterGroup->getItemGeometry().height()});
                setGraphEdited(true);
            }
        }
    }
}

void GraphModelVisualizer::setMacroController(MacroController *macroController)
{
    if (m_macroController == macroController)
    {
        return;
    }
    m_macroController = macroController;
    disconnect(m_macroControllerDestroyed);
    if (m_macroController)
    {
        m_macroControllerDestroyed = connect(m_macroController, &QObject::destroyed, this, std::bind(&GraphModelVisualizer::setMacroController, this, nullptr));
    }
    emit macroControllerChanged();
}

void GraphModelVisualizer::setPipeController(PipeController *controller)
{
    if (m_pipeController == controller)
    {
        return;
    }
    m_pipeController = controller;
    disconnect(m_pipeControllerDestroyed);
    if (m_pipeController)
    {
        m_pipeControllerDestroyed = connect(m_pipeController, &PipeController::destroyed, this, std::bind(&GraphModelVisualizer::setPipeController, this, nullptr));
    }
    else
    {
        m_pipeControllerDestroyed = {};
    }
    emit pipeControllerChanged();
}

void GraphModelVisualizer::setGraphNameEdited(bool set)
{
    if (m_graphNameEdited == set)
    {
        return;
    }
    m_graphNameEdited = set;
    emit graphNameEditedChanged();
}
