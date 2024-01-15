#include <QTest>
#include <fliplib/util/dummyFilter.h>
#include "../lineArithmetic.h"

class LineArithmeticTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testProceed_data();
    void testProceed();
};

using precitec::geo2d::Doublearray;
using precitec::geo2d::VecDoublearray;
using precitec::interface::GeoVecDoublearray;
using precitec::interface::GeoDoublearray;

Q_DECLARE_METATYPE(Doublearray)
Q_DECLARE_METATYPE(VecDoublearray)


void LineArithmeticTest::testProceed_data()
{
    using namespace precitec::interface;
    using namespace precitec::geo2d;

    QTest::addColumn<int>("parameter_operation");
    QTest::addColumn<VecDoublearray>("inputLine");
    QTest::addColumn<Doublearray>("inputValue");
    QTest::addColumn<VecDoublearray>("expectedResult");

    Doublearray line1 (50, 0, 255);
    Doublearray line2 (50, 0, 255);
    for (int i = 0; i < 50; i++)
    {
        line1.getData()[i] = std::sin(i/10.0);
        line2.getData()[i] = std::cos(i/10.0);
    }
    line1.getRank()[10] = 0;

    VecDoublearray inputLines{line1, line2};
    {
        Doublearray singleInputValue(1, 0.5, 255);
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = 0.5 + line1.getData()[i];
                expectedLine2.getData()[i] = 0.5 + line2.getData()[i];
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("addition") << 0 << inputLines << singleInputValue << expectedResult;
        }
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = 0.5 * line1.getData()[i];
                expectedLine2.getData()[i] = 0.5 * line2.getData()[i];
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("multiplication") << 1 << inputLines << singleInputValue << expectedResult;
        }
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = std::min(0.5, line1.getData()[i]);
                expectedLine2.getData()[i] = std::min(0.5, line2.getData()[i]);
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("minimum") << 2 << inputLines << singleInputValue << expectedResult;
        }
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = std::max(0.5, line1.getData()[i]);
                expectedLine2.getData()[i] = std::max(0.5, line2.getData()[i]);
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("maximum") << 3 << inputLines << singleInputValue << expectedResult;
        }
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = std::abs(0.5 - line1.getData()[i]);
                expectedLine2.getData()[i] = std::abs(0.5 - line2.getData()[i]);
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("delta") << 4 << inputLines << singleInputValue << expectedResult;
        }
    }
    {
        Doublearray inputValues;
        inputValues.push_back(-1.0, 255);
        inputValues.push_back(0.3, 255);
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = -1.0 + line1.getData()[i];
                expectedLine2.getData()[i] = 0.3 + line2.getData()[i];
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("addition") << 0 << inputLines << inputValues << expectedResult;
        }
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = -1.0 * line1.getData()[i];
                expectedLine2.getData()[i] = 0.3 * line2.getData()[i];
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("multiplication") << 1 << inputLines << inputValues << expectedResult;
        }
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = std::min(-1.0, line1.getData()[i]);
                expectedLine2.getData()[i] = std::min(0.3, line2.getData()[i]);
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("minimum") << 2 << inputLines << inputValues << expectedResult;
        }
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = std::max(-1.0, line1.getData()[i]);
                expectedLine2.getData()[i] = std::max(0.3, line2.getData()[i]);
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("maximum") << 3 << inputLines << inputValues << expectedResult;
        }
        {
            Doublearray expectedLine1 = line1;
            Doublearray expectedLine2 = line2;
            for (int i = 0; i < 50; i++)
            {
                expectedLine1.getData()[i] = std::abs(-1.0 - line1.getData()[i]);
                expectedLine2.getData()[i] = std::abs(0.3 - line2.getData()[i]);
            }
            VecDoublearray expectedResult{expectedLine1, expectedLine2};
            QTest::addRow("delta") << 4 << inputLines << inputValues << expectedResult;
        }
    }
}

void LineArithmeticTest::testProceed()
{
    QFETCH(VecDoublearray, inputLine);
    QFETCH(Doublearray, inputValue);
    QFETCH(int, parameter_operation);
    QFETCH(VecDoublearray, expectedResult);

    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<GeoVecDoublearray> inputPipeProfiles{&sourceFilter, "Profiles"};
    fliplib::SynchronePipe<GeoDoublearray> inputPipeValue{&sourceFilter, "Value"};

    precitec::filter::LineArithmetic filter;
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<GeoVecDoublearray>*>(filter.findPipe("Result"));
    QVERIFY(outPipe);
    QVERIFY(filter.connectPipe(&inputPipeProfiles, 1));
    QVERIFY(filter.connectPipe(&inputPipeValue, 1));
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipe, 1));

    filter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, parameter_operation);
    filter.setParameter();

    QVERIFY(!dummyFilter.isProceedCalled());
    precitec::interface::ImageContext context;
    GeoVecDoublearray geoLines (context, inputLine, precitec::interface::AnalysisOK, 1.0);
    GeoDoublearray geoValue (context, inputValue, precitec::interface::AnalysisOK, 1.0);
    inputPipeProfiles.signal(geoLines);
    inputPipeValue.signal(geoValue);
    QVERIFY(dummyFilter.isProceedCalled());

    auto& geoResult = outPipe->read(0);
    auto& result = geoResult.ref();
    QCOMPARE(result.size(), expectedResult.size());
    for (unsigned int i = 0; i< expectedResult.size(); i++)
    {
        QCOMPARE(result[i].getData(), expectedResult[i].getData());
        QCOMPARE(result[i].getRank(), expectedResult[i].getRank());
    }

}








QTEST_GUILESS_MAIN(LineArithmeticTest)

#include "testLineArithmetic.moc"
