///////////////////////////////////////////////////////////
//  Activator.h
//  Implementation of the Class Activator
//  Created on:      28-Jan-2008 11:27:50
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_A5852560_50C5_4168_B88B_69EAC5586604__INCLUDED_)
#define EA_A5852560_50C5_4168_B88B_69EAC5586604__INCLUDED_

#include "Poco/Foundation.h"
#include "Poco/ClassLoader.h"
#include "fliplib/Fliplib.h"
#include "fliplib/FilterHandle.h"

namespace fliplib
{
	/**
	 * Activator (Singleton)
	 *
	 * Enthaelt Methoden zum erzeugen neuer Objekte die von der Klasse BaseFilterInterface ableiten. Die Objekte
	 * koennen in externen Bibliotheken implementiert sein. Die zu aktivierende Klasse muss im Manifest deklariert werden
	 * 
	 * Der Activator ist dafuer verantwortlich, dass die Resource Library freigegeben wird, wenn aus dieser keine Filter-Instanzen 
	 * mehr existieren. Aus diesem Grund, muessen alle Instanzen welche mit createInstance erzeugt wurden wieder mit
	 * destroyInstance freigegeben werden. 
	 * 
	 */ 
	
	class FLIPLIB_API Activator
	{
	
	public:		
		/** 
		 * Erzeug eine neue Instance FilterHandle. Der FileHandle MUSS mit der Methode
		 * destroyInstance wieder freigegeben werden.
		 * 
		 * \param [in] componentUri Pfad auf die Komponente in welcher der Filter implementiert ist
		 * \param [in] filter Name des Filters
		 */
		static fliplib::FilterHandle * createInstance( const std::string& componentUri , const std::string& filter );
		
		/**
		 * Zerstoert die Instance FilterHandle und den Filter. Die Libraryresource wird freigeben
		 */
		static void destroyInstance(fliplib::FilterHandle* filterHandle);
		
	private:
		/**
		 * Konstrukor private, damit man sich keine Instanz holen kann
		 */
		Activator();
		virtual ~Activator();
		
		/**
		 * Den Kopierkonstruktor schuetzen um zu vermeiden, das das Objekt unbeabsichtigt kopiert wird
		 */
		Activator(const Activator& cc) {};
		
		/**
		 * Zeiger auf einzige Instance  (Singleton Pattern)
		 */
		static Activator* instance;
		
		/**
		 * Liefert einen Zeiger auf die einzige Instance Activator
		 */
		static Activator* getInstance();
		 
		 /**
		  * Poco Klassenloader
		  */
		Poco::ClassLoader<BaseFilterInterface>* classLoader_;
		 
		 /**
		 * Zerstoert die einzige Instanze Activator
		 */
		static void destroy();		
		 
		 
		 /** 
		  * Wird aufgerufen, wenn die Anwendung beendet wird;
		  */
		static void exit_handler();
	};

}
#endif // !defined(EA_A5852560_50C5_4168_B88B_69EAC5586604__INCLUDED_)
