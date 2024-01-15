///////////////////////////////////////////////////////////
//  NullRenderer.h
//  Implementation of the Class NullSourceFilter
//  Created on:      17-Okt-2007 17:45:04
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_9DB6BD8D_D2CC_4c7f_B735_BCF8937ABA1B__INCLUDED_)
#define EA_9DB6BD8D_D2CC_4c7f_B735_BCF8937ABA1B__INCLUDED_

#include "Poco/Activity.h"
#include "Poco/SharedPtr.h"
#include "fliplib/Fliplib.h"
#include "fliplib/SynchronePipe.h"
#include "fliplib/SourceFilter.h"
#include "fliplib/NullPipeContent.h"

namespace fliplib
{
	/** 
	 * Der NullSourceFilter ist ein "Test" Filter. Er generiert leere Nachrichten bzw. er versendet vorbereitete Nachrichten.  
	 * 
	 * Filtername:	NullSourceFilter
	 * Pipe:		keine
	 *  
	 */
	class FLIPLIB_API NullSourceFilter : public SourceFilter
	{

	public:
		NullSourceFilter();
		// Konstruktor fuer NullSourceFilter
		
		virtual ~NullSourceFilter();
						
		void fire();
		// Sendet ein leeres Paket
		
		static const std::string FILTERNAME;
		static const std::string PIPENAME;
		
	private:
		typedef SynchronePipe<NullPipeContent> NullSourcePipe;
		NullSourcePipe* output_;

	};
	
}
#endif // !defined(EA_9DB6BD8D_D2CC_4c7f_B735_BCF8937ABA1B__INCLUDED_)
