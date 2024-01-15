#pragma once
#include "fliplib/Fliplib.h"
#include "fliplib/SinkFilter.h"

namespace precitec
{
namespace filter
{
namespace bridges
{

class FILTER_API SampleSink : public fliplib::SinkFilter
{
public:
    SampleSink();
};

}
}
}
