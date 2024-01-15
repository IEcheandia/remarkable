///////////////////////////////////////////////////////////
//  AbstractGraphBuilder.h
//  Implementation of the Class AbstractGraphBuilder
//  Created on:      08-Nov-2007 14:13:52
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_8561E635_FD18_4753_9012_B36091147804__INCLUDED_)
#define EA_8561E635_FD18_4753_9012_B36091147804__INCLUDED_

#include <map>
#include <string>
#include "fliplib/Fliplib.h"
#include "fliplib/FilterHandle.h"
#include "fliplib/FilterGraph.h"
#include "fliplib/graphContainer.h"

namespace fliplib
{
	/**
	 * Abstrakte Klasse fuer GraphBuilder.  (Interface)
	 * 
	 * Erzeugt eine FilterGraph und verbindent saemtliche Pipes innerhalb des Filters
	 */
	class FLIPLIB_API AbstractGraphBuilder
	{
	public:	
		/** 
		 * Standardkonstruktor
		 */ 
		AbstractGraphBuilder();
		
		/**
		 * Destruktor
		 */
		virtual ~AbstractGraphBuilder();

		/**
		 * 	Erzeugt einen neuen Graph. Die Konfigurationsdatei liegt im Pfad von uri
		 *
		 * fliplib::FileNotFoundException wenn die Konfigurationsdatei nicht gefunden werden konnte
		 * fliplib::GraphBuilderException wenn beim parsen und erstellen ein Fehler aufgetreten ist
		 * 
		 * \param [in] uri Pfad oder URL 
		 */		
		virtual UpFilterGraph build(const std::string& uri) = 0;
		
		/**
		 * 	Erzeugt einen neuen Graph aus einem Konfigurationsstring
		 *
		 * fliplib::FileNotFoundException wenn die Konfigurationsdatei nicht gefunden werden konnte
		 * fliplib::GraphBuilderException wenn beim parsen und erstellen ein Fehler aufgetreten ist
		 * 
		 * \param [in] uri Pfad oder URL 
		 */		
		virtual SpFilterGraph buildString(const std::string& config) = 0;


        /**
        * @brief Builds a GraphContainer from an XML file
        * @exception fliplib::FileNotFoundException if the file was not found.
        * @exception fliplib::GraphBuilderException if it was not possible to parse the file.
        * @param [in] p_rUri Path or URL.
        * default implementation returns just a standard constructed GraphContainer
        **/
        virtual GraphContainer buildGraphDescription(const std::string &p_rUri);

	protected:		
	
		/**
		 * Liefert eine neue Instance eines Filters.
		 * 
		 * \param [in] componentUri	relativer oder absoluter filePath der Komponente (e.g /tmp/libfliplibTestComponent_g.so.1)
		 * \param [in] filter Name des Filters
		 * \return Kann der Filter nicht aktiviert werden wird NULL geliefert    
		 */
		virtual FilterHandle* activate(const std::string& componentUri, const std::string& filter);
				
	};

}
#endif // !defined(EA_8561E635_FD18_4753_9012_B36091147804__INCLUDED_)
