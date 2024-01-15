/*!
 *  \n Copyright:	Precitec Vision GmbH & Co. KG
 *  \n Project:		WM Filtertest
 *  \author			Simon Hilsenbeck (HS), Stefan Birmanns (SB)
 *  \date			2010-2011
 *  \file
 *  \brief
 */

#include "graphManager.h"

#include <iostream>							// cout
#include <algorithm>						// generate

#include "Poco/Glob.h"						// glob-style pattern matching
#include "Poco/Path.h"						// Working with files and directories
#include "Poco/DirectoryIterator.h"			// Working with files and directories
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"

#include "system/timer.h"					// Timer
#include "fliplib/GraphBuilderFactory.h"
#include "filter/sensorFilterInterface.h"	// SENSOR_IMAGE_FRAME_PIPE
#include "filter/armStates.h"				// arm states
#include "analyzer/graphAssistent.h"		// PipeScope
#include "analyzer/graphVisitors.h"			// visitors
#include "common/frame.h"					// ImageFrame
#include "common/bitmap.h"					// Bitmap
#include "image/image.h"					// genmodulopattern
#include "event/sensor.h" 
#include "common/sample.h"

#ifdef _WIN32
#define NULL_DEVICE "NUL:"
#else
#define NULL_DEVICE "/dev/null"
#endif

#ifdef HAVE_QT
#include <QCoreApplication>
#endif


extern FILE * g_pLoggerStream;
extern bool g_oLoggerStreamOpened;
extern bool g_oDebugTimings;

void redirectWmLogToStdOut()
{
    
    if (g_oLoggerStreamOpened)
    {
        fclose(g_pLoggerStream);
        g_oLoggerStreamOpened = false;
    }
    
    g_pLoggerStream = stdout;
}

void redirectWmLogToNull()
{
    
    if (g_oLoggerStreamOpened)
    {
        fclose(g_pLoggerStream);
        g_oLoggerStreamOpened = false;
    }
    
    g_pLoggerStream = fopen(NULL_DEVICE, "w");
    g_oLoggerStreamOpened = true;
}


using namespace fliplib;
using namespace Poco;
namespace precitec {
		using namespace interface;
		using namespace geo2d;
		using namespace system;
		using namespace image;
		using namespace analyzer;
	namespace filter {

	GraphManager::GraphManager( std::string oXML_Filename, std::string p_oBmpPath, 
                                 int inspectionVelocity_um_s, int triggerDelta_um,                 
                              std::map<interface::Sensor,int> defaultSamples,
                             bool p_oHasCanvas, std::string resultFolder )
		:	
		m_oXML_Filename				( oXML_Filename ),
		m_oBmpPath					( p_oBmpPath ),
		m_oPipeImageFrame			( &m_oNullSourceFilter, SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE ),
        m_oPipeSampleFrame			( &m_oNullSourceFilter, SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE ),
        m_oExternalProductData(
            0,  //seam series
            0, //seam
            inspectionVelocity_um_s, //inspection velocity [um/s]
            triggerDelta_um, //triggerDelta
            2000,  //numtrigger m_pActiveSeam->m_oLength / m_pActiveSeam->m_oTriggerDelta
			nullptr,  // Ref-curve
			1500,    // m_pActiveSeam->m_oLength
			1,    // m_pActiveSeam->m_oDirection
			33,    // m_pActiveSeam->m_oThicknessLeft
			44,    // m_pActiveSeam->m_oThicknessRight
            55), //m_pActiveSeam->m_oTargetDifference
		m_oRunloadImages	        ( *this, &GraphManager::loadImages ),
		m_oWorkerImgLoad			( "WorkerImgLoad" ),
		m_oImagesLoadedSema			( 0, 1 ),
        m_oNbImagesLoaded           ( 0 ),
        m_oDefaultSamples(std::move(defaultSamples))
	{
        g_oNbPar    =   1; 
        if (!resultFolder.empty())
        {
            writeResultsToFolder(resultFolder);
        }
		// init the graph manager		
		init(p_oHasCanvas);
	} // GraphManager()


/**
	* Initialize the test graph manager
	*/
void GraphManager::init(bool p_oHasCanvas)
{
	ScopedTimer		oScopedTimer		(__FUNCTION__);

	XML::DOMParser oParser;
	oParser.setFeature( Poco::XML::DOMParser::FEATURE_FILTER_WHITESPACE, true );
	AutoPtr<XML::Document> pDoc;
	pDoc = oParser.parse(m_oXML_Filename);
	const XML::Element* pGraph = pDoc->documentElement();

	if (!pGraph) {
		throw fliplib::GraphBuilderException("XML document invalid.");
	} // if
		
	if (pGraph->hasAttribute("id")) { // is a light xml graph
		m_pspGraph			= 	GraphBuilderFactory().createLight()->build( m_oXML_Filename );
	}
	else if (pGraph->getChildElement("Header") != nullptr) { // is a wm xml graph
		m_pspGraph			= 	GraphBuilderFactory().create()->build( m_oXML_Filename );
	}
	else {
		throw fliplib::GraphBuilderException("XML format not recognized.");
	}

	ResultHandlerConnector oResultHandlerConnector(m_oResultHandler);
	apply(oResultHandlerConnector);

	if (m_oBmpPath.empty()) 
    {
        
        m_oImgSize.width = MAX_CAMERA_WIDTH;
        m_oImgSize.height = MAX_CAMERA_HEIGHT;
        m_oHWROI.x = 0;
        m_oHWROI.y = 0;
        
        const auto  oNbImages       = 10;
        auto		oCounter		= 0;
        const auto	oFrameGenerator = [this, &oCounter] () -> ImageFrame
        {

            ImageContext		oImageContext;
            oImageContext.setImageNumber(oCounter);
            oImageContext.HW_ROI_x0 = m_oHWROI.x;
            oImageContext.HW_ROI_y0 = m_oHWROI.y;
            ++oCounter;

            const image::BImage		oImage				( m_oImgSize );
            const ImageFrame		oFrame				( oImageContext, oImage );
            return oFrame; 
            
        };
            
        std::generate_n(std::back_inserter(m_oVectorBmpData), oNbImages, oFrameGenerator);
        m_oNbImagesLoaded.store(oNbImages);
        
        m_oBmpFilePaths = {};
        
        
        oCounter		= 0;
        const auto	oSampleFrameGenerator = [this, &oCounter] () -> std::map<int, SampleFrame> {
            ImageContext		oImageContext;
            oImageContext.setImageNumber(oCounter);
            ++oCounter;

            std::map<int, SampleFrame> samples;
            for (auto & rDefault : m_oDefaultSamples)
            {
                const image::Sample		oSample				( 1, rDefault.second );
                const SampleFrame		oFrame				( oImageContext, oSample );
                samples[rDefault.first] = oFrame;
            }
            
            return samples;
        };

        std::generate_n(std::back_inserter(m_oSampleFrames), oNbImages, oSampleFrameGenerator);
        
	} // if
	else 
    {
        
        // create bmp file list

        std::cout << "GraphManager::init: Loading bmp files from "<< m_oBmpPath << " ..." << std::endl;

        Poco::File oFile(m_oBmpPath);
        if (oFile.exists()) 
        {
            if (oFile.isFile())
            {
                fileio::Bitmap oBitmap(m_oBmpPath);
                if (oBitmap.validSize())
                {
                    m_oBmpFilePaths = {m_oBmpPath};
                }
            }
            else
            {
                Poco::Path oPocoBmpPath(m_oBmpPath, "*.bmp");
                Poco::Glob::glob(oPocoBmpPath.makeFile(), m_oBmpFilePaths);
            }


        }

        std::cout << "GraphManager::init: " << m_oBmpFilePaths.size() <<  " bmp files found." << std::endl;

        //get size from first image
        if (m_oBmpFilePaths.size() > 0)
        {
            fileio::Bitmap oBitmap(*m_oBmpFilePaths.begin()); 
            m_oImgSize.width = oBitmap.width();
            m_oImgSize.height = oBitmap.height();

            image::TLineImage<byte> image(m_oImgSize);
            std::vector<unsigned char> oAdditionalData;
            oBitmap.load(image.begin(), oAdditionalData);
            //see vdr::add_data_indices:
            if ( oAdditionalData.size() >= 8 )
            {
                m_oHWROI.x = *(reinterpret_cast<unsigned short*>(&oAdditionalData[2]));
                m_oHWROI.y = *(reinterpret_cast<unsigned short*>(&oAdditionalData[4]));
            }
            else
            {
                //set the HW_ROI such that is not null, but still compatible with the image size
                const Size2D MaxSensorSize = (m_oImgSize.width < 1024 ) ? Size2D{1024,1024} : Size2D{MAX_CAMERA_WIDTH, MAX_CAMERA_HEIGHT} ;
                m_oHWROI.x = MaxSensorSize.width - m_oImgSize.width;
                m_oHWROI.y = MaxSensorSize.height - m_oImgSize.height;
            }
            std::cout << "GraphManager::init: Image size: (" << m_oImgSize.width << "x" << m_oImgSize.height
                << "), HWROI " << m_oHWROI.x << " " << m_oHWROI.y << ".\n";

        } //end getsize, hwroi

        // pre allocate all images and load them block-wise from bmp file path vector

        const auto  oNbImages       = m_oBmpFilePaths.size();
        auto		oCounter		= 0;
        const auto	oFrameGenerator = [this, &oCounter] () -> ImageFrame
        {

            ImageContext		oImageContext;
            oImageContext.setImageNumber(oCounter);
            oImageContext.HW_ROI_x0 = m_oHWROI.x;
            oImageContext.HW_ROI_y0 = m_oHWROI.y;
            ++oCounter;

            const image::BImage		oImage				( m_oImgSize );
            const ImageFrame		oFrame				( oImageContext, oImage );
            return oFrame; };
            
        std::generate_n(std::back_inserter(m_oVectorBmpData), oNbImages, oFrameGenerator);
    
        // create smp file list

        std::cout << "GraphManager::init: Loading smp files..." << std::endl;

        Poco::File oSampleFile(m_oBmpPath);
        if (oSampleFile.exists()) 
        {
            Poco::Path oPocoSmpPath(m_oBmpPath, "*.smp");
            Poco::Glob::glob(oPocoSmpPath.makeFile(), m_oSmpFilePaths);
            std::cout << "GraphManager::init: " << m_oSmpFilePaths.size() <<  " smp files found." << std::endl;
        }

        // generate all sample frames

        oCounter		= 0;
        const auto	oSampleFrameGenerator = [this, &oCounter] () -> std::map<int, SampleFrame> {
            ImageContext		oImageContext;
            oImageContext.setImageNumber(oCounter);
            ++oCounter;

            std::map<int, SampleFrame> samples;
            for (auto & rDefault : m_oDefaultSamples)
            {
                const image::Sample		oSample				( 1, rDefault.second );
                const SampleFrame		oFrame				( oImageContext, oSample );
                samples[rDefault.first] = oFrame;
            }
            
            return samples;
        };

        std::generate_n(std::back_inserter(m_oSampleFrames), oNbImages, oSampleFrameGenerator);
    }
    // start loading images
	m_oWorkerImgLoad.start		(m_oRunloadImages);

    // set always canvas buffer, as filters alaways paint
    auto oCanvasSetter	=	CanvasSetter( m_oCanvasBuffer.data() );
	apply(oCanvasSetter);

	if (p_oHasCanvas)
	{
		// now that we know the size of the images, we can create the canvas
#if defined __QNX__
		m_pCanvas.reset(new QnxCanvas(m_oImgSize.width, m_oImgSize.height));
#elif defined HAVE_QT
		m_pCanvas.reset(new QtCanvas(m_oImgSize.width, m_oImgSize.height));
#endif
		m_pCanvas->setTitle("Filtertest");
	}
        
    m_oImagesLoadedSema.wait(); //  preload all images(wait for previous block load finish signaled.)
	std::cout << "GraphManager::init: Done." << std::endl;
} // init


void GraphManager::setParameter() {
	std::cout << "GraphManager::setParameter()" << std::endl;
	auto			oActiveGraphAssistent	=	GraphAssistent	{ m_pspGraph.get() };
	oActiveGraphAssistent.setExternalData(m_oExternalProductData);

	ParameterSetter oParameterSetter;
	apply(oParameterSetter);
}

void GraphManager::apply (AbstractFilterVisitor& p_rVisitor) {
	m_pspGraph->control(p_rVisitor);
}

void GraphManager::fire() 
{


	const unsigned int			oNbImagesInFolder		( m_oVectorBmpData.size() );
	if (oNbImagesInFolder == 0) {
		std::cout << "GraphManager::fire: No images loaded.\n";
		return;
	} // if

    if (m_oRedirectLogMessages)
    {
        redirectWmLogToNull();
    }
    if (m_oPrintAllTimings)
    {
        BaseFilter::resetProcessingCounter(); 
        ResetFilterIndexesVisitor oResetFilterIndexesVisitor;
        apply(oResetFilterIndexesVisitor);
        
        SetAlwaysEnableTimingsVisitor oSetAlwaysEnableTimingsVisitor (true);
        apply(oSetAlwaysEnableTimingsVisitor);
    }
    
	FilterArm oFilterArmStart	{ eSeamStart };
	apply(oFilterArmStart);
    
    int imageSensor = 1; // from \DatabaseSkripts\FilterImageSource\ImageSource.sql) ),
	PipeScope<ImageFrame>   oPipeScopeImage     ( m_pspGraph.get(), imageSensor, &m_oPipeImageFrame );
    PipeScope<SampleFrame>  oPipeScopeSample    ( m_pspGraph.get(), eExternSensorDefault, &m_oPipeSampleFrame );

	std::cout << "GraphManager::fire: Processing images:\n";
	std::cout << ">> " << std::flush;

	// get time before the actual processing starts.
	system::Timer oTimerFrame       ("Timer total");
	oTimerFrame.start();

	// run the filter graph for every image
    unsigned int  numImagesToPlay = m_forceNumImages ? m_numImages : oNbImagesInFolder;
	unsigned int imageCounterTotal	= 0;
    unsigned int  seamNr = 0;
    unsigned int  numSeams = m_armAtSequenceRepetition ? numImagesToPlay / oNbImagesInFolder : 1;
    unsigned int  numImagesPerSeam = m_armAtSequenceRepetition ? oNbImagesInFolder : numImagesToPlay;
    unsigned int  numImagesLastSeam = numImagesToPlay - (numSeams -1)*numImagesPerSeam;
    
    std::cout << " Number of ImagesToPlay " << numImagesToPlay  << std::endl;
    
    auto oSignalAdapter = SignalAdapter{ 0, nullptr, &m_oPipeImageFrame, &m_oPipeSampleFrame, m_pspGraph.get() };

    while ( imageCounterTotal < numImagesToPlay) 
    {

        unsigned int imageCounterInSeam	= 0;
        oTimerFrame.elapsed();
        bool lastSeam =  seamNr == (numSeams -1);
        auto numImagesCurrentSeam = lastSeam ? numImagesPerSeam : numImagesLastSeam;
        auto seamStart_us = oTimerFrame.us();
        while(imageCounterInSeam < numImagesCurrentSeam)
        {
            checkImageLoading(imageCounterTotal);

            const auto	oIdxData        =   imageCounterTotal % oNbImagesInFolder;
            
            m_oVectorBmpData[oIdxData].context().setImageNumber(imageCounterInSeam);	// ascending image numbers are mandatory
            for (auto && pair : m_oSampleFrames.at(oIdxData))
            {
                pair.second.context().setImageNumber(imageCounterInSeam);
            }
            
            oSignalAdapter.setSamples(m_oSampleFrames[oIdxData]);
            oSignalAdapter.setImage(m_oVectorBmpData[oIdxData]);
            oSignalAdapter.setImageNumber(imageCounterInSeam);
            oSignalAdapter.run();

            drawFrame(imageCounterTotal);
            
            if (imageCounterTotal % 1000 == 0)
                std::cout << "\n" ;
            if (imageCounterTotal % 10 == 0)
                std::cout << "." << std::flush;

            oTimerFrame.elapsed();
            ++imageCounterTotal;
            ++imageCounterInSeam;
        }  // images in seam
        
        if (!lastSeam)
        { 
            assert(m_armAtSequenceRepetition);
            //Note: the timer is not stopped, the final average time per frame will be higher than the seam time per frame
            oTimerFrame.elapsed();
            auto seamEnd_us = oTimerFrame.us();
                
            FilterArm oFilterArmEnd	{ eSeamEnd };
            apply(oFilterArmEnd);
            std::cout << "\nGraphManager::fire: seam "<< seamNr << " " << std::fixed  << double(seamEnd_us - seamStart_us) / double(imageCounterInSeam) << " us per frame.\n" ;

            seamNr ++;
            
            FilterArm oFilterArmStart	{ eSeamStart };
            apply(oFilterArmStart);
            
            oTimerFrame.elapsed();
            seamStart_us = oTimerFrame.elapsed();
            std::cout << "Arm seamEnd, seamstart: " << seamStart_us - seamEnd_us  << " us \n";
        }
    } // images to play

    // for async notify test - wait for last image finished
    std::cout << "\n\tGraphManager::fire: Joining threads...\n";
    for (std::size_t oIdx = imageCounterTotal - g_oNbPar; oIdx < imageCounterTotal; ++oIdx)
    {
        if (m_oWorkers[oIdx % g_oNbPar].isRunning())
        {
            m_oWorkers[oIdx % g_oNbPar].join();
        }
        std::cout << "\tGraphManager::fire: Joined thread " << oIdx % g_oNbPar << " (img " << oIdx << ")." << std::endl;
    }

	// measure runtime and output stats.

	oTimerFrame.stop();
	std::cout << "\nGraphManager::fire: " << imageCounterTotal << " images in " << oTimerFrame.ms() << " ms processed." << std::endl;
	std::cout << "GraphManager::fire: " << std::fixed  << (double)( oTimerFrame.us() ) / imageCounterTotal << " us per frame." << std::endl;

    
    redirectWmLogToStdOut();

    LogProcessingTimeVisitor oLogProcessingTimeVisitor;
    m_pspGraph->controlAccordingToProcessingOrder(oLogProcessingTimeVisitor);
        
    if (m_oRedirectLogMessages)
    {
        redirectWmLogToNull();
    }
    
	FilterArm oFilterArmEnd	{ eSeamEnd };
	apply(oFilterArmEnd);

    ResultHandlerReleaser oResultHandlerReleaser(m_oResultHandler);
	apply(oResultHandlerReleaser);
    m_oResultHandler.clearInPipes();
 } // GraphManager::fire()




void GraphManager::loadImages() {
	//ScopedTimer		oTimerFrame		(__FUNCTION__);
		
	auto		oItFile			( m_oBmpFilePaths.cbegin() );
	auto		oItImgData		( std::begin(m_oVectorBmpData) );
    auto oItSampleFile(m_oSmpFilePaths.cbegin());
    auto oItSampleFrames( std::begin(m_oSampleFrames));
    auto oItSampleFileEnd(m_oSmpFilePaths.cend());
    
    std::map<int,bool> sampleCSVhasHeader;

	while (m_oNbImagesLoaded < m_oBmpFilePaths.size()) {
        if (m_oNbImagesLoaded % 50 == 0) {
            std::cout << std::flush << "\tLoading images (" << m_oNbImagesLoaded << " to " << std::min(m_oBmpFilePaths.size(), m_oNbImagesLoaded + 50u) << ")..." << std::endl;
        }

		// load header
		fileio::Bitmap		oBitmap			( *oItFile );
		const Size			oCurrentSize	( oBitmap.width(), oBitmap.height() );	
		if (oCurrentSize !=  m_oImgSize ) {
			std::cout << std::flush << "\tGraphManager::loadImages: Current image size '" << oCurrentSize << "' does not match initial size '" << m_oImgSize << "'. Image skipped." <<std::endl;
			++oItFile;
			++oItImgData;
			++m_oNbImagesLoaded;
            if (oItSampleFile != oItSampleFileEnd)
            {
                ++oItSampleFile;
            }
            ++oItSampleFrames;
			continue;
		} // if

		// now load the actual data
		if (!oBitmap.load( oItImgData->data().begin() ) ) {
			std::cout << std::flush << "\tGraphManager::loadImages: Could not load file: " << *oItFile << std::endl;
		}
		
		if (oItSampleFile != oItSampleFileEnd)
        {
            oItSampleFrames->clear();
            fileio::SampleDataHolder sampleDataHolder;
            fileio::Sample sample( *oItSampleFile );
            if( sample.readAllData(sampleDataHolder))
            {
                for (auto oneSensorData : sampleDataHolder.allData)
                {
                    int sensorID = oneSensorData.sensorID;
                    int nSamples =  oneSensorData.dataVector.size();
                    image::Sample sampleArray(nSamples); //not to be confused with fileio::Sample
                    for (int i = 0; i < nSamples; i++)
                    {
                        sampleArray[i] = oneSensorData.dataVector[i];
                    }
                    (*oItSampleFrames)[sensorID].data() = sampleArray;
                    if (!m_oResultHandler.getResultFolder().empty())
                    {
                        std::ofstream oStream(m_oResultHandler.getResultFolder() + "/"+  std::to_string(sensorID)+"sample.csv",std::ios_base::app);
                        if (!sampleCSVhasHeader[sensorID])
                        {                            
                            oStream << "imageNumber;sensorID;nsamples; value" << "\n";
                            sampleCSVhasHeader[sensorID] = true;
                        }
                        oStream << std::fixed << std::setprecision(4)
                            << m_oNbImagesLoaded << ";"  << sensorID << ";" << nSamples << "; ";
                        for (int i = 0, end = std::min<int>(MAX_ELEMENTS_TO_EXPORT, oneSensorData.dataVector.size()); i < end; i++)
                        {
                            oStream << oneSensorData.dataVector[i] << ";";
                        }
                        oStream << "\n";
                        oStream.close();
                    }
                }
            }
        }
        else
        {
            //we already defined the default sample frames
        }

		++oItFile;
		++oItImgData;
		++m_oNbImagesLoaded;
        if (oItSampleFile != oItSampleFileEnd)
        {
            ++oItSampleFile;
        }
        ++oItSampleFrames;

        //std::cout << std::flush << "\tImage " << m_oNbImagesLoaded << " loaded. "<<std::endl;
	}

    std::cout << std::flush << "\tGraphManager::loadImages: " << m_oNbImagesLoaded << " images loaded - loading complete." << std::endl;

	m_oImagesLoadedSema.set(); // signal block loaded.
} // loadImages



void GraphManager::checkImageLoading(int p_oI)
{
	if ((m_pCanvas == nullptr) || (p_oI <= (int)m_oNbImagesLoaded.load()))  // canvas -> not a benchmark -> load images blockwise
    {
        return;
    } // if

    const auto	oNbImages		=   m_oVectorBmpData.size();
    const auto	oModuloIndex    =   p_oI % oNbImages;

	while (oModuloIndex > m_oNbImagesLoaded.load()) {
		std::cout << "\twaiting for load of next chunk of images...\n";
		Thread::sleep(500/*ms*/);
	} // while
} // checkImageLoading




void GraphManager::drawFrame(int p_oI)
{
	if (m_pCanvas == nullptr) 
    {
        return;
    } // if

    const auto	oNbImages		=   m_oVectorBmpData.size();
    const auto	oIdxImg         =   (p_oI - g_oNbPar) % oNbImages; // index concering image of last joined thread
    const auto	oIdxWorkerCur   =   p_oI % g_oNbPar;
    std::string oShortFilePath = std::to_string(p_oI);
    if (m_oBmpFilePaths.size() > 0)
    {
          auto	oItFilePath     =   std::begin(m_oBmpFilePaths); std::advance(oItFilePath, oIdxImg);
          oShortFilePath  =   oItFilePath->substr(oItFilePath->find_last_of(Path::separator()) + 1, std::string::npos);
    }

	m_pCanvas->drawFrame( m_oVectorBmpData[oIdxImg] );
	m_pCanvas->setTitle(oShortFilePath);

    auto& rCurrCavas    =   m_oCanvasBuffer[oIdxWorkerCur];
    m_pCanvas->swap(rCurrCavas);  // put layer list of curr canvas buffer in actual canvas
	m_pCanvas->draw();
	m_pCanvas->clearShapes();    

#if defined HAVE_QT
    QCoreApplication::instance()->processEvents();
    auto* canvas = dynamic_cast<QtCanvas*>(m_pCanvas.get());

    if (m_pauseAfterEachImage)
    {
        canvas->setPaused();
    }

    while (canvas->isPaused())
    {
        QCoreApplication::instance()->processEvents(QEventLoop::WaitForMoreEvents);
    }
#endif
} // drawFrame


void GraphManager::writeResultsToFolder(std::string folder) 
{
    m_oResultHandler.setResultFolder(folder);
}


void GraphManager::printAllTimings(bool value)
{
    m_oPrintAllTimings = value;
    g_oDebugTimings = true;
}


void GraphManager::redirectLogMessages(bool value)
{
    m_oRedirectLogMessages = value;
}

void GraphManager::setNumImages(int numImages)
{
    m_forceNumImages = true;
    m_numImages = numImages;
}



void GraphManager::setArmAtSequenceRepetition(bool value)
{
    m_armAtSequenceRepetition = value;
}

} // namespace filter
} // namespace precitec


