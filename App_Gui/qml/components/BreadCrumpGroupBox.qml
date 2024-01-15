import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.general 1.0
import precitec.gui.components.application 1.0

/**
 * A GroupBox with a back button in the title.
 * Emits a signal back when the back button got clicked.
 **/
GroupBox {
    id: root
    /**
     * Emitted when the back button was clicked
     **/
    signal back()
    signal backToProductStructure()
    signal backToProduct()
    signal backToSeamSeries()
    signal backToSeam()
    signal switchToSeamComponent(var component)
    signal switchToSeam(int index)
    signal switchToSeamSeries(int index)
    property alias backButton: backButton.visible
    property var product: null
    property var seamSeries: null
    property var seam: null
    property alias lastLevelModel: seamToLastLevelCombo.model
    property alias productToNextLevelEnabled: productToNextLevelButton.enabled
    property alias seamSeriesToNextLevelEnabled: seamSeriesToNextLevelButton.enabled
    property alias navigationEnabled: seamToLastLevelCombo.navigationEnabled
    label: RowLayout {
        id: rootLayout
        function countNumbers(product, seamSeries, seam)
        {
            var number = 1;
            if (product)
            {
                number++;
            }
            if (GuiConfiguration.seamSeriesOnProductStructure && seamSeries)
            {
                number++;
            }
            if (seam)
            {
                number++;
            }
            return number;
        }
        function combinedImplicitWidth(product, seamSeries, seam)
        {
            var width = control.implicitWidth * (rootLayout.numberComponent + 1) + detailsLabel.implicitWidth;
            if (product)
            {
                width += productButton.implicitWidth;
            }
            if (GuiConfiguration.seamSeriesOnProductStructure && seamSeries)
            {
                width += seamSeriesButton.implicitWidth;
            }
            if (seam)
            {
                width += seamButton.implicitWidth;
            }
            return width
        }
        property int numberComponent: countNumbers(root.product, root.seamSeries, root.seam)
        property real actualImplicitWidth: combinedImplicitWidth(root.product, root.seamSeries, root.seam)
        property real availableWidthForComponents: root.availableWidth - control.implicitWidth * (numberComponent + 1)
        width: root.availableWidth
        spacing: 0
        ToolButton {
            id: control
            flat: true
            display: Button.IconOnly
            icon.name: "user-home"
            enabled: root.navigationEnabled
            onClicked: root.backToProductStructure()
            background: Rectangle {
                implicitWidth: 40
                implicitHeight: 40

                opacity: control.down ? 1.0 : 0.5
                visible: !control.flat || control.down || control.checked || control.highlighted
                color: control.down || control.checked || control.highlighted ? control.palette.mid : control.palette.button
            }
            Layout.leftMargin: root.leftPadding
        }
        Button {
            id: productButton
            icon.name: "select-product"
            icon.color: Settings.iconColor
            text: root.product ? product.name : ""
            enabled: root.navigationEnabled
            visible: root.product
            flat: true
            onClicked: root.backToProduct()
            Layout.maximumWidth: rootLayout.actualImplicitWidth > root.availableWidth ? rootLayout.availableWidthForComponents / rootLayout.numberComponent : -1
        }
        BreadCrumpComboBox {
            id: productToNextLevelButton
            model: root.product ? (GuiConfiguration.seamSeriesOnProductStructure ? root.product.seamSeries : root.product.allSeams) : undefined
            measureTask: true
            visible: productButton.visible
            navigationEnabled: root.navigationEnabled
            onItemSelected: {
                if (GuiConfiguration.seamSeriesOnProductStructure)
                {
                    root.switchToSeamSeries(index);
                }
                else
                {
                    root.switchToSeam(index);
                }
            }
        }
        Button {
            id: seamSeriesButton
            icon.name: "select-seam-series"
            icon.color: Settings.iconColor
            enabled: root.navigationEnabled
            text: root.seamSeries ? qsTr("%1 (%2)").arg(root.seamSeries.name).arg(root.seamSeries.visualNumber) : ""
            visible: GuiConfiguration.seamSeriesOnProductStructure && root.seamSeries
            flat: true
            onClicked: root.backToSeamSeries()
            Layout.maximumWidth: rootLayout.actualImplicitWidth > root.availableWidth ? rootLayout.availableWidthForComponents / rootLayout.numberComponent : -1
        }
        BreadCrumpComboBox {
            id: seamSeriesToNextLevelButton
            model: root.seamSeries ? root.seamSeries.seams : undefined
            measureTask: true
            visible: seamSeriesButton.visible
            navigationEnabled: root.navigationEnabled
            onItemSelected: root.switchToSeam(index)
        }
        Button {
            id: seamButton
            icon.name: "select-seam"
            icon.color: Settings.iconColor
            text: root.seam ? qsTr("%1 (%2)").arg(root.seam.name).arg(root.seam.visualNumber) : ""
            enabled: root.navigationEnabled
            visible: root.seam
            flat: true
            onClicked: root.backToSeam()
            Layout.maximumWidth: rootLayout.actualImplicitWidth > root.availableWidth ? rootLayout.availableWidthForComponents / rootLayout.numberComponent : -1
        }
        BreadCrumpComboBox {
            id: seamToLastLevelCombo
            visible: seamButton.visible
            onItemSelected: root.switchToSeamComponent(seamToLastLevelCombo.model.data(seamToLastLevelCombo.model.index(index, 0), Qt.UserRole + 2))
        }
        Label {
            id: detailsLabel
            text: root.title
            font.family: root.font.family
            font.pixelSize: root.font.pixelSize
            font.bold: true
            clip: true
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight

            Layout.fillWidth: true
        }
        ToolButton {
            id: backButton
            display: AbstractButton.IconOnly
            enabled: root.navigationEnabled
            icon.name: "arrow-left"
            onClicked: {
                root.back();
            }
        }
    }
}

