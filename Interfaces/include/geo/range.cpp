#include "range.h"

namespace precitec
{
const geo2d::TRange<int> No<geo2d::TRange<int> >::Value	= geo2d::TRange<int>(No<int>::Value,  No<int>::Value);
const geo2d::Range1u	No<geo2d::Range1u>::Value	= geo2d::Range1u(No<uInt>::Value,  No<uInt>::Value);
const geo2d::Range1f	No<geo2d::Range1f>::Value	= geo2d::Range1f(No<float>::Value,  No<float>::Value);
const geo2d::Range1d	No<geo2d::Range1d>::Value	= geo2d::Range1d(No<double>::Value,  No<double>::Value);

} // namespace precitec
