///////////////////////////////////////////////////////////
//  GraphBuilderFactory.cpp
//  Implementation of the Class GraphBuilderFactory
//  Created on:      09-Nov-2007 09:22:45
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include "Poco/SharedPtr.h"
#include "fliplib/GraphBuilderFactory.h"
#include "fliplib/XmlGraphBuilder.h"
#include "fliplib/XmlLightGraphBuilder.h"

using namespace fliplib;

GraphBuilderFactory::GraphBuilderFactory(){

}



GraphBuilderFactory::~GraphBuilderFactory(){

}

Poco::SharedPtr<AbstractGraphBuilder> GraphBuilderFactory::create()
{	
	Poco::SharedPtr<XmlGraphBuilder> ptrObj(new XmlGraphBuilder());
	return ptrObj;
}

Poco::SharedPtr<AbstractGraphBuilder> GraphBuilderFactory::createLight()
{	
	Poco::SharedPtr<XmlLightGraphBuilder> ptrObj(new XmlLightGraphBuilder());
	return ptrObj;
}
