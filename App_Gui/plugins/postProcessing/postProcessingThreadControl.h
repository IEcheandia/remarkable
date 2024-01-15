#pragma once
#include "postProcessingController.h"
#include "analyzer/processingThread.h"



namespace precitec
{

namespace gui
{
namespace components
{
namespace postprocessing 
{

using namespace analyzer;
class PostProcessingThreadControl
{
public:
    PostProcessingThreadControl();
    ~PostProcessingThreadControl();
    void threadInit(ProcessingThread worker);
    bool createThreads(uint32_t n_threads);
    
private:
    ProcessingThread                 m_worker;
    std::vector<ProcessingThread>    m_workingThreads;
    
    
}
}
}
}
