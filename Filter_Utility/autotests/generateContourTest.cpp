#include <QTest>

#include "../generateContour.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include "math/mathCommon.h"
#include <overlay/overlayCanvas.h>


using namespace precitec;
using namespace precitec::filter;


class GenerateContourTest: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testValidArcInput_data();
    void testValidArcInput();
};


struct DummyInput
{   
    
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInA1{ &m_sourceFilter, "A1"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInA2{ &m_sourceFilter, "A2"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInB1{ &m_sourceFilter, "B1"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInB2{ &m_sourceFilter, "B2"};
    
    enum InputDataType
    {
        A1, A2, B1, B2
    };
    std::array<precitec::interface::GeoDoublearray,4> m_inputData;

    struct InputData
    {
        double value = 0.0;
        int rank = 255;
        int trafoX = 0;
        int trafoY = 0;
    };
    
    DummyInput()
    {
        m_pipeInA1.setTag("a1");
        m_pipeInA2.setTag("a2");
        m_pipeInB1.setTag("b1");
        m_pipeInB2.setTag("b2");
    }
    
    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group =1;
        //connect  pipes
        bool ok = pFilter->connectPipe(&(m_pipeInA1), group);
        ok &= pFilter->connectPipe(&(m_pipeInA2), group);
        ok &= pFilter->connectPipe(&(m_pipeInB1), group);
        ok &= pFilter->connectPipe(&(m_pipeInB2), group);
        return ok;
    }
    
    precitec::interface::GeoDoublearray createGeoDoubleArray(precitec::interface::ImageContext baseContext, const InputData & inputData )
    {
        using namespace precitec::interface;
        return GeoDoublearray( ImageContext{baseContext, SmpTrafo{new LinearTrafo{ inputData.trafoX, inputData.trafoY }}},
                                   precitec::geo2d::Doublearray{1, inputData.value, inputData.rank},
                                   ResultType::AnalysisOK, Limit);
    }
    
    void fillData( int imageNumber, InputData inputDataA1, InputData inputDataA2, InputData inputDataB1, InputData inputDataB2)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;
        
        ImageContext context;
        context.setImageNumber(imageNumber);
        
        m_inputData[A1] = createGeoDoubleArray(context, inputDataA1);
        m_inputData[A2] = createGeoDoubleArray(context, inputDataA2);
        m_inputData[B1] = createGeoDoubleArray(context, inputDataB1);
        m_inputData[B2] = createGeoDoubleArray(context, inputDataB2);

    }
    void signal()
    {
        m_pipeInA1.signal(m_inputData[A1]);
        m_pipeInA2.signal(m_inputData[A2]);
        m_pipeInB1.signal(m_inputData[B1]);
        m_pipeInB2.signal(m_inputData[B2]);

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

void GenerateContourTest::testCtor()
{
    GenerateContour filter;
    QCOMPARE(filter.name(), std::string("GenerateContour"));
    QVERIFY(filter.findPipe("Contour") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    
    QVERIFY(filter.getParameters().exists(std::string("NumPoints")));
    QVERIFY(filter.getParameters().exists(std::string("InputType")));
    QVERIFY(filter.getParameters().exists(std::string("Radius")));
    
    QCOMPARE(filter.getParameters().findParameter(std::string("NumPoints")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("NumPoints")).getValue().convert<int>(), 3);
    
    QCOMPARE(filter.getParameters().findParameter(std::string("InputType")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("InputType")).getValue().convert<int>(), 0);

    QCOMPARE(filter.getParameters().findParameter(std::string("Radius")).getType(), fliplib::Parameter::TYPE_double);
    QCOMPARE(filter.getParameters().findParameter(std::string("Radius")).getValue().convert<double>(), 0.0);
    
}

void GenerateContourTest::testValidArcInput_data()
{
    
    QTest::addColumn<int>("parameter_numPoints");
    QTest::addColumn<int>("parameter_InputType");
    QTest::addColumn<double>("parameter_Radius");
    QTest::addColumn<DummyInput::InputData>("A1");
    QTest::addColumn<DummyInput::InputData>("A2");
    QTest::addColumn<DummyInput::InputData>("B1");
    QTest::addColumn<DummyInput::InputData>("B2");
    QTest::addColumn<TestDPoint>("expectedFirstPoint");
    QTest::addColumn<TestDPoint>("expectedLastPoint");

    //recognize the arc direction with (contour[numPoints / 4 ] - firstPoint)
    QTest::addColumn<bool>("expectedPositiveDeltaX");
    QTest::addColumn<bool>("expectedPositiveDeltaY");

    typedef DummyInput::InputData InputData;

    int numPoints = 5;

    const int inputArcPolarCoordinates = 1;
    const int inputArcTangentHorizontal = 2;
    const int inputArcTangentVertical = 3;

    auto polarToDPoint = [](geo2d::DPoint pointCenter, double radius, double theta)
    {
        return geo2d::DPoint{ pointCenter.x + std::cos(theta) * radius,
                                    pointCenter.y + std::sin(theta) * radius };
    };


    geo2d::DPoint pointCenter { -1.0, 2.0 };
    double radius = 5;

    //arc 90 deg -> 60 deg
    {
        double theta0 = 90.0 * M_PI / 180.0; // 90 deg, the arc is tangent to the x direction
        double theta1 = 60.0 * M_PI / 180.0;
        double theta025 = (90.0 - 7.5) * M_PI / 180.0;

        TestDPoint point0 = polarToDPoint(pointCenter, radius, theta0);
        TestDPoint point1 = polarToDPoint(pointCenter, radius, theta1);
        TestDPoint point1WithNoise {point1.x + 0.1*radius, point1.y};

        geo2d::DPoint point025 = polarToDPoint(pointCenter, radius, theta025);

        QTest::newRow("PolarCoordinates60") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0} << InputData{theta1}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);

        QTest::newRow("ApproximateEnd60") << numPoints << inputArcTangentHorizontal << radius
                                    << InputData{point0.x} << InputData{point0.y} <<  InputData{point1WithNoise.x} << InputData{point1WithNoise.y}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
    }

    //arc 90 deg -> 120 deg
    {
        double theta0 = 90.0 * M_PI / 180.0; // 90 deg, the arc is tangent to the x direction
        double theta1 = 120.0 * M_PI / 180.0;
        double theta025 = (90.0 + 7.5) * M_PI / 180.0;

        TestDPoint point0 = polarToDPoint(pointCenter, radius, theta0);
        TestDPoint point1 = polarToDPoint(pointCenter, radius, theta1);
        TestDPoint point1WithNoise {point1.x + 0.1*radius, point1.y};
        geo2d::DPoint point025 = polarToDPoint(pointCenter, radius, theta025);

        QTest::newRow("PolarCoordinates120") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0} << InputData{theta1}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
        QTest::newRow("ApproximateEnd120") << numPoints << inputArcTangentHorizontal << radius
                                    << InputData{point0.x} << InputData{point0.y} <<  InputData{point1WithNoise.x} << InputData{point1WithNoise.y}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
    }

    //arc -90 deg -> -60 deg
    {
        double theta0 = -90.0 * M_PI / 180.0; // 90 deg, the arc is tangent to the x direction
        double theta1 = -60.0 * M_PI / 180.0;
        double theta025 = (-90 + 7.5) * M_PI / 180.0;

        TestDPoint point0 = polarToDPoint(pointCenter, radius, theta0);
        TestDPoint point1 = polarToDPoint(pointCenter, radius, theta1);
        TestDPoint point1WithNoise {point1.x + 0.1*radius, point1.y};
        geo2d::DPoint point025 = polarToDPoint(pointCenter, radius, theta025);
        QTest::newRow("PolarCoordinates-60") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0} << InputData{theta1}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
        QTest::newRow("ApproximateEnd-60") << numPoints << inputArcTangentHorizontal << radius
                                    << InputData{point0.x} << InputData{point0.y} <<  InputData{point1WithNoise.x} << InputData{point1WithNoise.y}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
        QTest::newRow("PolarCoordinates300") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0 + 2 * M_PI} << InputData{theta1 + 2* M_PI}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
    }

    //arc -90 deg -> -120 deg
    {
        double theta0 = -90.0 * M_PI / 180.0; // 90 deg, the arc is tangent to the x direction
        double theta1 = -120.0 * M_PI / 180.0;
        double theta025 = (-90 - 7.5) * M_PI / 180.0;


        TestDPoint point0 = polarToDPoint(pointCenter, radius, theta0);
        TestDPoint point1 = polarToDPoint(pointCenter, radius, theta1);
        TestDPoint point1WithNoise {point1.x + 0.1*radius, point1.y};
        geo2d::DPoint point025 = polarToDPoint(pointCenter, radius, theta025);
        QTest::newRow("PolarCoordinates-120") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0} << InputData{theta1}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
        QTest::newRow("ApproximateEnd-120") << numPoints << inputArcTangentHorizontal << radius
                                    << InputData{point0.x} << InputData{point0.y} <<  InputData{point1WithNoise.x} << InputData{point1WithNoise.y}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
        QTest::newRow("PolarCoordinates240") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0 + 2 * M_PI} << InputData{theta1 + 2 * M_PI}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
    }

    //arc 0 deg -> 30 deg
    {
        double theta0 = 0.0 * M_PI / 180.0; // 0 deg, the arc is tangent to the y direction
        double theta1 = 30.0 * M_PI / 180.0;
        double theta025 = (7.5) * M_PI / 180.0;

        TestDPoint point0 = polarToDPoint(pointCenter, radius, theta0);
        TestDPoint point1 = polarToDPoint(pointCenter, radius, theta1);
        TestDPoint point1WithNoise {point1.x, point1.y + 0.1*radius};

        geo2d::DPoint point025 = polarToDPoint(pointCenter, radius, theta025);
        QTest::newRow("PolarCoordinates30") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0} << InputData{theta1}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
        QTest::newRow("ApproximateEnd30") << numPoints << inputArcTangentVertical << radius
                                    << InputData{point0.x} << InputData{point0.y} <<  InputData{point1WithNoise.x} << InputData{point1WithNoise.y}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
    }

    //arc 180 deg -> 150 deg
    {
        double theta0 = 180.0 * M_PI / 180.0; // 180 deg, the arc is tangent to the y direction
        double theta1 = 150.0 * M_PI / 180.0;
        double theta025 = (180 - 7.5) * M_PI / 180.0;

        TestDPoint point0 = polarToDPoint(pointCenter, radius, theta0);
        TestDPoint point1 = polarToDPoint(pointCenter, radius, theta1);
        TestDPoint point1WithNoise {point1.x, point1.y + 0.1*radius};

        geo2d::DPoint point025 = polarToDPoint(pointCenter, radius, theta025);
        QTest::newRow("PolarCoordinates150") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0} << InputData{theta1}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
        QTest::newRow("ApproximateEnd150") << numPoints << inputArcTangentVertical << radius
                                    << InputData{point0.x} << InputData{point0.y} <<  InputData{point1WithNoise.x} << InputData{point1WithNoise.y}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
    }

    //arc 0 deg -> -30 deg
    {
        double theta0 = 0.0 * M_PI / 180.0; // 0 deg, the arc is tangent to the y direction
        double theta1 = -30.0 * M_PI / 180.0;
        double theta025 = (- 7.5) * M_PI / 180.0;

        TestDPoint point0 = polarToDPoint(pointCenter, radius, theta0);
        TestDPoint point1 = polarToDPoint(pointCenter, radius, theta1);
        TestDPoint point1WithNoise {point1.x, point1.y + 0.1*radius};

        geo2d::DPoint point025 = polarToDPoint(pointCenter, radius, theta025);
        QTest::newRow("PolarCoordinates-30") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0} << InputData{theta1}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
        QTest::newRow("ApproximateEnd-30") << numPoints << inputArcTangentVertical << radius
                                    << InputData{point0.x} << InputData{point0.y} <<  InputData{point1WithNoise.x} << InputData{point1WithNoise.y}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);

        QTest::newRow("PolarCoordinates330") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0 + 2 * M_PI} << InputData{theta1 + 2 * M_PI}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
    }


    //arc 180 deg -> 210 deg
    {
        double theta0 = 180.0 * M_PI / 180.0; // 0 deg, the arc is tangent to the y direction
        double theta1 = 210.0 * M_PI / 180.0;
        double theta025 = (180 + 7.5) * M_PI / 180.0;

        TestDPoint point0 = polarToDPoint(pointCenter, radius, theta0);
        TestDPoint point1 = polarToDPoint(pointCenter, radius, theta1);
        TestDPoint point1WithNoise {point1.x, point1.y + 0.1*radius};

        geo2d::DPoint point025 = polarToDPoint(pointCenter, radius, theta025);
        QTest::newRow("PolarCoordinates210") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0} << InputData{theta1}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
        QTest::newRow("ApproximateEnd210") << numPoints << inputArcTangentVertical << radius
                                    << InputData{point0.x} << InputData{point0.y} <<  InputData{point1WithNoise.x} << InputData{point1WithNoise.y}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);

        QTest::newRow("PolarCoordinates-150") << numPoints << inputArcPolarCoordinates << radius
                                    << InputData{pointCenter.x} << InputData{pointCenter.y} <<  InputData{theta0 - 2 * M_PI} << InputData{theta1 - 2 * M_PI}
                                    << point0 << point1 << (( point025.x - point0.x) > 0) << (( point025.y - point0.y) > 0);
    }

}

void GenerateContourTest::testValidArcInput()
{
    using precitec::math::isClose;

    GenerateContour filter;

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());
    
    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));
            
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("Contour"), 0));
    
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray>*>(filter.findPipe("Contour"));
    QVERIFY(outPipe);
    
    // parameterize the filter
    QFETCH(int, parameter_numPoints);
    QFETCH(int, parameter_InputType);
    QFETCH(double, parameter_Radius);
    filter.getParameters().update(std::string("NumPoints"), fliplib::Parameter::TYPE_int, parameter_numPoints);
    filter.getParameters().update(std::string("InputType"), fliplib::Parameter::TYPE_int, parameter_InputType);
    filter.getParameters().update(std::string("Radius"), fliplib::Parameter::TYPE_double, parameter_Radius);
    filter.setParameter();


    QFETCH(DummyInput::InputData, A1);
    QFETCH(DummyInput::InputData, A2);
    QFETCH(DummyInput::InputData, B1);
    QFETCH(DummyInput::InputData, B2);
    int imageNumber = 0;
    dummyInput.fillData(imageNumber,A1,A2, B1, B2);

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();

    // the result is a GeoVecDPointarray with one element
    const auto result = outPipe->read(imageNumber);
    QCOMPARE(result.ref().size(), 1ul);
    const auto & contour = result.ref().front();
    QCOMPARE(contour.size(), parameter_numPoints);

    TestDPoint point0 {contour.getData()[0]};
    TestDPoint point1 {contour.getData()[parameter_numPoints - 1]};
    auto point025 = contour.getData()[parameter_numPoints/4];
    QTEST(point0, "expectedFirstPoint");
    QTEST(point1, "expectedLastPoint");
    QTEST(( point025.x - point0.x) > 0, "expectedPositiveDeltaX");
    QTEST(( point025.y - point0.y) > 0, "expectedPositiveDeltaY");

}
        


QTEST_GUILESS_MAIN(GenerateContourTest)
#include "generateContourTest.moc"

