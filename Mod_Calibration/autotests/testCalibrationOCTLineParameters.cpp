#include <QTest>

#include "../include/calibration/CalibrationOCTLineParameters.h"
#include "Poco/Util/XMLConfiguration.h"

using namespace precitec::calibration;
using namespace precitec::interface;
using precitec::Types;


namespace
{
    SmpKeyValue createDifferentKeyValue(SmpKeyValue reference, double preferredValue, double alternativeValue)
    {
        std::string key = reference->key();
        
        switch(reference->type())
        {
            case Types::TBool:
            {
                bool value = reference->value<bool>() != bool(preferredValue) ? preferredValue : alternativeValue;
                return SmpKeyValue{new TKeyValue<bool>(key,value)};
            }
            
            case Types::TInt:
            {
                int value = reference->value<int>() != int(preferredValue) ? preferredValue: alternativeValue;
                return SmpKeyValue{new TKeyValue<int>(key,value)};
            }
                
            case Types::TUInt: 
            {
                int value = reference->value<uint32_t>() != uint32_t(std::abs(preferredValue)) ? std::abs(preferredValue) : std::abs(alternativeValue);
                return SmpKeyValue{new TKeyValue<uint32_t>(key, std::abs(value))};
            }
            case Types::TString:
            {
                unsigned int stringLength = reference->value<std::string>().size();
                stringLength = stringLength != std::abs(preferredValue) ? std::abs(preferredValue) : std::abs(alternativeValue);
                std::string oValue="";
                for (unsigned int i = 0; i < stringLength; ++i)
                {
                    oValue += "?";
                }
                return SmpKeyValue{new TKeyValue<std::string>(key, oValue)};
            }
            case Types::TDouble:
            {
                double value = reference->value<double>() != preferredValue ? preferredValue: alternativeValue;
                return SmpKeyValue{new TKeyValue<double>(key, value)};
                break;
            }
                
            default:
                //Unknown type, set invalid handle
                return SmpKeyValue{new KeyValue{}};
                break;
        }
    return {};
    }

    bool comparableKeyValues(const SmpKeyValue & kv1, const SmpKeyValue&  kv2)
    {
        if (kv1->type() != kv2->type())
        {
            return false;
        }
        if (kv1->key() != kv2->key())
        {
            return false;
        }
        switch(kv1->type())
        {
            case Types::TBool:
            case Types::TInt:
            case Types::TUInt: 
            case Types::TString:
            case Types::TDouble:
                return true;
            default:
                return false;
        }

    }
    
    bool equalValues(const SmpKeyValue & kv1, const SmpKeyValue & kv2)
    {
        
        if (!comparableKeyValues(kv1, kv2))
        {
            return false;
        }
        switch(kv1->type())
        {
            case Types::TBool:
                return kv1->value<bool>() == kv2->value<bool>();
                break;
            
            case Types::TInt:
                return kv1->value<int>() == kv2->value<int>();
                break;
                
            case Types::TUInt: 
                return kv1->value<uint32_t>() == kv2->value<uint32_t>();
                break;
                
            case Types::TString:
                return kv1->value<std::string>() == kv2->value<std::string>();
                break;
                
            case Types::TDouble:
                return kv1->value<double>() == kv2->value<double>();
                break;
                
            default:
                return false;
                break;
        }
        
        return false;
    }
}


class testCalibrationOCTLineParameters: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStaticKeys();
    void testMakeConfiguration();
    void testGetKeyValue();
    void testSetKeyValue();
    void testLoadConfiguration();
    void testCorruptConfiguration();
};

void  testCalibrationOCTLineParameters::testStaticKeys()
{    
    QCOMPARE(CalibrationOCTLineParameters::s_keys.size(), CalibrationOCTLineParameters::key_id::NUM_IDs);
    std::set<std::string> unique_keys;
    //verify that all the keys are set and unique
    for (auto & key : CalibrationOCTLineParameters::s_keys)
    {
        QVERIFY(key != "");
        unique_keys.insert(key);
    }
    QCOMPARE( unique_keys.size(),  CalibrationOCTLineParameters::key_id::NUM_IDs);
}

void testCalibrationOCTLineParameters::testMakeConfiguration()
{
    CalibrationOCTLineParameters defaultParameters;
    auto config = defaultParameters.makeConfiguration();
    QCOMPARE(config.size(),  CalibrationOCTLineParameters::key_id::NUM_IDs);
   
    //verify that all the keys are  unique and correspond to the key list
    std::set<std::string> unique_keys;
    std::set<int> unique_ids;
    
    //verify that the default values are actually used in the constructor
    precitec::interface::Configuration configSetToDefault;
    for (const auto & rKeyValue : config)
    {
        auto it = std::find(CalibrationOCTLineParameters::s_keys.begin(), CalibrationOCTLineParameters::s_keys.end(), rKeyValue->key());
        QVERIFY(it != CalibrationOCTLineParameters::s_keys.end());
        unique_keys.insert(rKeyValue->key());
        unique_ids.insert(int(std::distance(CalibrationOCTLineParameters::s_keys.begin(), it)));
        
        auto keyValueCopy = rKeyValue->clone();
        keyValueCopy->resetToDefault();
        QCOMPARE(rKeyValue->toString(), keyValueCopy->toString());
    }
    
    QCOMPARE( unique_keys.size(),  CalibrationOCTLineParameters::key_id::NUM_IDs);
    QCOMPARE( unique_ids.size(),  CalibrationOCTLineParameters::key_id::NUM_IDs);
}

void testCalibrationOCTLineParameters::testGetKeyValue()
{
    CalibrationOCTLineParameters defaultParameters;
    
    //test wrong key
    {
        auto kv = defaultParameters.getKeyValue("");
        QVERIFY(kv.isNull() || !kv->isHandleValid());
    }
    
    //test all the valid keys
    for (auto & key : CalibrationOCTLineParameters::s_keys)
    {
        auto kv = defaultParameters.getKeyValue(key);
        QCOMPARE(kv->key(), key);
        
        SmpKeyValue reference_kv {kv->clone()};
        reference_kv->resetToDefault();
        
        QVERIFY(equalValues(kv, reference_kv));
        
    }
}

void testCalibrationOCTLineParameters::testSetKeyValue()
{
    CalibrationOCTLineParameters defaultParameters;

    //set empty key    
    {
        SmpKeyValue wrongKv { new KeyValue{}};
        auto h = defaultParameters.setKeyValue(wrongKv);
        QVERIFY(h.handle() == -1);    
    }
    //set unexisting key    
    {
        SmpKeyValue wrongKv { new TKeyValue<int> ("unexisting_key", 1)};
        auto h = defaultParameters.setKeyValue(wrongKv);
        QVERIFY(h.handle() == -1);    
    }
    
    for (auto & key : CalibrationOCTLineParameters::s_keys)
    {
        SmpKeyValue defaultKV { defaultParameters.getKeyValue(key)->clone()};        

        //set default key
        auto h = defaultParameters.setKeyValue(defaultKV);
        QVERIFY(h.handle() != -1);
        
        //set key with the wrong type
        if (defaultKV->type() == Types::TInt)
        {
            SmpKeyValue wrongKv { new TKeyValue<double> (key, 1.0)};
            auto h = defaultParameters.setKeyValue(wrongKv);
            QVERIFY(h.handle() == -1);    
        }
        else
        {
            SmpKeyValue wrongKv { new TKeyValue<int> (key, 1)};
            auto h = defaultParameters.setKeyValue(wrongKv);
            QVERIFY(h.handle() == -1);    
        }
        
        //set different value
        
        SmpKeyValue newKv = createDifferentKeyValue(defaultParameters.getKeyValue(key), 0.0, -1.0);
        QVERIFY(comparableKeyValues(defaultKV, newKv));
        QVERIFY(!equalValues(defaultKV, newKv));

        defaultParameters.setKeyValue(newKv);
        SmpKeyValue actualKv = defaultParameters.getKeyValue(key);
        QVERIFY(equalValues(actualKv, newKv));
                
    }
}


void testCalibrationOCTLineParameters::testLoadConfiguration()
{
    Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfIn {new Poco::Util::XMLConfiguration};
    pConfIn->loadEmpty("key_value_congfiguration");
    
    pConfIn->setInt("OCT_Line_MinNumValuesPerLayer", 1);
    pConfIn->setDouble("OCT_Line_MaxRangePerLayer", 2.0);
    pConfIn->setInt("OCT_Line_MinJumpPixel", 3);
    pConfIn->setInt("OCT_Line_MaxWidthPixel", 4);
    pConfIn->setDouble("OCT_Line_GapWidth", 5.0);
    pConfIn->setDouble("OCT_Line_GapHeight", 6.0);
    
    CalibrationOCTLineParameters oParams;
    oParams.load(pConfIn);
    
    QCOMPARE(oParams.m_minNumValuesPerLayer, 1);
    QCOMPARE(oParams.m_maxRangePerLayer, 2.0);
    QCOMPARE(oParams.m_minJumpPixel, 3);
    QCOMPARE(oParams.m_maxWidthPixel, 4);
    QCOMPARE(oParams.m_gapWidth, 5.0);
    QCOMPARE(oParams.m_gapHeight, 6.0);
    
    QCOMPARE(oParams.getKeyValue("OCT_Line_MinNumValuesPerLayer")->value<int>(), 1);
    QCOMPARE(oParams.getKeyValue("OCT_Line_MaxRangePerLayer")->value<double>(), 2.0);
    QCOMPARE(oParams.getKeyValue("OCT_Line_MinJumpPixel")->value<int>(), 3);
    QCOMPARE(oParams.getKeyValue("OCT_Line_MaxWidthPixel")->value<int>(), 4);
    QCOMPARE(oParams.getKeyValue("OCT_Line_GapWidth")->value<double>(), 5.0);
    QCOMPARE(oParams.getKeyValue("OCT_Line_GapHeight")->value<double>(), 6.0);
    
}


void testCalibrationOCTLineParameters::testCorruptConfiguration()
{
    Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfIn {new Poco::Util::XMLConfiguration};
    pConfIn->loadEmpty("key_value_congfiguration");
    
    pConfIn->setDouble("OCT_Line_MinNumValuesPerLayer", 1.5); //should have been int
    pConfIn->setDouble("OCT_Line_MaxRangePerLayer", 2.0);
    //pConfIn->setInt("OCT_Line_MinJumpPixel", 3); //skip
    pConfIn->setInt("OCT_Line_MaxWidthPixel", 4);
    pConfIn->setBool("OCT_Line_GapWidth", true); //should have been double
    pConfIn->setDouble("OCT_Line_GapHeight", 6.0);
    
    const CalibrationOCTLineParameters oDefaultParams;
    
    CalibrationOCTLineParameters oParams;
    oParams.load(pConfIn);
    
    QCOMPARE(oParams.m_minNumValuesPerLayer, oDefaultParams.m_minNumValuesPerLayer);
    QCOMPARE(oParams.m_maxRangePerLayer, 2.0);
    QCOMPARE(oParams.m_minJumpPixel, oDefaultParams.m_minJumpPixel);
    QCOMPARE(oParams.m_maxWidthPixel, 4);
    QCOMPARE(oParams.m_gapWidth, oDefaultParams.m_gapWidth);
    QCOMPARE(oParams.m_gapHeight, 6.0);

    QVERIFY(equalValues(oParams.getKeyValue("OCT_Line_MinNumValuesPerLayer"), oDefaultParams.getKeyValue("OCT_Line_MinNumValuesPerLayer")) );
    QCOMPARE(oParams.getKeyValue("OCT_Line_MaxRangePerLayer")->value<double>(), 2.0);
    QVERIFY(equalValues(oParams.getKeyValue("OCT_Line_MinJumpPixel"), oDefaultParams.getKeyValue("OCT_Line_MinJumpPixel")) );
    QCOMPARE(oParams.getKeyValue("OCT_Line_MaxWidthPixel")->value<int>(), 4);
    QVERIFY(equalValues(oParams.getKeyValue("OCT_Line_GapWidth"), oDefaultParams.getKeyValue("OCT_Line_GapWidth")) );
    QCOMPARE(oParams.getKeyValue("OCT_Line_GapHeight")->value<double>(), 6.0);
    
    //load empty xml : all values will be reset to default
    Poco::AutoPtr<Poco::Util::XMLConfiguration> pEmptyConf {new Poco::Util::XMLConfiguration};
    oParams.load(pEmptyConf);
    for (auto & key : CalibrationOCTLineParameters::s_keys)
    {
        QCOMPARE(oParams.getKeyValue(key)->toString(), oDefaultParams.getKeyValue(key)->toString() );
    }


    
}


QTEST_GUILESS_MAIN(testCalibrationOCTLineParameters)

#include "testCalibrationOCTLineParameters.moc"
