/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		Feb., 2015
 * 	@brief		Converts an binary image to a point list, several mode are available
 */

#ifndef IMAGETOPOINTLIST_H_
#define IMAGETOPOINTLIST_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "geo/geo.h"
#include "geo/array.h"

namespace precitec {
	using namespace image;
	using namespace interface;
	using namespace geo2d;
namespace filter {

class FILTER_API ImageToPointList  : public fliplib::TransformFilter
{
public:
	ImageToPointList();
	virtual ~ImageToPointList();
	
	static const std::string m_oFilterName;
	static const std::string PIPENAME;
	static const std::string PIPENAME_POINTARRAY_X;
	static const std::string PIPENAME_POINTARRAY_Y;
	static const std::string PIPENAME_POINTLISTLIST;
	
	void setParameter();
	void paint();

protected:	
	/// in pipe registration
	bool subscribe(fliplib::BasePipe& pipe, int group);
	// Verarbeitungsmethode IN Pipe
	void proceed(const void* sender, fliplib::PipeEventArgs& e);	
	
private:	
	bool m_isValid;

	const fliplib::SynchronePipe< ImageFrame >*		m_pPipeInImageFrame;	///< in pipe
	fliplib::SynchronePipe< GeoDoublearray >*		pipeXValues_;			//<- Output PIN X-Werte
	fliplib::SynchronePipe< GeoDoublearray >*		pipeYValues_;			//<- Output PIN Y-Werte
	fliplib::SynchronePipe< GeoVecAnnotatedDPointarray >*	pipeValues_;			//<- Output PIN Punkte

	interface::SmpTrafo							m_oSpTrafo;				///< roi translation
	int							m_oMode;	

	void DoQualas(BImage & image, Doublearray & oValX, Doublearray & oValY, AnnotatedDPointarray & oValPoint, bool useTop, bool useBottom);
	void DoSimple(BImage & image, Doublearray & oValX, Doublearray & oValY, AnnotatedDPointarray & oValPoint);
	std::vector<geo2d::AnnotatedDPointarray> m_outVecDPointArray;
};

}}

#endif /*IMAGETOPOINTLIST_H_*/
