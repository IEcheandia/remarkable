import QtQuick 2.8
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtWebEngine 1.8

import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0

ColumnLayout {
    property alias productModel: productController.productModel
    property alias graphDir: verificationController.graphDir
    property alias resultsConfigModel: verificationController.resultsConfigModel

    ProductController {
        id: productController
    }
    VerificationController {
        id: verificationController
        productController: productController
    }

    RowLayout {
        Button {
            text: qsTr("Prepare verification")
            onClicked: verificationController.prepare()
        }
        Button {
            text: qsTr("Remove verification")
            onClicked: verificationController.remove()
        }
        Button {
            text: qsTr("Perform verification")
            onClicked: {
                verificationController.performTest([
                    verificationController.createClickAction({
                        "objectName": "topBar-overview",
                        "parentObjectName": "topBar"}),
                    verificationController.createTimerAction({"timeout": 500}),
                    verificationController.createScreenshotAction(),
                    verificationController.createClickAction({
                        "objectName": "page-overview-sideTabView-listView-item-1",
                        "parentObjectName": "page-overview-sideTabView-listView"}),
                    verificationController.createTimerAction({"timeout": 500}),
                    verificationController.createScreenshotAction(),
                    verificationController.createClickAction({
                        "objectName": "page-overview-sideTabView-listView-item-2",
                        "parentObjectName": "page-overview-sideTabView-listView"}),
                    verificationController.createTimerAction({"timeout": 500}),
                    verificationController.createScreenshotAction(),

                    verificationController.createClickAction({
                        "objectName": "topBar-results",
                        "parentObjectName": "topBar"}),
                    verificationController.createTimerAction({"timeout": 500}),
                    verificationController.createScreenshotAction(),

                    verificationController.createClickAction({
                        "objectName": UserManagement.currentUser.name + "-results-product-selector-item-{07ed57c7-1a82-485c-b109-2f58fae7a3ef}",
                        "parentObjectName": UserManagement.currentUser.name + "-results-product-selector-listview"}),
                    verificationController.createTimerAction({"timeout": 500}),
                    verificationController.createScreenshotAction(),

                    verificationController.createClickAction({
                        "objectName": UserManagement.currentUser.name + "-results-product-instance-selector-listview-item-0",
                        "parentObjectName": UserManagement.currentUser.name + "-results-product-instance-selector-listview"}),
                    verificationController.createTimerAction({"timeout": 500}),
                    verificationController.createScreenshotAction(),

                    verificationController.createClickAction({
                        "objectName": UserManagement.currentUser.name + "-results-seam-selector-item-0",
                        "parentObjectName": UserManagement.currentUser.name + "-results-seam-selector"}),
                    verificationController.createTimerAction({"timeout": 10000}),
                    verificationController.createScreenshotAction(),

                    verificationController.createClickAction({
                        "objectName": "topBar-simulation",
                        "parentObjectName": "topBar"}),
                    verificationController.createTimerAction({"timeout": 500}),
                    verificationController.createScreenshotAction(),

                    verificationController.createClickAction({
                        "objectName": UserManagement.currentUser.name + "-simulation-product-selector-item-{07ed57c7-1a82-485c-b109-2f58fae7a3ef}",
                        "parentObjectName": UserManagement.currentUser.name + "-simulation-product-selector-listview"}),
                    verificationController.createTimerAction({"timeout": 500}),
                    verificationController.createScreenshotAction(),

                    verificationController.createClickAction({
                        "objectName": UserManagement.currentUser.name + "-simulation-product-instance-selector-listview-item-0",
                        "parentObjectName": UserManagement.currentUser.name + "-simulation-product-instance-selector-listview"}),
                    verificationController.createWaitForChangeAction({
                        "objectName": UserManagement.currentUser.name + "-simulation-viewer-player-controls",
                        "property": "enabled",
                        "value": true
                    }),
                    verificationController.createScreenshotAction(),
                    verificationController.createClickAction({
                        "objectName": UserManagement.currentUser.name + "-simulation-viewer-player-controls-play"}),
                    verificationController.createWaitForChangeAction({
                        "objectName": UserManagement.currentUser.name + "-simulation-viewer-player-controls-play",
                        "property": "enabled",
                        "value": true
                    }),
                    verificationController.createScreenshotAction()

                ]);
            }
        }
    }
    RowLayout {
        Label {
            text: "http://"
        }
        TextField {
            id: ipAddressField
            selectByMouse: true
            placeholderText: qsTr("IP address of SPS simulator")
            onEditingFinished: {
                webView.url = "http://" + ipAddressField.text + ":8080/webvisu.htm"
            }
            validator: RegExpValidator {
                // source: https://www.oreilly.com/library/view/regular-expressions-cookbook/9780596802837/ch07s16.html
                regExp: /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
            }
            Layout.fillWidth: true
        }
        Label {
            text: ":8080/webvisu.htm"
        }
    }
    WebEngineView {
        id: webView
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
