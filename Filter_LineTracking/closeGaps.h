/*!
*  @copyright		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2011
*  @file
*/

#ifndef CLOSEGAPS_H_
#define CLOSEGAPS_H_

#include <memory>						///< unique_ptr

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "common/frame.h"				///< ImageFrame
#include "image/image.h"				///< BImage
#include "geo/geo.h"					///< Size2d, VecDoublearray
#include "geo/array.h"					///< TArray


namespace precitec {
	using image::BImage;
	using interface::ImageFrame;
	using interface::Size2D;
	using geo2d::VecDoublearray;
	using interface::GeoVecDoublearray;
	using geo2d::VecDoublearray;
	namespace filter {


///  CloseGaps filter.
/**
* 
* Cane be used as inplace filter.
*/
class FILTER_API CloseGaps  : public fliplib::TransformFilter
{
public:
	CloseGaps();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName1;		///< Name Out-Pipe

	/// set filter parameter defined in database / xml file
	void setParameter();
	/// paint overerlay primitives
	void paint();

protected:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceed(const void* sender, fliplib::PipeEventArgs& e);
	//! Calculates closeGaps. Uses index sort with ring buffer.
	/*!
	\param p_rLaserlineIn Input laser line. Contains y values at a certain x sampling rate. Ignores all values with bad rank (= eRankMin). 
						  Thus, the median may base on a set of values that are not neighbouring.
	\return void
	\sa -
	*/
	void closeGaps(const VecDoublearray &p_rLaserlineIn);

private:
	typedef fliplib::SynchronePipe< GeoVecDoublearray >		geoVecDoublearrayPipe_t;
	typedef std::unique_ptr< geoVecDoublearrayPipe_t >		uPgeoVecDoublearrayPipe_t;
	typedef std::vector<geo2d::Range>						snippetVec_t;

	const geoVecDoublearrayPipe_t		*m_pPipeInLaserline;	///< in pipe
	const uPgeoVecDoublearrayPipe_t		m_oApPipeOutLaserline;	///< out pipe

	interface::SmpTrafo					m_oSpTrafo;				///< roi translation
	VecDoublearray						m_oLaserlineOut;		///< output laser line
	int									m_oMaxGapX;				///< maximum gap size in x direction
	int									m_oMaxGapY;				///< maximum gap size in y direction
	int									m_oMaxClosureLength;	///< maximum gap length to be closed by interpolation
	snippetVec_t						m_oSnippets;			///< line snippets
	snippetVec_t						m_oBigSnippets;			///< big line snippets
}; // CloseGaps



	} // namespace filter
} // namespace precitec 

#endif /*CLOSEGAPS_H_*/



