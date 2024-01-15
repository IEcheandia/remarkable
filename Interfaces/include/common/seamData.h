/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			AB, Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			Needed for inspection stops, see function stopInspect(...). Used in VideoRecorder interface.
 */

#ifndef SEAMDATA_H_20120223_INCLUDED
#define SEAMDATA_H_20120223_INCLUDED


#include	<ostream>

namespace precitec {
namespace interface {

/*
 * @brief	Needed for inspection stops, see function stopInspect(...). Used in VideoRecorder interface.
 */
struct SeamData {
	SeamData(int p_oSeamSeries = 0, int p_oSeam = 0, int p_oTriggerDelta = 0, const std::string &seamLabel = {}) :
		m_oSeamSeries	(p_oSeamSeries), 
		m_oSeam			(p_oSeam), 
		m_oTriggerDelta	(p_oTriggerDelta),
		m_oSeamLabel(seamLabel)
	{} // SeamData

	int m_oSeamSeries;
	int	m_oSeam;
	int m_oTriggerDelta;
    std::string m_oSeamLabel;
}; // SeamData


/**
 *	@brief		Inserts data into output stream.
 *	@param		p_rOStream	Out stream to be modified.
 *	@param		p_rData		Input data.
 *	@return		ostream		Modified stream.
*/
inline std::ostream& operator<<( std::ostream& p_rOStream, const SeamData &p_rData ) {
	p_rOStream << "<SeamData=\n";

	p_rOStream << "\tSeam series:\t" 	<< p_rData.m_oSeamSeries << '\n';
	p_rOStream << "\tSeam:\t\t" 		<< p_rData.m_oSeam << '\n';
	p_rOStream << "\tTrigger delta:\t" 	<< p_rData.m_oTriggerDelta << '\n';
	p_rOStream << "SeamData>";
	return p_rOStream;
} // operator<<


typedef std::vector<SeamData>	seamDataVector_t;


} // interface
} // precitec

#endif /* SEAMDATA_H_20120223_INCLUDED */
