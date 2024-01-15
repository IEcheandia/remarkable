#include "../include/workflow/stateMachine/shutdown.h"

namespace precitec {
namespace workflow {


Shutdown::Shutdown( StateContext* p_pContext ) : AbstractState( eShutdown , p_pContext)
{
}



void Shutdown::exit()
{
} // exit


} // workflow
} // precitec
