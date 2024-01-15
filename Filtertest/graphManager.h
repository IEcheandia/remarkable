/*!
 *  \n Copyright:	Precitec Vision GmbH & Co. KG
 *  \n Project:		WM Filtertest
 *  \author			Simon Hilsenbeck (HS)
 *  \date			2010-2011
 *  \file
 *  \brief
 */
#ifndef GRAPHMANAGER_H_
#define GRAPHMANAGER_H_

// stl includes
#include <atomic>
#include <vector>
#include <array>
#include <set>
#include <memory>
// Poco includes
#include <Poco/File.h>
#include <Poco/Thread.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/Semaphore.h>
// fliplib includes
#include <fliplib/Fliplib.h>
#include <fliplib/FilterGraph.h>
#include <fliplib/SynchronePipe.h>
#include <fliplib/NullSourceFilter.h>
// wm
#include "analyzer/signalAdapter.h"
#include "common/frame.h"
#include "filter/productData.h"
#include "overlay/overlayCanvas.h"
// local includes
#if defined __QNX__
	#include "QnxCanvas.h"
#elif defined HAVE_QT
	#include "QtCanvas.h"
#endif
#include "resultHandler.h"


namespace precitec {
namespace filter {


class GraphManager
{
public:
    typedef fliplib::SynchronePipe< interface::ImageFrame >  ImageFramePipe;
    typedef fliplib::SynchronePipe< interface::SampleFrame > SampleFramePipe;
    typedef Poco::RunnableAdapter<GraphManager>				 thread_adapter_t;
    typedef std::array<analyzer::SignalAdapter, g_oNbParMax> signal_adapters_t;

 	GraphManager( const std::string p_oFilename, const std::string p_oBmpPath, 
                  int inspectionVelocity_um_s, int triggerDelta_um,
                std::map<interface::Sensor,int> defaultSamples = {{interface::Sensor::eGenPurposeDigIn1, 43885}},
                bool p_oHasCanvas = true, std::string resultFolder = "");

	void init(bool p_oHasCanvas);
	void apply (fliplib::AbstractFilterVisitor& p_rVisitor);
    void writeResultsToFolder(std::string folder);
    void printAllTimings(bool value);
    void redirectLogMessages(bool value);
    void setNumImages(int numImages);
    void setArmAtSequenceRepetition(bool value);
	void setParameter();
    void setPauseAfterEachImage(bool pause)
    {
        m_pauseAfterEachImage = pause;
    }

	void fire();
private:
    
	void loadImages();
    void checkImageLoading(int p_oI);
    void drawFrame(int p_oI);

	const std::string										m_oXML_Filename;
	const std::string										m_oBmpPath;

	image::Size2d											m_oImgSize;
	geo2d::Point											m_oHWROI;
	std::vector< interface::ImageFrame >					m_oVectorBmpData;
    std::vector<std::map<int, interface::SampleFrame>>		m_oSampleFrames;
	std::set<std::string>									m_oBmpFilePaths;
    std::set<std::string>									m_oSmpFilePaths;

	fliplib::SpFilterGraph									m_pspGraph;
	fliplib::NullSourceFilter								m_oNullSourceFilter;
	ImageFramePipe											m_oPipeImageFrame;
    SampleFramePipe											m_oPipeSampleFrame;
	ResultHandler											m_oResultHandler;

	std::unique_ptr<image::OverlayCanvas>					m_pCanvas;					// holds WinCanvas or QnxCanvas
    std::array<image::OverlayCanvas, g_oNbParMax>			m_oCanvasBuffer;            // for buffering for pipelining
	analyzer::ProductData									m_oExternalProductData;		///< external product data, eg velocity
	thread_adapter_t										m_oRunloadImages;	        ///< Thread adapter that executes the loadImages() method
	Poco::Thread											m_oWorkerImgLoad;
    std::array<Poco::Thread, g_oNbParMax>					m_oWorkers;					// worker threads, number of used ones depends on user parameter
	signal_adapters_t                                       m_oSignalAdapters;           ///< signal adpaters for using worker threads with a data pipe and an image
    mutable Poco::Semaphore									m_oImagesLoadedSema;        ///< semaphore to signal that all images have been loaded
    std::atomic<std::size_t>                                m_oNbImagesLoaded;			// atomic buggy under gcc 4.61 - doesnt matter here
    std::map<interface::Sensor,int> m_oDefaultSamples;
    bool m_oPrintAllTimings; 
    bool m_oRedirectLogMessages; //redirectLogMessages to std null during process
    bool m_forceNumImages = false;
    bool m_armAtSequenceRepetition = false;
    int m_numImages;
    bool m_pauseAfterEachImage{false};


};

} // namespace filter
} // namespace precitec


#endif /*GRAPHMANAGER_H_*/
