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
 **/
class ConfigurationTabModel : public SecurityTabModel
{
    Q_OBJECT
public:
    explicit ConfigurationTabModel(QObject *parent = nullptr);
    ~ConfigurationTabModel() override;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ConfigurationTabModel*)
