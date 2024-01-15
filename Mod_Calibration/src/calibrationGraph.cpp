/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2013
 * 	@brief		Builds, manages and executes an image processing graph.
 */

// project includes
#include <calibration/calibrationGraph.h>
#include <calibration/calibrationManager.h>
#include <fliplib/XmlGraphBuilder.h>
#include <filter/sensorFilterInterface.h>
#include <filter/armStates.h>
#include "common/systemConfiguration.h"
#include "event/sensor.h"

using namespace Poco;
namespace precitec {
	using namespace interface;
	using namespace image;
namespace calibration {


CalibrationGraph::CalibrationGraph( CalibrationManager& p_rCalibrationManager, std::string p_oFilename ) : m_rCalibrationManager( p_rCalibrationManager ),
		m_oFilename( p_oFilename ),
		m_pImagePipe( nullptr ),
		m_pSamplePipe( nullptr ),
		m_oInitialized( false )
{
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            m_oHasCamera = SystemConfiguration::instance().getBool("HasCamera", true);
        }
        else
        {
            m_oHasCamera = false;
        }
    }
    else
    {
        m_oHasCamera = false;
    }

	init();
} // CTor

void CalibrationGraph::init()
{
	// make filename absolute and let it point to the correct subdir of wm_base_dir ...
	std::string oFinalPath;
	if ( getenv( "WM_BASE_DIR" ) )
		oFinalPath = std::string( getenv( "WM_BASE_DIR" ) );
	else
		oFinalPath = std::string( "/wm_inst" );
	oFinalPath.append( "/calib/" );
	oFinalPath.append( m_oFilename );
	m_oFilename = oFinalPath;

	// create the input pipes
	try
	{
		m_pImagePipe 	= new fliplib::SynchronePipe< ImageFrame > ( &m_oNullSourceFilter, SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE );
		m_pSamplePipe 	= new fliplib::SynchronePipe< SampleFrame >( &m_oNullSourceFilter, SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE );

	} catch (... )
	{
		wmLog( eError, "CalibrationGraph::CTor - exception caught during image pipe creation!\n" );
	}

	// now lets build and initialize the graph
	try
	{
		// first we need initialize the graph builder
		m_pBuilderFactory =	new fliplib::GraphBuilderFactory;
		m_pGraphBuilder	= m_pBuilderFactory->create();

		// now lets build the graph itself
		m_pGraph = m_pGraphBuilder->build( m_oFilename );

		// initialize graph using a visitor
		analyzer::GraphAssistent oAssistent( m_pGraph.get() );

		// register result and nio handler.
		oAssistent.setResultHandler(m_oResultHandler);

		// register calibration manager canvas, so that the filters later paint into the same overlay
		oAssistent.setCanvas( m_rCalibrationManager.getCanvas().get() );
		// init filter parameters.
		oAssistent.setParameter();

		// arm level 1
		oAssistent.init();
		// arm level 2
		oAssistent.arm(filter::eSeamStart);

		// OK, everything worked out, no exception was thrown
		m_oInitialized = true;
	}
	catch(const fliplib::GraphBuilderException &ex)
	{
		wmLog( eError, "CalibrationGraph: (fliplib::GraphBuilderException): %s\n", ex.message().c_str() );
	}
	catch(const NotImplementedException &ex)
	{
		wmLog( eError, "CalibrationGraph: (system::NotImplementedException): %s\n", ex.what() );
	}
	catch(const fliplib::DataException &ex)
	{
		wmLog( eError, "CalibrationGraph: (fliplib::DataException): %s\n", ex.message().c_str() );
	}
	catch(const fliplib::LogicException &ex)
	{
		wmLog( eError, "CalibrationGraph: (fliplib::logic exception): %s\n", ex.message().c_str() );
	}
	catch(const Poco::Exception &ex)
	{
		wmLog( eError, "CalibrationGraph: (poco::exception): %s - %s\n", ex.what(), ex.message().c_str() );
		if (ex.nested() != NULL)
		{
			wmLog( eError, " Nested exception: %s - %s\n", ex.what(), ex.nested()->displayText().c_str() );
		}
	}
	catch(const std::exception &ex)
	{
		wmLog( eError, "CalibrationGraph: (std::exception): %s\n", ex.what() );
	}
	catch(...)
	{
		wmLog( eError, "CalibrationGraph: unkown exception!\n" );
	}
}


bool CalibrationGraph::isInitialized()
{
	return m_oInitialized;

} // isInitialized

std::vector< interface::ResultArgs* > CalibrationGraph::execute(bool p_oShowImage, std::string p_oTitle, int p_oSensorId )
{
	if ( !m_oInitialized )
	{
		wmLog( eError, "CalibrationGraph::execute - Graph was not loaded or initialized correctly, cannot be executed!\n" );

	}
	else
	{
		try
		{
			// are we supposed to stuff an image into the graph or a sample?
			if ( p_oSensorId <= interface::eImageSensorMax )
			{
				BImage oImage;
				// get a new image from the camera
				try
				{
					oImage = m_rCalibrationManager.getImage();
				}
				catch (std::exception &p_rException)
				{
					wmLogTr(eError, "QnxMsg.Calib.BadCam", "Cannot access camera %d hardware parameter for calibration data! Exc.: %s", p_oSensorId, p_rException.what());
					return m_oResultHandler.getResults();
				}

				if (p_oShowImage)
				{
					int oX(0), oY(0), oW(0), oH(0);
					if ( m_oHasCamera && !m_rCalibrationManager.getHWRoi(oX, oY, oW, oH, p_oSensorId) )
					{
						wmLog(eError, "QnxMsg.Calib.BadCam", "Cannot access camera %d hardware parameter for calibration data! Exc.: %s", p_oSensorId, "HWRoi unavailable.");
					} // continue writing text at position 0, 0
					m_rCalibrationManager.clearCanvas();
					m_rCalibrationManager.drawText(oX, oY, p_oTitle, Color::Yellow());
					m_rCalibrationManager.renderImage(oImage);
				}

				analyzer::GraphAssistent oAssistent( m_pGraph.get() );

				// clear the results from previous runs
				m_oResultHandler.clear();
				// set the canvas in all filters
				m_rCalibrationManager.clearCanvas();

				// create a complete image frame for the retrieved image
				ImageContext oImageContext;
				oImageContext.setImageNumber( 0 );
				TaskContext oTaskContext;
				oImageContext.setTaskContext(oTaskContext);
				ImageFrame oFrame( oImageContext, oImage );

				int oImgSourceFilterSensorId = 1; //from the graph?
				analyzer::PipeScope<ImageFrame> oPipeScope(m_pGraph.get(), oImgSourceFilterSensorId, m_pImagePipe);
				// register calibration manager canvas
				oAssistent.setCanvas( m_rCalibrationManager.getCanvas().get() );

				// now stuff the image frame into the graph
				m_pImagePipe->signal( oFrame );

				// render the visual output of the filter graph
				oAssistent.paint( );
				// now send the image together with the overlay to the windows side ...
				m_rCalibrationManager.renderImage(oImage);
			}
			else
			{
				Sample oSample = m_rCalibrationManager.getSample( p_oSensorId );

				analyzer::GraphAssistent oAssistent( m_pGraph.get() );

				// clear the results from previous runs
				m_oResultHandler.clear();

				// create a complete image frame for the retrieved image
				ImageContext oImageContext;
				oImageContext.setImageNumber( 0 );
				TaskContext oTaskContext;
				oImageContext.setTaskContext(oTaskContext);
				SampleFrame oFrame( oImageContext, oSample );

				// connect the input pipe to the source filters
				int oPipeScopeSensorId = p_oSensorId; 
				analyzer::PipeScope<SampleFrame> oPipeScope(m_pGraph.get(), oPipeScopeSensorId, m_pSamplePipe);

				// now stuff the image frame into the graph
				m_pSamplePipe->signal( oFrame );

			} // if p_oSensorId
		} // try
		catch (Exception &e)
		{
			wmLog( eError, "CalibrationGraph:: Exception during graph execution!\n");
			std::cout << "ERROR : " << e.what() << std::endl;
		}
	} // if isInitialized

	// get the results
	return m_oResultHandler.getResults();

} // execute

std::vector< interface::ResultArgs* > CalibrationGraph::execute( bool p_oShowSourceImage, std::string p_oTitle, const BImage & p_oImage, const std::map<int,Sample> & p_oSamples, bool p_oClearCanvas)
{
    // clear the results from previous runs
    m_oResultHandler.clear();    
    if (p_oClearCanvas)
    {
        m_rCalibrationManager.clearCanvas();
    }
    
    if ( !m_oInitialized || m_pGraph == nullptr )
    {
        wmLog( eError, "CalibrationGraph::execute - Graph was not loaded or initialized correctly, cannot be executed!\n" );
        return {};
    }
    
    if (p_oShowSourceImage)
    {                
        m_rCalibrationManager.drawText(10, 10, p_oTitle, Color::Yellow());
        if (p_oImage.isValid())
        {
            m_rCalibrationManager.renderImage(p_oImage);
        }
        else
        {
            image::BImage dummyImage(image::Size2d(512, 512));
            m_rCalibrationManager.renderImage(dummyImage);
        }
    }

    // set the canvas in all filters
    analyzer::GraphAssistent oAssistent( m_pGraph.get() );    
    oAssistent.setCanvas( m_rCalibrationManager.getCanvas().get() );
    

    ImageContext oImageContext;
    oImageContext.setImageNumber( 0 );
    TaskContext oTaskContext;
    oImageContext.setTaskContext(oTaskContext);
    
    if ( p_oImage.isValid())
    {
        ImageFrame oFrame( oImageContext, p_oImage );

        int oImgSourceFilterSensorId = 1; //ImageSourceFilter uses sensorID = 1 
        analyzer::PipeScope<ImageFrame> oPipeScope(m_pGraph.get(), oImgSourceFilterSensorId, m_pImagePipe);
        m_pImagePipe->signal( oFrame );
    }
    
    for (auto & rSampleEntry : p_oSamples)
    {
        auto oSensorId = rSampleEntry.first;
        auto & rSample = rSampleEntry.second;

        SampleFrame oFrame( oImageContext, rSample );

        analyzer::PipeScope<SampleFrame> oPipeScope(m_pGraph.get(), oSensorId, m_pSamplePipe);
        m_pSamplePipe->signal( oFrame );
    } 

    // render the visual output of the filter graph
    oAssistent.paint( );
    // now send the image together with the overlay to the windows side ...
    m_rCalibrationManager.renderImage(p_oImage);

    // get the results
    return m_oResultHandler.getResults();

} // execute

std::shared_ptr<fliplib::FilterGraph> CalibrationGraph::getGraph()
{
	return m_pGraph;
}

bool CalibrationGraph::isNio()
{
	bool oNio(false);
	for (auto &oResult : m_oResultHandler.getResults())
	{
		oNio = oResult->isNio();
		if (oNio)
		{
			break;
		}
	}
	return oNio;
} // isNIO


std::vector< interface::ResultArgs* > CalibrationGraph::getResults()
{
	return m_oResultHandler.getResults();

} // getResults


} // namespace calibration
} // namespace precitec
