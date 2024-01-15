import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

import precitec.gui.components.ethercat 1.0 as EtherCAT
import precitec.gui.components.userManagement 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0

Item {
    id: root
    property alias serviceServer: slaveInfoModel.service
    property alias additionalTabsModel: additionalTabsRepeater.model

    function pageBecameVisible()
    {
        if (serviceServer)
        {
            serviceServer.monitorEvents = visible;
        }
    }

    onVisibleChanged: root.pageBecameVisible()

    Component.onCompleted: root.pageBecameVisible()

    EtherCAT.ViConfigService {
        id: viConfig
        configurationDir: WeldmasterPaths.configurationDir
    }

    EtherCAT.SlaveInfoModel {
        id: slaveInfoModel
    }

    EtherCAT.SlaveInfoFilterModel {
        id: slaveInfoFilterModel
        sourceModel: slaveInfoModel
    }

    ColumnLayout {
        anchors.fill: parent
        TabBar {
            id: tabBar
            Repeater {
                model: slaveInfoFilterModel
                TabButton {
                    text: model.display
                }
            }
            TabButton {
                id: signalAnalyzerTabButton
                property int index: TabBar.index
                text: qsTr("Signal Analyzer")
            }
            Repeater {
                id: additionalTabsRepeater
                TabButton {
                    property var source: model.source
                    property var sourceProperties: model.sourceProperties
                    visible: model.visible
                    text: model.tabTitle
                }
            }
            Layout.fillWidth: true
            Component.onCompleted: {
                tabBar.setCurrentIndex(0);
            }
        }

        ListView {
            currentIndex: tabBar.currentIndex < signalAnalyzerTabButton.index ? tabBar.currentIndex : -1
            model: slaveInfoFilterModel
            interactive: false
            snapMode: ListView.SnapOneItem
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            visible: currentIndex != -1

            highlightRangeMode: ListView.StrictlyEnforceRange
            preferredHighlightBegin: 0
            preferredHighlightEnd: 0
            highlightMoveDuration: 250

            delegate: Item {
                id: delegate
                width: ListView.view.width
                height: ListView.view.height

                Loader {
                    id: loader
                    asynchronous: true
                    anchors.fill: parent
                    onLoaded: {
                        if (model.type == EtherCAT.SlaveInfoModel.Gateway)
                        {
                            loader.item.viConfig = viConfig;
                            loader.item.model.setGateway(model.instance, model.inputSize, model.outputSize);
                            loader.item.model.inputData = Qt.binding(function () { return model.latestInputData; });
                            loader.item.model.outputData = Qt.binding(function () { return model.latestOutputData; });
                        } else if (model.type == EtherCAT.SlaveInfoModel.DigitalOut)
                        {
                            loader.item.output = true;
                            loader.item.model.byteData = Qt.binding(function () { return model.latestOutputData; });
                        } else if (model.type == EtherCAT.SlaveInfoModel.DigitalIn)
                        {
                            loader.item.output = false;
                            loader.item.model.byteData = Qt.binding(function () { return model.latestInputData; });
                        } else if (model.type == EtherCAT.SlaveInfoModel.AnalogIn || model.type == EtherCAT.SlaveInfoModel.AnalogOversamplingIn)
                        {
                            loader.item.model = slaveInfoModel;
                            loader.item.slaveIndex = slaveInfoFilterModel.mapToSource(slaveInfoFilterModel.index(index, 0));
                            loader.item.visible = Qt.binding(function() { return delegate.ListView.isCurrentItem; });
                        } else if (model.type == EtherCAT.SlaveInfoModel.AnalogOut0To10 || model.type == EtherCAT.SlaveInfoModel.AnalogOutPlusMinus10)
                        {
                            loader.item.type = model.type;
                            loader.item.model = slaveInfoModel;
                            loader.item.slaveIndex = slaveInfoFilterModel.mapToSource(slaveInfoFilterModel.index(index, 0));
                            loader.item.visible = Qt.binding(function() { return delegate.ListView.isCurrentItem; });
                        }
                    }
                }

                Component.onCompleted: {
                    if (model.type == EtherCAT.SlaveInfoModel.Gateway)
                    {
                        loader.source = "qrc:///precitec/ethercat/GatewayView.qml";
                    } else if (model.type == EtherCAT.SlaveInfoModel.DigitalOut || model.type == EtherCAT.SlaveInfoModel.DigitalIn)
                    {
                        loader.source = "qrc:///precitec/ethercat/DigitalSlaveView.qml";
                    } else if (model.type == EtherCAT.SlaveInfoModel.AnalogIn || model.type == EtherCAT.SlaveInfoModel.AnalogOversamplingIn)
                    {
                        loader.source = "qrc:///precitec/ethercat/AnalogInView.qml";
                    } else if (model.type == EtherCAT.SlaveInfoModel.AnalogOut0To10 || model.type == EtherCAT.SlaveInfoModel.AnalogOutPlusMinus10)
                    {
                        loader.source = "qrc:///precitec/ethercat/AnalogOutView.qml";
                    } else
                    {
                        loader.source = "";
                    }
                }

                Connections {
                    target: loader.item
                    ignoreUnknownSignals: true
                    function onBitToggled(byteIndex, bitIndex, state) {
                        if (!UserManagement.hasPermission(App.ModifyEthercat))
                        {
                            return;
                        }
                        var model = delegate.ListView.view.model;
                        slaveInfoModel.setOutputBit(model.mapToSource(model.index(index, 0)), byteIndex, bitIndex, state);
                    }
                }
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 5
        }
        SignalAnalyzer {
            id: signalAnalyzer
            viConfig: viConfig
            service: root.serviceServer
            storageDir: WeldmasterPaths.resultsBaseDir
            visible: tabBar.currentIndex == signalAnalyzerTabButton.index
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        Loader {
            id: additionalItemsLoader
            visible: tabBar.currentIndex >= signalAnalyzerTabButton.index + 1
            active: false

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 5

            onVisibleChanged: {
                if (additionalItemsLoader.active)
                {
                    return;
                }
                if (additionalItemsLoader.visible)
                {
                    additionalItemsLoader.active = true;
                }
            }
            Connections {
                target: tabBar
                function onCurrentIndexChanged() {
                    if (tabBar.currentItem.source !== undefined && additionalItemsLoader.source != tabBar.currentItem.source)
                    {
                        additionalItemsLoader.setSource(tabBar.currentItem.source, tabBar.currentItem.sourceProperties !== undefined ? tabBar.currentItem.sourceProperties : {});
                        additionalItemsLoader.active = true;
                    }
                }
            }
        }
    }
}
