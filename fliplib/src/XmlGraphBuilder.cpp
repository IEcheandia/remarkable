/**
 *	@file
 *	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Sevitec, Stefan Birmanns (SB)
 * 	@date		2007 - 2013
 * 	@brief		Builds a signal processing graph.
 */

// clib includes
#include <sstream>
// poco includes
#include <Poco/Path.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/Node.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/Exception.h>
#include <Poco/UUIDGenerator.h>
// project includes
#include <fliplib/Exception.h>
#include <fliplib/XmlGraphBuilder.h>
#include <fliplib/Parameter.h>
#include <fliplib/ParameterContainer.h>
#include <module/moduleLogger.h>

using Poco::XML::Document;
using Poco::XML::Element;
using Poco::XML::NodeIterator;
using Poco::XML::NodeList;
using Poco::XML::Node;
using Poco::XML::AutoPtr;

using namespace precitec;

namespace fliplib {

/*static*/ std::set<std::string> XmlGraphBuilder::m_oLibsLoaded;


XmlGraphBuilder::XmlGraphBuilder()
{

} // CTor.




Poco::UUID XmlGraphBuilder::getUUID( const Poco::XML::Element* p_pElement, std::string p_oTag )
{
    std::string oStr = getString( p_pElement, p_oTag );
    return createValidUUID( oStr );

} // getUUID



std::string XmlGraphBuilder::getString( const Poco::XML::Element* p_pElement, std::string p_oTag )
{
	Element* pChild = p_pElement->getChildElement( p_oTag );
	if ( !pChild )
		throw fliplib::GraphBuilderException("Node <" + p_oTag + "> not found.");

	return pChild->innerText();

} // getString

std::string XmlGraphBuilder::getOptionalString( const Poco::XML::Element* p_pElement, std::string p_oTag )
{
	Element* pChild = p_pElement->getChildElement( p_oTag );
	if ( !pChild )
		return {};

	return pChild->innerText();

} // getString

bool XmlGraphBuilder::getBool(const Poco::XML::Element* p_pElement, std::string p_oTag)
{
    return getString(p_pElement, p_oTag) == std::string{"true"};
}

std::string XmlGraphBuilder::getComponentPath( const std::string& p_rComponentName )
{
	std::string oCompPath;

	// WM always sets WM_BASE_DIR, Filtertest and other tools might not ...
	if ( getenv( "WM_BASE_DIR" ) )
		oCompPath = std::string( getenv( "WM_BASE_DIR" ) );
	else
		oCompPath = std::string( "/wm_inst" );
	oCompPath.append( "/lib/lib" );
	oCompPath.append( p_rComponentName );
#if not defined(NDEBUG) and defined(__QNX__)
	oCompPath.append("_g");
#endif
	oCompPath.append(".so");

	return oCompPath;

} // getComponentPath



void XmlGraphBuilder::parameterizeFilter( BaseFilter* p_pFilter, const InstanceFilter &instance )
{
	// get all parameters that the filter needs
	ParameterContainer& pPC = p_pFilter->getParameters();
    // ok, now lets loop through all the parameters that are in the xml file ...
    const auto attributes = instance.attributes;
	for(const auto &attribute : attributes)
	{
        //wmLog( eDebug, "Filter %s - Attribute %s set with value %s\n", p_pFilter->id().toString(), oAttrName.c_str(), oAttrValue.c_str() );

        try
        {
            Parameter oParam = pPC.findParameter( attribute.name );
            const Poco::DynamicAny oValue( attribute.value );
            pPC.add( attribute.name, oParam.getType(), oValue );

        } catch( Poco::Exception& )
        {
            // Exception seems to be too hard, quite a number of filters seem to have registered in the db, that they do not have in the cpp code ...
            wmLog( eWarning, "Filter %s does not have attribute %s!\n", p_pFilter->name().c_str(), attribute.name.c_str() );

            // Exception:
            // std::stringstream oSS;
            // oSS << "Parameter error:: Name:'" << oAttrName << "' FilterID: '" << oAttrFilterID.toString() << "'" << std::endl;
            // throw fliplib::GraphBuilderException( oSS.str() );
        }

	} // for pAttributes

} // parameterize



Poco::UUID XmlGraphBuilder::createValidUUID(const std::string& p_rId)
{
	try
	{
		Poco::UUID uuid;
		// Kann fehlschlagen wenn die Struktur stimmt aber nur unerlaubte Zeichen verwendet werden.
		if (uuid.tryParse( p_rId ))
			return uuid;
	}
	catch(Poco::SyntaxException&)
	{
	}

	static const Poco::UUID uuidNsid("8ed82e4c-5dbb-4f24-b268-774821c983fe");

	// Versuche einen neue GUID aufgrund des Namenens zu erzeugen
	Poco::UUID uuid ( Poco::UUIDGenerator::defaultGenerator().createFromName( uuidNsid, p_rId ) );
	Poco::Thread::sleep(2); // to prevent multiple UUID generation at the same time and thus leading to identical GUIDs

	return uuid;

} // createValidUUID



void XmlGraphBuilder::loadFilter( FilterGraph* p_pFilterGraph, const FilterDescription &filterDescription, const InstanceFilter &instance )
{
	std::string oFilterName = filterDescription.name;
	std::string oFilterPath = getComponentPath( filterDescription.component );

	// Filterinstanz erstellen
	FilterHandle* pHandle = nullptr;
	try
	{
#if !defined(NDEBUG)
		if (m_oLibsLoaded.find(oFilterPath) == std::end(m_oLibsLoaded)) {
			std::ostringstream oMsg;
			oMsg << "1st load of '" << oFilterPath << "'\n";
			wmLog(eDebug, oMsg.str());
			m_oLibsLoaded.insert(oFilterPath);
		} // if
#endif // !defined(NDEBUG)
		pHandle = activate( oFilterPath, oFilterName );
	}
	catch (const Poco::LibraryLoadException& exc)
	{
        std::cout << exc.message() << " " << exc.what() << std::endl;
	}
	if (pHandle == nullptr)
	{
		std::ostringstream oMsg;
		oMsg << "Could not load '" << oFilterName + "' from '" << oFilterPath << "'.\n";
		wmLog(eWarning, oMsg.str());
	}
	else // otherwise we will have a crash if pHandle == nullptr
	{
		// Filter in Graph speichern. Pipes sind noch nicht verbunden
		p_pFilterGraph->insert(instance.id, pHandle);
		// Filter parametrieren
		parameterizeFilter(pHandle->getFilter(), instance);
	}

} // loadFilter



void XmlGraphBuilder::renderGraph(FilterGraph* p_pFilterGraph, const GraphContainer &graphContainer)
{
	// Loop through all pipes in the instanz pipe list
	for( const auto &pipe : graphContainer.pipes)
	{
		//
		// Get receiver filter and input pipe name (tag) and group
		//

		Poco::UUID oReceiverInstFilterID = pipe.receiver;
		std::string oInputPipeName = pipe.receiverConnectorName;

		int oGroup = pipe.receiverConnectorGroup;
		BaseFilter* pReceiver = p_pFilterGraph->find( oReceiverInstFilterID );
		if ( !pReceiver )
			throw fliplib::GraphBuilderException("Filter with ReceiverInstFilterID " + oReceiverInstFilterID.toString() + " not found!");

		//
		// Get sender filter and output pipe name
		//

		Poco::UUID oSenderInstFilterID = pipe.sender;
		std::string oOutputPipeName = pipe.senderConnectorName;
		BaseFilter* pSender = p_pFilterGraph->find( oSenderInstFilterID );
		if ( !pSender )
			throw fliplib::GraphBuilderException("Filter with SenderInstFilterID " + oSenderInstFilterID.toString() + " not found!");

		//
		// OK, now we have everything, lets find the output pipe in the fliplib filter and connect it to the input pipe of the other filter.
		//

		BasePipe* pPipeOut = pSender->findPipe( oOutputPipeName );
		if ( !pPipeOut )
			throw fliplib::GraphBuilderException("Filter " + oSenderInstFilterID.toString() + " has no outpipe with name " + oOutputPipeName + "!");

		pPipeOut->setTag( oInputPipeName );

		//std::stringstream oSS; oSS << "'" << pSender->name() << "' with '" << pReceiver->name() << "' via pipe '" << pPipeOut->name() << "' linked. tag:" << pPipeOut->tag() << " group:" << oGroup << "\n";
		//wmLog( eDebug, oSS.str() );

		pReceiver->connectPipe( pPipeOut, oGroup);

	} // for

} // renderGraph

GraphContainer XmlGraphBuilder::buildGraphDescription(const std::string &p_rUri)
{
	std::stringstream oSt; oSt << "XmlGraphBuilder::build( " << p_rUri << " )\n";
	wmLog( eDebug, oSt.str().c_str() );

	Poco::XML::DOMParser parser;
	parser.setFeature( Poco::XML::DOMParser::FEATURE_FILTER_WHITESPACE, true );

	// Konfigurationsfile oeffnen und parsen
	AutoPtr<Document> pDoc;
	try
	{
		pDoc = parser.parse(p_rUri);
	}
	catch(Poco::Exception& ex)
	{
		throw fliplib::GraphBuilderException("XmlGraphBuilder cannot open or parse xml file!", ex);
	}

	try
	{
	 	return buildFilterGraph(pDoc);
	}
	catch(Poco::Exception& ex)
	{
		throw fliplib::GraphBuilderException("XmlGraphBuilder cannot build filter from xml file!", ex);
	}
}

UpFilterGraph XmlGraphBuilder::build(const std::string& p_rUri)
{
	std::stringstream oSt; oSt << "XmlGraphBuilder::build( " << p_rUri << " )\n";
	wmLog( eDebug, oSt.str().c_str() );

	Poco::XML::DOMParser parser;
	parser.setFeature( Poco::XML::DOMParser::FEATURE_FILTER_WHITESPACE, true );

	// Konfigurationsfile oeffnen und parsen
	AutoPtr<Document> pDoc;
	try
	{
		pDoc = parser.parse(p_rUri);
	}
	catch(Poco::Exception& ex)
	{
		throw fliplib::GraphBuilderException("XmlGraphBuilder cannot open or parse xml file!", ex);
	}

	try
	{
	 	return buildFilterGraph(buildFilterGraph(pDoc));
	}
	catch(Poco::Exception& ex)
	{
		throw fliplib::GraphBuilderException("XmlGraphBuilder cannot build filter from xml file!", ex);
	}
	return nullptr;

} // build



SpFilterGraph XmlGraphBuilder::buildString(const std::string& p_rXml)
{
	Poco::XML::DOMParser parser;
	parser.setFeature( Poco::XML::DOMParser::FEATURE_FILTER_WHITESPACE, true );

	// String parsen
	try
	{
	 	AutoPtr<Document> pDoc = parser.parseString(p_rXml);
 		return buildFilterGraph(buildFilterGraph(pDoc));

	}
	catch(Poco::Exception& ex)
	{
		std::stringstream ss;
		ss << "XmlGraphBuilder::Fehler beim Parsen des XML Dokumentes::" << ex.displayText() << std::endl;
		ss << p_rXml;
		throw fliplib::GraphBuilderException(ss.str(), ex);
	}
	return nullptr;

} // buildString

Position parsePosition(Poco::XML::Element *positionElement)
{
    if (!positionElement)
    {
        return {};
    }
    Position position;
    position.x = std::stoi(positionElement->getAttribute(std::string("PosX")));
    position.y = std::stoi(positionElement->getAttribute(std::string("PosY")));
    position.width = std::stoi(positionElement->getAttribute(std::string("Breite")));
    position.height = std::stoi(positionElement->getAttribute(std::string("Hoehe")));
    return position;
};

void XmlGraphBuilder::buildPortList(GraphContainer &graph, Poco::XML::Element *pGraph, std::map<Poco::UUID, std::vector<GraphItemExtension>> &itemExtensionMap)
{
    Element* pGraphPortList = pGraph->getChildElement( std::string("GraphPortList") );
    if ( !pGraphPortList )
    {
        return;
    }

    auto portList = pGraphPortList->getElementsByTagName(std::string("PortItem"));
    graph.ports.reserve(portList->length());
    for (unsigned long i = 0; i < portList->length(); i++)
    {
        Element* port = static_cast<Element*>( portList->item(i) );
        auto grouping = port->getChildElement(std::string("PortGrouping"));

        auto uuid = getUUID(port, std::string("PortID"));

        graph.ports.emplace_back(Port{
            uuid,
            std::stoi(getString(port, std::string("PortItemType"))),
            getUUID(port, std::string("ReceiverConnectorID")),
            getUUID(port, std::string("ReceiverInstFilterID")),
            getString(port, std::string("ReceiverName")),
            getUUID(port, std::string("SenderConnectorID")),
            getUUID(port, std::string("SenderInstFilterID")),
            getString(port, std::string("SenderName")),
            getString(port, std::string("Text")),
            std::stoi(grouping->getAttribute(std::string("Group"))),
            parsePosition(port->getChildElement(std::string("PortDescription"))),
            itemExtensionMap[uuid]
        });
    }
}

void XmlGraphBuilder::readGenericType(Poco::XML::Element *element, GenericType &type)
{
    type.id = getUUID(element, std::string("UserTypID"));
    type.name = getString(element, std::string("UserTypName"));
    type.enumType = std::stoi(getString(element, std::string("EnumTyp")));
    type.visibility = getBool(element, std::string("OverviewVisibility"));
    type.plotable = getBool(element, std::string("Plotable"));
    type.saveType = getBool(element, std::string("Savetyp"));
    type.selectable = getBool(element, std::string("Selectable"));
}

void XmlGraphBuilder::readErrorType(Poco::XML::Element *element, ErrorType &type)
{
    readGenericType(element, type);
    try
    {
        type.color = std::stoi(getString(element, std::string("LineColor")));
        type.hasColor = true;
    } catch (...)
    {
    }
    try
    {
        type.hardwareFunction = std::stoi(getString(element, std::string("HASSPECIALHWFUNCTION")));
        type.hasHardwareFunction = true;
    } catch (...)
    {
    }
    try
    {
        type.listOrder = std::stoi(getString(element, std::string("LISTORDER")));
        type.hasListOrder = true;
    } catch (...)
    {
    }
}

void XmlGraphBuilder::buildSensorList(GraphContainer &graph, Poco::XML::Element *pGraph)
{
    auto sensorTypeList = pGraph->getChildElement(std::string("SensorTypList"));
    if (!sensorTypeList)
    {
        return;
    }
    auto sensorList = sensorTypeList->getElementsByTagName(std::string("sensor"));
    graph.sensors.reserve(sensorList->length());
    for (unsigned long i = 0; i < sensorList->length(); i++)
    {
        Element *sensor = static_cast<Element*>(sensorList->item(i));
        GenericType type;
        readGenericType(sensor, type);
        graph.sensors.push_back(type);
    }
}

void XmlGraphBuilder::buildErrorList(GraphContainer &graph, Poco::XML::Element *pGraph)
{
    auto errorTypeList = pGraph->getChildElement(std::string("FehlertypList"));
    if (!errorTypeList)
    {
        return;
    }
    auto errorList = errorTypeList->getElementsByTagName(std::string("fehler"));
    graph.errors.reserve(errorList->length());
    for (unsigned long i = 0; i < errorList->length(); i++)
    {
        Element *error = static_cast<Element*>(errorList->item(i));
        ErrorType type;
        readErrorType(error, type);
        graph.errors.push_back(type);
    }
}

void XmlGraphBuilder::buildResultList(GraphContainer &graph, Poco::XML::Element *pGraph)
{
    auto resultTypeList = pGraph->getChildElement(std::string("MesswerttypList"));
    if (!resultTypeList)
    {
        return;
    }
    auto resultList = resultTypeList->getElementsByTagName(std::string("messwert"));
    graph.results.reserve(resultList->length());
    for (unsigned long i = 0; i < resultList->length(); i++)
    {
        Element *result = static_cast<Element*>(resultList->item(i));
        ResultType type;
        readErrorType(result, type);
        type.boundaryPlotable = getBool(result, std::string("BoundaryPlotable"));
        type.globalBoundaryPlotable = getBool(result, std::string("GlobalBoundaryPlotable"));
        try
        {
            type.min = std::stoi(getString(result, std::string("Min")));
            type.hasMin = true;
        } catch (...)
        {
        }
        try
        {
            type.max = std::stoi(getString(result, std::string("Max")));
            type.hasMax = true;
        } catch (...)
        {
        }
        type.unit = getString(result, std::string("Unit"));
        graph.results.push_back(type);
    }
}

void XmlGraphBuilder::buildMacroList(GraphContainer &graph, Poco::XML::Element *pGraph)
{
    auto macrosElement = pGraph->getChildElement({"Macros"});
    if (!macrosElement)
    {
        return;
    }
    auto macroList = macrosElement->getElementsByTagName({"Macro"});
    graph.results.reserve(macroList->length());
    for (unsigned long i = 0; i < macroList->length(); i++)
    {
        auto macroElement = static_cast<Element*>(macroList->item(i));
        fliplib::Macro macro;
        macro.macroId = getUUID(macroElement, {"ID"});
        macro.id = getUUID(macroElement, {"InstanceID"});

        macro.group = addFilterGroup(graph, macroElement->getChildElement({"Group"}));
        macro.position = parsePosition(macroElement->getChildElement({"Position"}));

        buildConnectorList(macro.inConnectors, macroElement->getChildElement({"InConnectors"}));
        buildConnectorList(macro.outConnectors, macroElement->getChildElement({"OutConnectors"}));

        graph.macros.push_back(std::move(macro));
    }
}

void XmlGraphBuilder::buildConnectorList(std::vector<Macro::Connector> &connectors, Poco::XML::Element *connectorsElement)
{
    if (!connectorsElement)
    {
        return;
    }
    auto connectorList = connectorsElement->getElementsByTagName({"Connector"});
    connectors.reserve(connectorList->length());
    for (unsigned long i = 0; i < connectorList->length(); i++)
    {
        auto connectorElement = static_cast<Element*>(connectorList->item(i));
        connectors.emplace_back(fliplib::Macro::Connector{
            getUUID(connectorElement, {"ID"}),
            getString(connectorElement, {"Name"}),
            fliplib::PipeConnector::DataType(std::stoi(getString(connectorElement, {"Type"}))),
            parsePosition(connectorElement->getChildElement({"Position"}))
        });
    }
}

std::map<Poco::UUID, std::vector<GraphItemExtension>> XmlGraphBuilder::parseGraphItemExtensions(Poco::XML::Element *pGraph)
{
    // GraphItemsExtensionList
    Element* pGraphItemExtensionList = pGraph->getChildElement( std::string("GraphItemsExtensionList") );
    if ( !pGraphItemExtensionList )
    {
        return {};
    }
    auto graphItemExtensions{pGraphItemExtensionList->getElementsByTagName(std::string("GraphItemExtension"))};
    std::map<Poco::UUID, std::vector<GraphItemExtension>> itemExtensionMap;
    for (unsigned long i = 0; i < graphItemExtensions->length(); i++)
    {
        Element* extension = static_cast<Element*>( graphItemExtensions->item(i) );
        auto itemId = getUUID(extension, std::string("GraphEditItemID"));
        itemExtensionMap[itemId].emplace_back(GraphItemExtension{
            getUUID(extension, std::string("ItemsExtensionID")),
            getUUID(extension, std::string("PipeInPortID")),
            std::stoi(getString(extension, std::string("GraphEditItemType")))
        });
    }
    return itemExtensionMap;
}

int XmlGraphBuilder::addFilterGroup(GraphContainer &graph, Poco::XML::Element *grouping)
{
    if (!grouping)
    {
        return -1;
    }
    const auto groupNumber = std::stoi(grouping->getAttribute(std::string("Group")));
    if (std::none_of(graph.filterGroups.begin(), graph.filterGroups.end(), [groupNumber] (const FilterGroup &group) { return group.number == groupNumber; }))
    {
        auto groupName = grouping->getAttribute(std::string("GroupName"));
        if (groupName.empty())
        {
            groupName = std::string("Unnamed");
        }
        graph.filterGroups.emplace_back(FilterGroup{
            groupNumber,
            std::stoi(grouping->getAttribute(std::string("Parent"))),
            groupNumber == -1 ? std::string("Not grouped") : groupName
        });
    }
    return groupNumber;
}

GraphContainer XmlGraphBuilder::buildFilterGraph(const Poco::XML::Document* p_pDoc)
{
	//
	// Get pointers to the main elements of the file
	//

	// Graph
	Element* pGraph = p_pDoc->documentElement();
	if ( !pGraph )
		throw fliplib::GraphBuilderException("Node <GraphData> not found.");

	// Header
	Element* pHeader = pGraph->getChildElement( std::string("Header") );
	if ( !pHeader  )
	{
		throw fliplib::GraphBuilderException("Node <Header> not found.");
	}

    GraphContainer graph;

	// Check version number
	Element* pVersion = pHeader->getChildElement( std::string("Version") );
	if ( pVersion && pVersion->hasAttribute( std::string("Major") ) && pVersion->hasAttribute( std::string("Minor") ) )
	{
		int oMajor; std::stringstream( pVersion->getAttribute( std::string( "Major" ) ) ) >> oMajor;
		int oMinor; std::stringstream( pVersion->getAttribute( std::string( "Minor" ) ) ) >> oMinor;

		if ( (oMajor * 100 + oMinor) < 101 )
			throw fliplib::GraphBuilderException("XML-file is too old. Please re-export graph in wmMain!");
	}

    graph.name = getString(pHeader, std::string("GraphName"));
    graph.comment = getOptionalString(pGraph, std::string("Kommentar"));
    graph.group = getOptionalString(pGraph, std::string("Group"));

	// Header Info List
	Element* pHeaderFilterInfoList = pHeader->getChildElement( std::string("HeaderFilterInfoList") );
	if ( !pHeaderFilterInfoList  )
		throw fliplib::GraphBuilderException("Node <HeaderInfoList> not found.");

	// InstanzFilterList
	Element* pInstanzFilterList = pGraph->getChildElement( std::string("InstanzFilterList") );
	if ( !pInstanzFilterList )
		throw fliplib::GraphBuilderException("Node <InstanzFilterList> not found.");

	// InstanzAttributeList
	Element* pInstanzAttributeList = pGraph->getChildElement( std::string("InstanzAttributeList") );
	if ( !pInstanzAttributeList )
		throw fliplib::GraphBuilderException("Node <InstanzAttributeList> not found.");

	//
	// OK, now lets build the filter graph ...
	//

    // create a global map of all attributes at the first call of this function ...
    AutoPtr<NodeList> pAttributes = pInstanzAttributeList->getElementsByTagName( std::string("InstanzAttribute") );
    std::map<Poco::UUID, std::vector<InstanceFilter::Attribute>> attributes;
    unsigned int oCount = pAttributes->length();
    for(unsigned long i=0;i< oCount; i++)
    {
        // get attribute
        Element* pAttribute = static_cast<Element*>( pAttributes->item(i) );
        // get filter id of attribute
        Poco::UUID oAttrFilterID = getUUID( pAttribute, std::string("InstanceFilterID") );
        // store attribute
        int blob = -1;
        bool hasBlob = false;
        try {
            blob = std::stoi(getString(pAttribute, std::string("Blob")));
            hasBlob = true;
        } catch (...)
        {
        }
        attributes[oAttrFilterID].emplace_back(InstanceFilter::Attribute{
            getUUID(pAttribute, std::string("AttributeID")),
            getUUID(pAttribute, std::string("InstanzAttributeID")),
            getString(pAttribute, std::string("AttributeName")),
            getString(pAttribute, std::string("Value")),
            std::stoi(getString(pAttribute, std::string("UserLevel"))),
            getBool(pAttribute, std::string("Visible")),
            getBool(pAttribute, std::string("Publicity")),
            blob,
            hasBlob,
            getUUID(pAttribute, std::string("InstanzVariantID")),
            getUUID(pAttribute, std::string("VariantID")),
            getOptionalString(pAttribute, std::string("HelpFile"))
        });
    }

    std::map<Poco::UUID, std::vector<GraphItemExtension>> itemExtensionMap = parseGraphItemExtensions(pGraph);

	// Create new filtergraph
    graph.id = getUUID( pGraph, std::string("GraphID") );

    AutoPtr<NodeList> pHeaderFilters = pHeaderFilterInfoList->getElementsByTagName( std::string("HeaderFilterInfo") );
    graph.filterDescriptions.reserve(pHeaderFilters->length());
    for( unsigned long i=0; i<pHeaderFilters->length(); i++ )
    {
        Element* pHeaderFilter = static_cast<Element*>( pHeaderFilters->item(i) );
        graph.filterDescriptions.emplace_back(FilterDescription{
            getUUID(pHeaderFilter, "FilterID"),
            getString(pHeaderFilter, "FilterName"),
            getString(pHeaderFilter, "Version"),
            getString(pHeaderFilter, "KomponenteAssemblyName"),
            getUUID(pHeaderFilter, "KomponenteID")
        });
    }

    AutoPtr<NodeList> pInstanzFilters = pInstanzFilterList->getElementsByTagName( std::string("InstanzFilter") );
    graph.instanceFilters.reserve(pInstanzFilters->length());
    for (unsigned long i = 0; i < pInstanzFilters->length(); i++)
    {
        Element* pInstanzFilter = static_cast<Element*>(pInstanzFilters->item(i));
        const int groupNumber = addFilterGroup(graph, pInstanzFilter->getChildElement(std::string("InstanzFilterGrouping")));

        const auto uuid = getUUID(pInstanzFilter, std::string("InstanzFilterID"));
        graph.instanceFilters.emplace_back(
            InstanceFilter{
                getUUID(pInstanzFilter, std::string("FilterID")),
                uuid,
                getString(pInstanzFilter, std::string("Name")),
                groupNumber,
                parsePosition(pInstanzFilter->getChildElement(std::string("InstanzFilterDescription"))),
                attributes[uuid],
                itemExtensionMap[uuid]
            });
    }

	// InstanzPipeList
	Element* pInstanzPipeList = pGraph->getChildElement( std::string("InstanzPipeList") );
	if (!pInstanzPipeList)
		throw fliplib::GraphBuilderException("Node <InstanzPipeList> not found.");

    buildMacroList(graph, pGraph);

    if (Element* inConnectorList = pGraph->getChildElement( std::string("InConnectors")); inConnectorList)
    {
        buildConnectorList(graph.inConnectors, inConnectorList);
    }

    if (Element* outConnectorList = pGraph->getChildElement( std::string("OutConnectors")); outConnectorList)
    {
        buildConnectorList(graph.outConnectors, outConnectorList);
    }

	// Loop through all pipes in the instanz pipe list
	AutoPtr<NodeList> oPipeList = pInstanzPipeList->getElementsByTagName( std::string("InstanzPipe") );
    graph.pipes.reserve(oPipeList->length());
	for( unsigned long i=0; i<oPipeList->length(); i++ )
	{
		Element* pInstanzPipe = static_cast<Element*>( oPipeList->item(i) );

        Poco::UUID uuid = getUUID( pInstanzPipe, std::string("InstanzPipeID") );
        Pipe pipe{
            getUUID( pInstanzPipe, std::string("ReceiverInstFilterID") ),
            getUUID( pInstanzPipe, std::string("SenderInstFilterID") ),
            getString( pInstanzPipe, std::string("ReceiverConnectorName") ),
            std::stoi(getString( pInstanzPipe, std::string("ReceiverConnectorGroup") )),
            getString( pInstanzPipe, std::string("SenderConnectorName") ),
            uuid,
            getUUID( pInstanzPipe, std::string("ReceiverConnectorID") ),
            getUUID( pInstanzPipe, std::string("SenderConnectorID") ),
            getString( pInstanzPipe, std::string("PipePath") ),
            itemExtensionMap[uuid]
        };
        if ((std::any_of(graph.instanceFilters.begin(), graph.instanceFilters.end(), [&pipe] (const auto &filter) { return filter.id == pipe.sender; }) ||
            std::any_of(graph.macros.begin(), graph.macros.end(), [&pipe] (const auto &macro) { return macro.id == pipe.sender; }) ||
             std::any_of(graph.inConnectors.begin(), graph.inConnectors.end(), [&pipe](const auto &connector) { return connector.id == pipe.senderConnectorId;} )) &&
            (std::any_of(graph.instanceFilters.begin(), graph.instanceFilters.end(), [&pipe] (const auto &filter) { return filter.id == pipe.receiver; }) ||
             std::any_of(graph.macros.begin(), graph.macros.end(), [&pipe] (const auto &macro) { return macro.id == pipe.receiver; }) ||
             std::any_of(graph.outConnectors.begin(), graph.outConnectors.end(), [&pipe](const auto &connector) { return connector.id == pipe.receiverConnectorId;}
             )))
        {
            graph.pipes.push_back(std::move(pipe));
        }
	} // for

    buildPortList(graph, pGraph, itemExtensionMap);
    buildSensorList(graph, pGraph);
    buildErrorList(graph, pGraph);
    buildResultList(graph, pGraph);

    // sort FilterGroups by name
    std::sort(graph.filterGroups.begin(), graph.filterGroups.end(), [] (const FilterGroup &a, const FilterGroup &b) { return a.name < b.name; });



	// Read the first ParameterSatz id, just to be able to export the graph again with the same one (no filtering on InstanzVariant)
	Element* pParameterSatz = nullptr;
	if ( Element* pParameterSatzList = pGraph->getChildElement( std::string("ParameterSatzList") ))
    {
        AutoPtr<NodeList> pParameterSatzElements = pParameterSatzList->getElementsByTagName( std::string("ParameterSatz") );
        if (pParameterSatzElements && pParameterSatzElements->length() > 0)
        {
            pParameterSatz = static_cast<Element*>( pParameterSatzElements->item(0) );
        }
    }

    if (pParameterSatz == nullptr)
    {
        graph.parameterSet = Poco::UUIDGenerator::defaultGenerator().create();

    }
    else
    {

        Poco::UUID oParameterSatzId = getUUID( pParameterSatz, std::string("ParametersatzID") );
        graph.parameterSet = oParameterSatzId;
    }

    return graph;
}

UpFilterGraph XmlGraphBuilder::buildFilterGraph(const GraphContainer &graphContainer)
{

	UpFilterGraph pFilterGraph = std::make_unique<FilterGraph>( graphContainer.id );

	// Create filter instances. Unfortunately we need to find two tags, a HeaderFilterInfo tag because it contains the component information and the InstanzFilter tag as it contains the InstanzFilterID.
	for( const auto &filterDescription : graphContainer.filterDescriptions )
	{
		// find the corresponding InstanzFilterInfo tag that has the same filter guid ...
        for (const auto &instance : graphContainer.instanceFilters)
        {
            if (instance.filterId != filterDescription.id)
            {
                continue;
            }
            loadFilter(pFilterGraph.get(), filterDescription, instance);
        }
	} // for

	// Create and connect pipes
	renderGraph( pFilterGraph.get(), graphContainer );
    validatePipes(pFilterGraph.get());
	return pFilterGraph;

} // buildFilterGraph

void XmlGraphBuilder::validatePipes(const FilterGraph* filterGraph) const
{
    for (const auto& filerHandle: filterGraph->getFilterMap())
    {
        const auto filter = filerHandle.second->getFilter();
        const auto isValid = filter->isValidConnected();

        if (!isValid)
        {
            throw fliplib::GraphBuilderException("Filter " + filter->name() + " is not correctly connected to the pipes!");
        }
    }
}

} // namespace fliplib
