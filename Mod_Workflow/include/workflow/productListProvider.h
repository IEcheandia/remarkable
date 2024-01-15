#pragma once

#include "common/product.h"
#include "workflow/stateMachine/abstractState.h"
#include "Mod_Workflow.h"

namespace precitec
{
namespace workflow
{

class MOD_WORKFLOW_API ProductListProvider
{
public:
    virtual ~ProductListProvider() {}
    virtual precitec::interface::ProductList& getProductList() = 0;
    
    virtual void beginInspect( int p_oSeamNumber, const std::string &label = {} ) = 0;
    virtual void endInspect() = 0;
    virtual void setSeamseries( int p_oSeamseries ) = 0;
    virtual State currentState() = 0;
    virtual void startAuto( uint32_t p_oProductType, uint32_t p_oProductNumber, const std::string& p_rExtendedProductInfo ) =0;
    virtual void stopAuto() =0;

    virtual void triggerSingle(const interface::TriggerContext &context) = 0;

protected:
    explicit ProductListProvider() {}
};

}
}
