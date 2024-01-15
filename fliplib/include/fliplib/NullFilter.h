///////////////////////////////////////////////////////////
//  NullFilter.h
//  Implementation of the Class NullFilter
//  Created on:      17-Okt-2007 17:45:04
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABA1D__INCLUDED_)
#define EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABA1D__INCLUDED_

#include "fliplib/Fliplib.h"
#include "fliplib/SynchronePipe.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/NullPipeContent.h"

namespace fliplib
{
	/** 
	 * Der Nullfilter ist ein "Test" Filter und besitzt einen Ausgang. Saemtliche Daten die dem NullFilter gesendet werden,
	 * werden an den Ausgang weitergeleitet. Die Daten werden nicht veraendert.
	 * 
	 * Filtername:	NullFilter
	 * Pipe:		NullPipe (PipeType:	void)
	 *  
	 */
	class FLIPLIB_API NullFilter : public TransformFilter
	{

	public:
		/**
		 * Konstruktor
		 */
		NullFilter();
		/**
		 * Destruktor
		 */
		virtual ~NullFilter();

		/**
		 * Fuer Testsuite. Wird bei jedem Proceed inkrementiert
		 */							
		inline int count() { return count_;	}
		
		static const std::string FILTERNAME;
		static const std::string PIPENAME;
			
	private:		
		/**
		 * (Override) Verarbeite saemtliche Messages
		 */						
		void proceed(const void* sender, PipeEventArgs& e);
	
		typedef SynchronePipe<NullPipeContent> NullPipe;		//NullPipe 
		NullPipe* output_;
		int count_;			
	};
	
}
#endif // !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABA1D__INCLUDED_)
