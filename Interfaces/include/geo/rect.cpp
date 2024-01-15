#include "system/typeTraits.h"
#include "geo/rect.h"

namespace precitec
{
	const geo2d::Rect	No<geo2d::Rect>::Value 	= geo2d::Rect(No<geo2d::Range>::Value,  No<geo2d::Range>::Value);

} // namespace precitec
