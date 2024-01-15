import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.4

import precitec.gui.components.scheduler 1.0

GroupBox {
    //: Title for a GroupBox
    title: qsTr("Build time schedule")

    property alias settings: manager.settings

    function save()
    {
        manager.everyMinute = everyMinute.checked;
        manager.everyHour = everyHour.checked;
        manager.everyDayOfMonth = everyDayOfMonth.checked;
        manager.everyMonth = everyMonth.checked;
        manager.everyDayOfWeek = everyDayOfWeek.checked;

        manager.specificMinute = specificMinuteSpinBox.value;
        manager.specificHour = specificHourSpinBox.value;
        manager.specificDayOfMonth = specificDayOfMonthSpinBox.value;
        manager.specificMonth = specificMonthCombo.currentIndex;
        manager.specificDayOfWeek = specificDayOfWeekCombo.currentIndex;
        return manager.save();
    }

    ColumnLayout {
        id: mainControl
        width: parent.width
        TabBar {
            id: cronTabBar
            TabButton {
                objectName: "cron-trigger-configuration-tabbar-minute"
                //: Title of a tab button
                text: qsTr("Minute")
            }
            TabButton {
                objectName: "cron-trigger-configuration-tabbar-hour"
                //: Title of a tab button
                text: qsTr("Hour")
            }
            TabButton {
                objectName: "cron-trigger-configuration-tabbar-day-of-month"
                //: Title of a tab button
                text: qsTr("Day of month")
            }
            TabButton {
                objectName: "cron-trigger-configuration-tabbar-month"
                //: Title of a tab button
                text: qsTr("Month")
            }
            TabButton {
                objectName: "cron-trigger-configuration-tabbar-day-of-week"
                //: Title of a tab button
                text: qsTr("Day of week")
            }
            Layout.fillWidth: true
        }

        CronConfigurationManager {
            id: manager
        }

        ColumnLayout {
            visible: cronTabBar.currentIndex == 0
            Layout.fillWidth: true

            ButtonGroup {
                id: minuteButtonGroup
                exclusive: true
            }

            RadioButton {
                id: everyMinute
                objectName: "cron-trigger-configuration-cron-every-minute"
                //: Title of a radio button in selection of "Every Minute" and "Specific Minute"
                text: qsTr("Every Minute")
                checked: manager.everyMinute
                ButtonGroup.group: minuteButtonGroup
            }
            RowLayout {
                id: specificMinuteLayout
                RadioButton {
                    id: specificMinute
                    objectName: "cron-trigger-configuration-cron-specific-minute"
                    //: Title of a radio button in selection of "Every Minute" and "Specific Minute"
                    text: qsTr("Specific Minute")
                    checked: !manager.everyMinute
                    ButtonGroup.group: minuteButtonGroup
                }
                SpinBox {
                    id: specificMinuteSpinBox
                    objectName: "cron-trigger-configuration-cron-spin-minute"
                    enabled: specificMinute.checked
                    from: 0
                    to: 59
                    value: manager.specificMinute
                }
            }
        }
        ColumnLayout {
            visible: cronTabBar.currentIndex == 1
            Layout.fillWidth: true

            ButtonGroup {
                id: hourButtonGroup
                exclusive: true
            }

            RadioButton {
                id: everyHour
                objectName: "cron-trigger-configuration-cron-every-hour"
                //: Title of a radio button in selection of "Every Hour" and "Specific Hour"
                text: qsTr("Every Hour")
                checked: manager.everyHour
                ButtonGroup.group: hourButtonGroup
            }
            RowLayout {
                RadioButton {
                    id: specificHour
                    objectName: "cron-trigger-configuration-cron-specific-hour"
                    //: Title of a radio button in selection of "Every Hour" and "Specific Hour"
                    text: qsTr("Specific Hour")
                    checked: !manager.everyHour
                    ButtonGroup.group: hourButtonGroup
                }
                SpinBox {
                    id: specificHourSpinBox
                    objectName: "cron-trigger-configuration-cron-spin-hour"
                    enabled: specificHour.checked
                    from: 0
                    to: 59
                    value: manager.specificHour
                }
            }
        }
        ColumnLayout {
            visible: cronTabBar.currentIndex == 2
            Layout.fillWidth: true

            ButtonGroup {
                id: dayOfMonthButtonGroup
                exclusive: true
            }

            RadioButton {
                id: everyDayOfMonth
                objectName: "cron-trigger-configuration-cron-every-day-of-month"
                //: Title of a radio button in selection of "Every day of month" and "Specific day of month"
                text: qsTr("Every day of month")
                checked: manager.everyDayOfMonth
                ButtonGroup.group: dayOfMonthButtonGroup
            }
            RowLayout {
                RadioButton {
                    id: specificDom
                    objectName: "cron-trigger-configuration-cron-specific-day-of-month"
                    //: Title of a radio button in selection of "Every day of month" and "Specific day of month"
                    text: qsTr("Specific day of month")
                    checked: !manager.everyDayOfMonth
                    ButtonGroup.group: dayOfMonthButtonGroup
                }
                SpinBox {
                    id: specificDayOfMonthSpinBox
                    objectName: "cron-trigger-configuration-cron-spin-day-of-month"
                    enabled: specificDom.checked
                    from: 0
                    to: 31
                    value: manager.specificDayOfMonth
                }
            }
        }
        ColumnLayout {
            visible: cronTabBar.currentIndex == 3
            Layout.fillWidth: true

            ButtonGroup {
                id: monthButtonGroup
                exclusive: true
            }

            RadioButton {
                id: everyMonth
                objectName: "cron-trigger-configuration-cron-every-month"
                //: Title of a radio button in selection of "Every month" and "Specific month"
                text: qsTr("Every month")
                checked: manager.everyMonth
                ButtonGroup.group: monthButtonGroup
            }
            RowLayout {
                RadioButton {
                    id: specificMonth
                    objectName: "cron-trigger-configuration-cron-specific-month"
                    //: Title of a radio button in selection of "Every month" and "Specific month"
                    text: qsTr("Specific month")
                    checked: !manager.everyMonth
                    ButtonGroup.group: monthButtonGroup
                }
                ComboBox {
                    id: specificMonthCombo
                    objectName: "cron-trigger-configuration-cron-combo-month"
                    enabled: specificMonth.checked
                    model: [
                        qsTr("None"),
                        locale.monthName(0, Locale.LongFormat),
                        locale.monthName(1, Locale.LongFormat),
                        locale.monthName(2, Locale.LongFormat),
                        locale.monthName(3, Locale.LongFormat),
                        locale.monthName(4, Locale.LongFormat),
                        locale.monthName(5, Locale.LongFormat),
                        locale.monthName(6, Locale.LongFormat),
                        locale.monthName(7, Locale.LongFormat),
                        locale.monthName(8, Locale.LongFormat),
                        locale.monthName(9, Locale.LongFormat),
                        locale.monthName(10, Locale.LongFormat),
                        locale.monthName(11, Locale.LongFormat)
                    ]
                    currentIndex: manager.specificMonth
                }
            }
        }
        ColumnLayout {
            visible: cronTabBar.currentIndex == 4
            Layout.fillWidth: true

            ButtonGroup {
                id: dayButtonGroup
                exclusive: true
            }

            RadioButton {
                id: everyDayOfWeek
                objectName: "cron-trigger-configuration-cron-every-day-of-week"
                //: Title of a radio button in selection of "Every day of week" and "Specific day of week"
                text: qsTr("Every day of week")
                checked: manager.everyDayOfWeek
                ButtonGroup.group: dayButtonGroup
            }
            RowLayout {
                RadioButton {
                    id: specificDay
                    objectName: "cron-trigger-configuration-cron-specific-day-of-week"
                    //: Title of a radio button in selection of "Every day of week" and "Specific day of week"
                    text: qsTr("Specific day of week")
                    checked: !manager.everyDayOfWeek
                    ButtonGroup.group: dayButtonGroup
                }
                ComboBox {
                    id: specificDayOfWeekCombo
                    objectName: "cron-trigger-configuration-cron-combo-day"
                    enabled: specificDay.checked
                    model: [
                        locale.dayName(0, Locale.LongFormat),
                        locale.dayName(1, Locale.LongFormat),
                        locale.dayName(2, Locale.LongFormat),
                        locale.dayName(3, Locale.LongFormat),
                        locale.dayName(4, Locale.LongFormat),
                        locale.dayName(5, Locale.LongFormat),
                        locale.dayName(6, Locale.LongFormat),
                        locale.dayName(0, Locale.LongFormat)
                    ]
                    currentIndex: manager.specificDayOfWeek
                }
            }
        }
    }
}
