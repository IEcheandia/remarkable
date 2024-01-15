/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR, SB
 *  @date			2009
 *  @brief			This class represents the initial state in the workflow state machine.
 */

#ifndef INIT_H_
#define INIT_H_

// project includes
#include "stateContext.h"
#include "abstractState.h"

namespace precitec {
namespace workflow {

/**
 * @ingroup Workflow
 * @brief This class represents the initial state in the workflow state machine.
 */
class Init : public AbstractState
{
public:

	Init( StateContext* p_pContext );


	void initialize();

    void exit();
private:
	bool handleCalibrationData();
    void GoToNextState(StateContext* p_pContext);
protected:
    void receiveProducts();

private:

	bool m_oWinInitialized;
};

} // namespace workflow
} // namespace precitec

#endif /*INIT_H_*/
