/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Sevitec, Stefan Birmanns (SB)
 * 	@date		2007 - 2013
 * 	@brief		Builds a signal processing graph.
 */

#ifndef XMLGRAPHBUILDER_H_
#define XMLGRAPHBUILDER_H_

// clib includes
#include <string>
#include <map>
// poco includes
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Document.h>
#include <Poco/SharedPtr.h>
// project includes
#include <fliplib/GraphBuilderFactory.h>
#include <fliplib/AbstractGraphBuilder.h>
#include <fliplib/FilterGraph.h>

namespace fliplib {

/**
 * @ingroup Fliplib
 * @brief	The XmlGraphBuilder builds a new fliplib signal processing graph by reading and interpreting an XML file.
 */
class FLIPLIB_API XmlGraphBuilder : public AbstractGraphBuilder
{
	friend class fliplib::GraphBuilderFactory;

private:
	/**
	 * @brief Hidden constructor. Can only be called by the GraphBuilderFactory.
	 */
	XmlGraphBuilder();

public:

	/**
	 * @brief Builds a new graph from an XML file.
	 * @exception fliplib::FileNotFoundException if the file was not found.
	 * @exception fliplib::GraphBuilderException if it was not possible to parse the file.
	 * @param [in] p_rUri Path or URL.
	 */
	UpFilterGraph build( const std::string& p_rUri ) override;

	/**
	 * @brief Builds a new graph from a string.
	 * @exception fliplib::GraphBuilderException if it was not possible to parse the file.
	 * @param [in] p_rXml std::string with the XML code.
	 */
	SpFilterGraph buildString( const std::string& p_rXml );

    /**
     * @brief Builds a GraphContainer from an XML file
     * @exception fliplib::FileNotFoundException if the file was not found.
     * @exception fliplib::GraphBuilderException if it was not possible to parse the file.
     * @param [in] p_rUri Path or URL.
     **/
    GraphContainer buildGraphDescription(const std::string &p_rUri) override;

private:

	/**
	 * @brief Load a single filter.
	 * @param p_pFilterGraph pointer to filtergraph object.
	 * @param filterDescription A description of the filter in the graph.
	 * @param instance The description of the filter instance in the graph.
	 */
	void loadFilter( FilterGraph* p_pFilterGraph, const FilterDescription &filterDescription, const InstanceFilter &instance );

	/**
	 * @brief Connect all pipes of a filtergraph.
	 * @param [in] p_pFilterGraph pointer to filtergraph object.
	 * @param [in] graphContainer the description of the parsed graph
	 */
	void renderGraph(FilterGraph* p_pFilterGraph, const GraphContainer &graphContainer);

    /**
     * @brief Valides if all pipes are connectd
     * @param [in] p_pFilterGraph pointer to filtergraph object.
     */
    void validatePipes(const FilterGraph* filterGraph) const;

	/**
	 * @brief Parameterizes the filtergraph by looking for the InstanzAttribute tags in the XML code.
	 * @param [in] p_pFilter pointer to fliplib filter object.
     * @param instance The filter instance containing the parameters for the filter
	 */
	void parameterizeFilter(BaseFilter* p_pFilter, const InstanceFilter &instance);

	/**
	 * @brief Creates the correct path for a filter component - depends on platform, is completely different under windows and qnx.
	 * @param [in] p_rComponentName name of the component.
	 */
	std::string getComponentPath( const std::string& p_rComponentName );

	/***
	 * @brief Main function that creates the filtergraph.
	 * @param [in] graphContainer The description of the parsed graph
	 */
	UpFilterGraph buildFilterGraph(const GraphContainer &graphContainer);

    /**
     * @brief build the GraphContainer from the XML document
     **/
    GraphContainer buildFilterGraph(const Poco::XML::Document* p_pDoc);

    void buildPortList(GraphContainer &graph, Poco::XML::Element *pGraph, std::map<Poco::UUID, std::vector<GraphItemExtension>> &itemExtensionMap);
    void buildSensorList(GraphContainer &graph, Poco::XML::Element *pGraph);
    void buildErrorList(GraphContainer &graph, Poco::XML::Element *pGraph);
    void buildResultList(GraphContainer &graph, Poco::XML::Element *pGraph);
    void buildMacroList(GraphContainer &graph, Poco::XML::Element *pGraph);
    void buildConnectorList(std::vector<Macro::Connector> &connectors, Poco::XML::Element *connectorsElement);
    void readGenericType(Poco::XML::Element *element, GenericType &type);
    void readErrorType(Poco::XML::Element *element, ErrorType &type);

    int addFilterGroup(GraphContainer &graph, Poco::XML::Element *grouping);

    std::map<Poco::UUID, std::vector<GraphItemExtension>> parseGraphItemExtensions(Poco::XML::Element *pGraph);

	/**
	 * @brief Create a valid poco uuid by interpreting a string.
	 * @param p_rId std::string with the guid.
	 * @return poco uuid object.
	 */
	Poco::UUID createValidUUID( const std::string& p_rId );

	/**
	 * @brief Look for a child element in an XML element and read its inner text and interpret it as poco uuid. Might throw if the element was not found.
	 * @param p_pElement pointer to poco xml parent element under which the child tag should be located.
	 * @param p_oTag tag of the child that we are looking for.
	 * @return poco uuid object.
	 * @exception fliplib::GraphBuilderException if it was not possible to find or interpret the xml child element.
	 */
	Poco::UUID getUUID( const Poco::XML::Element* p_pElement, std::string p_oTag );
	/**
	 * @brief Look for a child element in an XML element and read its inner text. Might throw if the element was not found.
	 * @param p_pElement pointer to poco xml parent element under which the child tag should be located.
	 * @param p_oTag tag of the child that we are looking for.
	 * @return std::string with the inner text of the child.
	 * @exception fliplib::GraphBuilderException if it was not possible to find the xml child element.
	 */
	std::string getString( const Poco::XML::Element* p_pElement, std::string p_oTag );

    /**
     * Like getString, but returns an empty string in case the child element is missing instead of throwing an exception
     **/
    std::string getOptionalString( const Poco::XML::Element* p_pElement, std::string p_oTag );

    /**
     * @brief Look for a child element in an XML element and read its inner text and compares it to "true". Might throw if the element was not found.
     * @param p_pElement pointer to poco xml parent element under which the child tag should be located.
     * @param p_oTag tag of the child that we are looking for.
     * @return bool whether the inner text matched the string true
     * @exception fliplib::GraphBuilderException if it was not possible to find the xml child element.
     */
    bool getBool(const Poco::XML::Element* p_pElement, std::string p_oTag);

	static std::set<std::string> m_oLibsLoaded;	// emit only one log messager per dll loaded per graph

};

} // namespace fliplib

#endif
