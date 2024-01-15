/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR
 * 	@date		2010
 * 	@brief		Builds a filter graph.
 */

#include "analyzer/stdGraphBuilder.h"


#include "Poco/Path.h"
#include "Poco/Exception.h"
#include "Poco/UUIDGenerator.h"

#include "fliplib/Exception.h"
#include "fliplib/FilterGraph.h"
#include "fliplib/Parameter.h"
#include "fliplib/ParameterContainer.h"

#include "system/types.h"

#include "module/moduleLogger.h"

using Poco::Exception;
using Poco::Path;
using Poco::SharedPtr;
using Poco::UUIDGenerator;

using fliplib::BaseFilter;
using fliplib::FilterGraph;
using fliplib::SpFilterGraph;
using fliplib::FileNotFoundException;
using fliplib::GraphBuilderException;

using fliplib::Parameter;
using fliplib::ParameterContainer;

namespace precitec
{
	using namespace interface;

	namespace analyzer
	{

		StdGraphBuilder::StdGraphBuilder()
		{
		}

		std::string StdGraphBuilder::getComponentPath(const Poco::UUID& componentID, const std::string& componentRootPath, ComponentList const& components)
		{
			for(ComponentList::const_iterator component=components.begin(); component!=components.end(); ++component)
			{
				if (component->id() == componentID)
				{
					Path path(componentRootPath + component->filename());

					return path.makeAbsolute().toString();
				}
			}
			return "unknown";
		}

		void StdGraphBuilder::parameterize(BaseFilter* filter, ParameterList const & parameters)
		{
			ParameterContainer& pc = filter->getParameters();

			// Durch Parameterliste traversieren
			for(ParameterList::const_iterator parameter= parameters.begin(); parameter != parameters.end(); ++parameter)
			{
				FilterParameter const* param = parameter->get();
				try
				{
                    Parameter oParam = pc.findParameter( param->name() );
					pc.add(param->name(), oParam.getType(), param->any());
				}
				catch(const ParameterException &ex)
				{
					std::stringstream ss;
					ss << "Parameter error:: Filter: '" + filter->name() + "' Parametername:'" << param->name() << "' Type:'" << param->typeToStr() << "' Value:'" << param->any().convert<std::string>() <<"' \n" << ex.what() << std::endl;
					wmLog(eDebug, ss.str().c_str());
					throw fliplib::GraphBuilderException(ss.str(), ex);
				}
			}
		}


		void StdGraphBuilder::loadFilter(FilterGraph* filterGraph, ComponentList const& components, FilterList const& filters, const std::string& componentRootPath)
		{
			static auto oLibsLoaded = std::set<std::string>();	// emit only one log messager per dll loaded
			for (FilterList::const_iterator filter=filters.begin(); filter!=filters.end(); ++filter)
			{
				std::string filterPath = getComponentPath(filter->component(), componentRootPath, components);
				size_t dotPos;
				dotPos = filterPath.rfind('.');
				if(dotPos==std::string::npos)
				{
					dotPos = filterPath.length();
				}

				size_t slashPos;
				slashPos = filterPath.rfind('\\');
				if(slashPos==std::string::npos)
				{
					slashPos = filterPath.rfind('/');
					if(slashPos==std::string::npos)
					{
						slashPos=0;
					}
					else
						slashPos++;
				}
				else
					slashPos++;

				std::string filterName = filterPath.substr( slashPos, dotPos-slashPos );

				size_t libPos;
				libPos = filterName.find("lib");
				if(libPos!=std::string::npos)
				{
					if(libPos == 0)
					{
						filterName = filterName.substr( libPos+3, filterName.length()-(libPos+3) );
					}
				}

#if not defined(NDEBUG) and defined(__QNX__)
				size_t debugPos;
				debugPos = filterName.find("_g");
				if(debugPos!=std::string::npos)
				{
					if(debugPos>=filterName.length()-2)
						filterName = filterName.substr( 0, filterName.length()-2 );
				}
#endif

				bool found = false;
				Poco::Path currentPath = Path::current();
				std::string pathStr = currentPath.toString();
				std::vector<std::string> searchPathes(2);
				searchPathes[1] = pathStr;
//				pathStr = currentPath.makeParent().toString();
//				pathStr.append("libs/");
				pathStr = std::string(getenv("WM_BASE_DIR"));
				pathStr.append("/lib/");
				searchPathes[0] = pathStr;
				for(unsigned int idx=0; idx<searchPathes.size(); idx++)
				{
					filterPath = searchPathes[idx];
					if(filterPath.size()==0)
						continue;
					filterPath.append("lib");
					filterPath.append(filterName);
#if not defined(NDEBUG) and defined(__QNX__)
					filterPath.append("_g");
#endif
					filterPath.append(".so");

					// Filterinstanze erstellen
					// ------------------------
					FilterHandle* handle = nullptr;
					try
					{
#if !defined(NDEBUG)
						if (oLibsLoaded.find(filterPath) == std::end(oLibsLoaded)) {
							std::ostringstream oMsg;	
							oMsg << "1st load of '" << filterPath << "'\n";
							wmLog(eDebug, oMsg.str());
							oLibsLoaded.insert(filterPath);
						} // if
#endif // !defined(NDEBUG)
						handle = activate( filterPath, filter->name() );
					}
					catch (const Poco::LibraryLoadException &)
					{
					}

					if (!handle)
					{
						std::ostringstream oMsg;
						oMsg << "Could not load '" << filter->name() + "' from '" << filterPath << "'.\n";
						wmLog(eWarning, oMsg.str());
						continue;
					}

					// Filter parametrieren
					parameterize(handle->getFilter(), filter->parameterList());

					// Filter in Graph speichern. Pipes sind noch nicht verbunden
					filterGraph->insert(filter->instanceID(), handle);
					found = true;
					break;
				}
				if(!found)
					throw fliplib::GraphBuilderException("can not load library '" + filterName + "' with filter " + filter->name() + ". Check referenced libraries as well!");
			}
		}


		// Laedt saemtliche Komponenten(DLL) und erzeugt die enthaltenen FilterInstanzen
		void StdGraphBuilder::loadFilter(FilterGraph* filterGraph, Graph const& graph)
		{
			// Rootelemente laden
			ComponentList const& components = graph.components();
			FilterList const& filters = graph.filters();

			// Root Pfad fuer Komponenten suchen. Der Pfad endet immer mit Seperator
			std::string componentRootPath = graph.pathComponents();
			if (componentRootPath != "")
			{
				if (componentRootPath.length() > 0 && componentRootPath[componentRootPath.length() - 1] != Path::separator())
					componentRootPath += Path::separator();
			}



			// Alle Filtertypen laden
			loadFilter(filterGraph, components, filters, componentRootPath);
		}

		// Verbindet die Filter gemaess Graphen
		void StdGraphBuilder::renderGraph(FilterGraph* filterGraph, Graph const& graph)
		{
			FilterList const& filters = graph.filters();

			// Durch saemtliche Filter traversieren
			for (FilterList::const_iterator filter=filters.begin(); filter!= filters.end(); ++filter)
			{
				// RECEIVER suchen
				BaseFilter* receiver = filterGraph->find(filter->instanceID());

				// InPipeliste lesen
				InPipeList const& inPipes = filter->inPipeList();
				for (InPipeList::const_iterator inPipe=inPipes.begin(); inPipe!= inPipes.end(); ++inPipe)
				{
					int group = 0;

					// SENDER suchen
					
					BaseFilter* sender = filterGraph->find( inPipe->sender() );

					if (sender == NULL)
						throw fliplib::GraphBuilderException("source filter:'" + inPipe->sender().toString() + "' in pipe of '" + filter->instanceID().toString() + "' not found! ");

					BasePipe* pipeInst = sender->findPipe(inPipe->name());

					// Pipe Instance suchen
					if ( inPipe->name() == "" )
						throw fliplib::GraphBuilderException("filter:'" + filter->instanceID().toString() + "' name in pipe expected! ");
					
					if (!pipeInst)
					{					
						throw fliplib::GraphBuilderException("sender:" + sender->id().toString() + " receiver:'" + filter->instanceID().toString() + "' pipe='" + inPipe->name() + "' not found! ");
					}

					group = inPipe->group();
					pipeInst->setTag(inPipe->tag());

					// Sender mit Receiver verbinden
					// -----------------------------
					//std::stringstream oSS; oSS << "\t'" << sender->name() << "' with '" << receiver->name() << "' via pipe '" << inPipe->name() << "' connected. Tag:" << inPipe->tag() << ", group:" << group << "\n";
					//wmLog( eDebug, oSS.str() );
					receiver->connectPipe(pipeInst, group);
				}
			}

		}



		UpFilterGraph StdGraphBuilder::build(Graph const& graph)
		{
			wmLog( eDebug, "Building graph with id %s\n", graph.id().toString().c_str() );

			// Neuer Filtergraph erstellen
			UpFilterGraph filterGraph = UpFilterGraph( new FilterGraph( graph.id() ) );
			// Filterinstanzen erzeugen
			loadFilter(filterGraph.get(), graph);
			// Pipes der Filter verbinden
			renderGraph(filterGraph.get(), graph);

			return filterGraph;
		}

	}

}
