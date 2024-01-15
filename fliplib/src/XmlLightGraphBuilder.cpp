/**
 *	@file
 *	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Sevitec, HS
 * 	@date		2007 - 2013
 * 	@brief		Builds a signal processing graph from a compact, easy readable XML format.
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
#include <Poco/SharedLibrary.h>
// project includes
#include <fliplib/Exception.h>
#include <fliplib/XmlLightGraphBuilder.h>
#include <fliplib/Parameter.h>
#include <fliplib/ParameterContainer.h>
#include <module/moduleLogger.h>
#include <functional>

using Poco::XML::Document;
using Poco::XML::Element;
using Poco::XML::NodeIterator;
using Poco::XML::NodeList;
using Poco::XML::Node;
using Poco::XML::AutoPtr;

using namespace precitec;

namespace fliplib {



XmlLightGraphBuilder::XmlLightGraphBuilder()
{

} // CTor.



Poco::UUID XmlLightGraphBuilder::getUUID( const Poco::XML::Element* p_pElement, std::string p_oTag )
{
	std::string oStr = getString( p_pElement, p_oTag );

	return createValidUUID( oStr );

} // getUUID



std::string XmlLightGraphBuilder::getString( const Poco::XML::Element* p_pElement, std::string p_oTag )
{
	Element* pChild = p_pElement->getChildElement( p_oTag );
	if ( !pChild )
		throw fliplib::GraphBuilderException("Node <" + p_oTag + "> not found.");

	return pChild->innerText();

} // getString



std::string XmlLightGraphBuilder::getComponentPath(const std::string& p_rComponentName, const std::string& p_rComponentRootPath, Poco::XML::Element* p_pComponents)
{
	AutoPtr<NodeList> componentList = p_pComponents->getElementsByTagName(ELEM_COMPONENT);
	for(unsigned long i=0; i < componentList->length(); i++)
	{
		Element* element = static_cast<Element*>(componentList->item(i));

		if (element->hasAttribute(ATTR_COMPONENT_ID) && element->getAttribute(ATTR_COMPONENT_ID) == p_rComponentName)
		{
			std::string file;
			if (element->hasAttribute(ATTR_COMPONENT_FILE))
				file = element->getAttribute(ATTR_COMPONENT_FILE);

			// Wenn Endung .* wird unter Windows nach .DLL gesucht. In QNX je nach Build _g.so oder nur .so
			if(file.length()>0 && file[file.length() - 1] == '*')
			{	
				bool isQNX=false;

				std::string suffix = Poco::SharedLibrary::suffix();
				std::string::size_type pos = suffix.rfind('.');
				if (pos != std::string::npos)
				{
					suffix = suffix.substr(pos);
					isQNX = (suffix == ".so");
					
                    #if not defined(NDEBUG) and defined(__QNX__)
						if (isQNX) suffix = "_g.so";
					#endif
				}

				pos = file.find_last_not_of(".*");
				if (pos != std::string::npos)
					file = file.substr(0, pos+1);

				file = file + suffix;

				// Unter Unix wird noch ein lib vorne angeheftet.
				if (isQNX)
					file = "lib" + file;

			}

			Poco::Path path(p_rComponentRootPath + file);

			return path.makeAbsolute().toString();
		}
	}

	return "Invalid component name <" + p_rComponentName + ">";
} // getComponentPath



Poco::UUID XmlLightGraphBuilder::createValidUUID(const std::string& p_rId)
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

	return uuid;

} // createValidUUID



void XmlLightGraphBuilder::loadFilter(FilterGraph* p_pFilterGraph, const Poco::XML::Element* p_pGraph)
// Erzeugt Instanzen von Filtern in Abhaengigkeit der XML Konfigurationsdatei die zu einem bestimmten
// Graphen gehoeren.
{
	// Rootelemente laden
	Element* components = p_pGraph->getChildElement(ELEM_COMPONENTS);
	Element* filters = p_pGraph->getChildElement(ELEM_FILTERS);

	if (!components)
		throw fliplib::GraphBuilderException("node <components> not found.");

	if (!filters)
		throw fliplib::GraphBuilderException("node <filters> not found.");

	// Root Pfad fuer Komponenten suchen
	std::string componentRootPath;
	if (components->hasAttribute(ATTR_COMPONENTS_PATH))
	{
		componentRootPath = components->getAttribute(ATTR_COMPONENTS_PATH);
		if (componentRootPath.length() > 0 && componentRootPath[componentRootPath.length() - 1] != Poco::Path::separator())
			componentRootPath += Poco::Path::separator();
	}

	// Filter laden
	loadFilter(p_pFilterGraph, components, filters, componentRootPath, ELEM_FILTER);

} // loadFilter



void XmlLightGraphBuilder::loadFilter(FilterGraph* p_pFilterGraph, Element* components, Element* p_pFilters, const std::string& componentRootPath, const std::string& p_rFiltername)
{
	auto oLibsLoaded = std::set<std::string>();	// emit only one log messager per dll loaded
	AutoPtr<NodeList> filterList = p_pFilters->getElementsByTagName(p_rFiltername);
	for(unsigned long i=0;i<filterList->length(); i++)
	{
		Element* element = static_cast<Element*>(filterList->item(i));

		Poco::UUID filterID;
		std::string filterName;
		std::string filterPath;

		if (element->hasAttribute(ATTR_FILTER_ID))
			filterID = createValidUUID( element->getAttribute(ATTR_FILTER_ID) );
		else
			throw fliplib::GraphBuilderException("<" + p_rFiltername+ "> id=\"...\" expected!");

		if (element->hasAttribute(ATTR_FILTER_NAME))
			filterName = element->getAttribute(ATTR_FILTER_NAME);
		else
			throw fliplib::GraphBuilderException("<" + p_rFiltername + "> name=\"...\" expected!");

		if (element->hasAttribute(ATTR_FILTER_COMPONENT))
			filterPath = getComponentPath(element->getAttribute(ATTR_FILTER_COMPONENT), componentRootPath, components);
		else
			throw fliplib::GraphBuilderException("<" + p_rFiltername + "> id='" + filterID.toString() + "' component expected! ");


		// Filterinstanze erstellen
		// ------------------------
		FilterHandle* handle = nullptr;
		try
		{
			if (oLibsLoaded.find(filterPath) == std::end(oLibsLoaded)) {
				std::ostringstream oMsg;	
				oMsg << "Loading '" << filterPath << "'.\n";
				wmLog(eDebug, oMsg.str());
				oLibsLoaded.insert(filterPath);
			} // if
			//wmLog( eDebug, "Activating '%s' from '%s'.\n", p_rFiltername.c_str(), filterPath.c_str() );
			handle = activate(filterPath, filterName);
		}
		catch (const Poco::LibraryLoadException&)
		{
		}
		if (!handle)
		{
			std::ostringstream oMsg;
			oMsg << "Could not load '" << filterName + "' from '" << filterPath << "'.\n";
			wmLog(eWarning, oMsg.str());
			continue;
		}
		// Filter parametrieren

		ParameterContainer& pc = handle->getFilter()->getParameters();

		// Durch Parameterliste traversieren
		AutoPtr<NodeList> params = element->getElementsByTagName(ELEM_PARAMETER);
		for(unsigned long i=0;i<params->length(); i++)
		{
			Element* element = static_cast<Element*>(params->item(i));
			if (element->hasAttribute(ATTR_FILTER_NAME))
			{
				std::string name = element->getAttribute(ATTR_FILTER_NAME);

				if (element->hasAttribute(ATTR_FILTER_TYPE))
				{
					std::string type = element->getAttribute(ATTR_FILTER_TYPE);
					//std::string id = element->getAttribute(ATTR_FILTER_ID);

					try
					{
						//pc.add(name, type, Poco::UUID(id), element->innerText());
						pc.addStr(name, type, element->innerText());
					}
					catch(const ParameterException& ex)
					{
						std::stringstream ss;
						ss << "Parameter error:: Name:'" << name << "' Type:'" << type << "' Value:'" << element->innerText() <<"' \n" << ex.what() << std::endl;
						throw fliplib::GraphBuilderException(ss.str(), ex);
					}
				}
			}
		}

		// Filter in Graph speichern. Pipes sind noch nicht verbunden
		p_pFilterGraph->insert(filterID, handle);
	}
} // loadFilter



void XmlLightGraphBuilder::renderGraph(FilterGraph* p_FilterGraph, const Poco::XML::Element* p_Graph)
// Verbindet die Pipes der geladenene Filter in Abhaengigkeit der XML Konfigurationsdate
{
	// Rootelemente laden
	Element* filters = p_Graph->getChildElement(ELEM_FILTERS);

	// Durch saemtliche Filter traversieren
	AutoPtr<NodeList> filterList = filters->getElementsByTagName(ELEM_FILTER);
	for(unsigned long i=0;i<filterList->length(); i++)
	{
		Element* filter = static_cast<Element*>(filterList->item(i));

		// RECEIVER suchen
		const auto oFilterId = filter->getAttribute(ATTR_FILTER_ID);
		Poco::UUID filterID(createValidUUID(oFilterId));
		BaseFilter* receiver = p_FilterGraph->find(filterID);
		if (receiver == nullptr)
		{
			throw fliplib::GraphBuilderException("filter:'" + oFilterId + "' sender not found! ");
		}

		// Pipeliste lesen
		AutoPtr<NodeList> pipeList = filter->getElementsByTagName(ELEM_PIPE_IN);
		for (unsigned long i=0; i<pipeList->length(); i++)
		{
			int group = 0;
			Element* pipe = static_cast<Element*>(pipeList->item(i));

			// SENDER suchen
			if (!pipe->hasAttribute(ATTR_PIPE_SENDER))
				throw fliplib::GraphBuilderException("filter:'" + filterID.toString() + "' sender in pipe expected! ");

			BaseFilter* sender = p_FilterGraph->find( createValidUUID ( pipe->getAttribute(ATTR_PIPE_SENDER) ) );
			if (sender == NULL)
			{
				const std::string	oSenderFilterName	( pipe->getAttribute(ATTR_PIPE_SENDER) );
				const std::string	oFilterName			( filter->getAttribute(ATTR_FILTER_NAME) );
				throw fliplib::GraphBuilderException("Filter '" + oFilterName  + "' could not find sender filter '" + oSenderFilterName + "'.");
			}

			// Pipe suchen
			if (!pipe->hasAttribute(ATTR_PIPE_NAME))
				throw fliplib::GraphBuilderException("filter:'" + filterID.toString() + "' name in pipe expected! ");

			BasePipe* pipeInst = sender->findPipe(pipe->getAttribute(ATTR_PIPE_NAME));
			if (!pipeInst) {
				const std::string	oFilterName			( filter->getAttribute(ATTR_FILTER_NAME) );
				throw fliplib::GraphBuilderException("Filter:'" + oFilterName + ": In sender filter '" + sender->name() + "' pipe='" + pipe->getAttribute(ATTR_PIPE_NAME) + "' not found.");
			}

			pipeInst->setTag(pipe->innerText());

			if (pipe->hasAttribute(ATTR_PIPE_GROUP))
			{
				// Die Gruppe wird definiert mit (true|false|[0-9])
				std::string sgroup = pipe->getAttribute(ATTR_PIPE_GROUP);
				std::locale loc;
				std::transform( sgroup.begin(), sgroup.end(), sgroup.begin(), std::bind1st( std::mem_fun( &std::ctype<char>::tolower ), &std::use_facet< std::ctype<char> >( loc ) ) );

				if (sgroup == "false") 		group = 0;
				else if (sgroup == "true") 	group = 1;
				else
				{  // Zahl konvertieren
					std::stringstream ssStream(sgroup);
				    ssStream >> group;
				}
			}

			// Sender mit Receiver verbinden
			// -----------------------------
			receiver->connectPipe(pipeInst, group);
		}
	}
} // renderGraph



UpFilterGraph XmlLightGraphBuilder::build(const std::string& p_rUri)
{
	std::stringstream oSt; oSt << "XmlGraphBuilder::build( " << p_rUri << " )" << std::endl;
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
	 	return buildFilterGraphXml(pDoc);
	}
	catch(Poco::Exception& ex)
	{
		throw fliplib::GraphBuilderException("XmlGraphBuilder cannot build filter from xml file!", ex);
	}

} // build



SpFilterGraph XmlLightGraphBuilder::buildString(const std::string& p_rXml)
{
	Poco::XML::DOMParser parser;
	parser.setFeature( Poco::XML::DOMParser::FEATURE_FILTER_WHITESPACE, true );

	// String parsen
	try
	{
	 	AutoPtr<Document> pDoc = parser.parseString(p_rXml);
 		return buildFilterGraphXml(pDoc);

	}
	catch(Poco::Exception& ex)
	{
		std::stringstream ss;
		ss << "XmlGraphBuilder::Fehler beim Parsen des XML Dokumentes::" << ex.displayText() << std::endl;
		ss << p_rXml;
		throw fliplib::GraphBuilderException(ss.str(), ex);
	}

} // buildString



UpFilterGraph XmlLightGraphBuilder::buildFilterGraphXml(const Document* p_pDoc)
// Verbindet die Pipes der geladenene Filter in Abhaengigkeit der XML Konfigurationsdate
{
	Element* graph = p_pDoc->documentElement();

	if (!graph)
		throw fliplib::GraphBuilderException("node <graph id=\"...\"> not found.");

	if (!graph->hasAttribute(ATTR_GRAPH_ID))
		throw fliplib::GraphBuilderException("id in node <graph> not found");

	// Neuer Filtergraph erstellen
	UpFilterGraph filterGraph = std::make_unique<FilterGraph>(Poco::UUID(graph->getAttribute(ATTR_GRAPH_ID)));
	// Filterinstanzen erzeugen
	loadFilter(filterGraph.get(), graph);
	// Pipes der Filter verbinden
	renderGraph(filterGraph.get(), graph);

	return filterGraph;
} // buildFilterGraphXml


const std::string fliplib::XmlLightGraphBuilder::ATTR_GRAPH_ID 			= std::string("id");
const std::string fliplib::XmlLightGraphBuilder::ATTR_FILTER_NAME 		= std::string("name");
const std::string fliplib::XmlLightGraphBuilder::ATTR_FILTER_ID 			= std::string("id");
const std::string fliplib::XmlLightGraphBuilder::ATTR_FILTER_COMPONENT	= std::string("component");
const std::string fliplib::XmlLightGraphBuilder::ATTR_FILTER_TYPE		= std::string("type");
const std::string fliplib::XmlLightGraphBuilder::ATTR_COMPONENTS_PATH 	= std::string("path");
const std::string fliplib::XmlLightGraphBuilder::ATTR_COMPONENT_FILE 	= std::string("file");
const std::string fliplib::XmlLightGraphBuilder::ATTR_COMPONENT_ID 		= std::string("id");
const std::string fliplib::XmlLightGraphBuilder::ATTR_PIPE_SENDER 		= std::string("sender");
const std::string fliplib::XmlLightGraphBuilder::ATTR_PIPE_NAME 			= std::string("pipe");
const std::string fliplib::XmlLightGraphBuilder::ATTR_PIPE_GROUP 		= std::string("group");

const std::string fliplib::XmlLightGraphBuilder::ELEM_COMPONENTS 		= std::string("components");
const std::string fliplib::XmlLightGraphBuilder::ELEM_COMPONENT 			= std::string("component");
const std::string fliplib::XmlLightGraphBuilder::ELEM_FILTERS			= std::string("filters");
const std::string fliplib::XmlLightGraphBuilder::ELEM_FILTER 			= std::string("filter");
const std::string fliplib::XmlLightGraphBuilder::ELEM_FILTER_SOURCE 		= std::string("sourcefilter");
const std::string fliplib::XmlLightGraphBuilder::ELEM_FILTER_TRANSFORM 	= std::string("transformfilter");
const std::string fliplib::XmlLightGraphBuilder::ELEM_FILTER_SINK 		= std::string("sinkfilter");
const std::string fliplib::XmlLightGraphBuilder::ELEM_FILTER_PARAM		= std::string("paramfilter");
const std::string fliplib::XmlLightGraphBuilder::ELEM_FILTER_RESULT 		= std::string("resultfilter");
const std::string fliplib::XmlLightGraphBuilder::ELEM_PIPE_IN			= std::string("in");
const std::string fliplib::XmlLightGraphBuilder::ELEM_PIPE_OUT			= std::string("out");
const std::string fliplib::XmlLightGraphBuilder::ELEM_PARAMETER			= std::string("param");


} // namespace fliplib
