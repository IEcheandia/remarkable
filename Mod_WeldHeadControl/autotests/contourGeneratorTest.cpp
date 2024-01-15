#include <QTest>

#include "../include/viWeldHead/Scanlab/contourGenerator.h"
#include <typeinfo>
#include <iostream>

using precitec::hardware::ContourGenerator;

/**
* Tests the code of the contour generator.
**/
class ContourGeneratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testInitialize();
    void testAddMark();
    void testAddJump();
    void testAddLaserPower();
    void testAddMarkSpeed();
    void testResetContour();
    void testAddDifferentTypes();
};

void ContourGeneratorTest::testCtor()
{
    ContourGenerator generator;
    QVERIFY(generator.empty());
    const auto& contour = generator.generatedContour();
    QVERIFY(contour.empty());
}

void ContourGeneratorTest::testInitialize()
{
    ContourGenerator generator;

    QVERIFY(generator.empty());

    generator.addInitialize();

    QVERIFY(!generator.empty());

    const auto& contour = generator.generatedContour();
    QCOMPARE(contour.size(), 1);
}

void ContourGeneratorTest::testAddMark()
{
    ContourGenerator generator;

    QVERIFY(generator.empty());

    std::size_t newContourSize = 3;
    auto xCalculation = [](const auto& i)
    {
        return i * 0.25;
    };
    auto yCalculation = [](const auto& i)
    {
        return i * -0.5 + 0.5;
    };
    for (std::size_t i = 0; i < newContourSize; i++)
    {
        generator.addMark(xCalculation(i), yCalculation(i));
    }
    QVERIFY(!generator.empty());
    const auto& contour = generator.generatedContour();
    QCOMPARE(contour.size(), newContourSize);
    for (std::size_t i = 0; i < contour.size(); i++)
    {
        const auto& command = contour.at(i);
        if (const auto mark = std::dynamic_pointer_cast<precitec::hardware::contour::Mark>(command))
        {
            QCOMPARE(mark->x, xCalculation(i));
            QCOMPARE(mark->y, yCalculation(i));
        }
    }
}

void ContourGeneratorTest::testAddJump()
{
    ContourGenerator generator;

    QVERIFY(generator.empty());

    std::size_t newContourSize = 5;
    auto xCalculation = [](const auto& i)
    {
        return i * 0.5;
    };
    auto yCalculation = [](const auto& i)
    {
        return i * -1.5 + 5.5;
    };
    for (std::size_t i = 0; i < newContourSize; i++)
    {
        generator.addJump(xCalculation(i), yCalculation(i));
    }
    QVERIFY(!generator.empty());

    const auto& contour = generator.generatedContour();
    QCOMPARE(contour.size(), newContourSize);
    for (std::size_t i = 0; i < contour.size(); i++)
    {
        const auto& command = contour.at(i);
        if (const auto jump = std::dynamic_pointer_cast<precitec::hardware::contour::Jump>(command))
        {
            QCOMPARE(jump->x, xCalculation(i));
            QCOMPARE(jump->y, yCalculation(i));
        }
    }
}

void ContourGeneratorTest::testAddLaserPower()
{
    ContourGenerator generator;

    QVERIFY(generator.empty());

    std::size_t newContourSize = 6;
    auto powerCalculation = [](const auto& i)
    {
        return 0.2 * i + 0.0;
    };
    for (std::size_t i = 0; i < newContourSize; i++)
    {
        generator.addLaserPower(powerCalculation(i));
    }
    QVERIFY(!generator.empty());

    const auto& contour = generator.generatedContour();
    QCOMPARE(contour.size(), newContourSize);
    for (std::size_t i = 0; i < contour.size(); i++)
    {
        const auto& command = contour.at(i);
        if (const auto laserPower = std::dynamic_pointer_cast<precitec::hardware::contour::LaserPower>(command))
        {
            QCOMPARE(laserPower->power, powerCalculation(i));
        }
    }
}

void ContourGeneratorTest::testAddMarkSpeed()
{
    ContourGenerator generator;

    QVERIFY(generator.empty());

    std::size_t newContourSize = 4;
    auto speedCalculation = [](const auto& i)
    {
        return 100.0 * i;
    };
    for (std::size_t i = 0; i < newContourSize; i++)
    {
        generator.addMarkSpeed(speedCalculation(i));
    }
    QVERIFY(!generator.empty());

    const auto& contour = generator.generatedContour();
    QCOMPARE(contour.size(), newContourSize);
    for (std::size_t i = 0; i < contour.size(); i++)
    {
        const auto& command = contour.at(i);
        if (const auto markSpeed = std::dynamic_pointer_cast<precitec::hardware::contour::MarkSpeed>(command))
        {
            QCOMPARE(markSpeed->speed, speedCalculation(i));
        }
    }
}

void ContourGeneratorTest::testResetContour()
{
    ContourGenerator generator;

    QVERIFY(generator.empty());
    generator.addJump(1.0, 1.0);
    QVERIFY(!generator.empty());
}

void ContourGeneratorTest::testAddDifferentTypes()
{
    ContourGenerator generator;

    QVERIFY(generator.empty());

    generator.addJump(100.0, 50.0);
    generator.addLaserPower(0.5);
    generator.addMarkSpeed(1000.0);
    generator.addMark(125.0, 75.0);
    generator.addMark(150.0, 100.0);
    generator.addMark(175.0, 125.0);

    QVERIFY(!generator.empty());
    const auto& contour = generator.generatedContour();
    QCOMPARE(contour.size(), 6);

    auto createMarkPositions = [](const auto& i)
    {
        return std::make_pair(25.0 * i + 125.0, 25.0 * i + 75.0);
    };
    int markCounter = 0;
    for (const auto& command : contour)
    {
        if (const auto mark = std::dynamic_pointer_cast<precitec::hardware::contour::Mark>(command))
        {
            const auto& targetPosition = createMarkPositions(markCounter);
            QCOMPARE(mark->x, targetPosition.first);
            QCOMPARE(mark->y, targetPosition.second);
            markCounter++;
        }
        else if (const auto jump = std::dynamic_pointer_cast<precitec::hardware::contour::Jump>(command))
        {
            QCOMPARE(jump->x, 100.0);
            QCOMPARE(jump->y, 50.0);
        }
        else if (const auto laserPower = std::dynamic_pointer_cast<precitec::hardware::contour::LaserPower>(command))
        {
            QCOMPARE(laserPower->power, 0.5);
        }
        else if (const auto markspeed = std::dynamic_pointer_cast<precitec::hardware::contour::MarkSpeed>(command))
        {
            QCOMPARE(markspeed->speed, 1000.0);
        }
        else
        {
            std::cout << "Something went wrong!" << std::endl;
        }
    }
}

QTEST_GUILESS_MAIN(ContourGeneratorTest)
#include "contourGeneratorTest.moc"
