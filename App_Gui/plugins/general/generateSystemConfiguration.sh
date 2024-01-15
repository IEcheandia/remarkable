#!/bin/bash

LINES=$(sed -E '/#|namespace|public|private|protected|static|\}|\{|\(|m_/d' ${1})

FOUND=0;
OUTPUT=${2}

BooleanKeys=();
IntKeys=();
StringKeys=();

for line in ${LINES[@]}; do
    if [[ $FOUND == 0 ]]; then
        if [[ $line == "BooleanKey"* ]]; then
            FOUND=1;
            continue;
        fi
    else
        if [[ $line == ":" ]]; then
            continue;
        fi
        if [[ $line == "std::size_t"* ]]; then
            continue;
        fi
        if [[ $line == "KeyCount"* ]]; then
            break;
        fi
        BooleanKeys+=(${line::-2});
    fi
done

FOUND=0;

for line in ${LINES[@]}; do
    if [[ $FOUND == 0 ]]; then
        if [[ $line == "IntKey"* ]]; then
            FOUND=1;
            continue;
        fi
    else
        if [[ $line == ":" ]]; then
            continue;
        fi
        if [[ $line == "std::size_t"* ]]; then
            continue;
        fi
        if [[ $line == "KeyCount"* ]]; then
            break;
        fi
        IntKeys+=(${line::-2});
    fi
done


FOUND=0;

for line in ${LINES[@]}; do
    if [[ $FOUND == 0 ]]; then
        if [[ $line == "StringKey"* ]]; then
            FOUND=1;
            continue;
        fi
    else
        if [[ $line == ":" ]]; then
            continue;
        fi
        if [[ $line == "std::size_t"* ]]; then
            continue;
        fi
        if [[ $line == "KeyCount"* ]]; then
            break;
        fi
        StringKeys+=(${line::-2});
    fi
done

cat <<EOF > ${OUTPUT}
#pragma once

#include "common/systemConfiguration.h"

#include <QObject>

namespace precitec
{

using interface::SystemConfiguration;

namespace gui
{

class SystemConfigurationQml : public QObject
{
    Q_OBJECT
EOF

for i in ${BooleanKeys[@]}; do
    echo "    Q_PROPERTY(bool $i READ get$i CONSTANT)" >> ${OUTPUT}
done

for i in ${IntKeys[@]}; do
    TYPE="int";
    if [[ $i == "Type_of_Sensor" ]];then
        TYPE="precitec::gui::SystemConfigurationQml::TypeOfSensor";
    fi
    if [[ $i == "ScannerModel" ]];then
        TYPE="precitec::gui::SystemConfigurationQml::ScannerModel";
    fi
    if [[ $i == "Scanner2DController" ]];then
        TYPE="precitec::gui::SystemConfigurationQml::ScannerModel";
    fi
    
    echo "    Q_PROPERTY(${TYPE} $i READ get$i CONSTANT)" >> ${OUTPUT}
done

for i in ${StringKeys[@]}; do
    echo "    Q_PROPERTY(QString $i READ get$i CONSTANT)" >> ${OUTPUT}
done

cat <<EOF >> ${OUTPUT}

public:
    ~SystemConfigurationQml() override;

    static SystemConfigurationQml* instance();

    enum class TypeOfSensor
    {
        Coax,
        Scheimpflug,
        LED,
    };
    Q_ENUM(TypeOfSensor)

    enum class ScannerModel
    {
        Scanlab,
        SmartMove,
    };
    Q_ENUM(ScannerModel)

    enum class ScannerGenMode
    {
        ScanMaster,
        ScanTracker2D,
    };
    Q_ENUM(ScannerGenMode)

EOF

for i in ${BooleanKeys[@]}; do
cat <<EOF >> ${OUTPUT}
    bool get${i}() const
    {
        return SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::${i});
    }
EOF
done

for i in ${IntKeys[@]}; do
    TYPE="int";
    if [[ $i == "Type_of_Sensor" ]];then
        TYPE="TypeOfSensor";
    fi
    if [[ $i == "ScannerModel" ]];then
        TYPE="ScannerModel";
    fi
    if [[ $i == "Scanner2DController" ]];then
        TYPE="ScannerModel";
    fi
    
cat <<EOF >> ${OUTPUT}
    ${TYPE} get${i}() const
    {
        return ${TYPE}(SystemConfiguration::instance().get(SystemConfiguration::IntKey::${i}));
    }
EOF
done

for i in ${StringKeys[@]}; do
cat <<EOF >> ${OUTPUT}
    QString get${i}() const
    {
        return QString::fromStdString(SystemConfiguration::instance().get(SystemConfiguration::StringKey::${i}));
    }
EOF
done

cat <<EOF >> ${OUTPUT}

private:
    SystemConfigurationQml(QObject* parent = nullptr);
};

}
}
EOF
