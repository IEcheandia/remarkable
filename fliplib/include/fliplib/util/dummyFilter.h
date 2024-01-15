#pragma once
#include "fliplib/NullSourceFilter.h"
#include "fliplib/BaseFilter.h"


//Dummy filter to be used in the unit test of the filters
class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter()
        : fliplib::BaseFilter("dummy")
    {
    }
    void proceed(const void* sender, fliplib::PipeEventArgs& event) override
    {
        (void) (sender);
        (void) (event);
        preSignalAction();
        m_proceedCalled = true;
    }
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) override
    {
        (void) (sender);
        (void) (e);
        preSignalAction();
        m_proceedCalled = true;
    }

    int getFilterType() const override
    {
        return BaseFilterInterface::SINK;
    }

    bool isProceedCalled() const
    {
        return m_proceedCalled;
    }

    void resetProceedCalled()
    {
        m_proceedCalled = false;
    }
private:
    bool m_proceedCalled = false;
};
