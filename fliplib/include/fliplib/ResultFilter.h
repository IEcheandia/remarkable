///////////////////////////////////////////////////////////
//  NullFilter.h
//  Implementation of the Class ResultFilter
//  Created on:      17-Okt-2007 17:45:04
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_9DB6BD8D_D2CC_4c7f_B735_4711937AEFA5__INCLUDED_)
#define EA_9DB6BD8D_D2CC_4c7f_B735_4711937AEFA5__INCLUDED_

#include "Poco/UUID.h"
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilter.h"

namespace fliplib
{
	/**
	 * Von der ResultFilter Klasse leiten alle ResultFilterimplementationen ab. Ein ResultFilter besitzt
	 * keine Ausgaenge.
	 */
	class FLIPLIB_API ResultFilter : public BaseFilter
	{

	public:
		/**
		 * Konstruktor
		 * 
		 * \param [in] name Name des Filters
		 */
		ResultFilter(const std::string& name);	
		ResultFilter(const std::string& name, const Poco::UUID & filterID);
		
		/** 
		 * Destruktor
		 */
		virtual ~ResultFilter();
			
		/**
		 * @brief Get filter type.
		 * @return int FilterType enum - here RESULT.
		 */
		virtual int getFilterType() const;

	protected:
		/*virtual*/ void setParameter();		// set base filter parameter that all filters have in common
		
	private:
		// verbiete eigene Pipes zu registrieren
		void Register(BasePipe *pipe);
		void Unregister(const BasePipe *pipe);
			
		// hide constructor			
		ResultFilter();
		ResultFilter(const BaseFilter &);	
		ResultFilter& operator=(ResultFilter&);	
	};
	
}
#endif // !defined(EA_9DB6BD8D_D2CC_4c7f_B735_4711937AEFA5__INCLUDED_)
