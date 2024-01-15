/*!
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			JS
*  @date			2016
*  @file
*  @brief			Fliplib filter 'LinearLut' in component 'Filter_ImageProcessing'. 
*                   process frame through a linear lut with min and max.
*/

#ifndef LINEARLUT_H_
#define LINEARLUT_H_


#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "common/frame.h"				///< ImageFrame
#include "image/image.h"				///< BImage

#include <string>						///< string class


namespace precitec {
	namespace filter {

///  Linear Lut filter.  
/**
*  min max values to normalize the incoming frame
* Performance: quadratic on image dimension. less than 2ms on a 400X400 ROI
*/
class FILTER_API LinearLut  : public fliplib::TransformFilter {

public:
	
	static const std::string m_oFilterName;		///< Name Filter
	static const std::string m_oPipeOutName;	///< Name Out-Pipe

	/// Standard constructor.
	LinearLut();


	//! Process look up tabelle on the incoming frame. Not inplace. 
	/*!
	\param p_rImageIn		Input image to be read.
	\param p_oMin			min value
	\param p_oMax			max value
	\param p_rImageOut		Output image .
	\return void
	\sa -
	*/
	static void 
	calcLinearLut(
		const image::BImage &p_rImageIn,
		const int	&p_Min,
		const int   &p_Max,
		image::BImage		&p_rImageOut);


	/// set filter parameter defined in database / xml file
	/*virtual*/ void 
	setParameter();

	/// paint overerlay primitives
	/*virtual*/ void 
	paint();


private:

	/*virtual*/ bool 
	subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);					///< in pipe registrastion

	/*virtual*/ void 
	proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);	///< in pipe event processing	

	const fliplib::SynchronePipe< interface::ImageFrame >*	m_pPipeInImageFrame;	///< in pipe
	fliplib::SynchronePipe		< interface::ImageFrame >	m_oPipeOutImgFrame;		///< out pipe

	const interface::ImageFrame								*m_pFrameIn;			///< input frame
	interface::SmpTrafo										m_oSpTrafo;				///< roi translation
	image::BImage											m_oLinearLutImageOut;	///< feature image
	int												        m_oMin;			        ///< min value of the lut
	int														m_oMax;					///< max value of the lut

}; // Linear Lut



	} // namespace filter
} // namespace precitec



#endif /*LINEARLUT_H_*/



