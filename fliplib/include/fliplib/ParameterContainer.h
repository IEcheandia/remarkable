///////////////////////////////////////////////////////////
//  ParameterContainer.h
//  Implementation of the Class ParameterContainer
//  Created on:      11-Dez-2007 17:22:41
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_35BB4086_367D_411b_A090_F339463DD547__INCLUDED_)
#define EA_35BB4086_367D_411b_A090_F339463DD547__INCLUDED_

#include <map>
#include <string>
#include "Poco/DynamicAny.h"
#include "fliplib/Parameter.h"

namespace fliplib
{
	/**
	 * Proxyklasse, die einen Parameter implizit konvertiert, so dass kein template-Parameter angegeben werden muss.
	 */
	class ConversionProxy; // defined below

	/**
	 * Stellt eine Containerklasse fuer die Auflistung von fliplib::Parameter-Objekten bereit.
	 * (nicht threadsafe)   
	 */ 
	class FLIPLIB_API ParameterContainer
	{			
	public:
		/**
		 * Initialisiert eine neue Instanz der ParameterContainer 
		 */
		ParameterContainer();
		
		/**
		 * Gibt alle Resourcen der ParameterInstanz wieder frei.
		 */
		virtual ~ParameterContainer();
		
		/**
		 * Fuegt ein Parameter mit eindeutigen Namen dem Inhalt des ParameterContainers hinzu. 
		 * Der Besitz des Parameter geht an den Conainer ueber
		 * 
		 * ParameterException wenn der Parameter bereits vorhanden ist
		 * 
		 * \param [in] name Name des Parameters
		 * \param [in] type siehe ParameterFactory
		 * \param [in] value Inhalt bzw. Instanz des Parameterwertes
		 */
		void addStr(const std::string& name, const std::string& type, const std::string& value);
		void add(const std::string& name, const std::string& type, const Poco::DynamicAny& value);
		void update(const std::string& name, const std::string& type, const Poco::DynamicAny& value);
		
		/**
		 * Ruft aus dem Parametercontainer ein Parameter ab, das einem bestimten Namen zugeordnet ist.
		 * 
		 * ParameterException wenn der Parameter nicht existiert
		 * 
		 * \param [in] name Name der Parameterinstanz
		 */
		 const Parameter& findParameter(const std::string& name) const;

		 /**
		 * Wie 'findParameter(const std::string& name)', aber mit automatischer Konvertierung durch Proxyobjekt.
		 */
		 ConversionProxy getParameter(const std::string& name) const;

		 const Poco::DynamicAny& getParamValue(const std::string& name);
		 
		 
		 /**
		  * Prueft ob im Parametercontainer ein spezifischer Parameter existiert
		  * 
		  * \param [in] name Name des Parameter der gesucht werden soll
		  */ 
		 bool exists(const std::string& name);
		 
		 /**
		  * Liefert true, wenn die Parameter vom Filter gelesen werden muessen
		  */
		 bool isUpdated();
		 
		 /**
		  * Saemtliche Parameter wurden gelesen, die Flags werden zurueckgesetzt
		  */
		 void confirm();
		 
		 
		 /**
		  * Liefert die Anzahl Parameter im Container
		  */
		 int count();
		 
		 
		 /**
		  * Generiert aus allen vorhanden Parameter ein XML Snipped
		  * Beispiel:
		  * <param name="path" type="string">/tmp/img/</param>
		  * <param name="x" type="int">1212</param>
		  */
		 std::string toXml() const; 
		 
		 
	protected:
		/**
		 * Loescht alle Parameter aus dem Inhalt des ParameterContainers und initialisiert die ParameterMap
		 */
		 void clear();
		 
		 
		 void add(const std::string& name, fliplib::Parameter* parameter);		 
				 
		
	private:		
		typedef std::map<std::string, Parameter*> ParameterMap;
		ParameterMap map_;	
		
		// hide copy cto & =
		ParameterContainer(const ParameterContainer &copy);
		ParameterContainer& operator=(ParameterContainer&);			
	}; // ParameterContainer


	/**
	 * Proxyklasse, die einen Parameter implizit konvertiert, so dass kein template-Parameter angegeben werden muss.
	 */
	class ConversionProxy {
		public:
			ConversionProxy(const std::string& name, const ParameterContainer&	parameterContainer) :
				name_				( name ),
				parameterContainer_	( parameterContainer )
			{}
			template <typename T>
			operator T() const { return parameterContainer_.findParameter(name_).convert<T>(); }	// implicit cast (template parameter is deducted on call)
			template <typename T>
			T convert() const { return parameterContainer_.findParameter(name_).convert<T>(); }		// explicit cast (template parameter must be provided), legacy, enums

		private:
			const std::string&			name_;
			const ParameterContainer&	parameterContainer_;
	}; // ConversionProxy

} // namespace fliplib
#endif // !defined(EA_35BB4086_367D_411b_A090_F339463DD547__INCLUDED_)

