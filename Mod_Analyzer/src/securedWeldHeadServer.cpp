/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		A secured device server for the weld-head process. This class was introduced because of the switch for the line laser. The line laser is controlled either by the weldhead or by the grabber / camera process.
 */

// WM includes
#include "module/moduleLogger.h"
#include <analyzer/securedWeldHeadServer.h>

#include "common/systemConfiguration.h"

namespace precitec { using namespace interface;
namespace analyzer {


SecuredWeldHeadServer::SecuredWeldHeadServer( const std::shared_ptr< SecuredDeviceHandler > &p_pGrabberHandler, precitec::system::module::Modules module, const Poco::UUID &id ) :
		SecuredDeviceServer( "WeldHeadControl", module, id ),
		m_pGrabberHandler( p_pGrabberHandler ),
		m_oLineLaser1ViaCamera( false ),
		m_oLineLaser2ViaCamera( false ),
		m_oFieldLight1ViaCamera( false ),
		m_oLineLaserCameraCommandSet( 0 )
{
} // CTor

void SecuredWeldHeadServer::init()
{
    usleep(10 * 1000);

	m_oLineLaserCameraCommandSet = SystemConfiguration::instance().getInt("LineLaser_CameraCommandSet", 0);
	wmLog(eDebug, "m_oLineLaserCameraCommandSet (int): %d\n", m_oLineLaserCameraCommandSet);
    
	// get the configuration of the switch from the weldhead device server
	SmpKeyValue oLineLaser1Via = proxy()->get( "LineLaser1ViaCamera", 0 );
	if ( !oLineLaser1Via.isNull() )
		m_oLineLaser1ViaCamera = oLineLaser1Via->value<bool>();
	SmpKeyValue oLineLaser2Via = proxy()->get( "LineLaser2ViaCamera", 0 );
	if ( !oLineLaser2Via.isNull() )
		m_oLineLaser2ViaCamera = oLineLaser2Via->value<bool>();
	SmpKeyValue oFieldLight1Via = proxy()->get( "FieldLight1ViaCamera", 0 );
	if ( !oFieldLight1Via.isNull() )
		m_oFieldLight1ViaCamera = oFieldLight1Via->value<bool>();

	// init line-lasers connected to camera. intensity is set to 0 for both, pwm is always enabled.
	SecuredDeviceServer* pServer = m_pGrabberHandler->getServer().get();
	if ( m_oLineLaser1ViaCamera || m_oLineLaser2ViaCamera || m_oFieldLight1ViaCamera)
	{
        wmLog( eDebug, "LG via Camera: Initialization of PWM.Enable to 1.\n");
        if (m_oLineLaserCameraCommandSet == 0)
        {
            SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.Enable", 1, 0, 1, 1 ) );
            pServer->set( pNewKeyValue, 0 );
        }
        else
        {
            // "new Precitec camera" MV4_D1280_L01_C132_FB
            SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "EnLaserTrigger", false, false, true, false ) );
            pServer->set( pNewKeyValue, 0 );
            pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "EnLaserPower", true, false, true, true ) );
            pServer->set( pNewKeyValue, 0 );
        }
	}
	if ( m_oLineLaser1ViaCamera )
	{
		SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM1", 0, 0, 255, 128 ) );
		pServer->set( pNewKeyValue, 0 );
		m_oLineLaser1Intensity = 0;
		m_oLineLaser1Enabled = false;

		wmLog( eDebug, "SecuredWeldHeadServer: Line-laser 1 is controlled by the grabber process / camera.\n");
	}
	else
		wmLog( eDebug, "SecuredWeldHeadServer: Line-laser 1 is controlled by the weldhead process / beckhoff module.\n");

	if ( m_oLineLaser2ViaCamera )
	{
        if (m_oLineLaserCameraCommandSet == 0)
        {
            SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM2", 0, 0, 255, 128 ) );
            pServer->set( pNewKeyValue, 0 );
        }
        else
        {
            // "new Precitec camera" MV4_D1280_L01_C132_FB
            SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "EnLaserTrigger", false, false, true, false ) );
            pServer->set( pNewKeyValue, 0 );
        }
		m_oLineLaser2Intensity = 0;
		m_oLineLaser2Enabled = false;

		wmLog( eDebug, "SecuredWeldHeadServer: Line-laser 2 is controlled by the grabber process / camera.\n");
	}
	else
		wmLog( eDebug, "SecuredWeldHeadServer: Line-laser 2 is controlled by the weldhead process / beckhoff module.\n");

	if ( m_oFieldLight1ViaCamera )
	{
		SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWMBel", 0, 0, 255, 128 ) );
		pServer->set( pNewKeyValue, 0 );
		m_oFieldLight1Intensity = 0;
		m_oFieldLight1Enabled = false;

		wmLog( eDebug, "SecuredWeldHeadServer: field-light 1 is controlled by the grabber process / camera.\n");
	}
	else
		wmLog( eDebug, "SecuredWeldHeadServer: field-light 1 is controlled by the weldhead process / beckhoff module.\n");
}

KeyHandle SecuredWeldHeadServer::set( SmpKeyValue p_pKeyValue, int p_oSubDevice )
{
	// is the server locked?
	if (!isSecure())
	{
		wmLogTr( eError, "Error.Device.SetNotSave", "Cannot change configuration of %s (%s) in this state!\n", name().c_str(), p_pKeyValue->key().c_str() );
		return KeyHandle();
	}

	// get server
	SecuredDeviceServer* pServer = m_pGrabberHandler->getServer().get();

	// is this a key that we have to route to the grabber?
	if ( pServer && m_oLineLaser1ViaCamera && p_pKeyValue->key() == "LineLaser1Intensity" )
	{
		// remember current intensity
		m_oLineLaser1Intensity = p_pKeyValue->value<int>();
		// create new key value
		SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM1", (int)( (float)(p_pKeyValue->value<int>() ) * 2.55f ), 0, 255, 128 ) );
		// send to grabber
		pServer->set( pNewKeyValue, p_oSubDevice );
	}
	if ( pServer && m_oLineLaser2ViaCamera && p_pKeyValue->key() == "LineLaser2Intensity" )
	{
		// remember current intensity
		m_oLineLaser2Intensity = p_pKeyValue->value<int>();
        if (m_oLineLaserCameraCommandSet == 0)
        {
            // create new key value
            SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM2", (int)( (float)(p_pKeyValue->value<int>() ) * 2.55f ), 0, 255, 128 ) );
            // send to grabber
            pServer->set( pNewKeyValue, p_oSubDevice );
        }
        else
        {
            // "new Precitec camera" MV4_D1280_L01_C132_FB
            // create new key value
            SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<float>( "LaserIntensity", (float)(p_pKeyValue->value<int>() ), 0.0, 100.0, 50.0 ) );
            // send to grabber
            pServer->set( pNewKeyValue, p_oSubDevice );
        }
	}
	if ( pServer && m_oFieldLight1ViaCamera && p_pKeyValue->key() == "FieldLight1Intensity" )
	{
		// remember current intensity
		m_oFieldLight1Intensity = p_pKeyValue->value<int>();
		// create new key value
		SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWMBel", (int)( (float)(p_pKeyValue->value<int>() ) * 2.55f ), 0, 255, 128 ) );
		// send to grabber
		pServer->set( pNewKeyValue, p_oSubDevice );
	}
	if ( pServer && m_oLineLaser1ViaCamera && p_pKeyValue->key() == "LineLaser1OnOff" )
	{
		if ( !p_pKeyValue->value<bool>() )
		{
			m_oLineLaser1Enabled = false;
			SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM1", 0, 0, 1, 1 ) );
			pServer->set( pNewKeyValue, p_oSubDevice );
		}
		else
		{
			m_oLineLaser1Enabled = true;
			SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM1", int(m_oLineLaser1Intensity * 2.55f), 0, 1, 1 ) );
			pServer->set( pNewKeyValue, p_oSubDevice );
		}
	}
	if ( pServer && m_oLineLaser2ViaCamera && p_pKeyValue->key() == "LineLaser2OnOff" )
	{
		if ( !p_pKeyValue->value<bool>() )
		{
			m_oLineLaser2Enabled = false;
            if (m_oLineLaserCameraCommandSet == 0)
            {
                SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM2", 0, 0, 1, 1 ) );
                pServer->set( pNewKeyValue, p_oSubDevice );
            }
            else
            {
                // "new Precitec camera" MV4_D1280_L01_C132_FB
                SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "EnLaserTrigger", false, false, true, false ) );
                pServer->set( pNewKeyValue, p_oSubDevice );
            }
		}
		else
		{
			m_oLineLaser2Enabled = true;
            if (m_oLineLaserCameraCommandSet == 0)
            {
                SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>("PWM.PWM2", int(m_oLineLaser2Intensity * 2.55f), 0, 1, 1));
                pServer->set( pNewKeyValue, p_oSubDevice );
            }
            else
            {
                // bei "Neue Precitec Kamera" EnLaserTrigger auf true setzen ?
                SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "EnLaserTrigger", true, false, true, false ) );
                pServer->set( pNewKeyValue, p_oSubDevice );
            }
		}
	}
	if ( pServer && m_oFieldLight1ViaCamera && p_pKeyValue->key() == "FieldLight1OnOff" )
	{
		if ( !p_pKeyValue->value<bool>() )
		{
			m_oFieldLight1Enabled = false;
			SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWMBel", 0, 0, 1, 1 ) );
			pServer->set( pNewKeyValue, p_oSubDevice );
		}
		else
		{
			m_oFieldLight1Enabled = true;
			SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>("PWM.PWMBel", int(m_oFieldLight1Intensity * 2.55f), 0, 1, 1));
			pServer->set( pNewKeyValue, p_oSubDevice );
		}
	}

	return SecuredDeviceServer::set( p_pKeyValue, p_oSubDevice );

} // set



/*virtual*/ KeyHandle SecuredWeldHeadServer::force( SmpKeyValue p_pKeyValue, int p_oSubDevice )
{
	// get server
	SecuredDeviceServer* pServer = m_pGrabberHandler->getServer().get();

	// is this a key that we have to route to the grabber?
	if ( pServer && m_oLineLaser1ViaCamera && p_pKeyValue->key() == "LineLaser1Intensity" )
	{
		// remember current intensity
		m_oLineLaser1Intensity = p_pKeyValue->value<int>();
		// create new key value
		SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM1", (int)( (float)(p_pKeyValue->value<int>() ) * 2.55f ), 0, 255, 128 ) );
		// send to grabber
		pServer->force( pNewKeyValue, p_oSubDevice );
	}
	if ( pServer && m_oLineLaser2ViaCamera && p_pKeyValue->key() == "LineLaser2Intensity" )
	{
		// remember current intensity
		m_oLineLaser2Intensity = p_pKeyValue->value<int>();
        if (m_oLineLaserCameraCommandSet == 0)
        {
            // create new key value
            SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM2", (int)( (float)(p_pKeyValue->value<int>() ) * 2.55f ), 0, 255, 128 ) );
            // send to grabber
            pServer->force( pNewKeyValue, p_oSubDevice );
        }
        else
        {
            // "new Precitec camera" MV4_D1280_L01_C132_FB
            // create new key value
            SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<float>( "LaserIntensity", (float)(p_pKeyValue->value<int>() ), 0.0, 100.0, 50.0 ) );
            // send to grabber
            pServer->force( pNewKeyValue, p_oSubDevice );
        }
	}
	if ( pServer && m_oFieldLight1ViaCamera && p_pKeyValue->key() == "FieldLight1Intensity" )
	{
		// remember current intensity
		m_oFieldLight1Intensity = p_pKeyValue->value<int>();
		// create new key value
		SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWMBel", (int)( (float)(p_pKeyValue->value<int>() ) * 2.55f ), 0, 255, 128 ) );
		// send to grabber
		pServer->force( pNewKeyValue, p_oSubDevice );
	}
	if ( pServer && m_oLineLaser1ViaCamera && p_pKeyValue->key() == "LineLaser1OnOff" )
	{
		if ( !p_pKeyValue->value<bool>() )
		{
			m_oLineLaser1Enabled = false;
			SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM1", 0, 0, 1, 1 ) );
			pServer->force( pNewKeyValue, p_oSubDevice );
		}
		else
		{
			m_oLineLaser1Enabled = true;
			SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>("PWM.PWM1", int(m_oLineLaser1Intensity * 2.55f), 0, 1, 1));
			pServer->force( pNewKeyValue, p_oSubDevice );
		}
	}
	if ( pServer && m_oLineLaser2ViaCamera && p_pKeyValue->key() == "LineLaser2OnOff" )
	{
		if ( !p_pKeyValue->value<bool>() )
		{
			m_oLineLaser2Enabled = false;
            if (m_oLineLaserCameraCommandSet == 0)
            {
                SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWM2", 0, 0, 1, 1 ) );
                pServer->force( pNewKeyValue, p_oSubDevice );
            }
            else
            {
                // "new Precitec camera" MV4_D1280_L01_C132_FB
                SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "EnLaserTrigger", false, false, true, false ) );
                pServer->force( pNewKeyValue, p_oSubDevice );
            }
		}
		else
		{
            m_oLineLaser2Enabled = true;
            if (m_oLineLaserCameraCommandSet == 0)
            {
                SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>("PWM.PWM2", int(m_oLineLaser2Intensity * 2.55f), 0, 1, 1));
                pServer->force( pNewKeyValue, p_oSubDevice );
            }
            else
            {
                // "new Precitec camera" MV4_D1280_L01_C132_FB
                SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "EnLaserTrigger", true, false, true, false ) );
                pServer->force( pNewKeyValue, p_oSubDevice );
            }
		}
	}
	if ( pServer && m_oFieldLight1ViaCamera && p_pKeyValue->key() == "FieldLight1OnOff" )
	{
		if ( !p_pKeyValue->value<bool>() )
		{
			m_oFieldLight1Enabled = false;
			SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>( "PWM.PWMBel", 0, 0, 1, 1 ) );
			pServer->force( pNewKeyValue, p_oSubDevice );
		}
		else
		{
			m_oFieldLight1Enabled = true;
			SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<int>("PWM.PWMBel", int(m_oFieldLight1Intensity * 2.55f), 0, 1, 1));
			pServer->force( pNewKeyValue, p_oSubDevice );
		}
	}

	return SecuredDeviceServer::force( p_pKeyValue, p_oSubDevice );

} // force



void SecuredWeldHeadServer::set( Configuration p_oConfig, int p_oSubDevice )
{
	// is the server locked?
	if (!isSecure())
	{
		wmLogTr( eError, "Error.Device.SetNotSave", "Cannot change configuration in this state!\n" );
		return;
	}

	// TODO: switch is not active here - but the function is also not used yet ...
	SecuredDeviceServer::set( p_oConfig, p_oSubDevice );

} // set



SmpKeyValue SecuredWeldHeadServer::get( Key p_oKey, int p_oSubDevice )
{
	// get server
	SecuredDeviceServer* pServer = m_pGrabberHandler->getServer().get();

	// is this a key that we have to route to the grabber?
	if ( pServer && m_oLineLaser1ViaCamera && p_oKey == "LineLaser1Intensity" )
	{
		// get key value from server
		SmpKeyValue pNewKeyValue = pServer->get( "PWM.PWM1", p_oSubDevice );
		if (!pNewKeyValue.isNull())
		{
			// create new key value
			SmpKeyValue pKeyValue = SmpKeyValue(new TKeyValue<int>( "LineLaser1Intensity", (int)( (float)(pNewKeyValue->value<int>() ) / 2.55f ), 0, 100, 50 ) );
			// send back
			return pKeyValue;
		}
	}
	if ( pServer && m_oLineLaser2ViaCamera && p_oKey == "LineLaser2Intensity" )
	{
		// get key value from server
        if (m_oLineLaserCameraCommandSet == 0)
        {
            SmpKeyValue pNewKeyValue = pServer->get( "PWM.PWM2", p_oSubDevice );
            if (!pNewKeyValue.isNull())
            {
                // create new key value
                SmpKeyValue pKeyValue = SmpKeyValue(new TKeyValue<int>( "LineLaser2Intensity", (int)( (float)(pNewKeyValue->value<int>() ) / 2.55f ), 0, 100, 50 ) );
                // send back
                return pKeyValue;
            }
        }
        else
        {
            // "new Precitec camera" MV4_D1280_L01_C132_FB
            SmpKeyValue pNewKeyValue = pServer->get( "LaserIntensity", p_oSubDevice );
            if (!pNewKeyValue.isNull())
            {
                // create new key value
                SmpKeyValue pKeyValue = SmpKeyValue(new TKeyValue<int>( "LineLaser2Intensity", (int)(pNewKeyValue->value<float>()), 0, 100, 50 ) );
                // send back
                return pKeyValue;
            }
        }
	}
	if ( pServer && m_oFieldLight1ViaCamera && p_oKey == "FieldLight1Intensity" )
	{
		// get key value from server
		SmpKeyValue pNewKeyValue = pServer->get( "PWM.PWMBel", p_oSubDevice );
		if (!pNewKeyValue.isNull())
		{
			// create new key value
			SmpKeyValue pKeyValue = SmpKeyValue(new TKeyValue<int>( "FieldLight1Intensity", (int)( (float)(pNewKeyValue->value<int>() ) / 2.55f ), 0, 100, 50 ) );
			// send back
			return pKeyValue;
		}
	}
	if ( pServer && m_oLineLaser1ViaCamera && p_oKey == "LineLaser1OnOff" )
	{
		// convert to bool
		SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "LineLaser1OnOff", m_oLineLaser1Enabled ) );
		// send back
		return pNewKeyValue;
	}
	if ( pServer && m_oLineLaser2ViaCamera && p_oKey == "LineLaser2OnOff" )
	{
		// convert to bool
		SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "LineLaser2OnOff", m_oLineLaser2Enabled ) );
		// send back
		return pNewKeyValue;
	}
	if ( pServer && m_oFieldLight1ViaCamera && p_oKey == "FieldLight1OnOff" )
	{
		// convert to bool
		SmpKeyValue pNewKeyValue = SmpKeyValue(new TKeyValue<bool>( "FieldLight1OnOff", m_oFieldLight1Enabled ) );
		// send back
		return pNewKeyValue;
	}

	return SecuredDeviceServer::get( p_oKey, p_oSubDevice );

} // get



SmpKeyValue SecuredWeldHeadServer::get( KeyHandle p_oHandle, int p_oSubDevice )
{
	// TODO: We have to remove the key-handle functions - the concept is not used anywhere, not implemented by the servers anyway.
	return SecuredDeviceServer::get( p_oHandle, p_oSubDevice );

} // get



Configuration SecuredWeldHeadServer::get( int p_oSubDevice )
{
	if ( !m_oLineLaser1ViaCamera && !m_oLineLaser2ViaCamera && !m_oFieldLight1ViaCamera)
		return SecuredDeviceServer::get( p_oSubDevice );

	// get server
	SecuredDeviceServer* pServer = m_pGrabberHandler->getServer().get();

	// get the configuration array from the weldhead process
	Configuration oConfiguration = SecuredDeviceServer::get( p_oSubDevice );

	// now scan for the line laser keys
	for( auto oIter = oConfiguration.begin(); oIter != oConfiguration.end(); ++oIter )
	{
		// is this a key that we have to route to the grabber?
		if ( pServer && m_oLineLaser1ViaCamera && (*oIter)->key() == "LineLaser1Intensity" )
		{
			// get key value from server
			SmpKeyValue pNewKeyValue = pServer->get( "PWM.PWM1", p_oSubDevice );
			if (!pNewKeyValue.isNull())
			{
				// create new key value
				SmpKeyValue pKeyValue = SmpKeyValue(new TKeyValue<int>( "LineLaser1Intensity", (int)( (float)(pNewKeyValue->value<int>() ) / 2.55f ), 0, 100, 50 ) );
				// copy back
				(*oIter) = pKeyValue;
			}
		}
		if ( pServer && m_oLineLaser2ViaCamera && (*oIter)->key() == "LineLaser2Intensity" )
		{
			// get key value from server
            if (m_oLineLaserCameraCommandSet == 0)
            {
                SmpKeyValue pNewKeyValue = pServer->get( "PWM.PWM2", p_oSubDevice );
                if (!pNewKeyValue.isNull())
                {
                    // create new key value
                    SmpKeyValue pKeyValue = SmpKeyValue(new TKeyValue<int>( "LineLaser2Intensity", (int)( (float)(pNewKeyValue->value<int>() ) / 2.55f ), 0, 100, 50 ) );
                    // copy back
                    (*oIter) = pKeyValue;
                }
            }
            else
            {
                // "new Precitec camera" MV4_D1280_L01_C132_FB
                SmpKeyValue pNewKeyValue = pServer->get( "LaserIntensity", p_oSubDevice );
                if (!pNewKeyValue.isNull())
                {
                    // create new key value
                    SmpKeyValue pKeyValue = SmpKeyValue(new TKeyValue<int>( "LineLaser2Intensity", (int)( pNewKeyValue->value<float>() ), 0, 100, 50 ) );
                    // copy back
                    (*oIter) = pKeyValue;
                }
            }
		}
		if ( pServer && m_oFieldLight1ViaCamera && (*oIter)->key() == "FieldLight1Intensity" )
		{
			// get key value from server
			SmpKeyValue pNewKeyValue = pServer->get( "PWM.PWMBel", p_oSubDevice );
			if (!pNewKeyValue.isNull())
			{
				// create new key value
				SmpKeyValue pKeyValue = SmpKeyValue(new TKeyValue<int>( "FieldLight1Intensity", (int)( (float)(pNewKeyValue->value<int>() ) / 2.55f ), 0, 100, 50 ) );
				// copy back
				(*oIter) = pKeyValue;
			}
		}
		if ( pServer && m_oLineLaser1ViaCamera && (*oIter)->key() == "LineLaser1OnOff" )
		{
			(*oIter) = new TKeyValue<bool>( "LineLaser1OnOff", m_oLineLaser1Enabled );
		}
		if ( pServer && m_oLineLaser2ViaCamera && (*oIter)->key() == "LineLaser2OnOff" )
		{
			(*oIter) = new TKeyValue<bool>( "LineLaser2OnOff", m_oLineLaser2Enabled );
		}
		if ( pServer && m_oFieldLight1ViaCamera && (*oIter)->key() == "FieldLight1OnOff" )
		{
			(*oIter) = new TKeyValue<bool>( "FieldLight1OnOff", m_oFieldLight1Enabled );
		}
	}

	return oConfiguration;

} // get


} // namespace workflow
} // namespace precitec
