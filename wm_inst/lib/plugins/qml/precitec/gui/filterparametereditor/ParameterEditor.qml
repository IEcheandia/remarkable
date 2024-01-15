import QtQuick 2.5
import QtQuick.Controls 2.3

import Precitec.AppGui 1.0

/**
 * Provides a gui to edit parameters.
 **/
Loader {
    id: loader
    /**
     * Signal emitted when the parameter value is modified by the editor
     **/
    signal parameterValueModified()
    /**
     * The Attribute describing the user interface for this editor.
     **/
    property var attribute: null
    /**
     * The Parameter holding the current value for this editor.
     **/
    property var parameter: null

    property var defaultValue: undefined

    /**
     * The value provided by the editor component
     **/
    property var parameterValue: undefined

    property bool milliFromMicro: false

    property var resultsConfigModel: null

    property var sensorConfigModel: null

    property var errorConfigModel: null

    readonly property bool dialogVisible: item && item.dialogVisible !== undefined && item.dialogVisible

    enabled: attribute && !attribute.readOnly

    function performLoad()
    {
        if (loader.source != "")
        {
            return;
        }
        if (!loader.attribute) {
            return;
        }
        if (loader.attribute.type == Parameter.Enumeration)
        {
            loader.source = "AttributeComboBox.qml";
        } else if (loader.attribute.type == Parameter.Integer || loader.attribute.type == Parameter.UnsignedInteger)
        {
            if (milliFromMicro)
            {
                loader.source = "AttributeMilliFromMicroSpinBox.qml";
            } else
            {
                loader.source = "AttributeIntSpinBox.qml";
            }
        } else if (loader.attribute.type == Parameter.Float || loader.attribute.type == Parameter.Double)
        {
            loader.source = "AttributeDoubleSpinBox.qml";
        } else if (loader.attribute.type == Parameter.Boolean)
        {
            loader.source = "AttributeCheckBox.qml";
        } else if (loader.attribute.type == Parameter.Result)
        {
            loader.source = "AttributeResult.qml";
            return;
        } else if (loader.attribute.type == Parameter.Error)
        {
            loader.source = "AttributeError.qml";
            return;
        } else if (loader.attribute.type == Parameter.Sensor)
        {
            loader.source = "AttributeSensor.qml";
            return;
        } else if (loader.attribute.type == Parameter.String)
        {
            loader.source = "AttributeString.qml";
            return;
        } else if (loader.attribute.type == Parameter.Text)
        {
            loader.source = "AttributeText.qml";
            return;
        } else if (loader.attribute.type == Parameter.SeamFigure)
        {
            loader.source = "AttributeSeamFigure.qml";
            return;
        } else if (loader.attribute.type == Parameter.WobbleFigure)
        {
            loader.source = "AttributeWobbleFigure.qml";
            return;
        } else if (loader.attribute.type == Parameter.File)
        {
            loader.source = "AttributeFile.qml";
            return;
        } else
        {
            loader.source = "";
            return;
        }
    }

    Component.onCompleted: loader.performLoad()
    onAttributeChanged: loader.performLoad()
    onLoaded: {
        loader.item.attribute = Qt.binding(function() { return loader.attribute; });
        loader.item.parameter = Qt.binding(function() { return loader.parameter; });
        loader.item.defaultValue = Qt.binding(function() { return loader.defaultValue; });
        if (loader.attribute.type == Parameter.Result)
        {
            loader.item.resultsConfigFilterModel = Qt.binding(function() { return loader.resultsConfigModel; });
        }
        if (loader.attribute.type == Parameter.Error)
        {
            loader.item.errorConfigModel = Qt.binding(function() { return loader.errorConfigModel; });
        }
        if (loader.attribute.type == Parameter.Sensor)
        {
            loader.item.sensorConfigModel = Qt.binding(function() { return loader.sensorConfigModel; });
        }
        loader.parameterValue = Qt.binding(function() { return loader.item.parameterValue; });
    }

    Connections {
        target: loader.item
        function onParameterValueModified() {
            loader.parameterValueModified()
        }
    }
}
