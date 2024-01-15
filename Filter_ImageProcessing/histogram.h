/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		
 */

#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "geo/geo.h"

namespace precitec {
	using namespace image;
	using namespace interface;
namespace filter {

/**
 * Schneidet aus einem ImageFrame den Region of intrests aus und sendet eine neue ImageFrame an den 
 * naechsten Filter
 */
class FILTER_API Histogram  : public fliplib::TransformFilter
{
public:
	Histogram();
	virtual ~Histogram();
	
	static const std::string m_oFilterName;
	static const std::string PIPENAME;
	static const std::string PIPENAME_MINIMUM;
	static const std::string PIPENAME_MAXIMUM;
	
	void setParameter();
	void paint();

protected:	
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& pipe, int group);
	// Verarbeitungsmethode IN Pipe
	void proceed(const void* sender, fliplib::PipeEventArgs& e);	
	
private:	
	const fliplib::SynchronePipe< ImageFrame >*	m_pPipeInImageFrame;	///< in pipe
	fliplib::SynchronePipe< ImageFrame >*		pipeImageFrame_;	//<- Output PIN fuer Kontrastverstaerktes Graubild
	fliplib::SynchronePipe< GeoDoublearray >*	pipeMinimum_;			//<- Output PIN fuer Minimum
	fliplib::SynchronePipe< GeoDoublearray >*	pipeMaximum_;			//<- Output PIN fuer Maximum

	interface::SmpTrafo							m_oSpTrafo;				///< roi translation	
	enum { LOOKUP_LENGTH=256 };
	int											lookup_[LOOKUP_LENGTH];	// Lookuptabelle mit den neuen Grauwerten
	int											minValue_;
	int											maxValue_;
};

}}

#endif /*HISTOGRAM_H_*/
