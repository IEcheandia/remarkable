/**
 * @file
 * @brief This device server is secured and only permits changes when the state is "normal mode" / aka as eOperate.
 *
 * @author 	SB
 * @date	06.09.12
 */

// WM includes
#include "analyzer/securedDeviceServer.h"
#include "module/moduleLogger.h"
#include "common/connectionConfiguration.h"
#include "module/moduleManagerConnector.h"

namespace precitec { using namespace interface;
namespace analyzer {

SecuredDeviceServer::SecuredDeviceServer(std::string p_oName, precitec::system::module::Modules module, const Poco::UUID &id)
    : m_oName(p_oName)
    , m_pProxy(std::unique_ptr<TDevice<MsgProxy>>(new TDevice<MsgProxy>()))
    , m_oLocked(false)
    , m_device(m_pProxy.get())
    , m_id(id)
    , m_module(module)
{
}

SecuredDeviceServer::SecuredDeviceServer(std::string p_oName, TDevice<AbstractInterface> *device, const Poco::UUID &id)
    : m_oName(p_oName)
    , m_pProxy()
    , m_oLocked(false)
    , m_device(device)
    , m_id(id)
{
}

void SecuredDeviceServer::init()
{
}

void SecuredDeviceServer::publish(precitec::framework::module::ModuleManagerConnector *moduleManager)
{
    if (m_module == precitec::system::module::AnyModule)
    {
        return;
    }
    moduleManager->registerPublication(m_pProxy.get(), m_module);
}

int SecuredDeviceServer::initialize( Configuration const& p_rConfig, int p_oSubDevice )
{
	return m_device->initialize( p_rConfig, p_oSubDevice );

} // initialize



void SecuredDeviceServer::uninitialize()
{
	m_device->uninitialize();

} // uninitialize



void SecuredDeviceServer::reinitialize()
{
	m_device->reinitialize();

} // reinitialize



KeyHandle SecuredDeviceServer::set( SmpKeyValue p_pKeyValue, int p_oSubDevice )
{
#if !defined(NDEBUG)
	//wmLog( eDebug, "SecuredDeviceServer::set() KeyValue - %s (%s)\n", m_oName.c_str(), p_pKeyValue->key().c_str());
#endif

	if (isSecure())
    {
        const auto ret = m_device->set( p_pKeyValue, p_oSubDevice );
        notify(p_pKeyValue->key());
        return ret;
    }
	else
	{
		wmLogTr( eError, "Error.Device.SetNotSave", "Cannot change configuration of %s (%s) in this state!\n", m_oName.c_str(), p_pKeyValue->key().c_str() );
		return KeyHandle();
	}

} // set



/*virtual*/ KeyHandle SecuredDeviceServer::force( SmpKeyValue p_pKeyValue, int p_oSubDevice )
{
#if !defined(NDEBUG)
	//wmLog( eDebug, "SecuredDeviceServer::force set() KeyValue - %s (%s)\n", m_oName.c_str(), p_pKeyValue->key().c_str() );
#endif

// Wenn das Senden der HW-Parameter kontrolliert werden soll, folgendes freischalten
#if 0
	if ( p_pKeyValue->type() == TInt )
	{
		int oValue = p_pKeyValue->value<int>();
		wmLog(eDebug, "Now set HW parameter on proxy: %s with value: %d\n", p_pKeyValue->key().c_str(), oValue);;
	}
	if ( p_pKeyValue->type() == TUInt )
	{
		uInt oValue = p_pKeyValue->value<uInt>();
		wmLog(eDebug, "Now set HW parameter on proxy: %s with value: %d\n", p_pKeyValue->key().c_str(), oValue);;
	}
	if ( p_pKeyValue->type() == TFloat )
	{
		float oValue = p_pKeyValue->value<float>();
		wmLog(eDebug, "Now set HW parameter on proxy: %s with value: %f\n", p_pKeyValue->key().c_str(), oValue);;
	}
	if ( p_pKeyValue->type() == TDouble )
	{
		double oValue = p_pKeyValue->value<double>();
		wmLog(eDebug, "Now set HW parameter on proxy: %s with value: %f\n", p_pKeyValue->key().c_str(), oValue);;
	}
	if ( p_pKeyValue->type() == TBool )
	{
		bool oValue = p_pKeyValue->value<bool>();
		wmLog(eDebug, "Now set HW parameter on proxy: %s with value: %d\n", p_pKeyValue->key().c_str(), oValue);;
	}
#endif

    const auto ret = m_device->set( p_pKeyValue, p_oSubDevice );
    notify(p_pKeyValue->key());
    return ret;

} // force set



void SecuredDeviceServer::set( Configuration p_oConfig, int p_oSubDevice )
{
#if !defined(NDEBUG)
	//wmLog( eDebug, "SecuredDeviceServer::set() Config - %s\n", m_oName.c_str() );
#endif

	if (isSecure())
    {
		m_device->set( p_oConfig, p_oSubDevice );
        for (const auto &kv : p_oConfig)
        {
            notify(kv->key());
        }
    }
	else
		wmLogTr( eError, "Error.Device.SetNotSave", "Cannot set configuration of %s in this state!\n", m_oName.c_str() );

} // set



SmpKeyValue SecuredDeviceServer::get( Key p_oKey, int p_oSubDevice )
{
#if !defined(NDEBUG)
	//wmLog( eDebug, "SecuredDeviceServer::get() Key - %s (%s)\n", m_oName.c_str(), p_oKey.c_str() );
#endif

	return m_device->get( p_oKey, p_oSubDevice );

} // get



SmpKeyValue SecuredDeviceServer::get( KeyHandle p_oHandle, int p_oSubDevice )
{
#if !defined(NDEBUG)
	//wmLog( eDebug, "SecuredDeviceServer::get() Handle - %s\n", m_oName.c_str() );
#endif

	return m_device->get( p_oHandle );

} // get



Configuration SecuredDeviceServer::get( int p_oSubDevice )
{
#if !defined(NDEBUG)
	//wmLog( eDebug, "SecuredDeviceServer::get() Config - %s\n", m_oName.c_str() );
#endif

	return m_device->get( p_oSubDevice );

} // get



bool SecuredDeviceServer::isSecure()
{
	return !m_oLocked;

} // isSecure



void SecuredDeviceServer::lock( bool p_oLocked )
{
	m_oLocked = p_oLocked;

} // lock

void SecuredDeviceServer::notify(const std::string &key)
{
    if (!m_deviceNotification)
    {
        return;
    }
    auto kv = m_device->get(key);
    if (kv.isNull()  || kv->key() == "?")
    {
        wmLog(eError, "SecuredDeviceServer Key %s not found \n", key.c_str());
        //wmFatal(eInternalError, "QnxMsg.Fatal.KeyNotFound", "SecuredDeviceServer Key %s not found \n", key.c_str());
        return;
    }
    m_deviceNotification->keyValueChanged(m_id, std::move(kv));
}


} // namespace workflow
} // namespace precitec
