///////////////////////////////////////////////////////////
//  GraphBuilderFactory.h
//  Implementation of the Class GraphBuilderFactory
//  Created on:      09-Nov-2007 09:22:45
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_FA3A345D_23B5_4b12_99AD_C8F0852E2C5F__INCLUDED_)
#define EA_FA3A345D_23B5_4b12_99AD_C8F0852E2C5F__INCLUDED_

#include "Poco/SharedPtr.h" 
#include "fliplib/Fliplib.h"
#include "fliplib/AbstractGraphBuilder.h"

namespace fliplib
{
	/**
	 * Factoryklasse fuer den GraphBuilder.
	 */
	class FLIPLIB_API GraphBuilderFactory
	{
	
	public:
		/** 
		 * Standardkonstruktor
		 */
		GraphBuilderFactory();
		/**
		 * Destruktor
		 */
		virtual ~GraphBuilderFactory();
		
		/**
		 * Erzeugt einen neuen WM XML GraphBuilder
		 */
		Poco::SharedPtr<AbstractGraphBuilder> create();
		
		/**
		 * Erzeugt einen neuen light XML GraphBuilder
		 */
		Poco::SharedPtr<AbstractGraphBuilder> createLight();				
	};
	
}
#endif // !defined(EA_FA3A345D_23B5_4b12_99AD_C8F0852E2C5F__INCLUDED_)
