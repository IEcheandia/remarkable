/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			SB, HS
 *  @date			2013
 *  @brief
 */


#ifndef HWPARAMETERS_H_INCLUDED_20130820
#define HWPARAMETERS_H_INCLUDED_20130820

// local

#include	"analyzer/centralDeviceManager.h"
#include	"message/db.interface.h"
#include	"common/graph.h"
#include	"fliplib/FilterGraph.h"

// poco

#include	"Poco/UUID.h"

// stdlib

#include	<map>

namespace precitec {
namespace analyzer {

class HwParameters {
public:
	static const Poco::UUID m_oDefaultHwSetID;
	static const Poco::UUID m_oNoChangeHwSetID;

	/**
		* @brief Erase a cached parameter set.
		* @param p_rParamSetId GUID of the hardware parameter set to be erased.
		*/
	void erase( const Poco::UUID &p_rParamSetId );

	/**
		* @brief Update a single key in a parameter set.
		* @param p_rParamSetId GUID of the hardware parameter set.
		* @param p_rDbProxy DB Proxy so that we can call getHardwareParameter().
		* @param p_oKey The key of the individual parameter that has changed.
		*/
	void updateHwParamSet( const Poco::UUID &p_rParamSetId, interface::SpFilterParameter p_oParam );

	/**
		* @brief Cache a hardware parameter set. The parameter set is retrieved from the db.
		* @param p_rParamSetId GUID of the hardware parameter set.
		*/
	void cacheHwParamSet( const Poco::UUID &p_rParamSetId, interface::TDb<AbstractInterface>& p_rDbProxy );
	
	/**
		* @brief Apply a single hardware parameter.
		* @param p_pParam shared_ptr to the parameter.
		* @param p_pDeviceManager Pointer to the central device manager.
		*/
	static void applyHwParam(interface::SpFilterParameter p_pParam, CentralDeviceManager* p_pDeviceManager);

	/**
		* @brief Activate a hardware parameter set. The parameter set is given to the device manager, which will then route it to the appropriate device servers.
		* @param p_rParamSetId GUID of the hardware parameter set.
		* @param p_pDeviceManager Pointer to the central device manager.
		*/
	void activateHwParamSet( const Poco::UUID &p_rParamSetId, CentralDeviceManager* p_pDeviceManager );
	
	/**
		* @brief Clears the hw prameter set cache.
		*/
	void clearCache()	{ m_oHwParameterSetMap.clear(); }


private:
	paramSet_t			m_oHwParameterSetMap;		///< see above, caches the hardware parameter sets.
	ParameterList		m_oDefaultHwParamSet;		///< Default hardware parameter set.
};

} // namespace analyzer
} // namespace precitec

#endif /* HWPARAMETERS_H_INCLUDED_20130820 */
