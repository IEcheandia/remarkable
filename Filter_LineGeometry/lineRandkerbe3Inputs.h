/**
 * 	@file
 * 	@copyright	Precitec 
 * 	@author		Christian Duchow (Duw)
 * 	@date		2015
 * 	@brief		Fliplib filter 'LineRandkerbe' in component 'Filter_LineGeometry'.
 * 	@detail		Compares the shape of the input line with a user-defined line template. 
 *				
 */

#ifndef LINE_RANDKERBE_3_INPUTS_H__
#define LINE_RANDKERBE_3_INPUTS_H__


#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "geo/geo.h"					///< Size2d, VecDoublearray
#include "geo/array.h"					///< TArray

namespace precitec {
namespace filter {

/**
 *	@ingroup	Filter_LineGeometry
 * 	@detail		Compares the shape of the input line with a user-defined line template. 
*/
class FILTER_API LineRandkerbe3Inputs  : public fliplib::TransformFilter {
public:

	/// CTOR
	LineRandkerbe3Inputs();

private:

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName;		///< Name Out-Pipe
	static const std::string m_oParameterLengthName;///< Name Parameter Length (left)
	static const std::string m_oParameterLengthRightName;///< Name Parameter Length (right)
	static const std::string m_oParameterThresholdName;///< Name Parameter Threshold
	static const std::string m_oParameterJumpName;///< Name Parameter Jump
	static const std::string m_oParameterAngleName;///< Name Parameter Angle
	static const std::string m_oParameterLineLengthName;///< Name Parameter Linienlaenge
	static const std::string m_oParameterLineDeviationName;///< Name Parameter Linienabweichung

	/// Set filter parameters as defined in database / xml file
	void setParameter();

	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/// in pipe event processing
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

	/// Paint overerlay primitives
	void paint();

	void arm(const fliplib::ArmStateBase& state);	///< arm filter

	/**
	* @brief Manages the detection of a step in the laser line.
	* @param p_rArrayIn			Intput laser line consisting of data and rank information.
	* @return					Value denoting whether a step has been detected.
	*/
	double detektiereKerbe(const geo2d::TArray<double> &p_rArrayIn);

	/**
	* @brief Calls further methods to facilitate the processing of the input laser line.
	* @param p_rLineIn			Intput laser lines consisting of several laser lines, only the first of which will be processed.
	* @return					Value denoting whether a step has been detected.
	*/
	double processFirstLine(const geo2d::VecDoublearray &p_rLineIn);

	/**
	* @brief Computes the correlation coefficient for a template signal and the input signal.
	* @param p_rInput			Intput laser line data.
	* @param p_rInRankLine			Intput laser line rank.
	* @param p_oCol			Input value denoting the column for which the correlation coefficient should be computed.
	* @return					Correlation coefficient.
	*/
	float berechneKorrelationsKoeffizient(const std::vector<double>& p_rInput, const std::vector<int>& p_rInRankLine, int p_oCol);
		
	/**
	* @brief Searches for maxima in the input correlation coefficients. Simultaneously, a high jump in the input laser line is enforced.
	* @param p_rInput			Intput laser line data.
	* @param p_rCorrCoefficient			Vector of correlation coefficients.
	* @param p_rMaxima			Resulting vector of maxima.
	*/
	void findeMaxima(const std::vector<double>& rInput, const std::vector<float>& p_rCorrCoefficient, std::vector<int>& p_rMaxima);

	/**
	* @brief Searches for the upper part which is required for a defect. A defect is detected only near the upper part.
	* @param p_rInDataLine		Input laser line data.
	* @param p_rInRankLine		Input laser line rank.
	* @param p_rMaxima			Resulting vector of maxima.
	*/
	void fordereOberblech(const std::vector<double>& p_rInDataLine, const std::vector<int>& p_rInRankLine, const std::vector<int>& p_rMaxima);
		
	const fliplib::SynchronePipe<interface::GeoVecDoublearray>*				m_pPipeInLine;				///< in pipe
	const fliplib::SynchronePipe<interface::GeoDoublearray>*				m_pPipeInXLeft;				///< in pipe
	const fliplib::SynchronePipe<interface::GeoDoublearray>*				m_pPipeInXRight;				///< in pipe
	fliplib::SynchronePipe<interface::GeoDoublearray>						m_oPipeOutKerbe;				///< out pipe

	interface::SmpTrafo				m_oSpTrafo;					///< roi translation
	int					m_oTemplateLength;			///< parameter - length of template (first left side, later total)
	int					m_oTemplateLengthRight;			///< parameter - length of template (right side)
	int					m_oTemplateLengthLeft;			///< parameter - length of template (left side)
	//int					m_oHalfLength;			///< half length of template
	int					m_oJump;					///< parameter - jump
	double							m_oThreshold;				///< threshold
	int								m_oLeftX;
	int								m_oRightX;
	double							m_oAngle;					///< Angle
	int								m_oLineLength;				///< Linienlaenge
	double							m_oLineDeviation;			///< Linienabweichung
	std::vector<double>				m_oFeature;					///< two connected straight line segments defined by length and angle
	std::vector<int>				m_oMaxima;
	std::vector<int>				m_oFinalMaxima;

//#define _DEBUG_DUCHOW
#ifdef _DEBUG_DUCHOW
	std::vector<geo2d::Point>		m_oPositionsLineStartDebug;   // geo2d::Point ist eh nur ein #def auf TPoint<int>
	std::vector<geo2d::Point>		m_oPositionsLineEndDebug;
#endif

	float m_oMeanTemplate;
	float m_oSumTemplateTemplate;
}; // LineRandkerbe

} // namespace filter
} // namespace precitec 

#endif /*LINEFEATURE_H_20140411_INCLUDED*/



