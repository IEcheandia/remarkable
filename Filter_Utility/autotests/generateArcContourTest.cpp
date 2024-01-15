#include <QTest>

#include "../generateArcContour.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include "math/mathCommon.h"
#include <overlay/overlayCanvas.h>

using namespace precitec::filter;


class GenerateArcContourTest: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();
};


using namespace precitec::filter;


enum InputType
{
    eCenterX, eCenterY, eRadius
};
struct DummyInput
{   
    
    precitec::interface::ImageFrame m_SensorImageFrame {
            precitec::interface::ImageContext{},
            precitec::image::genModuloPattern(precitec::geo2d::Size{512,512},50),
            precitec::interface::ResultType::AnalysisOK
        };
    
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> m_pipeInROI{&m_sourceFilter, "Roi"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInX{ &m_sourceFilter, "CenterX"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInY{ &m_sourceFilter, "CenterY"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInRadius{ &m_sourceFilter, "Radius"};
    
    precitec::interface::ImageFrame m_frame;
    precitec::interface::GeoDoublearray m_geoX;
    precitec::interface::GeoDoublearray m_geoY;
    precitec::interface::GeoDoublearray m_geoRadius;
    
    
    struct InputData
    {
        std::array<double, 3 > inputCircleParameters;
        std::array<precitec::geo2d::Point, 3 > inputCircleTrafos;
        precitec::geo2d::Point roiTrafo;
        int roiW;
        int roiH;
        
    };
    
    DummyInput()
    {
        m_pipeInROI.setTag("roi");
        m_pipeInX.setTag("centerX");
        m_pipeInY.setTag("centerY");
        m_pipeInRadius.setTag("radius");
    }
    
    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group =1;
        //connect  pipes
        bool ok = pFilter->connectPipe(&(m_pipeInROI), group);
        ok &= pFilter->connectPipe(&(m_pipeInX), group);
        ok &= pFilter->connectPipe(&(m_pipeInY), group);
        ok &= pFilter->connectPipe(&(m_pipeInRadius), group);
        return ok;
    }
    
    precitec::interface::GeoDoublearray createGeoDoubleArray(precitec::interface::ImageContext baseContext, const InputData & inputData, InputType inputType, int rank )
    {
        using namespace precitec::interface;
        return GeoDoublearray( ImageContext{baseContext, SmpTrafo{new LinearTrafo{ inputData.inputCircleTrafos[inputType] }}}, 
                                   precitec::geo2d::Doublearray{1, inputData.inputCircleParameters[inputType],rank}, 
                                   ResultType::AnalysisOK, Limit);
    }
    
    void fillData( int imageNumber, InputData inputData, int rank)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;
        
        ImageContext baseContext;
        baseContext.setImageNumber(imageNumber);
 
        auto & oSensorImage = m_SensorImageFrame.data();
        QVERIFY(oSensorImage.width() == 512);
        QVERIFY(inputData.roiW < oSensorImage.width());
        QVERIFY(inputData.roiH < oSensorImage.height());
        m_frame = precitec::interface::ImageFrame{
            ImageContext{baseContext, SmpTrafo{new LinearTrafo{inputData.roiTrafo}}},
            precitec::image::BImage(oSensorImage, precitec::geo2d::Size{inputData.roiW, inputData.roiH}, true), //the result image has size (roiW, roiH)
            ResultType::AnalysisOK
        };
        
        m_geoX = createGeoDoubleArray(baseContext, inputData, eCenterX, rank);
        m_geoY = createGeoDoubleArray(baseContext, inputData, eCenterY, rank);
        m_geoRadius = createGeoDoubleArray(baseContext, inputData, eRadius, rank);
        
    }
    void signal()
    {
        m_pipeInROI.signal(m_frame);
        m_pipeInX.signal(m_geoX);
        m_pipeInY.signal(m_geoY);
        m_pipeInRadius.signal(m_geoRadius);
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

Q_DECLARE_METATYPE(DummyInput::InputData);

enum IntersectionType
{
    eCircleInsideROI, eCircleOutsideROI, eCircleIntersectsROISingle, eCircleIntersectsROIMultiple
};

Q_DECLARE_METATYPE(IntersectionType);

void GenerateArcContourTest::testCtor()
{
    GenerateArcContour filter;
    QCOMPARE(filter.name(), std::string("GenerateArcContour"));
    QVERIFY(filter.findPipe("Contour") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    
    QVERIFY(filter.getParameters().exists(std::string("NumPoints")));
    QVERIFY(filter.getParameters().exists(std::string("MultipleArcsStrategy")));
    
    QCOMPARE(filter.getParameters().findParameter(std::string("NumPoints")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("NumPoints")).getValue().convert<int>(), 20);
    
    QCOMPARE(filter.getParameters().findParameter(std::string("MultipleArcsStrategy")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("MultipleArcsStrategy")).getValue().convert<int>(), 1);
    
}


void GenerateArcContourTest::testProceed_data()
{
    using precitec::geo2d::Point;
    
    QTest::addColumn<int>("parameter_numPoints");
    QTest::addColumn<DummyInput::InputData>("inputData");
    QTest::addColumn<IntersectionType>("expectedIntersection");
    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {30.0, 40.0, 10.5};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{30, 40};
        inputData.roiW = 20;
        inputData.roiH = 20;
        QTest::newRow("SimpleArc") << 100 << inputData << eCircleIntersectsROISingle;
        QTest::newRow("SimpleArc_0points") << 0 << inputData << eCircleIntersectsROISingle;
        QTest::newRow("SimpleArc_1point") << 1 << inputData << eCircleIntersectsROISingle;
    }
    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {0.0, 0.0, 10.5};
        inputData.inputCircleTrafos = {Point{30, 40}, Point{30, 40}, Point{30, 40}};
        inputData.roiTrafo = Point{30, 40};
        inputData.roiW = 20;
        inputData.roiH = 20;
        QTest::newRow("SimpleArcWithTrafo") << 20 << inputData << eCircleIntersectsROISingle;
    }    

    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {30.0, 40.0, 10.5};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{60, 60};
        inputData.roiW = 20;
        inputData.roiH = 20;
        QTest::newRow("CircleOutsideROI") << 100 << inputData << eCircleOutsideROI;
    }
    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {0.0, 0.0, 10.5};
        inputData.inputCircleTrafos = {Point{60, 50}, Point{60, 50}, Point{0, 0}};
        inputData.roiTrafo = Point{30, 30};
        inputData.roiW = 50;
        inputData.roiH = 50;
        QTest::newRow("CircleInsideROI") << 100 << inputData << eCircleInsideROI;
    }
    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {0.0, 0.0, 10.0};
        inputData.inputCircleTrafos = {Point{60, 50}, Point{60, 50}, Point{0, 0}};
        inputData.roiTrafo = Point{0, 40};
        inputData.roiW = 150;
        inputData.roiH = 150;
        QTest::newRow("TangentROIInside") << 100 << inputData << eCircleInsideROI;
    }
    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {0.0, 0.0, 10.0};
        inputData.inputCircleTrafos = {Point{60, 50}, Point{60, 50}, Point{0, 0}};
        inputData.roiTrafo = Point{0, 60};
        inputData.roiW = 150;
        inputData.roiH = 150;
        QTest::newRow("TangentROIOutside") << 100 << inputData << eCircleOutsideROI;
    }
    
    {
        int radius = 20;
        precitec::geo2d::Point center {60, 50};
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {0.0, 0.0, double(radius)};
        inputData.inputCircleTrafos = {center, center, center};
        inputData.roiTrafo = Point{center.x - radius, center.y - radius};
        inputData.roiW = 2*radius;
        inputData.roiH = 2*radius;
        QTest::newRow("InscribedCircle") << 100 << inputData << eCircleIntersectsROIMultiple; 
    }
    
    {
        double side = 30.0;
        double diameter = std::sqrt(2) * side;
        
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {side/2.0, side/2.0, diameter/2.0};
        inputData.inputCircleTrafos = {Point{100, 100}, Point{100, 100}, Point{100, 100}};
        inputData.roiTrafo = Point{100, 100};
        inputData.roiW = side;
        inputData.roiH = side;
        QTest::newRow("Circumscribed") << 100 << inputData << eCircleOutsideROI;
    }

    {

        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {110, 120, 100};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{100, 100};
        inputData.roiW = 50;
        inputData.roiH = 50;
        QTest::newRow("CircleOutsideCenterInside") << 100 << inputData << eCircleOutsideROI;
    }
    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {130.0, 140.0, 10.5};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{0, 145};
        inputData.roiW = 200;
        inputData.roiH = 100;
        QTest::newRow("ArcBelow") << 100 << inputData << eCircleIntersectsROISingle;
    }
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {130.0, 140.0, 10.5};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{0, 35};
        inputData.roiW = 200;
        inputData.roiH = 100;
        QTest::newRow("ArcAbove") << 100 << inputData << eCircleIntersectsROISingle;
    }
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {130.0, 140.0, 50.0};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{170, 80};
        inputData.roiW = 200;
        inputData.roiH = 300;
        QTest::newRow("ArcRight") << 100 << inputData << eCircleIntersectsROISingle;
    }
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {130.0, 140.0, 50.0};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{0, 80};
        inputData.roiW = 170;
        inputData.roiH = 300;
        QTest::newRow("ArcLeft") << 100 << inputData << eCircleIntersectsROISingle;
    }
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {130.0, 140.0, 10.5};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{0, 135};
        inputData.roiW = 200;
        inputData.roiH = 10;
        QTest::newRow("TwoArcs") << 100 << inputData << eCircleIntersectsROIMultiple;
    }
    {
        precitec::geo2d::Point center {100,100};
        int radius = 50;
        precitec::geo2d::Point roiTopLeft {center.x - radius +10, center.y - radius + 10};
        precitec::geo2d::Point roiTopRight {center.x + radius -10, roiTopLeft.y};
        int roiH = center.y + radius - roiTopLeft.y + 50;
        
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {0.0, double(center.y), double(radius)};
        inputData.inputCircleTrafos = {center, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = roiTopLeft;
        inputData.roiW = roiTopRight.x - roiTopLeft.x;
        inputData.roiH = roiH;
        QTest::newRow("ThreeArcs") << 100 << inputData << eCircleIntersectsROIMultiple;
    }
    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {30.0, 40.0, 10.5};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{30, 40};
        inputData.roiW = 20;
        inputData.roiH = 0;
        QTest::newRow("DegenerateRectangle") << 100 << inputData << eCircleOutsideROI;
    }
    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {30.0, 40.0, 10};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{30, 50};
        inputData.roiW = 1;
        inputData.roiH = 1;
        QTest::newRow("DegenerateRectangle1x1") << 100 << inputData << eCircleOutsideROI;
    }
    
    {
        DummyInput::InputData inputData;
        inputData.inputCircleParameters = {30.0, 40.0, 0};
        inputData.inputCircleTrafos = {Point{0, 0}, Point{0, 0}, Point{0, 0}};
        inputData.roiTrafo = Point{30, 40};
        inputData.roiW = 20;
        inputData.roiH = 20;
        QTest::newRow("DegenerateCircle") << 100 << inputData << eCircleOutsideROI;
    }

        
}

void GenerateArcContourTest::testProceed()
{
    using precitec::math::isClose;
    double tolerance = 1e-12;

    GenerateArcContour filter;

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());
    
    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));
            
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("Contour"), 0));
    
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray>*>(filter.findPipe("Contour"));
    QVERIFY(outPipe);
    
    
    for (int testStrategy = 0, imageNumber = 0; testStrategy <= 1; testStrategy ++, imageNumber++)
    {
        
        // parameterize the filter
        QFETCH(int, parameter_numPoints);
        filter.getParameters().update(std::string("NumPoints"), fliplib::Parameter::TYPE_int, parameter_numPoints);
        filter.getParameters().update(std::string("MultipleArcsStrategy"), fliplib::Parameter::TYPE_int, testStrategy);
        filter.setParameter();

        QFETCH(DummyInput::InputData, inputData);
        dummyInput.fillData(imageNumber, inputData, 255);
        QCOMPARE(dummyFilter.isProceedCalled(), false);
        dummyInput.signal();
        QCOMPARE(dummyFilter.isProceedCalled(), true);
        dummyFilter.resetProceedCalled();
        
        // the result is a GeoVecDPointarray with one element
        const auto result = outPipe->read(dummyInput.m_frame.context().imageNumber());
        QCOMPARE(result.ref().size(), 1ul);
        const auto & contour = result.ref().front();
        
        QFETCH(IntersectionType, expectedIntersection);
        if (expectedIntersection == eCircleOutsideROI)
        {
            QCOMPARE(contour.size(), 0ul); 
            //empty contour, nothing else to test
            continue;
        }
        
        if (expectedIntersection == eCircleIntersectsROIMultiple && testStrategy == 0)
        {
            //discard multiple intersections
            QCOMPARE(contour.size(), 0ul);
            continue;
        }
        
        QCOMPARE((int)(contour.size()), parameter_numPoints);
        
        
        // convert everything to image reference system
        DummyInput::InputData dataInROI = inputData;
        
        dataInROI.inputCircleParameters[eCenterX] = inputData.inputCircleParameters[eCenterX] + inputData.inputCircleTrafos[eCenterX].x - inputData.roiTrafo.x;
        dataInROI.inputCircleParameters[eCenterY] = inputData.inputCircleParameters[eCenterY] + inputData.inputCircleTrafos[eCenterY].y - inputData.roiTrafo.y;
        dataInROI.inputCircleParameters[eRadius] = inputData.inputCircleParameters[eRadius];
        for (auto && trafo : dataInROI.inputCircleTrafos)
        {
            trafo = inputData.roiTrafo;
        }
    
    
        double radius2 = dataInROI.inputCircleParameters[eRadius] * dataInROI.inputCircleParameters[eRadius];
        for (auto & point : contour.getData())
        {
            //verify that each point lies on the input circle 
            auto dx = point.x - dataInROI.inputCircleParameters[eCenterX];
            auto dy = point.y - dataInROI.inputCircleParameters[eCenterY];
            auto dist2 = dx*dx + dy*dy;
            QCOMPARE(dist2,radius2);
                        
            //verify that each point lies inside the ROI
            QVERIFY(point.x >= 0 - tolerance);
            QVERIFY(point.x <= dataInROI.roiW + tolerance);
            QVERIFY(point.y >= 0.0 - tolerance);
            QVERIFY(point.y <= dataInROI.roiH + tolerance);
        }
        
        if (parameter_numPoints <= 1)
        {
            //arc is empty or a single point, nothing else to test
            continue;
        }

        auto & firstPoint = contour.getData()[0];
        auto & lastPoint = contour.getData()[parameter_numPoints-1];

        if (expectedIntersection == eCircleInsideROI)
        {
            //the arc is the full circle, first and last point are close
            auto dist = precitec::geo2d::distance(firstPoint, lastPoint);
            QVERIFY(isClose(dist, 0.0, tolerance));
            
        }
        
        
        if (expectedIntersection == eCircleIntersectsROISingle || expectedIntersection == eCircleIntersectsROIMultiple)
        {
            //the arc start and ends on the ROI

            for (auto & point : {firstPoint, lastPoint})
            {
                QVERIFY( isClose(point.x, 0.0, tolerance) 
                    || isClose(point.x, double(dataInROI.roiW), tolerance)
                    || isClose(point.y, 0.0, tolerance) 
                    || isClose(point.y, double(dataInROI.roiH), tolerance)
                );
                    
            }
        }
    }
    
    
}
        




QTEST_GUILESS_MAIN(GenerateArcContourTest)
#include "generateArcContourTest.moc"
