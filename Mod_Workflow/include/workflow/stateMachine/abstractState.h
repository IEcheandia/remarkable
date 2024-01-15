/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR, SB
 *  @date			2009
 *  @brief			This class represents the base-class for all states in the workflow state machine.
 */

#ifndef ABSTRACTSTATE_H_
#define ABSTRACTSTATE_H_

// stl includes
#include <string>
#include <iostream>
#include <map>
#include <utility>
#include <cstdint>
// project includes
#include "event/systemStatus.h"
#include "system/types.h"
#include "system/exception.h"
#include "module/moduleLogger.h"
// poco includes
#include "Poco/UUID.h"

namespace precitec {
namespace workflow {

// Aufzaehlung saemtlicher States
enum State { eInit, eOperate, eAutomaticMode, eLiveMode, eShutdown, eCalibrate, eNotReady, eProductTeachIn, eExit, eEmergencyStop, eUps, eUpdateLiveMode, numStates };

typedef std::pair<int, PvString> IdAndString;
typedef std::map<int, PvString> IdToString;
#define ID_PAIR(Id) IdAndString(Id, PvString(#Id))
class StateNames {
public:
	typedef std::map<int, PvString> IdNameMap;
	StateNames() {
		map_.insert(ID_PAIR(eInit));
		map_.insert(ID_PAIR(eOperate));
		map_.insert(ID_PAIR(eAutomaticMode));
		map_.insert(ID_PAIR(eLiveMode));
		map_.insert(ID_PAIR(eShutdown));
		map_.insert(ID_PAIR(eCalibrate));
		map_.insert(ID_PAIR(eNotReady));
		map_.insert(ID_PAIR(eProductTeachIn));
		map_.insert(ID_PAIR(eEmergencyStop));
		map_.insert(ID_PAIR(eUps));
		map_.insert(ID_PAIR(eUpdateLiveMode));
	}
public:
	PvString operator [] (int i) const { return map_[i]; }
private:
	mutable IdToString map_;
};
#undef ID_PAIR

extern const StateNames StateName;

class StateContext; // fwd decl


/**
 * @brief Basisklasse State Machine. Alle States leiten von dieser Basisklasse ab.
 *
 * Die neuen Zustaende als (abstrakte) Memberfuktionen zu deklarieren, garantiert,
 * dass jeder Zustand mit jedem Uebergangs-Request fertig wird, und sei es,
 * dass er ihn (per default) ignoriert. Es kann also keine vergessenen Uebergaenge geben.
 *
 * @ingroup Workflow
 */
class AbstractState
{
public:

	AbstractState(State p_oStateType, StateContext* p_pContext)
	:
	m_oStateType	( p_oStateType ),
	m_pContext		( p_pContext )
	{}
	virtual ~AbstractState() {
		m_pContext = nullptr;	// provoke crash on use of deleted this->m_pContext()
	}

	// List of transitions
	virtual void initialize() 																	{ wmLog( eDebug, "Initalize not supported in state %s!\n", 						StateName[m_oStateType].c_str() ); };
	virtual void ready() 																		{ wmLog( eDebug, "Ready not supported in state %s!\n", 							StateName[m_oStateType].c_str() ); };
	virtual void startLive( const Poco::UUID& p_oProductID, int p_oSeamseries, int p_oSeam )	{ wmLog( eDebug, "Starting live-mode not supported in state %s!\n",				StateName[m_oStateType].c_str() ); };
	virtual void stopLive()																		{ wmLog( eDebug, "Stopping live-mode not supported in state %s!\n",				StateName[m_oStateType].c_str() ); };
	virtual void startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo )	{ wmLog( eDebug, "Starting automatic-mode not supported in state %s!\n",		StateName[m_oStateType].c_str() ); };
	virtual void stopAuto()																		{ wmLog( eDebug, "Stopping automatic-mode not supported in state %s!\n",		StateName[m_oStateType].c_str() ); };
	virtual void activateSeamSeries( int p_oSeamSeries )										{ wmLog( eDebug, "Activating seam series not supported in state %s!\n",			StateName[m_oStateType].c_str() ); };
	virtual void beginInspect( int p_oSeam, const std::string &label )							{ wmLog( eDebug, "Starting inspection not supported in state %s!\n",			StateName[m_oStateType].c_str() ); };
	virtual void endInspect()																	{ wmLog( eDebug, "Stopping inspection not supported in state %s!\n",			StateName[m_oStateType].c_str() ); };
	virtual void exit() 																		{ wmLog( eDebug, "Shutdown not supported in state %s!\n",						StateName[m_oStateType].c_str() ); };
	virtual void calibrate( unsigned int p_oMethod )											{ wmLog( eDebug, "Calibrate not supported in state %s!\n",						StateName[m_oStateType].c_str() ); };
	virtual void signalNotReady()																{ wmLog( eDebug, "Changing to not-ready-mode not supported in state %s!\n",		StateName[m_oStateType].c_str() ); };
	virtual void startProductTeachIn()															{ wmLog( eDebug, "Starting product-teach-in not supported in state %s!\n",		StateName[m_oStateType].c_str() ); };
	virtual void abortProductTeachIn()															{ wmLog( eDebug, "Aborting product-teach-in not supported in state %s!\n",		StateName[m_oStateType].c_str() ); };
	virtual void quitSystemFault()																{ wmLog( eDebug, "Quitting system-fault not supported in state %s!\n",			StateName[m_oStateType].c_str() ); };
	virtual void emergencyStop()																{ wmLog( eDebug, "Emergency stop not supported in state %s!\n",					StateName[m_oStateType].c_str() ); };
	virtual void resetEmergencyStop()															{ wmLog( eDebug, "ResetEmergency stop not supported in state %s!\n",					StateName[m_oStateType].c_str() ); };
	virtual void seamPreStart( int p_oSeam )													{ wmLog( eDebug, "Seam pre start not supported in state %s!\n",					StateName[m_oStateType].c_str() ); };
    virtual void updateLiveMode()
    {
        wmLog(eDebug, "Update live mode not supported in state %s!\n", StateName[m_oStateType].c_str());
    }

	// Liefert den Type des aktuellen Zustand
	State type() { return m_oStateType; }

    void sendOperationState();

protected:

	State 			m_oStateType;
	StateContext* 	m_pContext; ///< Pointer to StateContext.

    void setOperationState(precitec::interface::OperationState state);

private:
    precitec::interface::OperationState m_operationState = precitec::interface::NumMode;
};

} // namespace workflow
} // namespace precitec

#endif /*ABSTRACTSTATE_H_*/
