///////////////////////////////////////////////////////////
//  Activator.cpp
//  Implementation of the Class Activator
//  Created on:      28-Jan-2008 11:27:50
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include <iostream> 
#include <stdlib.h>
#include "Poco/Path.h"
#include "Poco/ClassLoader.h"
#include "Poco/Manifest.h"
#include "fliplib/Activator.h"

using std::cout;
using std::endl;
using Poco::Manifest;
using Poco::ClassLoader;
using fliplib::Activator;
using fliplib::FilterHandle;

Activator* Activator::instance = 0;

Activator::Activator()
	: classLoader_(new Poco::ClassLoader<BaseFilterInterface>())
{
	// install exit handler
	atexit( Activator::exit_handler );
}

Activator::~Activator()
{
	delete classLoader_;
	classLoader_ = 0;
}

void Activator::exit_handler()
// Wird aufgerufen, wenn das Program beendet wird. 
{ 
   	Activator::destroy(); 
} 


void Activator::destroy()
{
	delete Activator::instance;
	Activator::instance = 0;
}

void Activator::destroyInstance(fliplib::FilterHandle* filterHandle)
{
	
	// Library unloaden
	Activator* activator = Activator::getInstance();
	activator->classLoader_->unloadLibrary(filterHandle->getComponentUri());

	// und tschuess	
	delete filterHandle;
	filterHandle = 0;	
}

fliplib::FilterHandle * Activator::createInstance( const std::string& componentUri, const std::string& filter )
{	
	Activator* activator = Activator::getInstance();
	
	// Fuer jede Instance wird Load Library aufgerufen. 
	activator->classLoader_->loadLibrary(componentUri);
	
	// Library geladen und Klasse in Library vorhanden?
	if (activator->classLoader_->findClass(filter))
	{
		// aktiviere Filter
		BaseFilterInterface* pInterface = activator->classLoader_->classFor(filter).create();
		BaseFilter* pFilter = static_cast<BaseFilter*>(pInterface);
		
		FilterHandle* newFileHandle = new FilterHandle(pFilter, componentUri);
					
		return newFileHandle;
	}
	
 	return NULL;
}

Activator* Activator::getInstance()
{
	if ( ! Activator::instance )
		 Activator::instance = new Activator();
		
	return  Activator::instance;
}
