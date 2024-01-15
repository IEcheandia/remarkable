#include <QTest>

#include "../contourFromFile.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include "math/mathCommon.h"
#include <overlay/overlayCanvas.h>
#include <filter/armStates.h>

#include <QDebug>

using namespace precitec;
using namespace precitec::filter;
using precitec::interface::GeoDoublearray;

namespace
{
double roundTo4Digits(double value)
{
    return std::round(value * 10000) / 10000;
}

std::vector<double> roundTo4Digits(const std::vector<double>& unRoundedVector)
{
    std::vector<double> roundedVector;
    roundedVector.reserve(unRoundedVector.size());
    for (const auto& element : unRoundedVector)
    {
        roundedVector.emplace_back(roundTo4Digits(element));
    }
    return roundedVector;
}
}

class ContourFromFileTest: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();
    void testRamp_data();
    void testRamp();
};


struct DummyInput
{

    struct InputData
    {
        double value = 0.0;
        int rank = 255;
        int trafoX = 0;
        int trafoY = 0;
    };

    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInX{ &m_sourceFilter, "X"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInY{ &m_sourceFilter, "Y"};
    GeoDoublearray m_x;
    GeoDoublearray m_y;

    DummyInput()
    {
        m_pipeInX.setTag("X");
        m_pipeInY.setTag("Y");
    }

    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group =1;
        //connect  pipes
        bool ok = pFilter->connectPipe(&(m_pipeInX), group);
        ok &= pFilter->connectPipe(&(m_pipeInY), group);
        return ok;
    }

    precitec::interface::GeoDoublearray createGeoDoubleArray(precitec::interface::ImageContext baseContext, const InputData & inputData )
    {
        using namespace precitec::interface;
        return GeoDoublearray( ImageContext{baseContext, SmpTrafo{new LinearTrafo{ inputData.trafoX, inputData.trafoY }}},
                                   precitec::geo2d::Doublearray{1, inputData.value, inputData.rank},
                                   ResultType::AnalysisOK, Limit);
    }

    void fillData( int imageNumber, InputData inputDataX, InputData inputDataY)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;

        ImageContext context;
        context.setImageNumber(imageNumber);

        m_x = createGeoDoubleArray(context, inputDataX);
        m_y = createGeoDoubleArray(context, inputDataY);

    }
    void signal()
    {
        m_pipeInX.signal(m_x);
        m_pipeInY.signal(m_y);
    }
};

class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter() : fliplib::BaseFilter("dummy") {}
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

struct TestDPoint : public geo2d::DPoint
{
    TestDPoint() = default;
    TestDPoint(geo2d::DPoint p) : geo2d::DPoint{p} {}
    TestDPoint(double x, double y) : geo2d::DPoint{x,y}{}
};

bool operator == ( TestDPoint p1, TestDPoint p2)
{
    if (!qFuzzyCompare(p1.x, p2.x) && !qFuzzyCompare(p1.x + 1, p2.x +1))
    {
        return false;
    }
    if (!qFuzzyCompare(p1.y, p2.y) && !qFuzzyCompare(p1.y + 1, p2.y +1))
    {
        return false;
    }
    return true;
}

char * toString(const TestDPoint& point)
{
    std::stringstream oStream;
    oStream << std::setprecision(15) << point.x << "," << point.y ;
    char *dst = new char[oStream.str().size() + 1];
    return strcpy(dst, oStream.str().c_str());
}

Q_DECLARE_METATYPE(DummyInput::InputData);
Q_DECLARE_METATYPE(TestDPoint);

void ContourFromFileTest::testCtor()
{
    ContourFromFile filter;
    QCOMPARE(filter.name(), std::string("ContourFromFile"));
    QVERIFY(filter.findPipe("Contour") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);

    QVERIFY(filter.getParameters().exists(std::string("name")));
    QCOMPARE(filter.getParameters().findParameter(std::string("name")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("name")).getValue().convert<int>(), 0);
}

void ContourFromFileTest::testProceed_data()
{
    QTest::addColumn<DummyInput::InputData>("X");
    QTest::addColumn<DummyInput::InputData>("Y");
    QTest::addColumn<int>("parameterName");
    QTest::addColumn<std::vector<TestDPoint>>("expectedOutputContour");
    QTest::addColumn<std::vector<double>>("expectedOutputPower");
    QTest::addColumn<std::vector<double>>("expectedRingOutputPower");
    QTest::addColumn<std::vector<double>>("expectedOutputVelocity");

    std::vector<double> powerArray {25.0, 30.0, 55.0, 2.0, 3.0};
    std::vector<double> ringPowerArray {12.5, 15.0, 27.5, 1.0, 60.0};
    std::vector<double> velocityArray {100.0, 200.0, 130.0, 150.5, 250.0};
    std::vector<TestDPoint> outputNoOffset{ {0.0, 0.0}, {-2.5, 0.0}, {0.0, 0.0}, {2.5, 0.0}, {0.0, 0.0}};
    QTest::newRow("NoOffset1") << DummyInput::InputData{0.0} << DummyInput::InputData{0.0} << 1
        << outputNoOffset << powerArray << std::vector<double>{-1, -1, -1,-1,-1} << velocityArray;

    double offsetX = -10;
    double offsetY = 100;
    std::vector<TestDPoint> outputOffset(outputNoOffset.size());
    std::transform(outputNoOffset.begin(), outputNoOffset.end(), outputOffset.begin(),
        [&](TestDPoint point )
        {
            return TestDPoint{point.x + offsetX, point.y + offsetY};
        }
    );

    QTest::newRow("Offset1") << DummyInput::InputData{offsetX} << DummyInput::InputData{offsetY} << 1
        << outputOffset << powerArray << std::vector<double>{-1, -1, -1,-1,-1} << velocityArray;


    QTest::newRow("NoOffset2") << DummyInput::InputData{0.0} << DummyInput::InputData{0.0} << 2
        << std::vector<TestDPoint>{ {-2.0, 0.0}, {-2.5, 1.0}, {0.0, 0.0}, {2.5, 0.0}, {0.0, 0.0}}
        << std::vector<double>{25.2, 32.0, 55.2, 2.2, 3.2} << std::vector<double>{-1, -1, -1,-1,-1} << std::vector<double>{-1, -1, -1,-1,-1};

    QTest::newRow("NoOffset3") << DummyInput::InputData{0.0} << DummyInput::InputData{0.0} << 3 << outputNoOffset << powerArray << ringPowerArray << velocityArray;
}


void ContourFromFileTest::testProceed()
{

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir{dir.path()}.mkpath(QStringLiteral("config/weld_figure/")));

    auto testConfigFile = QFINDTESTDATA(QStringLiteral("testData/weldingSeam1.json"));
    QVERIFY( QFile::copy(testConfigFile, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam1.json"))));

    auto testConfigFile2 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam2.json"));
    QVERIFY( QFile::copy(testConfigFile2, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam2.json"))));

    auto testConfigFile3 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam3.json"));
    QVERIFY( QFile::copy(testConfigFile3, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam3.json"))));

    qputenv("WM_BASE_DIR", dir.path().toLocal8Bit());

    ContourFromFile filter;

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());

    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("Contour"), 0));

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray>*>(filter.findPipe("Contour"));
    QVERIFY(outPipe);


    QFETCH(int, parameterName);
    filter.getParameters().update(std::string("name"), fliplib::Parameter::TYPE_int, parameterName);
    filter.setParameter();
    filter.arm(precitec::filter::ArmState::eSeamStart);

    QFETCH(DummyInput::InputData, X);
    QFETCH(DummyInput::InputData, Y);
    int imageNumber = 0;
    dummyInput.fillData(imageNumber,X,Y);

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();

    const auto result = outPipe->read(imageNumber);
    QCOMPARE(result.ref().size(), 1ul);

    const auto & contour = result.ref().front();

    QFETCH(std::vector<TestDPoint>, expectedOutputContour);
    QCOMPARE(contour.size(), expectedOutputContour.size());
    for (unsigned int i = 0; i < expectedOutputContour.size(); i++)
    {
        QCOMPARE(TestDPoint{contour.getData()[i]}, expectedOutputContour[i]);
    }

    QFETCH(std::vector<double>, expectedOutputPower);
    const auto attributeLaserPower = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower;

    QVERIFY(contour.hasScalarData(attributeLaserPower));
    auto & rPowerArray = contour.getScalarData(attributeLaserPower);
    QCOMPARE(rPowerArray, expectedOutputPower);

    QFETCH(std::vector<double>, expectedRingOutputPower);
    const auto attributeLaserRingPower = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing;

    QVERIFY(contour.hasScalarData(attributeLaserRingPower));
    auto & rPowerRingArray = contour.getScalarData(attributeLaserRingPower);
    QCOMPARE(rPowerRingArray, expectedRingOutputPower);

    QFETCH(std::vector<double>, expectedOutputVelocity);
    const auto attributeLaserVelocity = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity;

    QVERIFY(contour.hasScalarData(attributeLaserVelocity));
    auto & rVelocityArray = contour.getScalarData(attributeLaserVelocity);
    QCOMPARE(rVelocityArray, expectedOutputVelocity);

}

void ContourFromFileTest::testRamp_data()
{
    QTest::addColumn<int>("parameterName");
    QTest::addColumn<std::vector<TestDPoint>>("expectedOutputContour");
    QTest::addColumn<std::vector<double>>("expectedOutputPower");
    QTest::addColumn<std::vector<double>>("expectedRingOutputPower");
    QTest::addColumn<std::vector<double>>("expectedOutputVelocity");

    std::vector<TestDPoint> outputNoRamp{ {0.0, 0.0}, {-2.5, 0.0}, {0.0, 0.0}, {2.5, 0.0}, {0.0, 0.0}};
    std::vector<double> outputPower{25.0, 30.0, 55.0, 2.0, 3.0};
    std::vector<double> outputRingPower{12.5, 15.0, 27.5, 1.0, 60.0};
    std::vector<double> outputVelocity {100.0, 200.0, 130.0, 150.5, 250.0};
    QTest::newRow("NoRamp") << 3 << outputNoRamp << outputPower << outputRingPower << outputVelocity;
    std::vector<TestDPoint> outputRampIn = {{0.0, 0.0}, {0.01, 0.0}, {0.02, 0.0}, {0.03, 0.0}, {0.04, 0.0}, {0.05, 0.0}, {1.0, 0.0}, {1.5, 0.0}, {2.5, 0.0}, {3.0, 0.0}};
    outputPower = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 0.3, 0.3, 0.3, 0.3};
    outputRingPower = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 0.2, 0.3, 0.3, 0.3};
    outputVelocity = {100.0, -1.0, -1.0, -1.0, -1.0, -1.0, 200.0, 130.0, 150.5, 250.0};
    QTest::newRow("RampInOnePart") << 4 << outputRampIn << outputPower << outputRingPower << outputVelocity;
    outputRampIn = {{0.0, 0.0}, {0.01, 0.0}, {0.02, 0.0}, {0.025, 0.0}, {0.035, 0.0}, {0.045, 0.0}, {0.05, 0.0}, {1.0, 0.0}, {1.5, 0.0}, {2.0, 0.0}};
    outputPower = {0.0, 0.2, 0.4, 0.5, 0.7, 0.9, 1.0, 0.3, 0.3, 0.3};
    outputRingPower = {0.2, 0.26, 0.32, 0.35, 0.41, 0.47, 0.5, 0.3, 0.3, 0.3};
    outputVelocity = {100.0, -1.0, -1.0, 200.0, -1.0, -1.0, -1.0, 130.0, 150.5, 250.0};
    QTest::newRow("RampInSecondPart") << 5 << outputRampIn << outputPower << outputRingPower << outputVelocity;
    outputRampIn = {{0.0, 0.0}, {0.01, 0.0}, {0.02, 0.0}, {0.025, 0.0}, {0.035, 0.0}, {0.045, 0.0}, {0.05, 0.0}, {0.06, 0.0}, {1.5, 0.0}, {2.0, 0.0}};
    outputPower = {0.0, 0.1667, 0.3333, 0.4167, 0.5833, 0.75, 0.8333, 1.0, 0.5, 0.7};
    outputRingPower = {0.75, 0.6667, 0.5833, 0.5417, 0.4583, 0.375, 0.3333, 0.25, 0.5, 0.7};
    outputVelocity = {100.0, -1.0, -1.0, 200.0, -1.0, -1.0, 130.0, -1.0, 150.5, 250.0};
    QTest::newRow("RampInThirdPart") << 6 << outputRampIn << outputPower << outputRingPower << outputVelocity;
    outputRampIn = {{0.0, 0.0}, {0.01, 0.0}, {0.02, 0.0}, {0.025, 0.0}, {0.05, 0.0}, {1.5, 0.0}, {2.0, 0.0}};
    outputPower = {0.2, 0.36, 0.52, 0.6, 0.2, 0.5, 0.4};
    outputRingPower = {0.1, 0.14, 0.18, 0.2, 0.2, 0.1, 0.7};
    outputVelocity = {100.0, -1.0, -1.0, 200.0, 130.0, 150.5, 250.0};
    QTest::newRow("RampInOnPoint") << 7 << outputRampIn << outputPower << outputRingPower << outputVelocity;
    std::vector<TestDPoint> outputRampOut = {{0.0, 0.0}, {0.5, 0.0}, {1.25, 0.0}, {1.5, 0.0}, {1.97, 0.0}, {1.98, 0.0}, {1.99, 0.0}, {2.0, 0.0}};
    outputPower = {0.25, 0.3, 0.8, 0.75, 1.0, 0.6667, 0.3333, 0.0};
    outputRingPower = {0.25, 0.2, 0.3, 0.3, 0.0, 0.1667, 0.3333, 0.5};
    outputVelocity = {100.0, 200.0, 130.0, 150.5, 150.5, -1.0, -1.0, 250};
    QTest::newRow("RampOut") << 8 << outputRampOut << outputPower << outputRingPower << outputVelocity;
    outputRampOut = {{0.0, 0.0}, {0.01, 0.0}, {0.02, 0.0}, {0.03, 0.0}, {0.5, 0.0}, {1.25, 0.0}, {1.5, 0.0}, {1.97, 0.0}, {1.98, 0.0}, {1.99, 0.0}, {2.0, 0.0}};
    outputPower = {0.1, 0.3667, 0.6333, 0.9, 0.8, 0.9, 0.85, 0.9, 0.7, 0.5, 0.3};
    outputRingPower = {1.0, 0.8333, 0.6667, 0.5, 0.2, 0.3, 0.3, 0.5, 0.4167, 0.3333, 0.25};
    outputVelocity = {100, -1, -1, -1, 200, 130, 150.5, 150.5, -1, -1, 250};
    QTest::newRow("RampInOut") << 9 << outputRampOut << outputPower << outputRingPower << outputVelocity;
    outputRampOut = {{0.0, 0.0}, {0.01, 0.0}, {0.02, 0.0}, {0.03, 0.0}, {1.25, 0.0}, {1.94, 0.0}, {1.95, 0.0}, {1.96, 0.0}, {1.97, 0.0}, {1.98, 0.0}, {1.99, 0.0}, {2.0, 0.0}};
    outputPower = {0.25, 0.4167, 0.5833, 0.75, 0.3, 1.0, 0.8333, 0.6667, 0.5, 0.3333, 0.1667, 0.0};
    outputRingPower = {0.5, 0.5833, 0.6667, 0.75, 0.3, 0.75, 0.6667, 0.5833, 0.5, 0.4167, 0.3333, 0.25};
    outputVelocity = {100, -1.0, 200.0, -1.0, 130, 130, 150.5, -1.0, -1.0, -1.0, 200, 250};
    QTest::newRow("RampInOut2") << 10 << outputRampOut << outputPower << outputRingPower << outputVelocity;
    std::vector<TestDPoint> outputRampMiddle = {{0.0, 0.0}, {0.02, 0.0}, {1.25, 0.0}, {1.26, 0.0}, {1.27, 0.0}, {1.28, 0.0}, {1.95, 0.0}, {2.0, 0.0}};
    outputPower = {0.25, 0.3, 0.0, 0.3333, 0.6667, 1.0, 0.3, 0.3};
    outputRingPower = {0.25, 0.2, 0.1, 0.1167, 0.1333, 0.15, 0.8, 0.9};
    outputVelocity = {100, 200, 100, -1, -1, -1, 150.5, 250};
    QTest::newRow("RampMiddle") << 11 << outputRampMiddle << outputPower << outputRingPower << outputVelocity;
    outputRampMiddle = {{0.0, 0.0}, {0.02, 0.0}, {1.25, 0.0}, {1.26, 0.0}, {1.27, 0.0}, {1.28, 0.0}, {2.0, 0.0}};
    outputPower = {0.25, 0.3, 0.2, 0.3, 0.4, 0.5, 0.9};
    outputRingPower = {0.25, 0.2, 0.25, 0.5, 0.75, 1.0, 0.3};
    outputVelocity = {100, 200, 100, -1, 150.5, -1, 250};
    QTest::newRow("RampMiddle2") << 12 << outputRampMiddle << outputPower << outputRingPower << outputVelocity;
    outputRampMiddle = {{0.0, 0.0}, {0.01, 0.0}, {0.02, 0.0}, {1.25, 0.0}, {1.26, 0.0}, {1.27, 0.0}, {1.28, 0.0}, {1.29, 0.0}, {2.0, 0.0}};
    outputPower = {0.0, 0.5, 1.0, 0.5, 0.425, 0.35, 0.275, 0.2, 0.3};
    outputRingPower = {0.0, 0.25, 0.5, 0.5, 0.4, 0.3, 0.2, 0.1, 0.1};
    outputVelocity = {100, -1, -1, 100, -1, 150.5, -1, -1, 250};
    QTest::newRow("RampInMiddle") << 13 << outputRampMiddle << outputPower << outputRingPower << outputVelocity;
    outputRampMiddle = {{0.0, 0.0}, {0.02, 0.0}, {0.03, 0.0}, {0.04, 0.0}, {0.05, 0.0}, {1.25, 0.0}, {1.27, 0.0}, {1.98, 0.0}, {1.99, 0.0}, {2.0, 0.0}};
    outputPower = {0.25, 0.0, 0.3333, 0.6667, 1.0, 0.3, 0.3, 0.5, 0.35, 0.2};
    outputRingPower = {0.25, 1.0, 0.7167, 0.4333, 0.15, 0.3, 0.3, 0.2, 0.475, 0.75};
    outputVelocity = {100, 100, -1, -1, -1, 130, 150.5, 150.5, -1, 250};
    QTest::newRow("RampMiddleOut") << 14 << outputRampMiddle << outputPower << outputRingPower << outputVelocity;
    outputRampMiddle = {{0.0, 0.0}, {0.01, 0.0}, {0.02, 0.0}, {0.03, 0.0}, {0.04, 0.0}, {0.05, 0.0}, {0.06, 0.0}, {1.25, 0.0}, {1.27, 0.0}, {1.98, 0.0}, {1.99, 0.0}, {2.0, 0.0}};
    outputPower = {0.0, 0.5, 1.0, 0.0, 0.3333, 0.6667, 1.0, 0.3, 0.3, 0.5, 0.35, 0.2};
    outputRingPower = {0.0, 0.5, 1.0, 0.75, 0.5833, 0.4167, 0.25, 0.3, 0.3, 1.0, 0.5, 0.0};
    outputVelocity = {100, -1, -1, 100, -1, -1, -1, 130, 150.5, 150.5, -1, 250};
    QTest::newRow("RampInMiddleOut") << 15 << outputRampMiddle << outputPower << outputRingPower << outputVelocity;
}

void ContourFromFileTest::testRamp()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir{dir.path()}.mkpath(QStringLiteral("config/weld_figure/")));

    auto testConfigFile = QFINDTESTDATA(QStringLiteral("testData/weldingSeam3.json"));
    QVERIFY( QFile::copy(testConfigFile, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam3.json"))));

    auto testConfigFile2 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam4.json"));
    QVERIFY( QFile::copy(testConfigFile2, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam4.json"))));

    auto testConfigFile3 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam5.json"));
    QVERIFY( QFile::copy(testConfigFile3, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam5.json"))));

    auto testConfigFile4 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam6.json"));
    QVERIFY( QFile::copy(testConfigFile4, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam6.json"))));

    auto testConfigFile5 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam7.json"));
    QVERIFY( QFile::copy(testConfigFile5, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam7.json"))));

    auto testConfigFile6 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam8.json"));
    QVERIFY( QFile::copy(testConfigFile6, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam8.json"))));

    auto testConfigFile7 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam9.json"));
    QVERIFY( QFile::copy(testConfigFile7, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam9.json"))));

    auto testConfigFile8 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam10.json"));
    QVERIFY( QFile::copy(testConfigFile8, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam10.json"))));

    auto testConfigFile9 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam11.json"));
    QVERIFY( QFile::copy(testConfigFile9, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam11.json"))));

    auto testConfigFile10 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam12.json"));
    QVERIFY( QFile::copy(testConfigFile10, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam12.json"))));

    auto testConfigFile11 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam13.json"));
    QVERIFY( QFile::copy(testConfigFile11, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam13.json"))));

    auto testConfigFile12 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam14.json"));
    QVERIFY( QFile::copy(testConfigFile12, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam14.json"))));

    auto testConfigFile13 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam15.json"));
    QVERIFY( QFile::copy(testConfigFile13, dir.filePath(QStringLiteral("config/weld_figure/weldingSeam15.json"))));

    qputenv("WM_BASE_DIR", dir.path().toLocal8Bit());

    ContourFromFile filter;

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());

    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("Contour"), 0));

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray>*>(filter.findPipe("Contour"));
    QVERIFY(outPipe);

    QFETCH(int, parameterName);
    filter.getParameters().update(std::string("name"), fliplib::Parameter::TYPE_int, parameterName);
    filter.setParameter();
    filter.arm(precitec::filter::ArmState::eSeamStart);

    int imageNumber = 0;
    dummyInput.fillData(imageNumber, DummyInput::InputData{0.0}, DummyInput::InputData{0.0});

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();

    const auto result = outPipe->read(imageNumber);
    QCOMPARE(result.ref().size(), 1ul);

    const auto& contour = result.ref().front();

    QFETCH(std::vector<TestDPoint>, expectedOutputContour);
    QCOMPARE(contour.size(), expectedOutputContour.size());
    for (unsigned int i = 0; i < expectedOutputContour.size(); i++)
    {
        QCOMPARE(TestDPoint{contour.getData()[i]}, expectedOutputContour[i]);
    }

    QFETCH(std::vector<double>, expectedOutputPower);
    const auto attributeLaserPower = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower;

    QVERIFY(contour.hasScalarData(attributeLaserPower));
    auto& rPowerArray = contour.getScalarData(attributeLaserPower);
    QCOMPARE(rPowerArray.size(), expectedOutputPower.size());
    QCOMPARE(roundTo4Digits(rPowerArray), expectedOutputPower);

    QFETCH(std::vector<double>, expectedRingOutputPower);
    const auto attributeLaserRingPower = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing;

    QVERIFY(contour.hasScalarData(attributeLaserRingPower));
    auto& rPowerRingArray = contour.getScalarData(attributeLaserRingPower);
    QCOMPARE(rPowerRingArray.size(), expectedRingOutputPower.size());
    QCOMPARE(roundTo4Digits(rPowerRingArray), expectedRingOutputPower);

    QFETCH(std::vector<double>, expectedOutputVelocity);
    const auto attributeLaserVelocity = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity;

    QVERIFY(contour.hasScalarData(attributeLaserVelocity));
    auto& rVelocityArray = contour.getScalarData(attributeLaserVelocity);

    QCOMPARE(rVelocityArray.size(), expectedOutputVelocity.size());
    QCOMPARE(rVelocityArray, expectedOutputVelocity);
}

QTEST_GUILESS_MAIN(ContourFromFileTest)
#include "contourFromFileTest.moc"
