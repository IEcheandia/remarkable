#include <QTest>
#include <QDebug>
#include "util/CalibrationCorrectionContainer.h"


// WM includes
#include <module/moduleLogger.h>
#include <common/logMessage.h>


typedef std::vector<std::pair<precitec::geo2d::DPoint, precitec::coordinates::CalibrationCameraCorrection>> t_offset;
Q_DECLARE_METATYPE(t_offset);

namespace {
    //helper functions 
    
    using namespace precitec::system::message;
    template <class T>
    void marshal(T const& value, MessageBuffer *sendBuffer_) 
    {
        Serializer<T, FindSerializer<T, Single>::Value> ::serialize( *sendBuffer_, value );
    }

    template <class T>
    void deMarshal(T & value, MessageBuffer *replyBuffer_) 
    {
        Serializer<T, FindSerializer<T, Single>::Value> ::deserialize( *replyBuffer_, value );
    }
}

namespace precitec {
    //dummy logger
    LogMessage* getLogMessageOfBaseModuleLogger()
    {
        static LogMessage msg;
        return &msg;
    }

    void invokeIncreaseWriteIndex()
    {
    }

    void redirectLogMessage(LogMessage* msg)
    {
        qDebug() << msg->format().c_str();
    }
}

class TestCalibrationCorrectionContainer: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase_data();
    void testInitFromValues();
    void testSerialization();
    void testCSV();
};

using precitec::coordinates::CalibrationCameraCorrectionContainer;

void TestCalibrationCorrectionContainer::initTestCase_data()
{
    QTest::addColumn<t_offset>("offsets");
    QTest::addColumn<bool>("validOffsets");
    QTest::addColumn<t_offset>("expectedInterpolatedValues");

    QTest::newRow("defaultOffset") << t_offset {
            {{-20.0, -20.0},  {2.8, -3.9}}, 
            {{  0.0, -20.0},  {2.0, -6.0} }, 
            {{ 20.0, -20.0},  {2.0, -8.0} }, 
            {{-20.0, 0.0},    {-0.4, 2.4}}, 
            {{  0.0, 0.0},    {0.0, 0.0} }, 
            {{ 20.0, 0.0},    {-0.2,-2.4} }, 
            {{-20.0, 20.0},   {-2.7, 6.9}}, 
            {{  0.0, 20.0},   {-2.2, 4.1} }, 
            {{ 20.0, 20.0},   {-1.7, 2.1} } }
        << true 
        << t_offset {
            {{-40.0, -40.0},  {2.8, -3.9}},
            {{ 10.0, 20.0},   {-1.95, 3.1}},
            {{ 0.0, 10.0},   {-1.1, 2.05}},
            {{ 10.0, 10.0},   {-1.025, 0.95}},
            {{ 10.0 ,30.0 }, {-1.95, 3.1}}
        };


    QTest::newRow("empty") << t_offset{}
        << true 
        << t_offset {
            {{-40.0, -40.0}, {0.0, 0.0}},
            {{ 10.0, 20.0},  {0.0, 0.0}},
            {{ 0.0, 10.0},   {0.0, 0.0}},
            {{ 10.0, 10.0},  {0.0, 0.0}},
            {{ 10.0 ,30.0 }, {0.0, 0.0}}
        };
    QTest::newRow("single_value") << t_offset{{{-20.0, -20.0},  {2.8, -3.9}}}
        << true 
        << t_offset {
            {{-40.0, -40.0},  {2.8, -3.9}},
            {{ 10.0, 20.0},   {2.8, -3.9}},
            {{ 0.0, 10.0},  {2.8, -3.9}},
            {{ 10.0, 10.0},   {2.8, -3.9}},
            {{ 10.0 ,30.0 }, {2.8, -3.9}}
        };
    QTest::newRow("single_row") << t_offset {
            {{-20.0, -20.0},  {2.8, -3.9}}, 
            {{  0.0, -20.0},  {2.0, -6.0} }, 
            {{ 20.0, -20.0},  {2.0, -8.0} }}
        << true 
        << t_offset {
            {{-40.0, -40.0}, {2.8, -3.9}},
            {{ 10.0, 20.0},  {2.0, -7.0}},
            {{ 0.0, 10.0},   {2.0, -6.0} },
            {{ 10.0, 10.0},  {2.0, -7.0}},
            {{ 10.0 ,30.0 }, {2.0, -7.0}}
        };
                
    QTest::newRow("single_column") << t_offset {
            {{-20.0, -20.0},  {2.8, -3.9}}, 
            {{-20.0, 0.0},    {-0.4, 2.4}}, 
            {{-20.0, 20.0},   {-2.7, 6.9}} }
        << true 
        << t_offset {
            {{-40.0, -40.0}, {2.8, -3.9}},
            {{ 10.0, 20.0},  {-2.7, 6.9}},
            {{ 0.0, 10.0},   {-1.55, 4.65}},
            {{ 10.0, 10.0},  {-1.55, 4.65}},
            {{ 10.0 ,30.0 }, {-2.7, 6.9}}
        };
        
    QTest::newRow("diagonal") << t_offset {
            {{-20.0, -20.0},  {2.8, -3.9}}, 
            {{ 0.0, 0.0},    {-0.4, 2.4}}, 
            {{20.0, 20.0},   {-2.7, 6.9}} }
        << false 
        << t_offset {
            {{-40.0, -40.0}, {0.0, 0.0}},
            {{ 10.0, 20.0},  {0.0, 0.0}},
            {{ 0.0, 10.0},   {0.0, 0.0}},
            {{ 10.0, 10.0},  {0.0, 0.0}},
            {{ 10.0 ,30.0 }, {0.0, 0.0}}
        };


    QTest::newRow("minJump") << t_offset {
            {{-20.0, -20.0},  {2.8, -3.9}},
            {{  0.0, -20.0},  {2.0, -6.0} },
            {{ 20.0, -20.0},  {2.0, -8.0} },
            {{ 20.0, 0.0},    {-0.2,-2.4} },
            {{  0.0, 0.0},    {0.0, 0.0} },
            {{-20.0, 0.0},    {-0.4, 2.4}},
            {{-20.0, 20.0},   {-2.7, 6.9}},
            {{  0.0, 20.0},   {-2.2, 4.1} },
            {{ 20.0, 20.0},   {-1.7, 2.1} } }
        << true
        << t_offset {
            {{-40.0, -40.0},  {2.8, -3.9}},
            {{ 10.0, 20.0},   {-1.95, 3.1}},
            {{ 0.0, 10.0},   {-1.1, 2.05}},
            {{ 10.0, 10.0},   {-1.025, 0.95}},
            {{ 10.0 ,30.0 }, {-1.95, 3.1}}
        };

}

void TestCalibrationCorrectionContainer::testInitFromValues()
{    
    QFETCH_GLOBAL(t_offset, offsets);
    QFETCH_GLOBAL(bool, validOffsets);
    QFETCH_GLOBAL(t_offset,expectedInterpolatedValues);

    CalibrationCameraCorrectionContainer testObj(offsets);

    if (validOffsets)
    {
        for (auto & rEntry : offsets)
        {
            auto & rScannerPosition = rEntry.first;
            auto & expected = rEntry.second;
            auto tcpOffset = testObj.get( rScannerPosition.x, rScannerPosition.y);
            QCOMPARE(tcpOffset.m_oTCPXOffset, expected.m_oTCPXOffset);
            QCOMPARE(tcpOffset.m_oTCPYOffset, expected.m_oTCPYOffset);
        }
    }
    else
    {
        QCOMPARE(testObj.m_data.getNumberOfMeasurements(), 0u);
    }

    for (auto & rEntry : expectedInterpolatedValues)
    {
        auto & rScannerPosition = rEntry.first;
        auto & expected = rEntry.second;
        auto tcpOffset = testObj.get( rScannerPosition.x, rScannerPosition.y);
        QCOMPARE(tcpOffset.m_oTCPXOffset, expected.m_oTCPXOffset);
        QCOMPARE(tcpOffset.m_oTCPYOffset, expected.m_oTCPYOffset);
    }
}


void TestCalibrationCorrectionContainer::testSerialization()
{    

    QFETCH_GLOBAL(t_offset, offsets);
    QFETCH_GLOBAL(bool, validOffsets);
    QFETCH_GLOBAL(t_offset, expectedInterpolatedValues);

    auto  pBuffer = std::make_unique<precitec::system::message::StaticMessageBuffer>(1024);

    {        
        //serialize
        CalibrationCameraCorrectionContainer testObj(offsets);
        marshal(testObj, pBuffer.get());
    }

    //deserialize
    pBuffer->rewind();
    CalibrationCameraCorrectionContainer deserialized;
    deMarshal(deserialized, pBuffer.get());

    if (validOffsets)
    {
        for (auto & rEntry : offsets)
        {
            auto & rScannerPosition = rEntry.first;
            auto & expected = rEntry.second;
            auto tcpOffset = deserialized.get( rScannerPosition.x, rScannerPosition.y);
            QCOMPARE(tcpOffset.m_oTCPXOffset, expected.m_oTCPXOffset);
            QCOMPARE(tcpOffset.m_oTCPYOffset, expected.m_oTCPYOffset);
        }
    }
    else
    {
        QCOMPARE(deserialized.m_data.getNumberOfMeasurements(), 0u);
    }

    for (auto & rEntry : expectedInterpolatedValues)
    {
        auto & rScannerPosition = rEntry.first;
        auto & expected = rEntry.second;
        auto tcpOffset = deserialized.get( rScannerPosition.x, rScannerPosition.y);
        QCOMPARE(tcpOffset.m_oTCPXOffset, expected.m_oTCPXOffset);
        QCOMPARE(tcpOffset.m_oTCPYOffset, expected.m_oTCPYOffset);
    }

}


void TestCalibrationCorrectionContainer::testCSV()
{
    QFETCH_GLOBAL(t_offset, offsets);
    QFETCH_GLOBAL(t_offset, expectedInterpolatedValues);
    
    QTemporaryDir tmp;
    QDir dir{tmp.path()};
    QFile file {dir.filePath(QStringLiteral("test.csv"))};
    std::string filename = file.fileName().toStdString();
    
    {
        CalibrationCameraCorrectionContainer testWrite(offsets);
        
        auto ok = CalibrationCameraCorrectionContainer::write(testWrite, filename);
        QVERIFY(ok);
        QVERIFY(file.exists());
    }
       
    {
        CalibrationCameraCorrectionContainer testRead = CalibrationCameraCorrectionContainer::load(filename);
        
        for (auto & rEntry : expectedInterpolatedValues)
        {
            auto & rScannerPosition = rEntry.first;
            auto & expected = rEntry.second;
            auto tcpOffset = testRead.get( rScannerPosition.x, rScannerPosition.y);
            QCOMPARE(tcpOffset.m_oTCPXOffset, expected.m_oTCPXOffset);
            QCOMPARE(tcpOffset.m_oTCPYOffset, expected.m_oTCPYOffset);
        }
    }
    
}



QTEST_MAIN(TestCalibrationCorrectionContainer)
#include "testCalibrationCorrectionContainer.moc"
