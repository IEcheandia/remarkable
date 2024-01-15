/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			SB, HS
 *  @date			2013
 *  @brief
 */


#include "analyzer/hwParameters.h"
#include "module/moduleLogger.h"


using namespace fliplib;
using Poco::UUID;

namespace precitec {
	using namespace interface;
namespace analyzer {


const UUID HwParameters::m_oDefaultHwSetID ( "BD189A23-A778-43AB-BA86-775BDC2550E2" ); // see "public static Guid HWParamGuidDefault = new Guid("BD189A23-A778-43AB-BA86-775BDC2550E2");" in ProductExport.cs
const UUID HwParameters::m_oNoChangeHwSetID( "5E2E3EA6-DC0F-4AA3-B1F9-844A4AA94A1F" ); // see "public static Guid HWParamGuidNoChange = new Guid("5E2E3EA6-DC0F-4AA3-B1F9-844A4AA94A1F");" in ProductExport.cs

void HwParameters::erase( const UUID &p_rParamSetId ) {
	m_oHwParameterSetMap.erase(p_rParamSetId);
} // erase



void HwParameters::updateHwParamSet( const UUID &p_rParamSetId, SpFilterParameter p_oParam )
{
	// Exchange the parameter in the set
	ParameterList oSet = m_oHwParameterSetMap[ p_rParamSetId ];
	for( auto oIter = std::begin( oSet ); oIter < std::end( oSet ); ++oIter )
	{
		if ( (*oIter)->name() == p_oParam->name() )
		{
			(*oIter) = p_oParam;
			wmLog( eDebug, "Key found in parameter set found and updated...\n" );
		}
	}

} // updateHwParamSet



void HwParameters::cacheHwParamSet( const UUID &p_rParamSetId, TDb<AbstractInterface>& p_rDbProxy )
{
	//system::ScopedTimer oScopedTimer(__FUNCTION__);

	// is this a valid param set guid?
	if ( p_rParamSetId.isNull() )
	{
		wmLog( eDebug, "cacheHwParamSet: ParamSet has null guid!\n");
		return;
	}

	std::ostringstream oMsg;

	// is the parameter set already in the map?
	const bool oParamSetNotFound( m_oHwParameterSetMap.find( p_rParamSetId ) == std::end( m_oHwParameterSetMap ) );
	if (oParamSetNotFound == false)
	{
		//oMsg << "Hw-Parameter set '" << p_rParamSetId.toString() << "' already cached.\n";
		//wmLog(eDebug, oMsg.str());
		return;
	}

	// OK, if not, we have to ask the db to get it ...
	const ParameterList oParameterList( p_rDbProxy.getHardwareParameterSatz(p_rParamSetId) );
	m_oHwParameterSetMap.insert( paramSet_t::value_type( p_rParamSetId, oParameterList) );
	//oMsg << "Hw-Parameter set '" << p_rParamSetId.toString() << "' inserted into cache.\n";
	//wmLog(eDebug, oMsg.str());

	// The default hardware parameter set is stored, in addition to the map, in a member variable ...
	if ( p_rParamSetId == m_oDefaultHwSetID )
	{
		m_oDefaultHwParamSet = m_oHwParameterSetMap[ p_rParamSetId ];
		//wmLog( eDebug, "Default hardware parameter set found and stored!\n" );
	}

} // cacheHwParamSet



/*static*/ void HwParameters::applyHwParam(SpFilterParameter p_pParam, CentralDeviceManager* p_pDeviceManager)
{
	if ( p_pParam->type() == TInt )
	{
		int oValue = p_pParam->value<int>();
		SmpKeyValue pKey( new TKeyValue<int>( p_pParam->name(), oValue, 0, 100000, 1 ) );
		wmLog( eDebug, "applyHwParam(): %s = %d\n", p_pParam->name().c_str(), oValue );

		p_pDeviceManager->force(p_pParam->typID(), pKey);
	}
	if ( p_pParam->type() == TUInt )
	{
		uInt oValue = p_pParam->value<uInt>();
		SmpKeyValue pKey( new TKeyValue<uInt>( p_pParam->name(), oValue, 0, 100000, 1 ) );
		wmLog( eDebug, "applyHwParam(): %s = %d\n", p_pParam->name().c_str(), oValue );

		p_pDeviceManager->force(p_pParam->typID(), pKey);
	}
	if ( p_pParam->type() == TFloat )
	{
		float oValue = p_pParam->value<float>();
		SmpKeyValue pKey( new TKeyValue<float>( p_pParam->name(), oValue, 0, 100000, 1 ) );
		wmLog( eDebug, "applyHwParam(): %s = %f\n", p_pParam->name().c_str(), oValue );

		p_pDeviceManager->force(p_pParam->typID(), pKey);
	}
	if ( p_pParam->type() == TDouble )
	{
		double oValue = p_pParam->value<double>();
		SmpKeyValue pKey( new TKeyValue<double>( p_pParam->name(), oValue, 0, 100000, 1 ) );
		wmLog( eDebug, "applyHwParam(): %s = %f\n", p_pParam->name().c_str(), oValue );

		p_pDeviceManager->force(p_pParam->typID(), pKey);
	}
	if ( p_pParam->type() == TBool )
	{
		bool oValue = p_pParam->value<bool>();
		SmpKeyValue pKey( new TKeyValue<bool>( p_pParam->name(), oValue, false, true, true ) );

		if ( oValue ) {
			wmLog( eDebug, "applyHwParam(): %s = TRUE\n", p_pParam->name().c_str() );
		} else {
			wmLog( eDebug, "applyHwParam(): %s = FALSE\n", p_pParam->name().c_str() );
		}

		p_pDeviceManager->force(p_pParam->typID(), pKey);
	}
    if ( p_pParam->type() == TString )
    {
        auto oValue = p_pParam->value<std::string>();
        SmpKeyValue pKey( new TKeyValue<std::string>( p_pParam->name(), oValue, {}, {}, {} ) );

        wmLog( eDebug, "applyHwParam(): %s = %s\n", p_pParam->name().c_str(), oValue );

        p_pDeviceManager->force(p_pParam->typID(), pKey);
    }

} // applyHwParam



void HwParameters::activateHwParamSet( const UUID &p_rParamSetId, CentralDeviceManager* p_pDeviceManager )
{
	//system::ScopedTimer oScopedTimer(__FUNCTION__);

	ParameterList& oParameterList = m_oHwParameterSetMap[ p_rParamSetId ];

	const bool oParamSetNotFound( m_oHwParameterSetMap.find( p_rParamSetId ) == std::end( m_oHwParameterSetMap ) );
	if (oParamSetNotFound == true)
	{
		wmLog( eDebug, "activateHwParamSet() - %s not found!\n", p_rParamSetId.toString().c_str() );
		return;
	}

#if defined __QNX__ || defined __linux__

	ParameterList oLater;
	for( auto oIter = std::begin( oParameterList ); oIter < std::end( oParameterList ); ++oIter )
	{
		// check for the weldhead properties, that have to be delayed until the end of the list ...
		if 	( (*oIter)->typID() == g_oWeldHeadID &&
				(
					(*oIter)->name() == "X_Axis_AbsolutePosition" ||
					(*oIter)->name() == "Y_Axis_AbsolutePosition" ||
					(*oIter)->name() == "Z_Axis_AbsolutePosition" ||
					(*oIter)->name() == "LC_Send_Data" ||
					(*oIter)->name() == "LEDSendData" ||
					(*oIter)->name() == "LineLaser1OnOff" ||
					(*oIter)->name() == "LineLaser2OnOff" ||
					(*oIter)->name() == "FieldLight1OnOff" ||
					(*oIter)->name() == "Generate_ScanTracker2D_Figure" ||
					(*oIter)->name() == "Scanner_DriveToPosition"
				)
			)
		{
			oLater.push_back(*oIter);
		}
		// ... and check for the camera properties
		else if ( (*oIter)->typID() == g_oCameraID && (*oIter)->name() == "Window.H" )
		{
			oLater.push_back(*oIter);
		}
		// ... and check for the IDM properties
		else if ( (*oIter)->typID() == g_oCHRCommunicationID && (*oIter)->name() == "Adaptive Exposure Mode On/Off" )
		{
			oLater.push_back(*oIter);
		}
		// the rest of the properties has to be applied immediately
		else
		{
			applyHwParam( (*oIter), p_pDeviceManager );
		}
	}
	for( auto oIter = std::begin( oLater ); oIter < std::end( oLater ); ++oIter )
	{
		wmLog( eDebug, "activateHwParamSet(): DELAYED %s\n", (*oIter)->name().c_str() );
		applyHwParam( (*oIter), p_pDeviceManager );
	}


	wmLog( eDebug, "activateHwParamSet() - %s applied!\n", p_rParamSetId.toString().c_str() );

#endif

} // activateHwParamSet



} // namespace analyzer
} // namespace precitec
