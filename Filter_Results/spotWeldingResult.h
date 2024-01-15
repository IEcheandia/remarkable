#pragma once

#include <fliplib/ResultFilter.h>
#include <fliplib/Fliplib.h>
#include <fliplib/SynchronePipe.h>
#include <event/results.h>

namespace precitec
{
namespace filter
{

class FILTER_API SpotWeldingResult: public fliplib::ResultFilter
{

public:
    enum class ScanmasterResultType{SpotWelding = 0};
    enum class InputType{WeldingDuration = 0, LaserPowerCenter = 1, LaserPowerRing = 2};

    SpotWeldingResult();

    bool subscribe(fliplib::BasePipe& pipe, int) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) override;

    void setParameter() override; 

    static const std::string m_pipeInLaserPowerName;
    static const std::string m_pipeIWeldingDurationName;
    static const std::string m_pipeResultName;

private:

    typedef fliplib::SynchronePipe<interface::GeoDoublearray> pipe_scalar_t;
    typedef fliplib::SynchronePipe<interface::ResultDoubleArray> pipe_result_t;

    const pipe_scalar_t* m_pipeInWeldingDuration;
    const pipe_scalar_t* m_pipeInLaserPowerCenter;
    const pipe_scalar_t* m_pipeInLaserPowerRing;
    pipe_result_t m_pipeResult; 
    ScanmasterResultType m_outputType;
    geo2d::TArray<double> m_resultArray;
    geo2d::Range1d m_minMaxRange;

};

}
}
