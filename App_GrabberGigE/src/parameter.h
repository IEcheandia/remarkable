#pragma once

// project includes
#include "message/device.h"
// clib includes
#include <map>
#include <cstdint>
#include <mutex>


namespace precitec {
namespace grabber  {

    
class Parameter 
{
    
public:
    
	Parameter();
    
    void clear();
    
    void addParameter( const interface::KeyValue& keyvalue );

   	interface::SmpKeyValue get(interface::Key p_key) const;
    
    interface::Configuration          m_paramConfig;

protected:
    
    interface::TKeyValue<bool>		  m_debug;
    interface::TKeyValue<bool>        m_statusLine;
	interface::TKeyValue<float>       m_exposureTime;
	interface::TKeyValue<int>	      m_windowX;
	interface::TKeyValue<int>	      m_windowY;
	interface::TKeyValue<int>	      m_windowW;
	interface::TKeyValue<int>	      m_windowH;
	interface::TKeyValue<int>	      m_windowWMax;
	interface::TKeyValue<int>	      m_windowHMax;
	interface::TKeyValue<std::string> m_cameraName;
    interface::TKeyValue<bool>        m_reuseLastImage;
};


} // grabber
} // precitec
