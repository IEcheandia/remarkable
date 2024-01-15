///////////////////////////////////////////////////////////
//  BaseFilterInterface.h
//  Implementation of the Interface BaseFilterInterface
//  Created on:      17-Okt-2007 11:51:29
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_A02DCABD_AB2D_452d_973B_73F6984FDA21__INCLUDED_)
#define EA_A02DCABD_AB2D_452d_973B_73F6984FDA21__INCLUDED_

#include <string>
#include "Poco/Foundation.h"
#include "fliplib/Fliplib.h"
#include "fliplib/Exception.h"

namespace fliplib
{

	class BasePipe;

	/**
	 * Interface Klasse BaseFilterInterface (Interface)
	 *
	 * Diese Abstrakte Klasse ist die Schnittstelle zwischen den Bibliotheken und er Hautpanwendung. Jede Exportierte Klasse
	 * muss von diesem Interface ableiten.
	 */
	class FLIPLIB_API BaseFilterInterface
	{

	public:
		/**
		 * Standardkonstruktor fuer die BaseFilterInterfaceInterface Klasse
		 */
		BaseFilterInterface() {}

		/**
		 * Destruktor
		 */
		virtual ~BaseFilterInterface() {}

		/**
     	* Enum fuer FilterType
     	*/
		enum FilterType
		{
			SOURCE  	= 1,	//< Filter ohne Eingaenge
			SINK    	= 2,	//< Filter ohne Ausgaenge
			TRANSFORM  	= 3,	//< Filter mit Ein und Ausgaengem
			RESULT		= 4		//< Aehnlich wie SINK mit zusaetzlichen Interface
		};

		virtual int getFilterType() const = 0;	///< Getter FilterType muss von der Subklasse implementiert werden!

		virtual std::string name() const { throw fliplib::NotImplementedException(); }; //< Getter Liefert den Namen des Filters

		virtual std::string description() const {throw fliplib::NotImplementedException();  } //< Liefert eine genaue Beschreibung des Filters

		virtual void registerPipe(BasePipe* pipe, const std::string& contentType, int channel) = 0;

		virtual void unregisterPipe(const BasePipe *pipe) = 0;

	};

}
#endif // !defined(EA_A02DCABD_AB2D_452d_973B_73F6984FDA21__INCLUDED_)
