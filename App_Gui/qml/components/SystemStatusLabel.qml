import QtQuick 2.5
import QtQuick.Controls 2.3
import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0

/**
 * Label with current system status
 **/

Control {
//     id: systemLabelRect
    height: PrecitecApplication.Settings.topBarHeight
    
    property alias systemStatus: systemLabelText.systemStatus

    property alias textColor: systemLabelText.color
    property alias backColor: systemLabelRect.color

    property color alertTextColor: 'slateblue'
    property color alertBackColor: 'gold'

    property color startColor: 'red'
    property color endColor: 'cornsilk'
    property int transitionTime: 100
    property int holdingTime: 500

    leftPadding: 15
    rightPadding: 15

    function animateOnState(state) {
        switch (state) {
        case SystemStatusServer.NotReady:
            //console.log("SystemStatusLabel.animateOnState() <" + state + "> to <true>");
            return true;
        default:
            //console.log("SystemStatusLabel.animateOnState() <" + state + "> to <false>");
            return false;
        }
    }
    SequentialAnimation on textColor {
        running: animateOnState(systemStatus.state)
        loops: Animation.Infinite
        ColorAnimation { from: startColor; to: endColor;  duration: transitionTime }
        PauseAnimation { duration: holdingTime }
        ColorAnimation { from: endColor; to: startColor; duration: transitionTime }
        PauseAnimation { duration: holdingTime }
    }
    SequentialAnimation on backColor {
        running: animateOnState(systemStatus.state)
        loops: Animation.Infinite
        ColorAnimation { from: endColor; to: startColor; duration: transitionTime }
        PauseAnimation { duration: holdingTime }
        ColorAnimation { from: startColor; to: endColor;  duration: transitionTime }
        PauseAnimation { duration: holdingTime }
     }
     
    // color the background
    function backColorOnState(state) {
        //console.log("SystemStatusLabel.backColorOnState() <" + state + ">");
        switch (state) {
        case SystemStatusServer.Calibration:
        case SystemStatusServer.Live:
        case SystemStatusServer.Unknown:
            systemLabelRect.color= alertBackColor
            break;
        case SystemStatusServer.EmergencyStop:
            systemLabelRect.color = startColor
            break;
        default:
            systemLabelRect.color= PrecitecApplication.Settings.alternateBackground
        }
    }
 
    // color of the text
    function txtColorOnState(state) {
        //console.log("SystemStatusLabel.txtColorOnState() <" + state + ">");
        switch (state) {
        case SystemStatusServer.Calibration:
        case SystemStatusServer.Live:
        case SystemStatusServer.Unknown:
            systemLabelText.color = alertTextColor
            break;
        case SystemStatusServer.EmergencyStop:
            systemLabelText.color = endColor
            break;
        default:
           systemLabelText.color= PrecitecApplication.Settings.alternateText
        }
    }

    background: Rectangle {
        id: systemLabelRect
    }

    contentItem: Label {
        id: systemLabelText
        property var systemStatus
        function stateToText(state) {
           txtColorOnState(state)
           backColorOnState(state);
           switch (state) {
            case SystemStatusServer.Normal:
                return qsTr("Ready");
            case SystemStatusServer.Live:
                return qsTr("Live");
            case SystemStatusServer.Automatic:
                return qsTr("Automatic");
            case SystemStatusServer.Calibration:
                return qsTr("Calibration");
            case SystemStatusServer.NotReady:
                return qsTr("Not ready");
            case SystemStatusServer.ProductTeachIn:
                return qsTr("Product teach in");
            case SystemStatusServer.EmergencyStop:
                return qsTr("Emergency stop");
            case SystemStatusServer.Unknown:
                return qsTr("Initializing");
            default:
                return qsTr("Error Case");
            }
        }
        text: stateToText(systemStatus.state)
        verticalAlignment: Text.AlignVCenter
   }
   
}
