///////////////////////////////////////////////////////////
//  BasePin.cpp
//  Implementation of the Class BasePin
//  Created on:      30-Okt-2007 14:19:36
//  Original author: Sevitec
///////////////////////////////////////////////////////////
#include <string.h>
#include <sstream>
#include "Poco/SharedPtr.h"
#include "fliplib/BasePipe.h"
#include "fliplib/BaseFilter.h"

using Poco::SharedPtr;
using fliplib::BasePipe;
using fliplib::BaseFilterInterface;


BasePipe::BasePipe(BaseFilterInterface* parent, const std::string& name)
	: parent_(parent), name_(name), tag_(""), hash_(0)
{
	parent_->registerPipe(this, "unknown", -1);
}

BasePipe::BasePipe(BaseFilterInterface* parent, const std::string& name, const std::string& contentType, int channel)
	: parent_(parent), name_(name), tag_(""), hash_(0)
{
	parent_->registerPipe(this, contentType, channel);
}

BasePipe::~BasePipe()
{
	parent_->unregisterPipe(this);
}


std::string BasePipe::name() const
// Liefert den Name  der Pipe
{
	return name_;
}


const std::type_info& BasePipe::type() const
{
	return getType();
}

std::string BasePipe::tag() const
{
	return tag_;
}

void BasePipe::setTag(const std::string& tag)
{
	tag_ = tag;
}

bool BasePipe::isPipeType(const std::type_info& type)
{
	if (!hash_)
		hash_ = getHash( getType().name(), strlen( getType().name() ));

	return hash_ == getHash( type );
}


unsigned int BasePipe::getHash()
{
	return getHash( getType().name(), strlen(getType().name()) );
}

unsigned int BasePipe::getHash(const std::type_info& type)
{
	return getHash( type.name(), strlen(type.name()) );
}

unsigned int BasePipe::getHash(const char* str, unsigned int len)
{
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int hash = 0;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = hash * a + (*str);
      a    = a * b;
   }

   return hash;
}
