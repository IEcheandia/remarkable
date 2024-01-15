import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.userManagement 1.0

RemovableDevices.UpdateControl {
    id: page
    property int updatePermission: -1
    property var sideTabView
    enabled: UserManagement.currentUser && UserManagement.hasPermission(page.updatePermission) && UserManagement.hasPermission(UserManagement.BlockAutomaticLogout)
    productName: Qt.application.name

    onInstallingChanged: {
        if (installing)
        {
            ApplicationWindow.header.blocked = true;
            sideTabView.enabled = false;
            UserManagement.blockAutomaticLogout();
        } else
        {
            ApplicationWindow.header.blocked = false;
            sideTabView.enabled = true;
            UserManagement.resumeAutomaticLogout();
        }
    }
}
