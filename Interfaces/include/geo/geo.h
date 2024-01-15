/**
*	@file
*	@copyright	Precitec Vision GmbH & Co. KG
*	@author		Wor, HS
*	@date		2011
*	@brief		TGeo, typedefs, global rank double[0.0, 1.0]
*/


#ifndef GEO_H_
#define GEO_H_

#include <ostream>
#include "Poco/SharedPtr.h"
#include "message/serializer.h"
#include "message/messageBuffer.h"
#include "geo/size.h"
#include "geo/rect.h"
#include "geo/range.h"
#include "geo/point.h"
#include "geo/array.h"
#include "geo/blob.h"
#include "geo/lineModel.h"
#include "geo/annotatedArray.h"
#include "event/resultType.h"

#include "common/geoContext.h"
#include "Analyzer_Interface.h"

namespace precitec {
namespace interface {

typedef geo2d::Size 		Size2D;

extern const double  INTERFACES_API  NotPresent;
extern const double  INTERFACES_API  Minimum;
extern const double  INTERFACES_API  Marginal;
extern const double  INTERFACES_API  Bad;
extern const double  INTERFACES_API  Doubtful;
extern const double  INTERFACES_API  NotGood;
extern const double  INTERFACES_API  Ok;
extern const double  INTERFACES_API  Good;
extern const double  INTERFACES_API  Perfect;
extern const double  INTERFACES_API  Limit;

/**
	*  Typen-Registrierung fuer die (de)Serialisierung von Klassen mit Parameter beliebigen Typs
	*  Result, IntlElement(Logger)
	*  diese Klassen muessen mit der Registrierung konsistent gehalten werden!!
	*  Eine alternative Implementierung waere eine TypeList. Nicht klar ist, wie ich dort einfach die
	*  Stream-Ausgabe regele. TypeList mit assoziierter map vllt.
	*/
enum RegTypes { 
	RegInt	=	0,	// deprecated
	RegDouble, 		// deprecated, was used before the 'RegDoubleArray' type existed. Still needed for backwards compatibility.
	RegRange, 		// deprecated
	RegRange1d, 	// deprecated
	RegPoint, 		// deprecated
	RegDoubleArray, // all current result filters use this type only
	NumRegTypes };

/// fuer (Debug)-Ausgaben
// removed, probably declared in the wrong scope
//std::ostream &operator <<(std::ostream &os, RegTypes const& r);

template<class T> struct  INTERFACES_API  RegisteredType { 
	static const int Value; 
}; // struct RegisteredType


/**
  * Unter anderem alle Messwerte werden hiervon abgeleitet
  */
template <class T>
class TGeo : public system::message::Serializable	{
public:
	TGeo()	:
		m_oValue(T()), m_oRank(NotPresent), m_oAnalysisResult(AnalysisOK) 
	{}

	TGeo(const ImageContext& p_rImageContext, const T& p_rValue, const ResultType p_oRes=AnalysisOK, double p_oRank = NotPresent)
		: 
		m_oImageContext		(p_rImageContext), 
		m_oValue			(p_rValue), 
		m_oRank				(clamp(p_oRank, geo2d::Range1d(NotPresent, Limit))),
		m_oAnalysisResult	(p_oRes) 
	{} 

	TGeo(MessageBuffer const& buffer)
		: m_oImageContext(Serializable::deMarshal<ImageContext>(buffer)), m_oValue(Serializable::deMarshal<T>(buffer)),
		m_oRank(Serializable::deMarshal<double>(buffer)), m_oAnalysisResult( (ResultType)(Serializable::deMarshal<int>(buffer)) ) {} // we assume the enum for the int does exist

	const T& ref() const { 
		return m_oValue; 
	}

	T& ref() { 
		return m_oValue; 
	}

	ResultType analysisResult() const { 
		return m_oAnalysisResult; 
	}
	void setAnalysisResult(const ResultType p_oRes) { 
		m_oAnalysisResult = p_oRes; 
	}

	void rank(double rank) { 
		m_oRank = clamp(rank, geo2d::Range1d(NotPresent, Limit)); 
	}

	double rank()  const { 
		return m_oRank; 
	}

	bool isValid() const { assert(m_oRank >= NotPresent); return m_oRank != NotPresent; 
	}

	const ImageContext& context() const { 
		return m_oImageContext; 
	}

	void serialize	( MessageBuffer &buffer ) const	{
		marshal(buffer, m_oImageContext);
		marshal(buffer, m_oValue);
		marshal(buffer, m_oRank);
		marshal(buffer, static_cast<int>(m_oAnalysisResult));
	}

	TGeo& operator=(TGeo const& rhs)
	{
		TGeo tmp(rhs);
		swap(tmp);
		return *this;
	}

	/// nur noetig wenn T kein POD
	void deserialize( MessageBuffer const&buffer ) { 
		TGeo<T> tmp(buffer); swap(tmp);	
	}

	void swap(TGeo<T> &rhs) {
		m_oImageContext.swap(rhs.m_oImageContext);
		Swap<T, IsPod<T>::Value >::swap(m_oValue, rhs.m_oValue);
		std::swap(m_oRank, rhs.m_oRank);
		std::swap(m_oAnalysisResult, rhs.m_oAnalysisResult);
	}
private:
	ImageContext	m_oImageContext;
	T				m_oValue;
	double			m_oRank; ///< [0.0, 1.0]
	ResultType		m_oAnalysisResult;
}; // TGeo

template <class T>
std::ostream &operator <<(std::ostream &os, TGeo<T> const& g) {
	os << "@" << g.context() << ":" << T(g) << "!" << g.rank() << ", " << g.analysisResult(); return os;
}

template<class T>
bool operator==( const TGeo<T>& p_rLhs, const TGeo<T>& p_rRhs )
{
	return  (p_rLhs.context()			== p_rRhs.context()) && 
			(T(p_rLhs)					== T(p_rRhs)) && 
			(p_rLhs.rank()				== p_rRhs.rank()) && 
			(p_rLhs.analysisResult()	== p_rRhs.analysisResult());
}

// Fuer Std-Typen ergibt die Trafo im Contxt keine Sinn, hier wird
// der Kontext fuer das REsult-Interface mitgeshleppt.
// Der Rank st immer sinnvoll

typedef TGeo<double> 										 GeoDouble;
typedef TGeo<geo2d::Point> 									 GeoPoint;

typedef TGeo<geo2d::TArray<byte>>							 GeoBytearray;
typedef TGeo<geo2d::TArray<double>>							 GeoDoublearray;
typedef TGeo<geo2d::TArray<geo2d::Point>>					 GeoPointarray;
typedef TGeo<geo2d::TArray<geo2d::DPoint>>					 GeoDPointarray;
typedef TGeo<geo2d::TArray<geo2d::Blob>>					 GeoBlobarray;
typedef TGeo<geo2d::TArray<geo2d::SeamFinding>>				 GeoSeamFindingarray;
typedef TGeo<geo2d::TArray<geo2d::PoorPenetrationCandidate>> GeoPoorPenetrationCandidatearray;
typedef TGeo<geo2d::TArray<geo2d::HoughPPCandidate>>         GeoHoughPPCandidatearray;
typedef TGeo<geo2d::TArray<geo2d::StartEndInfo>>             GeoStartEndInfoarray;
typedef TGeo<geo2d::TArray<geo2d::SurfaceInfo>>              GeoSurfaceInfoarray;
typedef TGeo<geo2d::TArray<geo2d::LineModel>>                GeoLineModelarray;


typedef TGeo<std::vector<geo2d::Bytearray>>			         GeoVecBytearray;
typedef TGeo<std::vector<geo2d::Intarray>>					 GeoVecIntarray;
typedef TGeo<std::vector<geo2d::Doublearray>>				 GeoVecDoublearray;
typedef TGeo<std::vector<geo2d::DPointarray>>				 GeoVecDPointarray;


typedef TGeo<std::vector<geo2d::TAnnotatedArray<geo2d::DPoint>>>	GeoVecAnnotatedDPointarray;

} // namespace interface
} // namespace precitec

#endif /*GEO_H_*/
