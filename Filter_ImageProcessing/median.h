/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			Simon Hilsenbeck (HS)
*	@date			2010
*	@brief			NICHT OPTIMIERTE 2-d Medianberechnung. Randbereich wird ausgelassen. Erzeugt neues Bild selber Groesse.
*/


#ifndef MEDIAN_H_
#define MEDIAN_H_


#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"


namespace precitec {
	using namespace image;
	using namespace interface;
namespace filter {

/**
 * Operiert auf einem ImageFrame und sendet einen neuen ImageFrame an den naechsten Filter
 */
class FILTER_API Median  : public fliplib::TransformFilter
{
public:
	Median();

	static const std::string m_oFilterName;
	static const std::string PIPENAME;

	void setParameter();

protected:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& pipe, int group);
	// Verarbeitungsmethode IN Pipe
	void proceed(const void* sender, fliplib::PipeEventArgs& e);
	/**
	 * @brief Paint overlay output.
	 */
	void paint();

private:
	const fliplib::SynchronePipe< ImageFrame >*		m_pPipeInImageFrame;	///< in pipe
	fliplib::SynchronePipe< ImageFrame >			m_oPipeImageFrame;		//<- Output PIN fuer verarbeitetes Graubild

	static const unsigned int	MAXGRAYVAL;
	static const unsigned int	NGRAYVAL;
	unsigned int				m_oFilterRadius;	///< filter radius (filter lenght-1 / 2)
	interface::SmpTrafo			m_oSpTrafo;			///< roi translation
	std::vector<unsigned int>	m_oHistogram;		///< local histogramm
	std::array<image::BImage, g_oNbParMax>				m_oMedianImageOut;	///< median image
};

}}

#endif /*MEDIAN_H_*/

