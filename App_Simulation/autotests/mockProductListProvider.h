#pragma once

#include "workflow/productListProvider.h"
#include "workflow/stateMachine/abstractState.h"

#include "mockTriggerCmd.h"

#include <optional>


class MockProductListProvider : public precitec::workflow::ProductListProvider
{
public:
    explicit MockProductListProvider() = default;

    precitec::interface::ProductList &getProductList() override
    {
        return m_productList;
    }
    void beginInspect( int p_oSeamNumber, const std::string &label = {} ) override
    {
        m_inspectionSeam = p_oSeamNumber;
        m_inspectionSeamLabel = label;
    }
	void endInspect() override
    {
        m_inspectionSeam = std::nullopt;
        m_inspectionSeamSeries = std::nullopt;
        m_inspectionSeamLabel = {};
    }
    void setSeamseries( int p_oSeamseries ) override
    {
        m_inspectionSeamSeries = p_oSeamseries;
    }

    precitec::workflow::State currentState() override
    {
        return precitec::workflow::eOperate;
    }
    
    void startAuto( uint32_t p_oProductType, uint32_t p_oProductNumber, const std::string& p_rExtendedProductInfo ) override
    {
    }
    
    void stopAuto() override
    {
    }
        
    template <typename... Args>
    void addProduct(Args&&... args)
    {
        m_productList.emplace_back(std::forward<Args>(args)...);
    }

    const std::optional<int> &inspectionSeam() const
    {
        return m_inspectionSeam;
    }

    const std::optional<int> &inspectionSeamSeries() const
    {
        return m_inspectionSeamSeries;
    }

    const std::string &inspectionSeamLabel() const
    {
        return m_inspectionSeamLabel;
    }

    void triggerSingle(const precitec::interface::TriggerContext &context) override
    {
        m_mockTriggerCmd.single({{}}, context);
    }

    MockTriggerCmd &triggerCmd()
    {
        return m_mockTriggerCmd;
    }

private:
    precitec::interface::ProductList m_productList;
    std::optional<int> m_inspectionSeam;
    std::optional<int> m_inspectionSeamSeries;
    std::string m_inspectionSeamLabel;
    MockTriggerCmd m_mockTriggerCmd;
};

