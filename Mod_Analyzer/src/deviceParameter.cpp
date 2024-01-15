/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2016
 * 	@brief		Contains key value parameter.
 */

#include "analyzer/deviceParameter.h"
#include "module/moduleLogger.h"
#include "common/defines.h"
#include "common/connectionConfiguration.h"
#include "geo/range.h"
#include "system/tools.h"

#include "Poco/Util/XMLConfiguration.h"

#include <iostream>
#include <limits>

namespace  {
    const std::string keyDebugTimings   =	"DebugTimings";
    const std::string keyConservativeCheckOvertriggering   =	"ConservativeCheckOvertriggering";
    const std::string keyDebugInspectManagerTime_us = "DebugInspectManagerTime_us";
    const std::string keyLogAllFilterProcessingTime = "LogAllFilterProcessingTime";
    const std::string keyToleranceOvertriggering_ms = "ToleranceOvertriggering_ms";
}

using namespace Poco;
namespace precitec {
	using namespace interface;
	using namespace geo2d;
	using namespace system;
namespace analyzer {


/*static*/ const std::string			DeviceParameter::m_oKeyIsParallelInspection	         =	"IsParallelInspection";
/*static*/ const std::string			DeviceParameter::m_oKeyDisableHWResults	             =	"DisableHWResults";
/*static*/ const std::string			DeviceParameter::m_oKeyLineLaser1DefaultIntensity    =	"LineLaser1DefaultIntensity";
/*static*/ const std::string			DeviceParameter::m_oKeyLineLaser2DefaultIntensity    =	"LineLaser2DefaultIntensity";
/*static*/ const std::string			DeviceParameter::m_oKeyFieldLight1DefaultIntensity   =	"FieldLight1DefaultIntensity";
/*static*/ const std::string            DeviceParameter::m_oKeyRealTimeGraphProcessing       =  "RealTimeGraphProcessing";



DeviceParameter::DeviceParameter(bool simulationStation)
    : m_simulationStation(simulationStation)
{
	//
	// parameter definition				  							key								value		min				max				default		// value and default should be equal

    auto fInsertParameter = [this] (sp_key_value_t && kv)
        {
            m_oParameters.insert(key_value_map_t::value_type{ kv->key(), kv });
        };
	auto oSpKeyValue0	=	sp_key_value_t{ new TKeyValue<bool>{	m_oKeyIsParallelInspection,		false,		false,			true,			false } };
	m_oParameters.insert(key_value_map_t::value_type{ oSpKeyValue0->key(), oSpKeyValue0 });
	auto oSpKeyValue1	=	sp_key_value_t{ new TKeyValue<bool>{	m_oKeyDisableHWResults,		    false,		false,			true,			false } };
	m_oParameters.insert(key_value_map_t::value_type{ oSpKeyValue1->key(), oSpKeyValue1 });
	auto oSpKeyValue2	=	sp_key_value_t{ new TKeyValue<int>{	    m_oKeyLineLaser1DefaultIntensity,   0,		    0,		    100,			0 } };
	m_oParameters.insert(key_value_map_t::value_type{ oSpKeyValue2->key(), oSpKeyValue2 });
	auto oSpKeyValue3	=	sp_key_value_t{ new TKeyValue<int>{	    m_oKeyLineLaser2DefaultIntensity,  80,		    0,		    100,			80 } };
	m_oParameters.insert(key_value_map_t::value_type{ oSpKeyValue3->key(), oSpKeyValue3 });
	auto oSpKeyValue4	=	sp_key_value_t{ new TKeyValue<int>{	    m_oKeyFieldLight1DefaultIntensity,  0,		    0,		    100,			0 } };
	m_oParameters.insert(key_value_map_t::value_type{ oSpKeyValue4->key(), oSpKeyValue4 });
    
    fInsertParameter(sp_key_value_t{ new TKeyValue<bool>{	keyDebugTimings,	g_oDebugTimings,		false,			true,			false } }); 
    fInsertParameter(sp_key_value_t{ new TKeyValue<bool>{	keyConservativeCheckOvertriggering,		m_oConservativeCheckOvertriggeringValue,		false,			true,			false } });
    fInsertParameter(sp_key_value_t{ new TKeyValue<int>{	keyDebugInspectManagerTime_us,		g_oDebugInspectManagerTime_us,		0,			10000,			400 } });
    fInsertParameter(sp_key_value_t{ new TKeyValue<bool>{	keyLogAllFilterProcessingTime,		m_oLogAllFilterProcessingTime,		false,			true,			false } });
    fInsertParameter(sp_key_value_t{ new TKeyValue<bool>{	m_oKeyRealTimeGraphProcessing,		m_realTimeGraphProcessing,		false,			true,			false } });
    fInsertParameter(sp_key_value_t{ new TKeyValue<double>{	keyToleranceOvertriggering_ms,		m_oToleranceOvertriggering_ms, 0.0, 100.0, 1.0, 1} });

    //	get parameters from xml
	//

	try
	{
		// build xml configuration directory.

		auto oWmBaseDir	=	getenv("WM_BASE_DIR") ? (std::string(getenv("WM_BASE_DIR")) + "/") : "/tmp/precitec/";
		auto oConfigDir	=	Path{ oWmBaseDir };
		m_oConfigDir.pushDirectory("config");
		m_oConfigDir.setFileName("workflow.xml");

		// fetch parameters from xml

		File oConfigFile(m_oConfigDir);
		if (oConfigFile.exists() == true) {
			AutoPtr<Util::XMLConfiguration> pConfIn;
			try { // poco syntax exception might be thrown or sax parse excpetion
				pConfIn	= new Util::XMLConfiguration(m_oConfigDir.toString());
			} // try
			catch(const Exception &p_rException) {
				wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
				wmLog(eDebug, "Could not read parameters from file:\n");
				wmLog(eDebug, "'%s'.\n", m_oConfigDir.toString().c_str());
				wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, p_rException.message().c_str());
			} // catch
			for(auto it(std::begin(m_oParameters)); it != std::end(m_oParameters); ++it) {
				//std::cout << __FUNCTION__ << "reading key:\n" << it->second->key() << "\n"; // debug
				const Types oType	( it->second->type() );
				try {
					switch (oType) {
					case TBool:
						it->second->setValue( pConfIn->getBool(		it->second->key(), it->second->defValue<bool>() 		) ); // assign value
						break;
					case TInt:
						it->second->setValue( pConfIn->getInt(		it->second->key(), it->second->defValue<int>() 			) ); // assign value
						break;
					case TUInt: // NOTE: theres is no 'setUInt()' or 'getUInt()', thus needs cast, because written with 'setInt()'
						it->second->setValue(
								static_cast<uint32_t>(pConfIn->getInt(
										it->second->key(), it->second->defValue<uint32_t>()))); // assign value
						break;
					case TString:
						it->second->setValue( pConfIn->getString(	it->second->key(), it->second->defValue<std::string>() 	) ); // assign value
						break;
					case TDouble:
						it->second->setValue( pConfIn->getDouble(	it->second->key(), it->second->defValue<double>() 		) ); // assign value
						break;
					default:
						std::ostringstream oMsg;
						oMsg << "Invalid value type: '" << oType << "'\n";
						wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
						break;
					} // switch
				} // try
				catch(const Exception &p_rException) {
					wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
					std::ostringstream oMsg;
					oMsg << "Parameter '" << it->second->key().c_str() << "' of type '" << oType << "' could not be converted. Reset to default value.\n";
					wmLog(eDebug, oMsg.str());
					wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
				} // catch
			} // for

		} // if
		else {
			std::ostringstream oMsg;
			oMsg << "Configuration file '" << m_oConfigDir.toString() << "' not found.\n";
			wmLog(eDebug, oMsg.str()); oMsg.str("");
			oMsg << "Using default parameters. File will be created.\n";
			wmLog(eDebug, oMsg.str());

            //	write new configuration to file
	        //

	        try
	        {
		        const auto oConf = makeConfiguration(*this);
		        writeToFile(m_oConfigDir.toString(), oConf);
	        } // try
	        catch(...) {
		        logExcpetion(__FUNCTION__, std::current_exception());
	        } // catch
		} // else

		invalidToDefault(*this);
	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch

	//	set current parameter values
	//

	const auto oItFoundParallel			=	m_oParameters.find(m_oKeyIsParallelInspection);
	const auto oIsParallelInspection	=	oItFoundParallel->second->value<bool>();

	g_oNbPar    =	oIsParallelInspection && !m_simulationStation ? g_oNbParMax : 1u;

    const auto oFoundKeyLG1Intensity        =   m_oParameters.find(m_oKeyLineLaser1DefaultIntensity);
    g_oLineLaser1DefaultIntensity           =   oFoundKeyLG1Intensity->second->value<int>();
    const auto oFoundKeyLG2Intensity        =   m_oParameters.find(m_oKeyLineLaser2DefaultIntensity);
    g_oLineLaser2DefaultIntensity           =   oFoundKeyLG2Intensity->second->value<int>();
    const auto oFoundKeyLG3Intensity        =   m_oParameters.find(m_oKeyFieldLight1DefaultIntensity);
    g_oFieldLight1DefaultIntensity          =   oFoundKeyLG3Intensity->second->value<int>();
    

    m_oParameters[keyDebugTimings]->setValue(g_oDebugTimings); //not persistent, ignore what was in the xml file - NB as a side-effect it's not reloaded in simulation 
    
    m_oConservativeCheckOvertriggeringValue = m_oParameters.at(keyConservativeCheckOvertriggering)->value<bool>();
    g_oDebugInspectManagerTime_us = m_oParameters.at(keyDebugInspectManagerTime_us)->value<int>();
    
    m_oParameters[m_oKeyDisableHWResults]->setValue(g_oDisableHWResults); // not persistent, ignore what was in the xml file  
    m_oLogAllFilterProcessingTime = m_oParameters.at(keyLogAllFilterProcessingTime)->value<bool>();
    m_oToleranceOvertriggering_ms = m_oParameters.at(keyToleranceOvertriggering_ms)->value<double>();

    m_realTimeGraphProcessing = m_oParameters.at(m_oKeyRealTimeGraphProcessing)->value<bool>();
    
} // DeviceParameter()



KeyHandle DeviceParameter::set(SmpKeyValue p_oSmpKeyValue)
{        
#if defined __QNX__ || defined __linux__ // otherwise wm host clr error
	UNUSED const auto&& rScopedLock	=	std::lock_guard<std::mutex>{ m_oWriteMutex };
#endif

	const auto &rKey		( p_oSmpKeyValue->key() );
	      auto oItFound		( m_oParameters.find(rKey) );
	const auto oItMapEnd	( m_oParameters.end() );
    
	if (oItFound == oItMapEnd)
	{
		std::ostringstream oMsg;
		oMsg << __FUNCTION__ << ":Key '" << rKey << "'NOT found.\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
		return -1;
	} // if

	bool valid = false;
    const Types oType(oItFound->second->type());
    switch ( oType )
    {
        case TInt:
            valid = setValue<int>(p_oSmpKeyValue, oItFound->second);
            break;
        case TDouble:
            valid = setValue<double>(p_oSmpKeyValue, oItFound->second);
            break;
        case TBool:
            valid = setValue<bool>(p_oSmpKeyValue, oItFound->second);
            break;
        default:
            std::ostringstream oMsg;
            oMsg << __FUNCTION__ << ": invalid value type: " << oType << "\n";
            wmLogTr(eWarning, "QnxMsg.Calib.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
            break;
    }

    if (!valid)
    {
        return -1;
    }
    
	// trigger subsequent actions
	//

	if (rKey == m_oKeyIsParallelInspection && !m_simulationStation)
	{
		const auto oIsParallelInspection	=	p_oSmpKeyValue->value<bool>();

		g_oNbPar    =	oIsParallelInspection ? g_oNbParMax : 1u;
        
	} // if
	
	if (rKey == m_oKeyDisableHWResults)
	{
		g_oDisableHWResults	    =	p_oSmpKeyValue->value<bool>();
        
        if ( g_oDisableHWResults )
        {
            wmLog( eDebug, "HW Results were disabled!\n");
        } else {
            wmLog( eDebug, "HW Results are enabled!\n");
        }
        
	} // if

	if (rKey == m_oKeyLineLaser1DefaultIntensity)
	{
		g_oLineLaser1DefaultIntensity = p_oSmpKeyValue->value<int>();
        
    } // if
    
	if (rKey == m_oKeyLineLaser2DefaultIntensity)
	{
		g_oLineLaser2DefaultIntensity = p_oSmpKeyValue->value<int>();
        
    } // if
    
	if (rKey == m_oKeyFieldLight1DefaultIntensity)
	{
		g_oFieldLight1DefaultIntensity = p_oSmpKeyValue->value<int>();
        
    } // if
    
	if (rKey == keyDebugTimings)
	{
		g_oDebugTimings = p_oSmpKeyValue->value<bool>();
        
    }
    if (rKey == keyConservativeCheckOvertriggering)
    {
        m_oConservativeCheckOvertriggeringValue = p_oSmpKeyValue->value<bool>();
    }
    if (rKey == keyDebugInspectManagerTime_us)
    {
        g_oDebugInspectManagerTime_us = p_oSmpKeyValue->value<int>();
    }

    if (rKey == keyLogAllFilterProcessingTime)
    {
        m_oLogAllFilterProcessingTime = p_oSmpKeyValue->value<bool>();
    }
    if (rKey == keyToleranceOvertriggering_ms)
    {
        m_oToleranceOvertriggering_ms = p_oSmpKeyValue->value<double>();
    }

    if (rKey == m_oKeyRealTimeGraphProcessing && !m_simulationStation)
    {
        m_realTimeGraphProcessing = p_oSmpKeyValue->value<bool>();
    }

	//	write new configuration to file
	//

	try
	{
		const auto oConf = makeConfiguration(*this);
		writeToFile(m_oConfigDir.toString(), oConf);
	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch

	return oItFound->second->handle(); // return handle
} // set



SmpKeyValue DeviceParameter::get(Key p_oKey) const
{
	const auto oItFound		( m_oParameters.find(p_oKey) );
	const auto oItMapEnd	( m_oParameters.end() );

	if (oItFound == oItMapEnd)
	{
		std::ostringstream oMsg;
		oMsg << "ERROR" <<  __FUNCTION__ << ":Key '" << p_oKey << "'NOT found.\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());

		return new KeyValue;
	} // if

	return oItFound->second->clone();
} // get

/**
 * Free functions
 */

void invalidToDefault(DeviceParameter &p_rParameter)
{
	// reset invalid parameters to default value

	for(auto it(std::begin(p_rParameter.m_oParameters)); it != std::end(p_rParameter.m_oParameters); ++it)
	{
		if (it->second->isValueValid() == false)
		{
			const std::string&	rValue		( it->second->toString() );
			wmLog(eDebug, "Invalid value '%s' is reset to default.\n", rValue.c_str());
			wmLogTr(eWarning, "QnxMsg.Vdr.InvalidValue", "Invalid value '%s' in the procedure '%s'.", rValue.c_str(), __FUNCTION__);
			it->second->resetToDefault();
		} // if
	} // for

} // invalidToDefault



void print(const DeviceParameter &p_rParameter)
{
	std::ostringstream oMsg;
	for(auto it(std::begin(p_rParameter.m_oParameters)); it != std::end(p_rParameter.m_oParameters); ++it)
	{
		oMsg.str("") ;
		oMsg << it->second->toString() << "\n";
		wmLog(eDebug, oMsg.str());
	} // for
} // print



Configuration makeConfiguration(const DeviceParameter &p_rParameter)
{
	Configuration oConfiguration;
	oConfiguration.reserve(p_rParameter.m_oParameters.size());
	for(auto it(std::begin(p_rParameter.m_oParameters)); it != std::end(p_rParameter.m_oParameters); ++it)
	{
		oConfiguration.push_back( it->second->clone() );
	} // for
	return oConfiguration;
} // makeConfiguration



bool DeviceParameter::getConservativeCheckOvertriggering() const 
{
    poco_assert_dbg(m_oConservativeCheckOvertriggeringValue == m_oParameters.at(keyConservativeCheckOvertriggering)->value<bool>());
    return m_oConservativeCheckOvertriggeringValue;
}


bool DeviceParameter::getLogAllFilterProcessingTime() const
{
    return m_oLogAllFilterProcessingTime;
}

bool DeviceParameter::getRealTimeGraphProcessing() const
{
    return m_realTimeGraphProcessing;
}

double DeviceParameter::getToleranceOvertriggering_ms() const
{
    return m_oToleranceOvertriggering_ms;
}

} // analyzer
} // precitec
