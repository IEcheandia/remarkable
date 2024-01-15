#include "workflow/stateMachine/abstractState.h"
#include "workflow/stateMachine/stateContext.h"

namespace precitec
{
namespace workflow
{

void AbstractState::sendOperationState()
{
    if (m_operationState != precitec::interface::NumMode)
    {
        m_pContext->getSystemStatus().operationState(m_operationState);
    }
}

void AbstractState::setOperationState(precitec::interface::OperationState state)
{
    m_operationState = state;
    sendOperationState();
}

}
}
