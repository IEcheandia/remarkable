/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Sevitec, HS
 * 	@date		2007 - 2013
 * 	@brief		Builds a signal processing graph from a compact, easy readable XML format.
 */

#ifndef XMLLIGHTGRAPHBUILDER_H_
#define XMLLIGHTGRAPHBUILDER_H_

// clib includes
#include <string>
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
 * @brief	The XmlLightGraphBuilder builds a new fliplib signal processing graph by reading and interpreting an XML file.
 */
class FLIPLIB_API XmlLightGraphBuilder : public AbstractGraphBuilder
{
	friend class fliplib::GraphBuilderFactory;

private:
	/**
	 * @brief Hidden constructor. Can only be called by the GraphBuilderFactory.
	 */
	XmlLightGraphBuilder();

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


private:

	/**
	 * @brief Load filters.
	 * @param [in] p_pFilterGraph pointer to filtergraph object.
	 * @param [in] p_pGraph pointer to poco xml element.
	 */
	void loadFilter(FilterGraph* p_pFilterGraph, const Poco::XML::Element* p_pGraph);	
	
	/**
	 * @brief Load filter.
	 * @param [in] p_pFilterGraph pointer to filtergraph object.
	 * @param [in] p_pComponents pointer to poco xml element.
	 * @param [in] p_pFilters pointer to poco xml element.
	 * @param [in] p_rComponentRootPath Path to module.
	 * @param [in] p_rFiltername Filter name.
	 */	
	void loadFilter(FilterGraph* p_pFilterGraph, Poco::XML::Element* p_rComponents, Poco::XML::Element* p_rFilters, const std::string& p_rComponentRootPath, const std::string& p_rFiltername);

	/**
	 * @brief Connect all pipes of a filtergraph.
	 * @param [in] p_pFilterGraph pointer to filtergraph object.
	 * @param [in] p_pGraph pointer to poco xml element.
	 */
	void renderGraph(FilterGraph* filterGraph, const Poco::XML::Element* p_pGraph);

	/**
	 * @brief Parameterizes the filtergraph by looking for the InstanzAttribute tags in the XML code.
	 * @param [in] p_pFilter pointer to fliplib filter object.
	 * @param [in] p_pInstanzVariantList pointer to poco xml element with the instanzvariants.
	 */
	void parameterizeFilter(BaseFilter* p_pFilter, const Poco::XML::Element* p_pInstanzVariantList);

	/**
	 * @brief Creates the correct path for a filter component - depends on platform, is completely different under windows and qnx.
	 * @param [in] p_rComponentName name of the component.
	 * @param [in] p_rComponentRootPath Root path of component / module.
	 * @param [in] p_pComponents pointer to poco xml document.
	 */
	std::string getComponentPath(const std::string& p_rComponentName, const std::string& p_rComponentRootPath, Poco::XML::Element* p_pComponents);

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
	std::string getString( const Poco::XML::Element* pElement, std::string p_oTag );
	
	/***
	 * @brief Main function that creates the filtergraph from an xml document in old format.
	 * @param [in] p_pDoc pointer to poco xml document.
	 */
	UpFilterGraph buildFilterGraphXml(const Poco::XML::Document* p_pDoc);

	static const std::string ATTR_GRAPH_ID;
	static const std::string ATTR_FILTER_NAME;
	static const std::string ATTR_FILTER_ID;
	static const std::string ATTR_FILTER_COMPONENT;
	static const std::string ATTR_COMPONENTS_PATH;
	static const std::string ATTR_COMPONENT_FILE;
	static const std::string ATTR_COMPONENT_ID;
	static const std::string ATTR_PIPE_SENDER;
	static const std::string ATTR_PIPE_NAME;
	static const std::string ATTR_PIPE_GROUP;
	static const std::string ATTR_FILTER_TYPE;
		
	static const std::string ELEM_COMPONENTS;
	static const std::string ELEM_COMPONENT;
	static const std::string ELEM_FILTERS;
	static const std::string ELEM_FILTER;
	static const std::string ELEM_PIPE_IN;
	static const std::string ELEM_PIPE_OUT;	
	static const std::string ELEM_PARAMETER;		
						 			
	static const std::string ELEM_FILTER_SOURCE;		
	static const std::string ELEM_FILTER_TRANSFORM;		
	static const std::string ELEM_FILTER_SINK;		
	static const std::string ELEM_FILTER_PARAM;		
	static const std::string ELEM_FILTER_RESULT;

};

} // namespace fliplib

#endif
