///////////////////////////////////////////////////////////
//  PipeGroupEvent.h
//  Implementation of the Class PipeGroupEvent
//  Created on:      30-Okt-2007 14:29:47
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(A2_BB421F9D_81A0_4d66_ADB3_A12E7EBA67DC__INCLUDED_)
#define A2_BB421F9D_81A0_4d66_ADB3_A12E7EBA67DC__INCLUDED_

#include <typeinfo>
#include <string>
#include <map>
#include <array>

#include "Poco/Foundation.h"
#include "Poco/BasicEvent.h"

#include "fliplib/Fliplib.h"
#include "fliplib/BasePipe.h"
#include "fliplib/PipeEventArgs.h"

#include "common/defines.h"

namespace fliplib
{		
	/**
	 * PipeGroupEvent
	 * 
	 * Eine Pipe signalisiert, wenn neue Daten bereitstehen. Mit Hilfe der Klasse PipeGroupEvent lassen sich die Signale mehrerer Pipes
	 * zu einer einzigen zusammenfassen.   
	 * 
	 * Der Filter installiert eine seine Handlermethode wie folgt:
	 * \code
	 * 		mygroupevent_+= Delegate<MyFilter, fliplib::PipeEventArgs>(this, &MyFilter::myEventHandlerMethod);
	 * \endcode
	 * 	 
	 **/
	class FLIPLIB_API PipeGroupEvent : public Poco::BasicEvent<fliplib::PipeGroupEventArgs>
	{
		
	public:	
	
		/** 
		 * Konstruktor
		 */
		PipeGroupEvent();

		/**
		 * Destruktor
		 */		
		virtual ~PipeGroupEvent();

		/**
		 * Eine Pipe wird in die Gruppe aufgenommen
		 * 
		 * \param [in] pipe Referenz auf die Pipe
		 */	
		void add(BasePipe &pipe);
		
		/**
		 *  Loescht eine Pipe aus der Gruppe
		 * 
		 *  \param [in] pipe Referenz auf die Pipe
		 */
		void remove(BasePipe &pipe);		
			
		/**
		 * Liefert die Pipe in Abhaengigkeit ihres Namens
		 */
		BasePipe* pipe(const std::string& name) const;
		BasePipe* pipeByTag(const std::string& tag);
		
		/** 
		 * Liefert die Pipe in Abhaengigkeit des Types. Null wenn keine Pipe gefunden wurde
		 */
		BasePipe* pipe(const std::type_info& type);
		
		/** 
		 * Liefert true, wenn eine Pipe mit einem bestimmten Namen vorhanden ist
		 */
		bool exists(const std::string& name) const;
		
		/**
		 * Liefert die Anzahl der Pipes welche in der Gruppe zusammengefasst sind
		 */
		int count() const;

		/**
		 * Setzt den signal-Zaehler fuer alle Eingangspipes zurueck. Sinnvoll bei Arm Nahtstart falls noch auf Signals gewartet wird.
		 */
		void resetSignalCounters();

		/**
		 * Resets the signal counter for the @p imageNumber. That is for a specific processing thread
		 *
		 * Useful in case not all samples were signaled in case a sensor doesn't provide data.
		 */
		void resetSignalCounter(int imageNumber);

	protected:
		/**
		 * Diese Methode verarbeitet die Signale der Pipes 
		 */
		void signalhandler(const void* sender, fliplib::PipeEventArgs& e);	
		
	private:
        typedef	std::map<BasePipe*, std::array<bool, g_oNbParMax>>	signaled_map_t;
    			
		int					            members_;					// Anzahl Pipes in der Gruppe
		typedef std::vector<BasePipe*> 	SenderPipeList;
		SenderPipeList		            list_;
		signaled_map_t	    			signalerCounters_;	        // Pipes welche signalisiert haben
	};
	
}
	
#endif // !defined(A2_BB421F9D_81A0_4d66_ADB3_A12E7EBA67DC__INCLUDED_)
