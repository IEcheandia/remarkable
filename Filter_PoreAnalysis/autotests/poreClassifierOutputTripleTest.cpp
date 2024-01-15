
#include <QtTest/QtTest>

#include "../poreClassifierOutputTriple.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>

Q_DECLARE_METATYPE(precitec::geo2d::Blob)
Q_DECLARE_METATYPE(precitec::interface::GeoDoublearray)
Q_DECLARE_METATYPE(precitec::interface::GeoBlobarray)

// If some printed output is required
#define  TEXT_OUT  0

class PoreClassifierOutputTripleTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
    void testProceed_data();
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

    void proceedGroup ( const void * sender, fliplib::PipeGroupEventArgs & e ) override
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

void PoreClassifierOutputTripleTest::testCtor()
{
    precitec::filter::PoreClassifierOutputTriple filter;

    QCOMPARE(filter.name(), std::string("PoreClassifierOutputTriple"));
    QVERIFY(filter.findPipe("PoreCount") != nullptr);
    QVERIFY(filter.findPipe("maxPoreSize") != nullptr);
    QVERIFY(filter.findPipe("minPoreSize") != nullptr);
}


void PoreClassifierOutputTripleTest::testProceed_data()
{

    QTest::addColumn< std::vector<double> > ("bounding_box_dx");
    QTest::addColumn< std::vector<double> > ("bounding_box_dy");
    QTest::addColumn< std::vector<double> > ("pc_ratio");
    QTest::addColumn< std::vector<double> > ("gradient");
    QTest::addColumn< std::vector<double> > ("surface");
    QTest::addColumn< std::vector<precitec::geo2d::Blob> > ("contour");
    QTest::addColumn< std::vector<double> > ("expectedPoreCount");
    QTest::addColumn< std::vector<double> > ("expectedMaxPoreSize");
    QTest::addColumn< std::vector<double> > ("expectedMinPoreSize");

    // used values of Blob are xmin, xmax, ymin, ymax and npix
    precitec::geo2d::Blob blb;
    blb.xmin = 471;
    blb.xmax = 671;
    blb.ymin = 103;
    blb.ymax = 603;
    blb.npix = 89537; // max possible npix 100000 = 200*500
    precitec::geo2d::Blob blb1;
    blb1.xmin = 461;
    blb1.xmax = 681;
    blb1.ymin = 103;
    blb1.ymax = 593;
    blb1.npix = 106537; // max possible npix 107800 = 220*490
    precitec::geo2d::Blob blb2;
    blb2.xmin = 471;
    blb2.xmax = 562;
    blb2.ymin = 103;
    blb2.ymax = 445;
    blb2.npix = 14382; // max possible npix 31122 = 91*342

    //      bb dx                                   bb dy                                       ratio
    //      gradient                                surface                                     blob
    //      expected pore count                     expected max pore size                      expected min pore size
    QTest::newRow("1 Pore")
        << std::vector<double>{ 0.5 }             << std::vector<double>{ 2. }               << std::vector<double>{ 1.01 }
        << std::vector<double>{ 90. }             << std::vector<double>{ 100. }             << std::vector<precitec::geo2d::Blob>{ blb }
        << std::vector<double>{ 1010101. }        << std::vector<double>{ 0.89537 }          << std::vector<double>{ 0.89537 };

    QTest::newRow("Test 2") // expected sizes: { 0.89537, 0.79062708719, 0.99087090803 }
        << std::vector<double>{ 0.5, 1.0, 1.51 }  << std::vector<double>{ 2., 0.8, 1.42 }    << std::vector<double>{ 0.803, 1.32, 0.97 }
        << std::vector<double>{ 90., 88., 132. }  << std::vector<double>{ 100., 32., 456. }  << std::vector<precitec::geo2d::Blob>{ blb, blb1, blb2 }
        << std::vector<double>{ 1030302. }        << std::vector<double>{ 0.990871 }         << std::vector<double>{ 0.790627 };

    QTest::newRow("No Pore")
        << std::vector<double>{ 0.3, 1.0, 3.51 }  << std::vector<double>{ 2., 0.8, 1.42 }    << std::vector<double>{ 0.703, 0.52, 0.97 }
        << std::vector<double>{ 40., 88., 132. }  << std::vector<double>{ 100., 29., 556. }  << std::vector<precitec::geo2d::Blob>{ blb, blb1, blb2 }
        << std::vector<double>{ 1000000. }        << std::vector<double>{ -1 }               << std::vector<double>{ -1 };

    QTest::newRow("Test 4") // expected sizes: { 0.00660827067, 0.79062708719, 0.99087090803 }
        << std::vector<double>{ 0.11, 1.0, 3.51 } << std::vector<double>{ 0.13, 0.8, 1.42 }  << std::vector<double>{ 0.803, 1.32, 0.97 }
        << std::vector<double>{ 80., 82., 81. }   << std::vector<double>{ 100., 32., 756. }  << std::vector<precitec::geo2d::Blob>{ blb2, blb1, blb2 }
        << std::vector<double>{ 1000101. }        << std::vector<double>{ 0.79062708719 }    << std::vector<double>{ 0.790627 };
}

void PoreClassifierOutputTripleTest::testProceed()
{
    precitec::filter::PoreClassifierOutputTriple filter;

    // In-Pipes
    fliplib::NullSourceFilter sourceFilterBoundingBoxDX;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray>  pipeBoundingBoxDX{ &sourceFilterBoundingBoxDX, "bounding_box_dx" };
    pipeBoundingBoxDX.setTag("bounding_box_dx");
    QVERIFY(filter.connectPipe(&pipeBoundingBoxDX, 1));

    fliplib::NullSourceFilter sourceFilterBoundingBoxDY;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray>  pipeBoundingBoxDY{ &sourceFilterBoundingBoxDY, "bounding_box_dy" };
    pipeBoundingBoxDY.setTag("bounding_box_dy");
    QVERIFY(filter.connectPipe(&pipeBoundingBoxDY, 1));

    fliplib::NullSourceFilter sourceFilterPcRatio;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipePcRatio{ &sourceFilterPcRatio, "pc_ratio" };
    pipePcRatio.setTag("pc_ratio");
    QVERIFY(filter.connectPipe(&pipePcRatio, 1));

    fliplib::NullSourceFilter sourceFilterGradient;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeGradient{ &sourceFilterGradient, "gradient" };
    pipeGradient.setTag("gradient");
    QVERIFY(filter.connectPipe(&pipeGradient, 1));

    fliplib::NullSourceFilter sourceFilterSurface;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray>  pipeSurface{ &sourceFilterSurface, "surface" };
    pipeSurface.setTag("surface");
    QVERIFY(filter.connectPipe(&pipeSurface, 1));

    fliplib::NullSourceFilter sourceFilterBlob;
    fliplib::SynchronePipe<precitec::interface::GeoBlobarray>  pipeBlob{ &sourceFilterBlob, "contour" };
    pipeBlob.setTag("contour");
    QVERIFY(filter.connectPipe(&pipeBlob, 1));

    // Create Out-Pipes and connect to PoreClassifierOutputTriple filter
    // --------------------------------------------------

    DummyFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(filter.findPipe("PoreCount"), 1));
    QVERIFY(filterOutData.connectPipe(filter.findPipe("maxPoreSize"), 1));
    QVERIFY(filterOutData.connectPipe(filter.findPipe("minPoreSize"), 1));

    filter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 0);
    filter.getParameters().update(std::string("ActiveParameterSets"), fliplib::Parameter::TYPE_int, 3);

    filter.getParameters().update(std::string("SizeMin"), fliplib::Parameter::TYPE_double, 0.01);
    filter.getParameters().update(std::string("SizeMax"), fliplib::Parameter::TYPE_double, 0.9);
    filter.getParameters().update(std::string("BoundingBoxDXMin"), fliplib::Parameter::TYPE_double, 0.1);
    filter.getParameters().update(std::string("BoundingBoxDXMax"), fliplib::Parameter::TYPE_double, 3.);
    filter.getParameters().update(std::string("BoundingBoxDYMin"), fliplib::Parameter::TYPE_double, 0.1);
    filter.getParameters().update(std::string("BoundingBoxDYMax"), fliplib::Parameter::TYPE_double, 3.);
    filter.getParameters().update(std::string("PcRatioMin"), fliplib::Parameter::TYPE_double, 0.6);
    filter.getParameters().update(std::string("PcRatioMax"), fliplib::Parameter::TYPE_double, 1.4);
    filter.getParameters().update(std::string("GradientMin"), fliplib::Parameter::TYPE_double, 50);
    filter.getParameters().update(std::string("GradientMax"), fliplib::Parameter::TYPE_double, 240);
    filter.getParameters().update(std::string("SurfaceMin"), fliplib::Parameter::TYPE_double, 0);
    filter.getParameters().update(std::string("SurfaceMax"), fliplib::Parameter::TYPE_double, 500);

    filter.getParameters().update(std::string("SizeMin1"), fliplib::Parameter::TYPE_double, 0.1);
    filter.getParameters().update(std::string("SizeMax1"), fliplib::Parameter::TYPE_double, 1.);
    filter.getParameters().update(std::string("BoundingBoxDXMin1"), fliplib::Parameter::TYPE_double, 0.5);
    filter.getParameters().update(std::string("BoundingBoxDXMax1"), fliplib::Parameter::TYPE_double, 3.);
    filter.getParameters().update(std::string("BoundingBoxDYMin1"), fliplib::Parameter::TYPE_double, 0.5);
    filter.getParameters().update(std::string("BoundingBoxDYMax1"), fliplib::Parameter::TYPE_double, 3.);
    filter.getParameters().update(std::string("PcRatioMin1"), fliplib::Parameter::TYPE_double, 0.8);
    filter.getParameters().update(std::string("PcRatioMax1"), fliplib::Parameter::TYPE_double, 1.4);
    filter.getParameters().update(std::string("GradientMin1"), fliplib::Parameter::TYPE_double, 80);
    filter.getParameters().update(std::string("GradientMax1"), fliplib::Parameter::TYPE_double, 240);
    filter.getParameters().update(std::string("SurfaceMin1"), fliplib::Parameter::TYPE_double, 30);
    filter.getParameters().update(std::string("SurfaceMax1"), fliplib::Parameter::TYPE_double, 500);

    filter.getParameters().update(std::string("SizeMin2"), fliplib::Parameter::TYPE_double, 0.01);
    filter.getParameters().update(std::string("SizeMax2"), fliplib::Parameter::TYPE_double, 1.1);
    filter.getParameters().update(std::string("BoundingBoxDXMin2"), fliplib::Parameter::TYPE_double, 0.1);
    filter.getParameters().update(std::string("BoundingBoxDXMax2"), fliplib::Parameter::TYPE_double, 3.5);
    filter.getParameters().update(std::string("BoundingBoxDYMin2"), fliplib::Parameter::TYPE_double, 0.1);
    filter.getParameters().update(std::string("BoundingBoxDYMax2"), fliplib::Parameter::TYPE_double, 3.5);
    filter.getParameters().update(std::string("PcRatioMin2"), fliplib::Parameter::TYPE_double, 0.6);
    filter.getParameters().update(std::string("PcRatioMax2"), fliplib::Parameter::TYPE_double, 1.6);
    filter.getParameters().update(std::string("GradientMin2"), fliplib::Parameter::TYPE_double, 83);
    filter.getParameters().update(std::string("GradientMax2"), fliplib::Parameter::TYPE_double, 255);
    filter.getParameters().update(std::string("SurfaceMin2"), fliplib::Parameter::TYPE_double, 0);
    filter.getParameters().update(std::string("SurfaceMax2"), fliplib::Parameter::TYPE_double, 700);

    filter.setParameter();

    // dummy data
    int imageNumber  =   0;
    int position     = 300;

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    // parse test data
    QFETCH(std::vector<double>, bounding_box_dx);
    QFETCH(std::vector<double>, bounding_box_dy);
    QFETCH(std::vector<double>, pc_ratio);
    QFETCH(std::vector<double>, gradient);
    QFETCH(std::vector<double>, surface);
    QFETCH(std::vector<precitec::geo2d::Blob>, contour);
    QFETCH(std::vector<double>, expectedPoreCount);
    QFETCH(std::vector<double>, expectedMaxPoreSize);
    QFETCH(std::vector<double>, expectedMinPoreSize);

    // convert test array into appropriate data structure
    precitec::geo2d::Doublearray boundingBoxDX;
    boundingBoxDX.getData().insert(boundingBoxDX.getData().begin(), bounding_box_dx.begin(), bounding_box_dx.end());
    boundingBoxDX.getRank().assign(boundingBoxDX.getData().size(), precitec::filter::eRankMax);

    precitec::geo2d::Doublearray boundingBoxDY;
    boundingBoxDY.getData().insert(boundingBoxDY.getData().begin(), bounding_box_dy.begin(), bounding_box_dy.end());
    boundingBoxDY.getRank().assign(boundingBoxDY.getData().size(), precitec::filter::eRankMax);

    precitec::geo2d::Doublearray pcRatio;
    pcRatio.getData().insert(pcRatio.getData().begin(), pc_ratio.begin(), pc_ratio.end());
    pcRatio.getRank().assign(pcRatio.getData().size(), precitec::filter::eRankMax);

    precitec::geo2d::Doublearray geoGradient;
    geoGradient.getData().insert(geoGradient.getData().begin(), gradient.begin(), gradient.end());
    geoGradient.getRank().assign(geoGradient.getData().size(), precitec::filter::eRankMax);

    precitec::geo2d::Doublearray geoSurface;
    geoSurface.getData().insert(geoSurface.getData().begin(), surface.begin(), surface.end());
    geoSurface.getRank().assign(geoSurface.getData().size(), precitec::filter::eRankMax);

    precitec::geo2d::Blobarray geoBlob;
    geoBlob.getData().insert(geoBlob.getData().begin(), contour.begin(), contour.end());
    geoBlob.getRank().assign(geoBlob.getData().size(), precitec::filter::eRankMax);

    // signal pipes
    auto inPipe = precitec::interface::GeoDoublearray {
        context,
        boundingBoxDX,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    inPipe.rank(255);
    pipeBoundingBoxDX.signal(inPipe);

    inPipe = precitec::interface::GeoDoublearray {
        context,
        boundingBoxDY,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    inPipe.rank(255);
    pipeBoundingBoxDY.signal(inPipe);

    inPipe = precitec::interface::GeoDoublearray {
        context,
        pcRatio,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    inPipe.rank(255);
    pipePcRatio.signal(inPipe);

    inPipe = precitec::interface::GeoDoublearray {
        context,
        geoGradient,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    inPipe.rank(255);
    pipeGradient.signal(inPipe);

    inPipe = precitec::interface::GeoDoublearray {
        context,
        geoSurface,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    inPipe.rank(255);
    pipeSurface.signal(inPipe);

    auto bInPipe = precitec::interface::GeoBlobarray {
        context,
        geoBlob,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    bInPipe.rank(255);
    pipeBlob.signal(bInPipe);

    //verify that the filter has run
    QVERIFY(filterOutData.isProceedCalled());

    // compare signaled data

    // check if PoreCount is correct
    auto outPipeData = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("PoreCount"));
    QVERIFY(outPipeData);
    QCOMPARE(outPipeData->read( context.imageNumber()).ref().size() , expectedPoreCount.size() );
    for (size_t i= 0; i < outPipeData->read(context.imageNumber()).ref().size() && i < expectedPoreCount.size(); i++)
    {
#if TEXT_OUT
        std::cout << "Output pipe 'Pore Count': " << outPipeData->read(context.imageNumber()).ref().getData()[0] << "  exp: " << expectedPoreCount.at(i) << std::endl;
#endif
        QCOMPARE(outPipeData->read(context.imageNumber()).ref().getData(), expectedPoreCount);
        QCOMPARE(outPipeData->read(context.imageNumber()).ref().getData().size() , expectedPoreCount.size());
    }

    // check if maxSize is correct
    outPipeData = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("maxPoreSize"));
    QVERIFY(outPipeData);
    QCOMPARE(outPipeData->read( context.imageNumber()).ref().size() , expectedMaxPoreSize.size() );
    for (size_t i= 0; i < outPipeData->read(context.imageNumber()).ref().size() && i < expectedMaxPoreSize.size(); i++)
    {
#if TEXT_OUT
        std::cout << "Output pipe 'max size': " << outPipeData->read(context.imageNumber()).ref().getData()[0] << "  exp: " << expectedMaxPoreSize.at(i) << std::endl;
#endif
        for (size_t i = 0; i < expectedMaxPoreSize.size(); i++)
        {
            qFuzzyCompare(outPipeData->read(context.imageNumber()).ref().getData()[i], expectedMaxPoreSize[i]);
        }
        QCOMPARE(outPipeData->read(context.imageNumber()).ref().getData().size() , expectedMaxPoreSize.size());
    }

    // check if maxSize is correct
    outPipeData = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("minPoreSize"));
    QVERIFY(outPipeData);
    QCOMPARE(outPipeData->read( context.imageNumber()).ref().size() , expectedMinPoreSize.size() );
    for (size_t i= 0; i < outPipeData->read(context.imageNumber()).ref().size() && i < expectedMinPoreSize.size(); i++)
    {
#if TEXT_OUT
        std::cout << "Output pipe 'min size': " << outPipeData->read(context.imageNumber()).ref().getData()[0] << "  exp: " << expectedMinPoreSize.at(i) << std::endl;
#endif
        for (size_t i = 0; i < expectedMinPoreSize.size(); i++)
        {
            qFuzzyCompare(outPipeData->read(context.imageNumber()).ref().getData()[i], expectedMinPoreSize[i]);
        }
        QCOMPARE(outPipeData->read(context.imageNumber()).ref().getData().size() , expectedMinPoreSize.size());
    }
}

QTEST_GUILESS_MAIN(PoreClassifierOutputTripleTest)

#include "poreClassifierOutputTripleTest.moc"

