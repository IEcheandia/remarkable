/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2013
 * 	@brief		Builds and manages an image processing graph.
 */

#ifndef CALIBRATIONGRAPH_H_
#define CALIBRATIONGRAPH_H_

// project includes
#include <Mod_Calibration.h>
#include <analyzer/graphAssistent.h>
#include <common/graph.h>
#include <fliplib/NullSourceFilter.h>
#include <fliplib/GraphBuilderFactory.h>
#include <calibration/calibrationResultHandler.h>
#include <calibration/calibrationNioHandler.h>
// poco includes
#include <Poco/UUID.h>

namespace precitec {
namespace calibration {

class CalibrationManager;
class CalibrationGraphScanmaster;
/**
 * @ingroup Calibration
 * @brief Builds and manages an image processing graph. The objects of this class are stored and called in the calibration procedures.
 */
class MOD_CALIBRATION_API CalibrationGraph
{
public:

	/**
	 * @brief CTor.
	 * @param p_rCalibrationManager reference to CalibrationManager object.
	 * @param p_oFilename name of XML file with the graph. Graph will be loaded and instantiated.
	 */
	CalibrationGraph( CalibrationManager& p_rCalibrationManager, std::string p_oFilename );

	/**
	 * @brief Was the graph successfully loaded and initialized?
	 * @return true, if the graph is ready to be used.
	 */
	bool isInitialized();

	/**
	 * @brief Execute the graph. This function gets data from a sensor (typically the grabber) and pumps it into the graph.
	 * @param p_oSensorId id/number of the sensor that should be asked for the data - 1 (default) is the camera, the others are typically hardware-related (e.g. 10001, the y-axis encoder positions).
	 * The ids are defined in Interfaces/include/event/sensor.h.
	 * @return Vector with the results. The NIOs can be retrieved by calling getNIOResults.
	 */
	std::vector< interface::ResultArgs* > execute( bool p_oShowImage, std::string p_oTitle, int p_oSensorId =1 );

	/**
	 * @brief Execute the graph with the the data provided (without requesting data to  the sensor).
	 * @param p_oSensorId id/number of the sensor that should be asked for the data - 1 (default) is the camera, the others are typically hardware-related (e.g. 10001, the y-axis encoder positions).
	 * The ids are defined in Interfaces/include/event/sensor.h.
	 * @return Vector with the results. The NIOs can be retrieved by calling getNIOResults.
	 */
	std::vector< interface::ResultArgs* > execute( bool p_oShowSourceImage, std::string p_oTitle, const image::BImage & p_oImage, const std::map<int, image::Sample> & p_oSamples, bool p_oClearCanvas = true);
    
	/**
	 * @brief Was the last run NIO?
	 * @return true, if the last processed image was NIO.
	 */
	bool isNio();


	/**
	 * @brief Get the results. Alternative function, the results are already returned by execute().
	 * @return Vector with the results.
	 */
	std::vector< interface::ResultArgs* > getResults();

	/**
	 * @brief Get the filtergraph. Necessary for setting parameters.
	 * @return Shared pointer of graph.
	 */
	std::shared_ptr<fliplib::FilterGraph> getGraph();

protected:
	/**
	 * @brief Loads XML file and initializes graph. Called from CTor and setGraph()
	 */
	void init();


private:

	CalibrationManager& 									m_rCalibrationManager;		///< Reference to calibration manager.
	std::string												m_oFilename;				///< Name of XML file.
	std::shared_ptr<fliplib::FilterGraph>					m_pGraph;					///< Pointer to the actual graph object.
	Poco::SharedPtr<fliplib::GraphBuilderFactory>			m_pBuilderFactory;			///< Factory for the graph builder objects.
	Poco::SharedPtr<fliplib::AbstractGraphBuilder>			m_pGraphBuilder;			///< Graph builder itself.
	fliplib::SynchronePipe< interface::ImageFrame >*		m_pImagePipe;				///< Input-pipe for incoming images into the graph.
	fliplib::SynchronePipe< interface::SampleFrame >*		m_pSamplePipe;				///< Input-pipe for incoming samples into the graph.
	fliplib::NullSourceFilter 								m_oNullSourceFilter;		///< Dummy filter for the input pipe
	CalibrationResultHandler								m_oResultHandler;			///< The result handler sink-filter.
	bool													m_oInitialized;				///< Was the graph loaded and instantiated correctly?
    bool													m_oHasCamera;				///< is a physical camera present ?
    friend CalibrationGraphScanmaster;
};

} // namespace calibration
} // namespace precitec

#endif /* CALIBRATIONGRAPH_H_ */
