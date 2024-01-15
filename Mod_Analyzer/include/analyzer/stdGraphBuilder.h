/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR
 * 	@date		2010
 * 	@brief		Builds a filter graph.
 */

#ifndef STDGRAPHBUILDER_H_
#define STDGRAPHBUILDER_H_

#include <string>
#include "Poco/SharedPtr.h"
#include "Poco/UUID.h"
#include "fliplib/AbstractGraphBuilder.h"

#include "common/graph.h"

using namespace fliplib;

namespace precitec
{
	using namespace interface;

namespace analyzer
{

	class StdGraphBuilder : public AbstractGraphBuilder
	{
	public:
		StdGraphBuilder();
		
		UpFilterGraph build(Graph const& graph);
		
	private:
		std::string getComponentPath(const  Poco::UUID& componentID, const std::string& componentRootPath, ComponentList const& components);
		void parameterize(BaseFilter* filter, ParameterList const& parameters);
		void loadFilter(FilterGraph* filterGraph, Graph const& graph);
		void loadFilter(FilterGraph* filterGraph, ComponentList const& components, FilterList const& filters, const std::string& componentRootPath); 
		void renderGraph(FilterGraph* filterGraph, Graph const& graph);
		
		UpFilterGraph build(const std::string& uri) override { return NULL; }
		SpFilterGraph buildString(const std::string& config) { return NULL; }
		
	};

}

}

#endif /*STDGRAPHBUILDER_H_*/
