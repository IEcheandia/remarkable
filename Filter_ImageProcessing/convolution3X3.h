/*!
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2011
*  @file
*  @brief			Fliplib filter 'Convolution3X3' in component 'Filter_ImageProcessing'. Convolutes image with a 3 x 3 filter kernel. Not inplace.
*/

#ifndef CONVOLUTION3X3_20110811_H_
#define CONVOLUTION3X3_20110811_H_


#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "common/frame.h"				///< ImageFrame
#include "image/image.h"				///< BImage

#include <string>						///< string class


namespace precitec {
	namespace filter {

///  Convolution3X3 filter. Convolutes image with a 3 x 3 filter kernel. Not inplace. 
/**
* Eg for sobel, laplace, ....
* Performance: quadratic on image dimension. less than 2ms on a 400X400 ROI
*/
class FILTER_API Convolution3X3  : public fliplib::TransformFilter {

public:
	///  Filter coefficients for image convolution3X3.
	struct FilterCoeff {
		FilterCoeff() : m_oCoeff1(1), m_oCoeff2(1), m_oCoeff3(1), m_oCoeff4(1), m_oCoeff5(1), m_oCoeff6(1), m_oCoeff7(1), m_oCoeff8(1), m_oCoeff9(1) {}
		int m_oCoeff1, m_oCoeff2, m_oCoeff3, m_oCoeff4, m_oCoeff5, m_oCoeff6, m_oCoeff7, m_oCoeff8, m_oCoeff9; // nine coefficients for 3X3 filter mask
	};

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string m_oPipeOut1Name;	///< Name Out-Pipe

	/// Standard constructor.
	Convolution3X3();


	//! Convolutes image with a 3 x 3 filter kernel. Not inplace. 
	/*!
	\param p_rImageIn		Input image to be read.
	\param p_oFilterCoeff	filter coefficients
	\param p_rImageOut		Output image to be calculated.
	\return void
	\sa -
	*/
	static void 
	calcConvolution3X3(
		const image::BImage &p_rImageIn,
		const FilterCoeff	&p_FilterCoeff,
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

	interface::SmpTrafo										m_oSpTrafo;				///< roi translation
	std::array<image::BImage, g_oNbParMax>					m_oConvolutedImageOut;	///< feature image
	FilterCoeff												m_oFilterCoeff;			///< filter coefficients

}; // Convolution3X3



	} // namespace filter
} // namespace precitec



#endif /*CONVOLUTION3X3_20110811_H_*/



