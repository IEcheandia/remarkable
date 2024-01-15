#include "systemConfigurationQml.h"

namespace precitec
{
namespace gui
{

SystemConfigurationQml::SystemConfigurationQml(QObject* parent)
    : QObject(parent)
{
}

SystemConfigurationQml::~SystemConfigurationQml() = default;


SystemConfigurationQml* SystemConfigurationQml::instance()
{
    static SystemConfigurationQml s_instance;
    return &s_instance;
}

}
}
