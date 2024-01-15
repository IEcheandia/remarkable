import QtQuick 2.0
import QtTest 1.2

import "../../qml/private" as Private

Item {
    id: testCase

    TestCase {
        name: "LogFilteringControlTest"
        when: windowShown

        Item {
            id: logFilterModel
            property bool includeInfo: true
            property bool includeWarning: true
            property bool includeError: true
            property bool includeDebug: false
            property bool canIncludeDebug: true
        }

        Component {
            id: testComponent

            Private.LogFilteringControl {
                anchors.fill: parent
                moduleModel: ListModel {
                    id: moduleModel
                    ListElement {
                        display: "All"
                    }
                    ListElement {
                        display: "Test"
                    }
                    ListElement {
                        display: "Foo"
                    }
                    ListElement {
                        display: "Bar"
                    }
                }
            }
        }

        function test_status()
        {
            var item = createTemporaryObject(testComponent, testCase);
            verify(item);
            var checkbox = findChild(item, "statusCheckBox");
            verify(checkbox);
            compare(checkbox.checked, true);
            mouseClick(checkbox);
            tryCompare(checkbox, "checked", false);
            compare(logFilterModel.includeInfo, false);
            mouseClick(checkbox);
            tryCompare(checkbox, "checked", true);
            compare(logFilterModel.includeInfo, true);
        }

        function test_warning()
        {
            var item = createTemporaryObject(testComponent, testCase);
            verify(item);
            var checkbox = findChild(item, "warningCheckBox");
            verify(checkbox);
            compare(checkbox.checked, true);
            mouseClick(checkbox);
            tryCompare(checkbox, "checked", false);
            compare(logFilterModel.includeWarning, false);
            mouseClick(checkbox);
            tryCompare(checkbox, "checked", true);
            compare(logFilterModel.includeWarning, true);
        }

        function test_error()
        {
            var item = createTemporaryObject(testComponent, testCase);
            verify(item);
            var checkbox = findChild(item, "errorCheckBox");
            verify(checkbox);
            compare(checkbox.checked, true);
            mouseClick(checkbox);
            tryCompare(checkbox, "checked", false);
            compare(logFilterModel.includeError, false);
            mouseClick(checkbox);
            tryCompare(checkbox, "checked", true);
            compare(logFilterModel.includeError, true);
        }

        function test_debug()
        {
            var item = createTemporaryObject(testComponent, testCase);
            verify(item);
            var checkbox = findChild(item, "debugCheckBox");
            verify(checkbox);
            compare(checkbox.checked, false);
            mouseClick(checkbox);
            tryCompare(checkbox, "checked", true);
            compare(logFilterModel.includeDebug, true);
            mouseClick(checkbox);
            tryCompare(checkbox, "checked", false);
            compare(logFilterModel.includeDebug, false);
        }

        function test_moduleSelector()
        {
            var item = createTemporaryObject(testComponent);
            verify(item);
            compare(item.currentModuleText, "All");
            compare(item.currentModuleIndex, 0);
            var combo = findChild(item, "moduleSelector");
            verify(combo);
            compare(combo.count, 4);
            combo.incrementCurrentIndex();
            compare(item.currentModuleText, "Test");
            compare(item.currentModuleIndex, 1);
        }
    }
}
