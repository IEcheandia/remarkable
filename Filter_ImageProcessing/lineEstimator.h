/*!
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			DUW
 *  @date			2015
 *  @file
 *  @brief			Fliplib filter 'LineEstimator' in component 'Filter_ImageProcessing'. Estimates a line given 3 points in 2D.
 */

#ifndef LINEESTIMATOR_H_
#define LINEESTIMATOR_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"	
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"
#include "fliplib/ArmState.h"

#include "common/frame.h"    // ist wohl benoetigt um interface::GeoDoublearray bekannt zu machen
#include "common/geoContext.h"

namespace precitec {
namespace filter {


/**
 * Schaetzt eine Gerade gegeben 3 Punkte in 2D.
 */
class FILTER_API LineEstimator : public fliplib::TransformFilter
{
public:
	static const std::string m_oFilterName;
	static const std::string PIPENAMEX;
	static const std::string PIPENAMEY;
	static const std::string PIPENAMEBETA;
	static const std::string PIPENAMELINE;

	LineEstimator();

	void setParameter();							///< parameter setzen
	void paint();									///< Zeichnet die gefundene Gerade plus die Eingabedaten
	void arm (const fliplib::ArmStateBase& state);	///< arm filter

private:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& pipe, int group);

	/// Verarbeitungsmethode IN Pipe
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

	//! Die eigentliche Filterfunktion. Nach der Berechnung wird das Ergebnis in den Membervariablen gespeichert, weil es sowieso auch in der Methode paint() zur Verfuegung stehen muss.
	/*!
	\param p_X0		X-Coordinate of first point.
	\param p_Y0		Y-Coordinate of first point.
	\param p_X1		X-Coordinate of second point.
	\param p_Y1		Y-Coordinate of second point.
	\param p_X2		X-Coordinate of third point.
	\param p_Y2		Y-Coordinate of third point.
	*/
	bool estimateLine(double p_X0, double p_Y0, double p_X1, double p_Y1, double p_X2, double p_Y2);

	typedef fliplib::SynchronePipe<interface::GeoDoublearray>		pipe_scalar_t;
	typedef fliplib::SynchronePipe<interface::GeoLineModelarray>	pipe_line_t;


	const pipe_scalar_t		*m_pPipeInDoubleX0;		///< In-Pipe scalar
	const pipe_scalar_t		*m_pPipeInDoubleY0;		///< In-Pipe scalar
	const pipe_scalar_t		*m_pPipeInDoubleX1;		///< In-Pipe scalar
	const pipe_scalar_t		*m_pPipeInDoubleY1;		///< In-Pipe scalar
	const pipe_scalar_t		*m_pPipeInDoubleX2;		///< In-Pipe scalar
	const pipe_scalar_t		*m_pPipeInDoubleY2;		///< In-Pipe scalar
	pipe_scalar_t		m_oPipeOutDoubleX;	///< outpipe 
	pipe_scalar_t		m_oPipeOutDoubleY;	///< outpipe 
	pipe_scalar_t		m_oPipeOutDoubleBeta;	///< outpipe 
	pipe_line_t			m_oPipeOutLineEquation;	///< outpipe 


	double m_oXMean;
	double m_oYMean;
	double m_oBeta;

	std::vector<double> m_oInputX;
	std::vector<double> m_oInputY;
};



} // namespace filter
} // namespace precitec

#endif /*ROISELECTOR_H_*/
