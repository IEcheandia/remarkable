/*!
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			Claudius Batzlen (CB)
*  @date			2017
*  @file
*  @brief			Fliplib filter 'poorPenetrationEdge' in component 'Filter_ImageProcessing'. Performs line detection.
*/

#ifndef poorPenetrationEdge_20170714_H_
#define poorPenetrationEdge_20170714_H_


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
* Takes an edge detected binary image and the corresponding grey image as input. Output is normal struct of values.
* Maps cartesian x, y image space to polar r, phi hough space.
* Maximum in accumulated hough space corresponds with strongest line.
* Calcualtion time O(n) for n = number of true pixels. Invariant to line interruptions.
*/
		class FILTER_API PoorPenetrationEdge : public fliplib::TransformFilter {
public:
	PoorPenetrationEdge();

private:
	static const std::string m_oFilterName;		///< Name Filter
	static const std::string m_oPipeOutCandidateName;	///< Name Out-Pipe

	typedef fliplib::SynchronePipe<interface::ImageFrame>			image_pipe_t;
	typedef std::vector<std::vector<int>>							vector_int_2d_t;
	typedef std::tuple<int, int, int>								triple_int_t;
	typedef std::vector<triple_int_t>								vector_triple_int_t;

	enum HoughTriple { eMax, eRad, ePhi }; // define tuple<int, int, int> access

	
	void setParameter();                                  /// set filter parameter defined in database / xml file
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);                       /// in pipe registrastion
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE);	/// in pipe event processing	
	void paint();  	                                                                /// paint overerlay primitives
	
	// main function
	void calcHough(const image::BImage& p_rImageIn_Binary, const image::BImage&	p_rImageIn_Grey, geo2d::HoughPPCandidate & houghCandiate);

	// Hough functions
    double calcLineFktParameter_m(int theta);
	double calcLineFktParameter_b(int rho, int theta);
	int    getHoughLinePosX(int y, int rho, int theta);
	void   calcRoiLinePos(const image::Size2d  oSizeImgIn, triple_int_t ResLine1, triple_int_t ResLine2);

	// Calc functions
	bool calcLineIntersection(const image::Size2d	oSizeImgIn, const int iLinePos1, const int iLinePos2);
	void calcLineInterupttions(const image::BImage&	p_rImageIn, triple_int_t ResLine1, triple_int_t ResLine2, geo2d::HoughPPCandidate & houghCandiate);
	int calcGreyScaleOfGap(const image::BImage&	p_rImageIn);

	// Inpipe images
	const image_pipe_t*					m_pPipeInImageBinaryFrame;	/// in pipe
	const image_pipe_t*					m_pPipeInImageGreyFrame;	/// in pipe
	// Outpipe struct
	fliplib::SynchronePipe< interface::GeoHoughPPCandidatearray > m_oPipeOutHoughPPCandidate;

	// Parameter and Variables
	int									m_oMinAngle;			/// min angle of normal vector in hough space [deg]
	int									m_oMaxAngle;			/// max angle of normal vector in hough space [deg]
	unsigned int						m_oMinRadiusDistancePix;/// min distance between radiuses [pixel]
	double								m_oMinRadiusDistance;	/// min distance between radiuses [mm]
	HoughDirectionType					m_oSearchStart;			/// Selection of search start left: 0 / right: 1

	geo2d::HoughPPCandidatearray	    m_oHoughPPCandidateOut; /// output Hough Candidates	

	int									m_oMaxima0_PosX;	    /// for drawing
	int									m_oMaxima0_PosY;		/// for drawing
	int									m_oMaxima1_PosX;		/// for drawing
	int									m_oMaxima1_PosY;		/// for drawing
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
    }; // poorPenetrationEdge

  } // namespace filter
} // namespace precitec



#endif /*poorPenetrationEdge_20170714_H_*/



