/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2011
 * 	@brief		Receives the sensor data - images, encoder positions.
 */

#ifndef CALIBRATIONSENSORSERVER_H_
#define CALIBRATIONSENSORSERVER_H_

// project includes
#include <Mod_Calibration.h>
#include <event/sensor.h>
#include <event/sensor.interface.h>
#include <event/imageShMem.h>
#include <calibration/calibrationManager.h>

namespace precitec {
namespace calibration {

/**
 * @ingroup Calibration
 * @brief Server class, receiving the sensor data packages (images, encoder positions).
 * @details This is the server of the sensor interface in the calibration module. It receives the data from the sensors, e.g. images, analog values, etc.
 */
class CalibrationSensorServer : public TSensor<AbstractInterface>
{
public:

	/**
	 * @brief CTor.
	 * @param p_rCalibrationManager Reference to calibration manager that is called whenever a sensor data element is received.
	 */
	CalibrationSensorServer(CalibrationManager& p_rCalibrationManager) :
		m_rCalibrationManager( p_rCalibrationManager )
	{
	};
	/**
	 * @brief DTor.
	 */
	virtual ~CalibrationSensorServer() {};

public:
	/**
	 * @brief The function is called if the server receives a single image from a sensor. The function calls the inspect function in the calibration manager.
	 * @param p_oSensorId Integer with the ID of the sensor that produced the image.
	 * @param p_rContext TriggerContext object.
	 * @param p_rData BImage object.
	 */
	void data(int p_oSensorId, TriggerContext const& p_rContext, BImage const& p_rData)
	{
		m_rCalibrationManager.image( p_oSensorId, p_rContext, p_rData);
	};

	/**
	 * @brief The function is called if the server receives a single data sample from a sensor. The function calls the inspect function in the calibration manager.
	 * @param p_oSensorId integer with the ID of the sensor that produced the image.
	 * @param p_rContext TriggerContext object.
	 * @param p_rData the Sample object.
	 */
	void data( int p_oSensorId, TriggerContext const& p_rContext, Sample const& p_rData )
	{
		m_rCalibrationManager.sample( p_oSensorId, p_rContext, p_rData );
	};

    void simulationDataMissing(UNUSED TriggerContext const& p_rContext) override
    {
    }

    void initSharedMemory(bool simulation)
    {
        imageShMem_ = std::make_unique<SharedMem>(precitec::greyImage::sharedMemoryHandle(), precitec::greyImage::sharedMemoryName(), precitec::system::SharedMem::StdServer, greyImage::sharedMemorySize(simulation));
    }

private:

	CalibrationManager& m_rCalibrationManager; ///< Reference to calibration manager that is called whenever a sensor data element is received.
	std::unique_ptr<SharedMem> imageShMem_;

};

}	// analyzer
}	// precitec

#endif /* CALIBRATIONSENSORSERVER_H_ */
