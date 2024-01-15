///////////////////////////////////////////////////////////
//  AbstractFilterVisitor.h
//  Implementation of the Class AbstractFilterVisitor
//  Created on:      22-Jan-2008 14:02:04
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_9605A939_B148_4cc9_B0DA_E74B3140AB48__INCLUDED_)
#define EA_9605A939_B148_4cc9_B0DA_E74B3140AB48__INCLUDED_

#include <string> 
#include "Poco/Foundation.h"
#include "fliplib/Fliplib.h"
#include "fliplib/Exception.h" 
#include "fliplib/FilterControlInterface.h"

namespace fliplib
{
	/**
	 * Abstrakte Interfaceklasse AbstractFilterVisitor
	 * 
	 * Stellt Methoden zur Verfuegung die dafuer sorgen, dass die Filter vor der eigentlichen Verarbeitung der Daten
	 * die Moeglichkeit haben bestimmte Arbeiten vorher zu erledigen. Zum Beispiel Allokation und freigabe von
	 * Speicher oder anderen Resourcen. Fuer jede Phase ist ein anderer Provider zustaendig und implementiert
	 * ein anderes Verhalten. 
	 * 
	 */	
	class FLIPLIB_API AbstractFilterVisitor
	{		
	public:
		virtual void control (FilterControlInterface& filter) = 0;
		virtual ~AbstractFilterVisitor() = 0;
	};
	
}
#endif // !defined(EA_9605A939_B148_4cc9_B0DA_E74B3140AB48__INCLUDED_)
