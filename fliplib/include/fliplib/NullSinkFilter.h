///////////////////////////////////////////////////////////
//  NullRenderer.h
//  Implementation of the Class NullSinkFilter
//  Created on:      17-Okt-2007 17:45:04
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABA1B__INCLUDED_)
#define EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABA1B__INCLUDED_

#include "fliplib/Fliplib.h"
#include "fliplib/SinkFilter.h"

namespace fliplib
{
	/** 
	 * Der NullSinkFilter ist ein "Test" Filter. Saemtliche Daten die dem NullSinkFilter gesendet werden verarbeitet.  
	 * 
	 * Filtername:	NullSinkFilter
	 * Pipe:		keine
	 *  
	 */
	class FLIPLIB_API NullSinkFilter : public SinkFilter
	{

	public:
		/**
		 * Konstruktor
		 */
		NullSinkFilter();
		
		/**
		 * Destruktor
		 */ 
		virtual ~NullSinkFilter();
		
		/**
		 * Fuer Testsuite. Wird bei jedem Proceed inkrementiert
		 */	
		inline int count() { return count_; };
		
		static const std::string FILTERNAME;
		
	private:
		/**
		 * Verarbeitet saemtliche Ereignisse anderer Filter. Achtung. Daten werden keine geloescht!
		 */									
		void proceed(const void* sender, PipeEventArgs& e);
		int count_;	 

	};
	
}
#endif // !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABA1B__INCLUDED_)
