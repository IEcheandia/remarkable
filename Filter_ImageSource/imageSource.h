/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		
 */

#ifndef IMAGESOURCE_H_
#define IMAGESOURCE_H_


#include "fliplib/Fliplib.h"
#include "fliplib/SourceFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include <vector>
#include "Poco/SharedPtr.h"
#include <overlay/overlayPrimitive.h>
#include "common/frame.h"

namespace precitec {
	using namespace image;
	using namespace interface; 	
namespace filter {
	
class FILTER_API ImageSource : public fliplib::SourceFilter {
	typedef Poco::SharedPtr<image::OverlayShape> tOverlayShapePtr;

public:
	ImageSource();
			
	static const std::string m_oFilterName;
	static const std::string m_oPipeName0;
				
protected:
		
	// Dieser Filter darf nur mit einer bestimmten Pipe verbunden werden
	bool subscribe(fliplib::BasePipe& pipe, int group);
	// Verarbeitungsmethode IN Pipe
	void proceed(const void* sender, fliplib::PipeEventArgs& e);
		
private:
	typedef	fliplib::SynchronePipe< ImageFrame >	imageFramePipe_t;

	const imageFramePipe_t*		m_pPipeFrameIn;		//<- Input PIN fuer Images
	imageFramePipe_t			m_oPipeFrameOut;	//<- Output PIN fuer Images

	std::vector<std::pair<int, int>> m_oGrid;
}; // class ImageSource

} // namespace
} // namespace

#endif /*IMAGESOURCE_H_*/
