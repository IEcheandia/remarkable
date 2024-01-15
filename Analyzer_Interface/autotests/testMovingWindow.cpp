#include <QTest>
#include "filter/movingWindow.h"
#include "filter/algoArray.h"
#include "geo/array.h" 

Q_DECLARE_METATYPE(precitec::geo2d::Doublearray)

int g_BenchmarkFilterSize = 2;

class TestMovingWindow: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCompareOptimizedMethods_data();
    void testCompareOptimizedMethods();
   /*
    void benchmarkMedianGeneric();
    void benchmarkMedianOptimized();
    void benchmarkMeanGeneric();
    void benchmarkMeanOptimized();
    */
};

using precitec::filter::MovingWindow;
using precitec::geo2d::TArray;
using precitec::filter::calcMedian1d;
using precitec::geo2d::Doublearray;

void TestMovingWindow::testCompareOptimizedMethods_data()
{
    QTest::addColumn<int>("filterSize");
    QTest::addColumn<Doublearray>("inputArray");
    QTest::addColumn<bool>("passThroughBadRank");
    for (int filterSize : {1, 3, 4, 5,200})
    {
        for (bool passThroughBadRank: {true, false})
        {
            QTest::addRow("Empty_%d",  filterSize) << filterSize << Doublearray(0) << passThroughBadRank ;
            QTest::addRow("AllBadRank_%d",  filterSize) << filterSize << Doublearray(10) << passThroughBadRank;
            QTest::addRow("Constant_%d",  filterSize) << filterSize << Doublearray(12, -2.5,255) << passThroughBadRank;
            {            
                int outlierTrigger = 0;
                Doublearray input(600);
                for (int i =0 ; i < 600; ++i)
                {
                    if (i%2 == 0)
                    {
                        input.getData()[i] = (i %60) * 1.5 - 100 ;
                        input.getRank()[i] = 255;
                        ++ outlierTrigger;
                        if (outlierTrigger % 25==0)
                        {
                            input.getData()[i] =1.23;
                        }
                    }
                }
                QTest::addRow("Sampled_%d_%d",  filterSize, passThroughBadRank) << filterSize << input << passThroughBadRank;

            }
            {
                Doublearray input(44,0.1,255);
                double value = -3;
                int i = 2;
                for ( ; i < 10;++i)
                {
                    input.getData()[i] = ++value;
                }
                for (i =5; i < 15; i+=2)
                {
                    input.getData()[i] *= (-20);
                }
                
                for (; i < 30; i+=2)
                {
                    input.getData()[i] *= (-1.5);
                }
                
                for (i = 0; i < 44; ++i)
                {
                    if (i % 6 < 2)
                    {
                        input.getRank()[i] = 0;
                    }
                }
                QTest::addRow("Variable_%d_%d",  filterSize, passThroughBadRank) << filterSize << input << passThroughBadRank;
            }

            
        }
    }

}

void TestMovingWindow::testCompareOptimizedMethods()
{
    QFETCH(int, filterSize);
    QFETCH(bool, passThroughBadRank);
    QFETCH(Doublearray, inputArray);

    
    Doublearray outputMeanGeneric(inputArray.size());
    Doublearray outputMeanOptimized(inputArray.size());    
    
    MovingWindow<double> oMovingWindowMean(filterSize, precitec::filter::calcMean<double> , passThroughBadRank);
    oMovingWindowMean.processCentric(inputArray, outputMeanGeneric);
    
    MovingWindow<double>::movingMean(inputArray, outputMeanOptimized, filterSize, passThroughBadRank);
    
    //QCOMPARE(outputMeanNew.getData(), outputMeanOld.getData());
    //beacause of floating point arithmetic, we need some tolerance for comparing values (e.g 100 + 0.1 - 100 - 0.1 != 0)
    for (int i  = 0; i < (int)(outputMeanOptimized.size()); ++i)
    {
        QVERIFY(std::abs(outputMeanOptimized.getData()[i]- outputMeanGeneric.getData()[i]) < 5e-14);
    }    
    QCOMPARE(outputMeanOptimized.getRank(), outputMeanGeneric.getRank());
    

    
    Doublearray outputArrayOptimized(inputArray.size());
    std::vector<double> valuesInWindow(filterSize);
    MovingWindow<double>::movingMedian(inputArray, outputArrayOptimized, valuesInWindow, passThroughBadRank);

    
    Doublearray outputArrayGeneric(inputArray.size());
    MovingWindow<double> oMovingWindowMedian(filterSize, calcMedian1d<double> , passThroughBadRank);

    oMovingWindowMedian.processCentric(inputArray, outputArrayGeneric);

    QCOMPARE(outputArrayOptimized.getData(), outputArrayGeneric.getData());
    QCOMPARE(outputArrayOptimized.getRank(), outputArrayGeneric.getRank());

    

}

/*
void TestMovingWindow::benchmarkMedianGeneric()
{
    Doublearray input(44,0.1,255);
    double value = -3;
    int i = 2;
    for ( ; i < 10;++i)
    {
        input.getData()[i] = ++value;
    }
    for (i =5; i < 15; i+=2)
    {
        input.getData()[i] *= (-1.5);
    }

    for (; i < 30; i+=2)
    {
        input.getData()[i] *= (-1.5);
    }
    for (i = 0; i < 44; ++i)
    {
        if (i % 6 < 2)
        {
        input.getRank()[i] = 0;
        }
    }
    Doublearray outputArray(input.size());
    
    MovingWindow<double> obj(g_BenchmarkFilterSize, calcMedian1d<double> , true);

    
    QBENCHMARK
    {
        obj.processCentric(input, outputArray);
    }

}


void TestMovingWindow::benchmarkMedianOptimized()
{
    Doublearray input(44,0.1,255);
    double value = -3;
    int i = 2;
    for ( ; i < 10;++i)
    {
        input.getData()[i] = ++value;
    }
    for (i =5; i < 15; i+=2)
    {
        input.getData()[i] *= (-1.5);
    }

    for (; i < 30; i+=2)
    {
        input.getData()[i] *= (-1.5);
    }
    for (i = 0; i < 44; ++i)
    {
        if (i % 6 < 2)
        {
        input.getRank()[i] = 0;
        }
    }
    Doublearray outputArray(input.size());
    QBENCHMARK
    {
        MovingWindow<double>::movingMedian(input, outputArray, g_BenchmarkFilterSize, true);
    }

}


void TestMovingWindow::benchmarkMeanGeneric()
{
    Doublearray input(44,0.1,255);
    double value = -3;
    int i = 2;
    for ( ; i < 10;++i)
    {
        input.getData()[i] = ++value;
    }
    for (i =5; i < 15; i+=2)
    {
        input.getData()[i] *= (-1.5);
    }

    for (; i < 30; i+=2)
    {
        input.getData()[i] *= (-1.5);
    }
    for (i = 0; i < 44; ++i)
    {
        if (i % 6 < 2)
        {
        input.getRank()[i] = 0;
        }
    }
    Doublearray outputArray(input.size());
    
    MovingWindow<double> obj(g_BenchmarkFilterSize, precitec::filter::calcMean<double> , true);

    
    QBENCHMARK
    {
        obj.processCentric(input, outputArray);
    }

}


void TestMovingWindow::benchmarkMeanOptimized()
{
    Doublearray input(44,0.1,255);
    double value = -3;
    int i = 2;
    for ( ; i < 10;++i)
    {
        input.getData()[i] = ++value;
    }
    for (i =5; i < 15; i+=2)
    {
        input.getData()[i] *= (-1.5);
    }

    for (; i < 30; i+=2)
    {
        input.getData()[i] *= (-1.5);
    }
    for (i = 0; i < 44; ++i)
    {
        if (i % 6 < 2)
        {
        input.getRank()[i] = 0;
        }
    }
    Doublearray outputArray(input.size());
    QBENCHMARK
    {
        MovingWindow<double>::movingMean(input, outputArray, g_BenchmarkFilterSize, true);
    }

}

*/



QTEST_MAIN(TestMovingWindow)
#include "testMovingWindow.moc"
