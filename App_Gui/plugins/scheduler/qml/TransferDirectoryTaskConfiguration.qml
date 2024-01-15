import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.4

import precitec.gui.components.scheduler 1.0

GroupBox {
    //: title of a Group Box
    title: qsTr("Transfer directory to remote host")

    property alias settings: manager.settings
    property alias placeholderInfo: placeholderLayout.visible
    property alias hasFileName: manager.hasFileName
    property bool acceptableInput: ip.acceptableInput && path.acceptableInput && port.acceptableInput

    function save()
    {
        manager.path = path.text;
        manager.ip = ip.text;
        if (port.text == "")
        {
            manager.port = -1;
        }
        else
        {
            manager.port = Number.fromLocaleString(locale, port.text);
        }
        manager.userName = userName.text;
        manager.password = password.text;
        manager.path = path.text;
        manager.fileName = fileName.text;
        manager.debug = debugCheckbox.checked;
        manager.protocol = protocol.currentIndex;
        manager.httpMethod = httpMethod.currentIndex;

        return manager.save();
    }

    TransferDirectoryConfigurationManager {
        id: manager
    }

    GridLayout {
        width: parent.width
        columns: 2
        Label {
            //: title for the transport protocol (ComboBox)
            text: qsTr("Protocol:")
        }
        ComboBox {
            id: protocol
            objectName: "scheduler-transfer-directory-configuration-protocol"
            model: manager.protocols()
            currentIndex: manager.protocol
        }
        Label {
            text: qsTr("HTTP Method:")
            visible: httpMethod.visible
        }
        ComboBox {
            id: httpMethod
            objectName: "scheduler-transfer-directory-configuration-http-method"
            model: manager.httpMethods()
            currentIndex: manager.httpMethod
            visible: protocol.currentIndex == TransferDirectoryConfigurationManager.Http || protocol.currentIndex == TransferDirectoryConfigurationManager.Https
        }
        Label {
            //: title for a text field
            text: qsTr("IP Address:")
        }
        TextField {
            id: ip
            objectName: "scheduler-transfer-directory-configuration-ip"
            text: manager.ip
            palette.text: ip.acceptableInput ? "black" : "red"
            validator: RegExpValidator {
                // source: https://www.oreilly.com/library/view/regular-expressions-cookbook/9780596802837/ch07s16.html
                regExp: /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
            }
            Layout.fillWidth: true
            //: placeholder text for a text field
            placeholderText: qsTr("Ip address of remote system")
        }
        Label {
            //: title for a text field, Port from TCP/IP
            text: qsTr("Port")
        }
        TextField {
            id: port
            objectName: "scheduler-transfer-directory-configuration-port"
            text: manager.port == -1 ? "" : manager.port
            palette.text: port.acceptableInput ? "black" : "red"
            validator: OptionalPortValidator {}
            Layout.fillWidth: true
            //: placeholder text for a text field
            placeholderText: qsTr("Port of remote system, %1 by default").arg(manager.defaultPort(protocol.currentIndex))
        }
        Label {
            //: title for a text field
            text: qsTr("User name:")
            visible: protocol.currentIndex == TransferDirectoryConfigurationManager.Sftp
        }
        TextField {
            id: userName
            objectName: "scheduler-transfer-directory-configuration-user"
            text: manager.userName
            visible: protocol.currentIndex == TransferDirectoryConfigurationManager.Sftp
            Layout.fillWidth: true
            //: placeholder text for a text field
            placeholderText: qsTr("User name of remote system")
        }
        Label {
            //: title for a text field
            text: qsTr("Password:")
            visible: protocol.currentIndex == TransferDirectoryConfigurationManager.Sftp
        }
        TextField {
            id: password
            objectName: "scheduler-transfer-directory-configuration-password"
            text: manager.password
            visible: protocol.currentIndex == TransferDirectoryConfigurationManager.Sftp
            echoMode: TextInput.Password
            Layout.fillWidth: true
            //: placeholder text for a text field
            placeholderText: qsTr("Password of remote system")
        }
        Label {
            //: title for a text field
            text: qsTr("Path:")
        }
        TextField {
            id: path
            objectName: "scheduler-transfer-directory-configuration-path"
            text: manager.path
            validator: PathValidator {
                localFileSystem: false
            }
            palette.text: path.acceptableInput ? "black" : "red"
            Layout.fillWidth: true
            //: placeholder text for a text field
            placeholderText: qsTr("Path on remote system")
        }
        Label {
            //: title for a text field
            text: qsTr("File name:")
            visible: manager.hasFileName
        }
        TextField {
            id: fileName
            objectName: "scheduler-transfer-directory-configuration-filename"
            text: manager.fileName
            visible: manager.hasFileName
            Layout.fillWidth: true
            //: placeholder text for a text field
            placeholderText: qsTr("File name in path on remote system, leave empty to use default file name")
        }
        CheckBox {
            id: debugCheckbox
            //: title of a check box
            text: qsTr("Log debug messages to log file")
            checked: manager.debug
            Layout.columnSpan: 2
        }
        GroupBox {
            id: placeholderLayout
            //: Title of a GroupBox, Path references the Label of the Path text field, File name the File name text field
            title: manager.hasFileName ? qsTr("Supported Placeholders for Path and File name") : qsTr("Supported Placeholders for Path")

            GridLayout {
                columns: 2
                Label {
                    text: "${PRODUCT_NAME}"
                }
                Label {
                    text: qsTr("Name of the Product")
                }
                Label {
                    text: "${PRODUCT_UUID}"
                }
                Label {
                    text: qsTr("UUID of the Product")
                }
                Label {
                    text: "${PRODUCT_TYPE}"
                }
                Label {
                    text: qsTr("Type of the Product")
                }
                Label {
                    text: "${SERIALNUMBER}"
                }
                Label {
                    text: qsTr("Serial number of product instance")
                }
                Label {
                    text: "${UUID}"
                }
                Label {
                    text: qsTr("UUID of product instance")
                }
                Label {
                    text: "${DATE}"
                }
                Label {
                    text: qsTr("Date of product instance")
                }
                Label {
                    text: "${EXTENDED_PRODUCT_INFO}"
                }
                Label {
                    text: qsTr("Extended product info of product instance")
                }
            }

            Layout.columnSpan: 2
        }
    }
}
