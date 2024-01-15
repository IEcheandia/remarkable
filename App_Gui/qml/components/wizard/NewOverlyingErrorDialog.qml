import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.userManagement 1.0
import Precitec.AppGui 1.0

Dialog {
    id: newErrorDialog

    property var controller: null
    property var configuration: null
    property alias screenshotTool: screenshotHeader.screenshotTool

    title: qsTr("Select Error Type")

    onRejected: {
        destroy();
    }

    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            newErrorDialog.reject()
        }
    }

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: newErrorDialog.title
    }

    OverlyingErrorModel {
        id: errorModel
    }

    contentItem: GridLayout {
        columns: 2
        Repeater {
            id: repeater
            model: errorModel

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Image {
                    width: 350
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: model.image
                    mipmap: true
                    fillMode: Image.PreserveAspectFit

                    Label {
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            bottom: parent.top
                        }
                        font.bold: true
                        text: model.name
                    }

                    Text {
                        width: parent.width
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            top: parent.bottom
                        }
                        text: model.description
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (controller && configuration)
                        {
                            var error = controller.createError(model.type);
                            productStackView.push(configuration, {"error": error, "type": model.name, "isTypeError": model.isTypeError});
                        }
                        newErrorDialog.reject();
                    }
                }
            }
        }
    }
}
