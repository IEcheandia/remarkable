#pragma once

#include <Poco/DynamicAny.h>
#include <Poco/UUID.h>
#include <vector>
#include "PipeConnector.h"

namespace fliplib
{

/**
 * Additional information for items in the graph.
 * Required by the Graph Editor
 **/
struct GraphItemExtension
{
    Poco::UUID id;                              //In XML-File: ItemsExtensionID
    Poco::UUID localScope;                      //In XML-File: PipeInPortID
    /**
     * From win/wmMain/Precitec.ExportImport/GraphVisualisation/Helper/DataAndTypes.cs
     * @li filterItem
     * @li pipeToInPortItem
     * @li portInItem
     * @li portOutItem
     * @li groupItem
     * @li annotationItem
     * @li subGraphItem
     * @li pipeItem
     *
     * Note: the subgraphitem does not correspond to the subgraph concept using bridge filters.
     **/
    int type;
};

/**
 * Description of a pipe in the GraphContainer
 **/
struct Pipe
{
    /**
     * UUID of the filter instance receiving this pipe.
     **/
    Poco::UUID receiver;
    /**
     * UUID of the filter instance sending this pipe.
     **/
    Poco::UUID sender;
    /**
     * Name of the receiver connector.
     **/
    std::string receiverConnectorName;
    /**
     * Group of the receiver connector.
     **/
    int receiverConnectorGroup;
    /**
     * Name of the sender connector
     **/
    std::string senderConnectorName;

    /**
     * The instance id of this pipe
     **/
    Poco::UUID id;
    /**
     * The id of the receiver connector
     **/
    Poco::UUID receiverConnectorId;
    /**
     * The id of the sender connector
     **/
    Poco::UUID senderConnectorId;
    /**
     * Dot.NET specific path describing the visual path
     **/
    std::string path;

    std::vector<GraphItemExtension> extensions;
};

struct Position {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

/**
 * Description of one filter instance
 **/
struct InstanceFilter
{
    /**
     * UUID of the filter (see FilterDescription::id)
     **/
    Poco::UUID filterId;
    /**
     * UUID of the filter instance.
     **/
    Poco::UUID id;
    /**
     * Name of the filter instance.
     **/
    std::string name;
    /**
     * The number of the group this instance is element of
     **/
    int group;

    /**
     * The position of this InstanceFilter in Graph Editor.
     **/
    Position position;

    /**
     * Description of an attribute of the InstanceFilter
     **/
    struct Attribute {
        /**
         * Id of the attribute
         **/
        Poco::UUID attributeId;
        /**
         * Unique id of this attribute instance
         **/
        Poco::UUID instanceAttributeId;
        /**
         * User readable name
         **/
        std::string name;
        /**
         * Value of the Attribute.
         **/
        Poco::DynamicAny value;
        int userLevel;
        bool visible;
        bool publicity;
        int blob = -1;
        bool hasBlob = false;

        /**
         * The instance variant this attribute belongs to.
         **/
        Poco::UUID instanceVariantId;
        /**
         * The variant this attribute belongs to.
         **/
        Poco::UUID variantId;
        /**
         * Name of a help file for this Instance attribute
         **/
        std::string helpFile;
    };
    /**
     * The Attributes of the InstanceFilter
     **/
    std::vector<Attribute> attributes;

    std::vector<GraphItemExtension> extensions;
};

/**
 * Generic description of a filter in the GraphContainer
 **/
struct FilterDescription
{
    /**
     * UUID of the filter
     **/
    Poco::UUID id;
    /**
     * Name of the filter
     **/
    std::string name;

    /**
     * Version string of the filter
     **/
    std::string version;

    /**
     * Component (that is library) name containing the filter.
     **/
    std::string component;
    /**
     * The id of the component (that is library) containing the filter.
     **/
    Poco::UUID componentId;

    friend bool operator==(const FilterDescription &left, const FilterDescription &right)
    {
        return left.id == right.id;
    }
};

struct FilterGroup
{
    int number;
    int parent;
    std::string name;
    Poco::UUID sourceGraph;
};

struct Port
{
    Poco::UUID id;
    int type;
    Poco::UUID receiverConnectorId;
    Poco::UUID receiverInstanceFilterId;
    std::string receiverName;
    Poco::UUID senderConnectorId;
    Poco::UUID senderInstanceFilterId;
    std::string senderName;
    std::string text;
    int group;
    Position position;
    std::vector<GraphItemExtension> extensions;
};

struct Macro
{
    /**
     * The id of the Macro which gets integrated into the FilterGraph
     **/
    Poco::UUID macroId;
    /**
     * The id of the specific Macro instance.
     * Multiple Macros of same type have same id, but different instanceIds.
     **/
    Poco::UUID id;
    /**
     * Group number.
     * Used as group number for all filters when resolving into a graph.
     **/
    int group;
    /**
     * The visual position.
     **/
    Position position;

    struct Connector
    {
        Poco::UUID id;
        std::string name;
        PipeConnector::DataType type;
        /**
         * Position is optional and only used for top level elements in graphs representing a macro.
         * Not used for a macro inside another graph
         **/
        Position position;
    };
    /**
     * Connectors for pipes going into the Macro
     **/
    std::vector<Connector> inConnectors;
    /**
     * Connectors for pipes going out of the Macro
     **/
    std::vector<Connector> outConnectors;

};

struct GenericType
{
    Poco::UUID id;
    std::string name;
    int enumType;
    bool visibility;
    bool plotable;
    bool saveType;
    bool selectable;
};

struct ErrorType : public GenericType
{
    bool hasColor = false;
    bool hasHardwareFunction = false;
    bool hasListOrder = false;
    int color = -1;
    int hardwareFunction = -1;
    int listOrder = -1;
};

struct ResultType : public ErrorType
{
    bool boundaryPlotable;
    bool globalBoundaryPlotable;
    bool hasMax = false;
    bool hasMin = false;
    int max = -1;
    int min = -1;
    std::string unit;
};

/**
 * Abstract description of a graph as parsed from Xml
 **/
struct GraphContainer
{
    /**
     * Name of the graph.
     **/
    std::string name;

    /**
     * Comment of the graph.
     **/
    std::string comment;

    /**
     * File name of the graph
     **/
    std::string fileName;

    /**
     * Path name of the graph
     **/
    std::string path;

    /**
     * Group this graph belongs to.
     **/
    std::string group;

    /**
     * UUID of the graph.
     **/
    Poco::UUID id;
    /**
     * The filters used in the graph
     **/
    std::vector<FilterDescription> filterDescriptions;
    /**
     * The concrete filter instances in the graph
     **/
    std::vector<InstanceFilter> instanceFilters;
    /**
     * The pipes connecting the filter instances
     **/
    std::vector<Pipe> pipes;
    /**
     * The groups into which the instance filters are sorted
     **/
    std::vector<FilterGroup> filterGroups;

    std::vector<Port> ports;

    std::vector<GenericType> sensors;

    std::vector<ErrorType> errors;

    std::vector<ResultType> results;

    std::vector<Macro> macros;

    std::vector<Macro::Connector> inConnectors;
    std::vector<Macro::Connector> outConnectors;

    /**
     * loaded parameterset id.
     **/
    Poco::UUID parameterSet;
};


}
