#include "workflow/querySystemStatusServer.h"
#include "workflow/stateMachine/stateContext.h"

#include <iostream>

namespace precitec
{
namespace workflow
{

void QuerySystemStatusServer::requestOperationState()
{
    if (m_stateContext)
    {
        m_stateContext->sendOperationState();
    }
}

}
}
