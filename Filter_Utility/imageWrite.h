/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		
 * 	@date		
 * 	@brief		This filter writes an image to file
 */


#ifndef IMAGEWRITE_H
#define IMAGEWRITE_H

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"
#include <filter/armStates.h>

#include "common/frame.h"


namespace precitec {
	using namespace image;
	using namespace interface;
	namespace filter {



class FILTER_API ImageWrite : public fliplib::TransformFilter
{
public:

	/**
	 * CTor.
	 */
	ImageWrite();
	/**
	 * @brief DTor.
	 */
	virtual ~ImageWrite();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Out-pipe (same as input)


	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();


protected:

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/**
	 * @brief Processing routine.
	 * @param p_pSender
	 * @param p_rEvent
	 */
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

protected:
	const fliplib::SynchronePipe< ImageFrame >*		m_pPipeInImageFrame;	///< in pipe
	fliplib::SynchronePipe< ImageFrame >			m_oPipeImageFrame;		//<- Output PIN für verarbeitetes Graubild
	std::string m_oOutputFolder;
	std::string m_oOutputFilename;
	unsigned int m_oOutputFolderN;
	unsigned int m_oOutputFilenameN;
}; 

} // namespace filter
} // namespace precitec

#endif /* IMAGEWRITE_H */
