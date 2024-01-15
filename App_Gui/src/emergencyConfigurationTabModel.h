#pragma once
#include "securityTabModel.h"

#include <vector>

namespace precitec
{
namespace gui
{

/**
 * Model exposing the names of the configuration tabs (as Qt::DisplayRole)
 * and whether the current user is allowed to access the tab (as Qt::UserRole).
 *
 * Meant as a replacement for the ConfigurationTabModel in the crash-restart case.
 **/
class EmergencyConfigurationTabModel : public SecurityTabModel
{
    Q_OBJECT
public:
    explicit EmergencyConfigurationTabModel(QObject *parent = nullptr);
    ~EmergencyConfigurationTabModel() override;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::EmergencyConfigurationTabModel*)
