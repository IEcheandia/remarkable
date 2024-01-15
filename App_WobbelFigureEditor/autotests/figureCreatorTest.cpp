#include <QTest>

#include "../src/FigureCreator.h"
#include "../src/FileModel.h"

using precitec::scantracker::components::wobbleFigureEditor::FigureCreator;

namespace testHelper
{
struct TestHelper
{
    QPointF limitPrecisionToThreeDigits(const QPointF& point)
    {
        QPoint pointCopy = {qRound(point.x() * 1000), qRound(point.y() * 1000)};
        return QPointF {pointCopy.x() / 1000.0, pointCopy.y() / 1000.0};
    }
};
}

class FigureCreatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testApplyTranslation();
    void testEight_data();
    void testEight();
};

void FigureCreatorTest::testCtor()
{
    FigureCreator creator;

    QCOMPARE(creator.fileModel(), nullptr);
    QCOMPARE(creator.figureShape(), FigureCreator::Line);
    QCOMPARE(creator.pointNumber(), 100);
    QCOMPARE(creator.alpha(), 0.0);
    QCOMPARE(creator.coordinateStart(), QPointF{});
    QCOMPARE(creator.amplitude(), 0.0);
    QCOMPARE(creator.phaseShift(), 0.0);
    QCOMPARE(creator.frequence(), 0.0);
    QCOMPARE(creator.frequenceVer(), 0.0);
    QCOMPARE(creator.windings(), 3.0);
    QCOMPARE(creator.coordinateCenter(), QPointF{});
    QCOMPARE(creator.angle(), 360.0);
    QCOMPARE(creator.figureWidth(), 3.0);
    QCOMPARE(creator.figureHeight(), 6.0);
    QCOMPARE(creator.coordinateEnd(), QPointF(3.0, 0.0));
    QCOMPARE(creator.length(), 3.0);
    QCOMPARE(creator.interference(), 2);
    QCOMPARE(creator.freeInterference(), -1);
    QCOMPARE(creator.overlayFilename(), QString{});
}

void FigureCreatorTest::testApplyTranslation()
{
    FigureCreator creator;
    testHelper::TestHelper helper;

    QPointF origin{0.0, 0.0};
    QPointF translation{0.0, 0.0};

    QCOMPARE(origin, QPointF{});
    QCOMPARE(translation, QPointF{});

    translation.setX(10.0);
    QCOMPARE(helper.limitPrecisionToThreeDigits(creator.applyTranslation(origin, translation)), QPointF(10.0, 0.0));

    translation.setY(75.25);
    QCOMPARE(helper.limitPrecisionToThreeDigits(creator.applyTranslation(origin, translation)), QPointF(10.0, 75.25));

    translation.setX(-5.55);
    QCOMPARE(helper.limitPrecisionToThreeDigits(creator.applyTranslation(origin, translation)), QPointF(-5.55, 75.25));

    translation.setY(-0.5);
    QCOMPARE(helper.limitPrecisionToThreeDigits(creator.applyTranslation(origin, translation)), QPointF(-5.55, -0.5));

    QPointF point{-2.0, 5.0};
    QCOMPARE(helper.limitPrecisionToThreeDigits(creator.applyTranslation(point, translation)), QPointF(-7.55, 4.5));

    translation.setX(9.0);
    QCOMPARE(helper.limitPrecisionToThreeDigits(creator.applyTranslation(point, translation)), QPointF(7.0, 4.5));
}

void FigureCreatorTest::testEight_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("Eight1") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{-0.643, 0.234},
        QPointF{-0.985, 0.826},
        QPointF{-0.866, 1.5},
        QPointF{-0.342, 1.94},
        QPointF{0.342, 1.94},
        QPointF{0.866, 1.5},
        QPointF{0.985, 0.826},
        QPointF{0.643, 0.234},
        QPointF{0.0, 0.0},
        QPointF{-0.643, -0.234},
        QPointF{-0.985, -0.826},
        QPointF{-0.866, -1.5},
        QPointF{-0.342, -1.94},
        QPointF{0.342, -1.94},
        QPointF{0.866, -1.5},
        QPointF{0.985, -0.826},
        QPointF{0.643, -0.234},
        QPointF{0.0, 0.0}
    };
}

void FigureCreatorTest::testEight()
{
    FigureCreator creator;
    testHelper::TestHelper helper;

    creator.createEight(19, 2.0, QPointF{0.0, 0.0});
    const auto& createdFigure = creator.getFigure();

    QFETCH(QVector<QPointF>, figure);
    QCOMPARE(figure.size(), createdFigure->size());

    for (std::size_t i = 0; i < createdFigure->size(); i++)
    {
        QCOMPARE(helper.limitPrecisionToThreeDigits(createdFigure->at(i)), figure.at(i));
    }
}

QTEST_GUILESS_MAIN(FigureCreatorTest)
#include "figureCreatorTest.moc"
