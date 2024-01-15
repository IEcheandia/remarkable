/*!
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2011
*  @file
*  @brief			Fliplib filter 'Hough' in component 'Filter_ImageProcessing'. Performs line detection.
*/

#ifndef HOUGH_20110816_H_
#define HOUGH_20110816_H_


#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework

#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output
#include "filter/parameterEnums.h"
#include "geo/houghPPCandidate.h"

#include "common/frame.h"				///< ImageFrame
#include "geo/geo.h"					///< GeoPointarray
#include "image/image.h"				///< BImage

#include <string>						///< string class


namespace precitec {
	namespace filter {


///  Hough filter. Performs line detection.
/**
* Takes an edge detected binary image as input. Output is normal vector of detected line (Point).
* Maps cartesian x, y image space to polar r, phi hough space.
* Maximum in accumulated hough space corresponds with strongest line.
* Calcualtion time O(n) for n = number of true pixels. Invariant to line interruptions.
*/
class FILTER_API Hough  : public fliplib::TransformFilter {
public:
	Hough();

private:
	static const std::string m_oFilterName;		///< Name Filter
	static const std::string m_oPipeOut1Name;	///< Name Out-Pipe
	static const std::string m_oPipeOut2Name;	///< Name Out-Pipe
	static const std::string m_oPipeOutCandidateName;	///< Name Out-Pipe

	typedef fliplib::SynchronePipe<interface::ImageFrame>			image_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoDoublearray >		scalar_pipe_t;
	typedef std::vector<std::vector<int>>							vector_int_2d_t;
	typedef std::tuple<int, int, int>								triple_int_t;
	typedef std::vector<triple_int_t>								vector_triple_int_t;

	enum HoughTriple { eMax, eRad, ePhi }; // define tuple<int, int, int> access

	/// set filter parameter defined in database / xml file
	/*virtual*/ void 
	setParameter();

	/*virtual*/ bool 
	subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);					/// in pipe registrastion

	/*virtual*/ void 
	proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);	/// in pipe event processing	

	//! Performs a hough trafo and searches the clearest line.
	/*!
	\param p_rImageIn			Input image to be read.
	*/
	void	calcHough(const image::BImage				&p_rImageIn, geo2d::HoughPPCandidate & houghCandiate);

	/// paint overerlay primitives
	/*virtual*/ void 
	paint();

    double calcLineFktParameter_m(int theta);
	double calcLineFktParameter_b(int rho, int theta);
	int    getHoughLinePosX(int y, int rho, int theta);
	void calcRoiLinePos(const image::Size2d  oSizeImgIn, triple_int_t ResLine1, triple_int_t ResLine2);

	bool calcLineIntersection(const image::Size2d	oSizeImgIn, const int iLinePos1, const int iLinePos2);
	void calcLineInterupttions(const image::BImage&	p_rImageIn, triple_int_t ResLine1, triple_int_t ResLine2, geo2d::HoughPPCandidate & houghCandiate);
	int calcGreyScaleOfGap(const image::BImage&	p_rImageIn);

	const image_pipe_t*					m_pPipeInImageFrame;	/// in pipe
	scalar_pipe_t						m_oPipeOutPosLeft;		/// out pipe
	scalar_pipe_t						m_oPipeOutPosRight;		/// out pipe
	fliplib::SynchronePipe< interface::GeoHoughPPCandidatearray > m_oPipeOutHoughPPCandidate;

	int									m_oMinAngle;			/// min angle of normal vector in hough space [deg]
	int									m_oMaxAngle;			/// max angle of normal vector in hough space [deg]
	unsigned int						m_oMinLineLengthPix;	/// min length of found line [pixel]
	double								m_oMinLineLength;		/// min length of found line [%]
	unsigned int						m_oMinRadiusDistancePix;/// min distance between radiuses [pixel]
	double								m_oMinRadiusDistance;	/// min distance between radiuses [mm]
	unsigned int						m_oMaxRadiusDistancePix;/// max distance between radiuses [pixel]
	double								m_oMaxRadiusDistance;	/// max distance between radiuses [mm]
	HoughDirectionType					m_oSearchStart;			/// Selection of search start left: 0 / right: 1

	geo2d::Doublearray					m_oPosLeftOut;			/// output left seamposition
	geo2d::Doublearray					m_oPosRightOut;			/// output right seamposition	
	geo2d::HoughPPCandidatearray	    m_oHoughPPCandidateOut; /// output Hough Candidates	

	int									m_oPosLeftY;			/// for drawing
	int									m_oPosRightY;			/// for drawing
	interface::SmpTrafo					m_oSpTrafo;				/// roi translation	
	geo2d::Size							m_oImageSize;			///< image size	
	vector_triple_int_t					m_oMaxima;				/// hough space maxima
	vector_int_2d_t						m_oHistogram;			/// hough space accumulator. x, y -> phi, radius
	image::BImage						m_oHoughImage;

	double                              m_oMaxima0_LineFkt_b;   /// Line function factor b of Maxima0
	double                              m_oMaxima0_LineFkt_m;   /// Line function factor m of Maxima0
	double                              m_oMaxima1_LineFkt_b;   /// Line function factor b of Maxima1
	double                              m_oMaxima1_LineFkt_m;   /// Line function factor m of Maxima1

	int                                 m_oXPosRoiStartLine1;
	int                                 m_oXPosRoiEndLine1;
	int                                 m_oXPosRoiStartLine2;
	int                                 m_oXPosRoiEndLine2;

}; // Hough



	} // namespace filter
} // namespace precitec



#endif /*HOUGH_20110816_H_*/



