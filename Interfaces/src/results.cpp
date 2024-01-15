/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		WoR, AB, HS
 * 	@date		2011
 * 	@brief		Provides result- and  typ enum as well as base classes necessary for handling results (TResultArgs, ...)
 */

#include "event/results.h"

using Poco::UUID;
namespace precitec {
namespace interface {

void ResultArgs::copyValues(ResultArgs const& rhs) {
	valueType_	= rhs.valueType_;
	filterID_	= rhs.filterID_;
	resultType_ = rhs.resultType_;
	nioType_	= rhs.nioType_;
	isNio_		= rhs.isNio_;
	isValid_	= rhs.isValid_;
	context_ = rhs.context_;
    signalQuality_ = rhs.signalQuality_;
    nioPercentage_ = rhs.nioPercentage_;
    upperReference_ = rhs.upperReference_;
    lowerReference_ = rhs.lowerReference_;
    assert (!isValid_ || ra_ != nullptr);
}



ResultArgs::ResultArgs(
	const UUID&			filterID, 
	ResultType			rs, 
	ResultType			nio, 
	const ImageContext& c,
	bool				isValidFlag, 
	bool				isNioFlag, 
	RegTypes			type, 
	ResultArgsImpl*		ra)
	: 
	valueType_	(type),
	ra_			(ra),
	filterID_	(filterID),
	resultType_	(rs),
	nioType_	(nio),
	isValid_	(isValidFlag),
	isNio_		(isNioFlag),
	context_	(c)
{
    assert (!isValid_ || ra_ != nullptr);
}


} // namespace interface
} // namespace precitec

