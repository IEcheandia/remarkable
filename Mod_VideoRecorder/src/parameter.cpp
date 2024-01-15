/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Contains recording parameter.
 */

#include "videoRecorder/parameter.h"
#include "videoRecorder/nioModeType.h"
#include "videoRecorder/literal.h"
#include "module/moduleLogger.h"
#include "common/defines.h"
#include "geo/range.h"
#include <iostream>
#include <limits>

namespace precitec {
	using namespace interface;
	using geo2d::Range;
namespace vdr {	


Parameter::Parameter() :
		// 					  key					value		min				max				default		// value and default should be equal

		m_oIsShutOff		( g_oIsShutOff,			false,		false,			true,			false		),
		m_oIsEnabled		( g_oIsEnabled,			false,		false,			true,			false		),
		m_oNioMode			( g_oNioMode,			eAllImages,	eNioModeMin,	eNioModeMax,	eAllImages	),
		m_oNbProductsToKeep	( g_oNbProductsToKeep,	500,		1,				10000,			500			), // repo max 10k sequences
		m_oNbLiveModeToKeep	( g_oNbLiveModeToKeep,	50,			1,				1000,			50			), // repo max 10k sequences
		m_oNbMaxImgLive		( g_oNbMaxImgLiveKey,	10000,		0,				100000,			10000		),
		m_oMaxDiskUsage		( g_oMaxDiskUsage,		0.9,		0.0,			1.0,			0.9			),
        m_skipIfFull		(skipIfFull,			true,		false,			true,			true		),
		m_oParamMap			( createMap() )
{
} // Parameter()



bool Parameter::isEnabled() const
{
	if( m_oIsShutOff.value() )
	{
		return false;
	}

	return m_oIsEnabled.value();
} // isEnabled



void Parameter::isEnabled(bool p_oIsEnabled)
{
	UNUSED const auto&& rScopedLock	=	std::lock_guard<std::mutex>{ m_oWriteMutex };

	m_oIsEnabled.value(p_oIsEnabled);
} // isEnabled



key_value_map_t Parameter::createMap() {
	// put all params in a hash map
	key_value_map_t oParamMap;

	oParamMap.insert(key_value_map_t::value_type(m_oIsShutOff.key(),		&m_oIsShutOff));
	oParamMap.insert(key_value_map_t::value_type(m_oIsEnabled.key(),		&m_oIsEnabled));
	oParamMap.insert(key_value_map_t::value_type(m_skipIfFull.key(),		&m_skipIfFull));
	oParamMap.insert(key_value_map_t::value_type(m_oNioMode.key(),			&m_oNioMode));
	oParamMap.insert(key_value_map_t::value_type(m_oNbProductsToKeep.key(),	&m_oNbProductsToKeep));
	oParamMap.insert(key_value_map_t::value_type(m_oNbLiveModeToKeep.key(),	&m_oNbLiveModeToKeep));
	oParamMap.insert(key_value_map_t::value_type(m_oNbMaxImgLive.key(),		&m_oNbMaxImgLive));
	oParamMap.insert(key_value_map_t::value_type(m_oMaxDiskUsage.key(),		&m_oMaxDiskUsage));
	return oParamMap;
} // createMap



void invalidToDefault(Parameter &p_rParameter) {
	// reset invalid parameters to default value

	for(auto it(std::begin(p_rParameter.m_oParamMap)); it != std::end(p_rParameter.m_oParamMap); ++it) {
		const KeyValue&	rKeyValue		( *it->second );
		if (rKeyValue.isValueValid() == false) {
			const std::string&	rValue		( rKeyValue.toString() );
			wmLog(eDebug, "Invalid value '%s' is reset to default.\n", rValue.c_str());
			wmLogTr(eWarning, "QnxMsg.Vdr.InvalidValue", "Invalid value '%s' in the procedure '%s'.", rValue.c_str(), __FUNCTION__);
			it->second->resetToDefault();
		} // if
	} // for

} // invalidToDefault



void print(const Parameter &p_rParameter) {
	std::ostringstream oMsg;
	for(auto it(std::begin(p_rParameter.m_oParamMap)); it != std::end(p_rParameter.m_oParamMap); ++it) {
		oMsg.str("") ;
		oMsg << it->second->toString() << "\n";
		wmLog(eDebug, oMsg.str());
	} // for
} // print



Configuration makeConfiguration(const Parameter &p_rParameter, bool p_oSeamPlus1) {
	Configuration oConfiguration;
	oConfiguration.reserve(p_rParameter.m_oParamMap.size());
	for(auto it(std::begin(p_rParameter.m_oParamMap)); it != std::end(p_rParameter.m_oParamMap); ++it) {
		oConfiguration.push_back( it->second->clone() );
	} // for
	return oConfiguration;
} // makeConfiguration

} // vdr
} // precitec
