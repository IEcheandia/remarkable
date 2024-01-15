///////////////////////////////////////////////////////////
//  NullFilter.h
//  Implementation of the Class TransformFilter
//  Created on:      17-Okt-2007 17:45:04
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABB02__INCLUDED_)
#define EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABB02__INCLUDED_

#include <string>
#include "Poco/UUID.h"
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilter.h"

namespace fliplib
{
	/** 
	 * Von der SinkFilter Klasse leiten alle SourceFilterimplementationen ab. 
	 */
	class FLIPLIB_API TransformFilter : public BaseFilter
	{

	public:
		/**
		 * Konstruktor
		 * 
		 * \param [in] name Name des Filters
		 */
		TransformFilter(const std::string& name);
		TransformFilter(const std::string& name, const Poco::UUID & filterID);
		
		/**
		 * Destruktor
		 */
		virtual ~TransformFilter();
		
		/**
		 * @brief Get filter type.
		 * @return int FilterType enum - here TRANSFORM.
		 */
		virtual int getFilterType() const;

	protected:
		/*virtual*/ void setParameter();		// set base filter parameter that all filters have in common
		
	private:
		// hide constructor
		TransformFilter();
		TransformFilter(const BaseFilter &);
		TransformFilter& operator=(TransformFilter&);
	};
	
}
#endif // !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABB02__INCLUDED_)
