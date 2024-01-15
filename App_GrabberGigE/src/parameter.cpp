// project includes
#include "parameter.h"

namespace precitec {
namespace grabber  {

    
Parameter::Parameter()
        // 					  key					value		min				max				default		
		: m_debug	        ( "Debug",		        false,		false,			true,			false		)
        , m_statusLine      ( "StatusLine",         false,      false,          true,           false       )
        , m_exposureTime    ( "ExposureTime",       1.0f,       0.01f,          1000.0f,        1.0f        )
        , m_windowX         ( "Window.X",           0,          0,              2000.0,         0.0         )
        , m_windowY         ( "Window.Y",           0,          0,              2000.0,         0.0         )
        , m_windowW         ( "Window.W",           320,        10,             2000.0,         320.0       )
        , m_windowH         ( "Window.H",           200,        10,             2000.0,         200.0       )
        , m_windowWMax      ( "Window.WMax",        1024,       10,             2000.0,         1024.0      )
        , m_windowHMax      ( "Window.HMax",        1024,       10,             2000.0,         1024.0      )
        , m_cameraName      ( "CameraName",         "WeldMaster", "WeldMaster", "WeldMaster",   "WeldMaster")
        , m_reuseLastImage  ( "ReuseLastImage",     false,      false,          true,           false       )
{
    // set the read-only flags
    m_windowWMax.setReadOnly( true );
    m_windowHMax.setReadOnly( true );
    m_cameraName.setReadOnly( true );
    
    clear();
}



void Parameter::clear()
{
    // clear the array and the map
    m_paramConfig.clear();

    // create configuration container
    addParameter( m_debug );
    addParameter( m_statusLine );
    addParameter( m_exposureTime );
    addParameter( m_windowX );
    addParameter( m_windowY );
    addParameter( m_windowW );
    addParameter( m_windowH );
    addParameter( m_windowWMax );
    addParameter( m_windowHMax );
    addParameter( m_cameraName );    
    addParameter( m_reuseLastImage );
}



void Parameter::addParameter( const interface::KeyValue& p_keyvalue)
{
   	const auto &rKey( p_keyvalue.key() );
    
    for ( auto iter : m_paramConfig )
    {
        if ( iter->key() == rKey )
        {
            return;
        }
    }        
    
    auto keyvalue = p_keyvalue.clone();    
    m_paramConfig.push_back( keyvalue );
}



interface::SmpKeyValue Parameter::get(interface::Key p_key) const
{
    for ( auto iter : m_paramConfig )
    {
        if ( iter->key() == p_key )
        {
            return iter;
        }
    }            
    
    wmLog(eDebug, "Camera::get(): Unknown key-value %s\n", p_key.c_str() );
	return interface::SmpKeyValue(new interface::KeyValue(TInt,"?",-1) );
}


} // namespache grabber
} // namespace precitec
