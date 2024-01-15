///////////////////////////////////////////////////////////
//  NullFilter.h
//  Implementation of the Class SinkFilter
//  Created on:      17-Okt-2007 17:45:04
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937AEFA2__INCLUDED_)
#define EA_9DB6BD8D_D2CC_4c7f_B735_A458937AEFA2__INCLUDED_

#include "event/results.h"
#include "fliplib/SynchronePipe.h"
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilter.h"
#include "Poco/UUID.h"
#include <vector>

namespace fliplib
{
	/**
	 * Von der SinkFilter Klasse leiten alle SinkFilterimplementationen ab. Ein Sinkfilter besitzt
	 * keine Ausgaenge.
	 */
	class FLIPLIB_API SinkFilter : public BaseFilter
	{

	public:
		/**
		 * Konstruktor
		 * 
		 * \param [in] name Name des Filters
		 */	
		SinkFilter(const std::string& name);
		SinkFilter(const std::string& name, const Poco::UUID & filterID);
		
		/**
		 * @brief Get filter type.
		 * @return int FilterType enum - here SINK.
		 */
		virtual int getFilterType() const;
	   
        /**
	     * @brief Clears in pipes
	     */
	    void clearInPipes() /*final*/;

    protected:
	    /**
	     * @brief In-pipe registration.
	     * @param p_rPipe Reference to pipe that is getting connected to the filter.
	     * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	     */
	    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) /*final*/; // result handler get only one type of in pipes. thus, subscribe can be generalized here in base class

        typedef fliplib::SynchronePipe<precitec::interface::ResultDoubleArray>	pipe_result_t;

        std::vector<const pipe_result_t*>   m_oInPipes; // result handler get only one type of in pipes.

	private:

		// verbiete eigene Pipes zu registrieren
		void Register(BasePipe *pipe);
		void Unregister(const BasePipe *pipe);
			
		// hide constructor			
		SinkFilter();
		SinkFilter(const BaseFilter &);	
		SinkFilter& operator=(SinkFilter&);	
	};
	
}
#endif // !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937AEFA2__INCLUDED_)
