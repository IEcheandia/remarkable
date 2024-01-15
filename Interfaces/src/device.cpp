/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, WOR, HS
 * 	@date		2012
 * 	@brief		Key value type.
 */

// project includes
#include "message/device.h"
// poco includes
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/AutoPtr.h"
#include <cstdint>

using Poco::AutoPtr;
using Poco::Util::XMLConfiguration;


namespace precitec {
namespace interface {


template <class T>
KeyValue* CreateKeyValue( MessageBuffer const& buffer ) {
	return new TKeyValue<T>( buffer );
} // CreateKeyValue



typedef KeyValue* (*KeyValueFactory)(MessageBuffer const& buffer);
KeyValueFactory keyValueFactoryList[TNumTypes] = {
	CreateKeyValue<char>,
	CreateKeyValue<byte>,
	CreateKeyValue<int>,
	CreateKeyValue<uInt>,
	CreateKeyValue<bool>,
	CreateKeyValue<float>,
	CreateKeyValue<double>,
	CreateKeyValue<std::string>
}; // keyValueFactoryList



KeyValue* KeyValue::create(int type , MessageBuffer const& buffer) {
	return keyValueFactoryList[type]( buffer );
} // create	// Erzeugt via Factory eine neue Version



void writeToFile(const std::string &p_rFilePath, const Configuration &p_rConfiguration) {
	try {
		AutoPtr<XMLConfiguration> pConf(new XMLConfiguration);
		pConf->loadEmpty("key_value_configuration"); // abitrary root node name - not needed for reading

		for(auto it(std::begin(p_rConfiguration)); it != std::end(p_rConfiguration); ++it) {
			const Types oType	(it->get()->type());
			switch (oType) { 
			case TBool: {
				const auto pKvInt	(static_cast<const TKeyValue<bool>*>(it->get()));
				pConf->setBool(pKvInt->key(), pKvInt->value());
				break;
			}
			case TInt: {
				const auto pKvInt	(static_cast<const TKeyValue<int>*>(it->get()));
				pConf->setInt(pKvInt->key(), pKvInt->value());
				break;	
			}
			case TUInt: {
				const auto pKvInt	(static_cast<const TKeyValue<uint32_t>*>(it->get()));
				pConf->setInt(pKvInt->key(), static_cast<int>(pKvInt->value())); // NOTE: uint not possible, needs cast when read with getInt()
				break;
			}
			case TString: {
				const auto pKvString	(static_cast<const TKeyValue<std::string>*>(it->get()));
				pConf->setString(pKvString->key(), pKvString->value());
				break;
			}
            case TFloat: {
				const auto pKvFloat(static_cast<const TKeyValue<float>*>(it->get()));
				pConf->setDouble(pKvFloat->key(), pKvFloat->value()); // NOTE: there is no setFloat in the Poco-baseclass ...
				break;
			}
            case TDouble: {
				const auto pKvDouble(static_cast<const TKeyValue<double>*>(it->get()));
				pConf->setDouble(pKvDouble->key(), pKvDouble->value());
				break;
			}
			default:
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << ": invalid value type: " << oType;
				throw system::NotSupportedException(oMsg.str());
			} // switch
		} // for
		pConf->save(p_rFilePath);
	}
	catch(const Poco::Exception &p_rException) {
		std::cerr  << "Poco::Exception in " << __FUNCTION__ << ": " << p_rException.what() << "\n\t" << p_rException.message().c_str() << std::endl;
		throw;
	}
} // writeToFile


} // interface
} // precitec
