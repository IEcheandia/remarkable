import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.userManagement 1.0
import Precitec.AppGui 1.0

Dialog {
    id: newErrorDialog

    property var controller: null
    property var staticConfiguration: null
    property var referenceConfiguration: null
    property alias interval: groupFilter.interval
    property alias screenshotTool: screenshotHeader.screenshotTool

    title: qsTr("Select Error %1").arg(swipe.currentIndex == 0 ? "Group" : "Type")

    implicitWidth: Math.max(groupGrid.implicitWidth, errorGrid.implicitWidth) + newErrorDialog.leftPadding + newErrorDialog.rightPadding

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

    SimpleErrorModel {
        id: abstractController
    }

    ErrorGroupFilterModel {
        id: groupFilter
        sourceModel: abstractController
    }

    contentItem: ColumnLayout {
        SwipeView {
            Layout.fillHeight: true
            Layout.fillWidth: true

            id: swipe
            clip: true
            interactive: false

            // workaround for https://bugreports.qt.io/browse/QTBUG-80750
            onCurrentIndexChanged: {
                if (currentIndex === 0) {
                    Qt.callLater(function() { interactive = false })
                    groupFilter.filterGroup = -1;
                } else {
                    Qt.callLater(function() { interactive = true })
                }
            }

            GridLayout {
                id: groupGrid

                columns: 2
                columnSpacing: 30

                Repeater {
                    id: repeater
                    model: ErrorGroupModel {}

                    Item {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 350

                        Image {
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                                right: parent.right
                            }

                            id: groupImage

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
                        }

                        Text {
                            anchors {
                                left: parent.left
                                right: parent.right
                                top: groupImage.bottom
                                margins: 1
                            }
                            text: model.description
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                swipe.currentIndex = 1;
                                groupFilter.filterGroup = model.type;
                            }
                        }
                    }
                }
            }

            GridLayout {
                flow: GridLayout.TopToBottom
                rows: 1 + (newErrorDialog.interval ? 0 : 1)

                id: errorGrid

                Repeater {
                    model: groupFilter

                    Item {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 350
                        Layout.leftMargin: 15
                        Layout.rightMargin: 15

                        Image {
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                                right: parent.right
                            }

                            id: errorImage

                            source: model.image
                            mipmap: true
                            fillMode: Image.PreserveAspectFit

                            Label {
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                    bottom: parent.top
                                }
                                font.bold: true
                                text: model.name
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        Text {
                            anchors {
                                left: parent.left
                                right: parent.right
                                top: errorImage.bottom
                                margins: 1
                            }
                            text: model.description
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (controller)
                                {
                                    var error = controller.createError(model.type);
                                    if (error.boundaryType == SeamError.Static && staticConfiguration)
                                    {
                                        productStackView.push(staticConfiguration, {"error": error, "type": model.name});
                                    } else if (error.boundaryType == SeamError.Reference && referenceConfiguration)
                                    {
                                        productStackView.push(referenceConfiguration, {"error": error, "type": model.name});
                                    }
                                }
                                newErrorDialog.reject();
                            }
                        }
                    }
                }
            }
        }

        PageIndicator {
            Layout.alignment: Qt.AlignHCenter

            id: indicator
            visible: swipe.currentIndex != 0

            count: swipe.count
            currentIndex: swipe.currentIndex
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 20

            visible: !indicator.visible
        }
    }
}

