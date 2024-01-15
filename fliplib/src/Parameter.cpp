///////////////////////////////////////////////////////////
//  Parameter.cpp
//  Implementation of the Class Parameter
//  Created on:      11-Dez-2007 17:22:25
//  Original author: Sevitec
///////////////////////////////////////////////////////////
#include <string>
#include <sstream>
#include "Poco/DynamicAny.h"
#include "Poco/Types.h"

#include "fliplib/Fliplib.h"
#include "fliplib/Parameter.h"
#include "fliplib/Exception.h"

using Poco::DynamicAny;
using fliplib::Parameter;

const  std::string fliplib::Parameter::TYPE_Int8 	= "Int8";
const  std::string fliplib::Parameter::TYPE_Int16	= "Int16";
const  std::string fliplib::Parameter::TYPE_Int32	= "Int32";
const  std::string fliplib::Parameter::TYPE_Int64	= "Int64";
const  std::string fliplib::Parameter::TYPE_UInt8	= "UInt8";
const  std::string fliplib::Parameter::TYPE_UInt16	= "UInt16";
const  std::string fliplib::Parameter::TYPE_UInt32	= "UInt32";
const  std::string fliplib::Parameter::TYPE_UInt64	= "UInt64";
const  std::string fliplib::Parameter::TYPE_int		= "int";
const  std::string fliplib::Parameter::TYPE_uint	= "uint";
const  std::string fliplib::Parameter::TYPE_bool	= "bool";
const  std::string fliplib::Parameter::TYPE_float	= "float";
const  std::string fliplib::Parameter::TYPE_double	= "double";
const  std::string fliplib::Parameter::TYPE_char	= "char";
const  std::string fliplib::Parameter::TYPE_string	= "string";
const  std::string fliplib::Parameter::TYPE_text	= "text";

// Exceptions (premises for inspection not met)


Parameter::Parameter(const std::string& name, const std::string& type, const Poco::DynamicAny& value) :
	name_(name),
	type_(type),
	value_(convertToType(type, value)),
	updated_(true)
{
}

Parameter::Parameter(const std::string& name, const std::string& type, const std::string& value) :
	name_(name),
	type_(type),
	value_(convertToType(type, value)),
	updated_(true)
{
}

Parameter::~Parameter(){

}

Parameter::Parameter(const Parameter& obj)
{
	name_ = obj.name_;
	type_ = obj.type_;
	value_ = obj.value_;
	updated_ = obj.updated_;
}

Parameter& Parameter::operator=(const Parameter& obj)
{
	name_ = obj.name_;
	type_= obj.type_;
	value_ = obj.value_;
	updated_ = obj.updated_;

  	return *this;
}

const std::string& Parameter::getName() const
{
	return name_;
}

Poco::DynamicAny& Parameter::getValue()
{
	return value_;
}


const Poco::DynamicAny& Parameter::getValue() const
{
	return value_;
}

const std::string& Parameter::getType() const
{
	return type_;
}

void Parameter::setValue(const Poco::DynamicAny& value)
{
	value_ = value;
	updated_ = true;
}

std::string Parameter::toXml() const
{
	std::stringstream ss;
	std::string value = value_;

	ss << "<parameter";
	ss << " name=\"" << name_ ;
	ss << "\" dataType=\"" << type_;
	ss << "\" defaultValue=\"" << value;
	ss << "\"/>";

	return ss.str();
}



Poco::DynamicAny Parameter::convertToType(const std::string& type, const Poco::DynamicAny& value) {
	using namespace Poco;

	bool typeExist(true);

	Poco::DynamicAny any(value);
	if (type == Parameter::TYPE_bool)
		any = any.convert<bool>();
	else if (type == Parameter::TYPE_char)
		any = any.convert<char>();
	else if (type == Parameter::TYPE_double)
		any = any.convert<double>();
	else if (type == Parameter::TYPE_float)
		any = any.convert<float>();
	else if (type == Parameter::TYPE_int)
		any = any.convert<int>();
	else if (type == Parameter::TYPE_uint)
		any = any.convert<unsigned int>();
	else if (type == Parameter::TYPE_Int8)
		any = any.convert<Int8>();
	else if (type == Parameter::TYPE_Int16)
		any = any.convert<Int16>();
	else if (type == Parameter::TYPE_Int32)
		any = any.convert<Int32>();
	else if (type == Parameter::TYPE_Int64)
		any = any.convert<Int64>();
	else if (type == Parameter::TYPE_string)
		any = any.convert<std::string>();
	else if (type == Parameter::TYPE_text)
		any = any.convert<std::string>();
	else if (type == Parameter::TYPE_UInt8)
		any = any.convert<UInt8>();
	else if (type == Parameter::TYPE_UInt16)
		any = any.convert<UInt16>();
	else if (type == Parameter::TYPE_UInt32)
		any = any.convert<UInt32>();
	else if (type == Parameter::TYPE_UInt64)
		any = any.convert<UInt64>();
	else
		typeExist = false;

	if (typeExist == false) {
		throw ParameterException(std::string("parameter with type '") + value.type().name() + "' not convertible to type '" + type + "'.\n");
	}
	return any;
} // convertToType

