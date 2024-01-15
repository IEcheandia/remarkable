
#include "postProcessingThreadControl.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace postprocessing 
{

bool PostProcessingThreadControl::createThreads(uint32_t n_threads)
{
    
}


void PostProcessingThreadControl::threadInit(ProcessingThread worker, uint32_t threadId)
{
    worker.setNice(1);
    worker.setBatch(true);
    worker.setName(std::string{"Thread#"} + std::string{threadId});
    worker.startThread();
  //  worker.setWorkDoneCallback(std::bind();
}

void PostProcessingThreadControl::setRTPriorityToThreads()
{
    
}

}
}
}
}
