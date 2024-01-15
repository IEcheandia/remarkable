#include <QTest>

#include "../include/calibration/calibrationManager.h"
#include "../include/calibration/CalibrationOCTData.h"
#include "../include/calibration/CalibrationOCTLineParameters.h"


using precitec::calibration::CalibrationOCTData;
using precitec::calibration::CalibrationOCTMeasurementModel;
using precitec::calibration::CalibrationOCTConfigurationIDM;
using precitec::Types;

// dummy implementation of CalibrationManager
namespace precitec
{
namespace calibration
{

    interface::SmpKeyValue CalibrationManager::getIDMKeyValue(precitec::interface::Key p_key, int p_subDevice) 
    { 
        return precitec::interface::SmpKeyValue{};
    }
    interface::KeyHandle CalibrationManager::setIDMKeyValue ( interface::SmpKeyValue p_keyValue, int p_subDevice ) 
    {
        return interface::KeyHandle{};
    }

    math::CalibrationData& CalibrationManager::getCalibrationData(unsigned int p_oSensorId)
    {
        return m_oCalData[math::SensorId(p_oSensorId)];
    } 
    void CalibrationManager::sendCalibDataChangedSignal(int p_oSensorID, bool p_oInit)
    {

    }

}
}

class testOCTParameters: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDefaultParameters();
    void testgetParametersFromModel();
};

void  testOCTParameters::testDefaultParameters()
{
    QCOMPARE(CalibrationOCTData::s_DefaultParameters.size(), CalibrationOCTData::Parameter::NumParameters);
    for (auto smpKeyVal : CalibrationOCTData::s_DefaultParameters)
    {
        QVERIFY(!smpKeyVal.isNull());
    }
    
}


void testOCTParameters::testgetParametersFromModel()
{
    auto oDefaultMeasurementModel = CalibrationOCTData::getModelFromParameters(CalibrationOCTData::s_DefaultParameters);
    QVERIFY(oDefaultMeasurementModel.isInitialized());

    auto oParameters = CalibrationOCTData::getParametersFromModel(oDefaultMeasurementModel);
    for (unsigned int i = 0;  i < oParameters.size(); i ++ )
    {
        auto & rKeyVal = oParameters[i];
        auto & rRefKeyVal = CalibrationOCTData::s_DefaultParameters[i];
        
        QVERIFY(!rKeyVal.isNull());
        QVERIFY(rKeyVal->toString().size() > 0);
        QCOMPARE( rKeyVal-> type(), rRefKeyVal->type() );
        switch (rKeyVal->type())
        {
            case Types::TInt:
                QCOMPARE(rKeyVal->value<int>(), rRefKeyVal->value<int>());
                break;
            case Types::TDouble:
                QCOMPARE(rKeyVal->value<double>(), rRefKeyVal->value<double>());
                break;
            default:
                QFAIL("Unexpected KeyValue type ");
        }
        
    }
    
    auto oEmptyParameters = CalibrationOCTData::getParametersFromModel(CalibrationOCTMeasurementModel{});

    precitec::interface::Configuration oConfiguration {oParameters.begin(), oParameters.end() };
    
}

QTEST_GUILESS_MAIN(testOCTParameters)

#include "testOCTParameters.moc"
