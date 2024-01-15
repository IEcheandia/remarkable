///////////////////////////////////////////////////////////
//  FilterGraph.h
//  Implementation of the Class FilterGraph
//  Created on:      30-Okt-2007 13:33:00
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_269F1164_B563_483b_B8EF_D2BE0DD47CD3__INCLUDED_)
#define EA_269F1164_B563_483b_B8EF_D2BE0DD47CD3__INCLUDED_

#include <map>
#include <string>
#include <memory>
#include "Poco/Foundation.h"
#include "fliplib/BaseFilter.h"
#include "fliplib/FilterHandle.h"
#include "fliplib/AbstractFilterVisitor.h"

namespace fliplib
{
	/**
	 * Die Klasse FilterGraph ist eine Containerklasse fuer alle Filter eines Graphen und 
	 * wird vom GraphManager verwaltet. 
	 */
	class FLIPLIB_API FilterGraph
	{

	public:
		/**
		 * Konstruktor
		 * 
		 * \param [id] Identifikation des Graphen
		 */
		FilterGraph(const Poco::UUID& id);
		
		/**
		 * Destruktor
		 */
		virtual ~FilterGraph();
		
		/**
		 * Fuegt einen neuen Filter in den Graphen ein
		 * 
		 * \param [in] id	Eindeutiger id des Filters
		 * \param [in] handle Zeiger auf FilterHandle 
		 */
		void insert(const Poco::UUID& id, FilterHandle* handle);

		/**
		 * Loescht einen Filter aus den Graph. Der Filter wird zerstoert
		 * 
		 * \param[in] id name des zu loeschenden Filters
		 */		
		void remove (const Poco::UUID& id);
				
		/**
		 * Der Visitor steuert die Filter. Dem Visitor werden saemtliche Filter im Graphen 
		 * uebergeben.
		 * 
		 * \param[in] visitor Instanz initialisiert den Filter 
		 */
		void control(AbstractFilterVisitor& visitor);

        void control(const std::list<std::unique_ptr<fliplib::AbstractFilterVisitor>> &visitors);
		
		void controlAccordingToProcessingOrder(AbstractFilterVisitor& visitor);
        
				
		/**
		 * Sucht einen Filter im Graph
		 * 
		 * \param [in] id Name des zu suchenden Filters
		 * \return Zeiger auf den Filter
		 */
		BaseFilter* find(const Poco::UUID& id);
		
		/**
		 * Loescht alle Filter aus den Graph; Die Filterinstancen werden zerstoert
		 */
		void clear();

		/**
		 * TEST: Triggert den Graphen
		 */ 
		void fire();

		const Poco::UUID& id() const; 	//< Getter Liefert die ID des FilterGraphen

		const std::map<Poco::UUID, FilterHandle*>&	getFilterMap() const; //< Getter Liefert die FilterMap und damit die FilterIds
		
		/**
		 * Gibt die Struktur des FilterGraphens aus
		 */
		std::string toString() const;
		std::string toStringVerbose() const;
		
	private:
		// provisorisch
		void bind(BaseFilter* filter);
		void unbind(BaseFilter* filter);
		// Loest saemtliche Verbindungen des Filters 
								
	private:
		mutable Poco::FastMutex mutex_;
		typedef std::map<Poco::UUID, FilterHandle*> FilterMap;
		FilterMap map_;			
		Poco::UUID id_; 			
	};
	typedef std::shared_ptr<FilterGraph>					SpFilterGraph;    	///< SharedPtr auf FilterGraph
	typedef std::unique_ptr<FilterGraph>					UpFilterGraph;    	///< unique_ptr auf FilterGraph
	typedef std::map<Poco::UUID, fliplib::UpFilterGraph>	graph_map_t;
}
#endif // !defined(EA_269F1164_B563_483b_B8EF_D2BE0DD47CD3__INCLUDED_)
