#include <QTest>

#include "../hardwareParameter.h"
#include <filter/sensorFilterInterface.h>
#include <fliplib/NullSourceFilter.h>
#include <../Filter_Results/autotests/DummyResultHandler.h>
#include <math/mathCommon.h>


struct DummyInput
{
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> m_imageInPipe{&m_sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE};
    precitec::interface::ImageFrame m_frame;

    DummyInput():
    m_imageInPipe{&m_sourceFilter, "ImageIn"}
    {
    }

    void setImageIn()
    {

        auto oSensorImage = precitec::image::genModuloPattern(precitec::geo2d::Size(10, 10), 2);
        m_frame.data() = precitec::image::BImage(oSensorImage, precitec::geo2d::Size{60,70}, true);
        m_frame.context().setImageNumber(0);
                     m_frame.context().setPosition(300);
                m_frame.context().HW_ROI_x0 = 40;
                m_frame.context().HW_ROI_y0 = 50;
                m_frame.context().m_ScannerInfo.m_hasPosition = true;
                m_frame.context().m_ScannerInfo.m_x = 0;
                m_frame.context().m_ScannerInfo.m_y = 0;
    }

    precitec::interface::ImageFrame getFrame()
    {
        return m_frame;
    }

    bool connectToFilter(fliplib::BaseFilter* filter)
    {
        m_imageInPipe.setTag("ImageIn");
        int group{1};
        //connect  pipes
        bool ok = filter->connectPipe(&(m_imageInPipe), group);
        return ok;
    }

    void signal()
    {
        m_imageInPipe.signal(m_frame);
    }
};

class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter() : fliplib::BaseFilter("dummy") {}

    void proceedGroup(const void *sender, fliplib::PipeGroupEventArgs &e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }

    void proceed(const void *sender, fliplib::PipeEventArgs &e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }

    int getFilterType() const override
    {
        return BaseFilterInterface::SINK;
    }

    bool isProceedCalled() const
    {
        return m_proceedCalled;
    }

    void resetProceedCalled()
    {
        m_proceedCalled = false;
    }

private:
    bool m_proceedCalled = false;
};


using precitec::filter::HardwareParameter;
using precitec::analyzer::HardwareData;
class HardwareParameterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testProceed_data();
    void testProceed();
};

Q_DECLARE_METATYPE(std::string);
Q_DECLARE_METATYPE(Poco::UUID);

void HardwareParameterTest::testCtor()
{
    HardwareParameter filter;
    QCOMPARE(filter.name(), std::string("HardwareParameter"));
    QVERIFY(filter.findPipe("DataOut") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    QVERIFY(filter.getParameters().exists(std::string("AppName")));
    QVERIFY(filter.getParameters().exists(std::string("ParameterName")));

    QCOMPARE(filter.getParameters().findParameter(std::string("AppName")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("AppName")).getValue().convert<int>(), 0);
    QCOMPARE(filter.getParameters().findParameter(std::string("ParameterName")).getType(), fliplib::Parameter::TYPE_string);
    QCOMPARE(filter.getParameters().findParameter(std::string("ParameterName")).getValue().convert<std::string>(), "xcc");
}

void HardwareParameterTest::initTestCase()
{
}

void HardwareParameterTest::testProceed_data()
{
    QTest::addColumn<int>("appName");
    QTest::addColumn<std::string>("inputParameterName");
    QTest::addColumn<double>("inputValue");
    QTest::addColumn<double>("expectedResult");
    QTest::addColumn<double>("expectedRank");

    QTest::newRow("Service") << 3 << std::string("Type_of_Sensor") << 1. << 1. << 1.;
    QTest::newRow("VideoRecorder") << 4 << std::string("IsEnabled") << 1. << 1. << 1.;
    QTest::newRow("Workflow") << 6 << std::string("IsParallelInspection") << 1. << 1. << 1.;
    QTest::newRow("Calibration") << 0 << std::string("beta0") << 0.5 << 0.5 << 1.;
}

void HardwareParameterTest::testProceed()
{
    QFETCH(int, appName);
    QFETCH(std::string, inputParameterName);
    QFETCH(double, inputValue);
    QFETCH(double, expectedResult);
    QFETCH(double, expectedRank);

    HardwareParameter filter{};
    DummyFilter dummyFilter{};
    DummyInput dummyInput{};
    dummyInput.setImageIn();
    QVERIFY(dummyInput.connectToFilter(&filter));
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("DataOut"), 0));

    auto externalHardwareData = HardwareData{};
    externalHardwareData.set(appName, inputParameterName, inputValue);
    filter.setExternalData(&externalHardwareData);
    filter.arm(precitec::filter::ArmState::eSeamStart);
    filter.getParameters().update(std::string("AppName"), fliplib::Parameter::TYPE_int, appName);
    filter.getParameters().update(std::string("ParameterName"), fliplib::Parameter::TYPE_string, inputParameterName);
    filter.setParameter();

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("DataOut"));
    QVERIFY(outPipe);
    const auto result = outPipe->read(dummyInput.getFrame().context().imageNumber());
    QCOMPARE(result.ref().size(), 1);
    QCOMPARE(result.rank(), expectedRank);
    QVERIFY(precitec::math::isClose(result.ref().getData()[0], expectedResult, 1e-3));
}

QTEST_GUILESS_MAIN(HardwareParameterTest)
#include "hardwareParameterTest.moc"
