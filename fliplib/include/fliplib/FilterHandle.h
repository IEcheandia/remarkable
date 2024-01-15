///////////////////////////////////////////////////////////
//  FilterHandle.h
//  Implementation of the Class FilterHandle
//  Created on:      28-Jan-2008 11:27:58
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_E92C0ADD_999C_4414_8DAA_515925E57640__INCLUDED_)
#define EA_E92C0ADD_999C_4414_8DAA_515925E57640__INCLUDED_

#include <string>
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilter.h"

namespace fliplib
{
	class FLIPLIB_API FilterHandle
	{
	
	public:
		/**
		 * Initalisiert eine neue Instanz der Klasse FilterHandle.
		 * \param [in] filter Zeiger auf Filterinstanz
		 * \param [in] componentUri Pfad der Library in welcher der Filter implementiert ist
		 */
		FilterHandle(fliplib::BaseFilter* filter, const std::string& componentUri);			
		
		/** 
		 * Zerstoert den FilterHandle. 
		 */ 
		virtual ~FilterHandle();
		
		fliplib::BaseFilter * getFilter() { return filter_ ; }
		const std::string& getComponentUri() const { return componentUri_; } 
		void free() { delete filter_; filter_ = 0; }  
		
	private:
		fliplib::BaseFilter* filter_;
		const std::string componentUri_;
		
		FilterHandle();
		FilterHandle(FilterHandle& cc);
	
	};
}

#endif // !defined(EA_E92C0ADD_999C_4414_8DAA_515925E57640__INCLUDED_)
