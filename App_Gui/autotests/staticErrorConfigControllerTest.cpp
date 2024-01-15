#include <QTest>
#include <QSignalSpy>

#include "../src/staticErrorConfigController.h"
#include "seamError.h"
#include "precitec/dataSet.h"
#include "precitec/infiniteSet.h"


using precitec::gui::StaticErrorConfigController;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::components::plotter::InfiniteSet;
using precitec::storage::SeamError;

class StaticErrorConfigControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSeamError();
    void testVisualReference();
    void testLowerBoundary();
    void testUpperBoundary();
    void testMinMaxFromReference();
};

void StaticErrorConfigControllerTest::testCtor()
{
    StaticErrorConfigController control{this};

    QVERIFY(!control.seamError());
    QVERIFY(control.visualReference());
    QVERIFY(control.visualReference()->isEmpty());
    QCOMPARE(control.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(control.visualReference()->drawingOrder(), DataSet::DrawingOrder::OnTop);

    QCOMPARE(control.lowerBoundary()->name(), QStringLiteral("Min"));
    QCOMPARE(control.lowerBoundary()->color(), QColor("magenta"));

    QCOMPARE(control.upperBoundary()->name(), QStringLiteral("Max"));
    QCOMPARE(control.upperBoundary()->color(), QColor("magenta"));

    QCOMPARE(control.shiftedLowerBoundary()->name(), QStringLiteral("Min Shift"));
    QCOMPARE(control.shiftedLowerBoundary()->color(), QColor("magenta"));

    QCOMPARE(control.shiftedUpperBoundary()->name(), QStringLiteral("Max Shift"));
    QCOMPARE(control.shiftedUpperBoundary()->color(), QColor("magenta"));
}

void StaticErrorConfigControllerTest::testSeamError()
{
    StaticErrorConfigController control{this};
    QVERIFY(!control.seamError());

    QSignalSpy seamErrorChangedSpy(&control, &StaticErrorConfigController::seamErrorChanged);
    QVERIFY(seamErrorChangedSpy.isValid());

    QCOMPARE(seamErrorChangedSpy.count(), 0);

    auto seamError = new SeamError{this};
    QSignalSpy destroyedSpy(seamError, &SeamError::destroyed);
    control.setSeamError(seamError);
    QCOMPARE(control.seamError(), seamError);
    QCOMPARE(seamErrorChangedSpy.count(), 1);

    control.setSeamError(seamError);
    QCOMPARE(seamErrorChangedSpy.count(), 1);

    seamError->deleteLater();
    QVERIFY(destroyedSpy.wait());
    QVERIFY(!control.seamError());
    QCOMPARE(seamErrorChangedSpy.count(), 2);
}

void StaticErrorConfigControllerTest::testVisualReference()
{
    StaticErrorConfigController control{this};
    QVERIFY(control.visualReference());
    QVERIFY(control.visualReference()->isEmpty());
    QCOMPARE(control.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(control.visualReference()->drawingOrder(), DataSet::DrawingOrder::OnTop);

    QSignalSpy visualReferenceChangedSpy(&control, &StaticErrorConfigController::visualReferenceChanged);
    QVERIFY(visualReferenceChangedSpy.isValid());

    auto dataSet = new DataSet{this};
    dataSet->setName(QStringLiteral("Some Result"));
    dataSet->setColor("blue");
    dataSet->addSample(QVector2D{1.3, 3.6});
    dataSet->addSample(QVector2D{2.4, 7.5});
    dataSet->addSample(QVector2D{3.6,-2.3});

    control.setVisualReference(dataSet);
    QCOMPARE(visualReferenceChangedSpy.count(), 1);
    QCOMPARE(control.visualReference()->sampleCount(), 3);
    QCOMPARE(control.visualReference()->name(), QStringLiteral("Some Result"));
    QCOMPARE(control.visualReference()->color(), QColor{"blue"});

    control.setVisualReference(nullptr);
    QCOMPARE(visualReferenceChangedSpy.count(), 2);
    QVERIFY(control.visualReference()->isEmpty());
    QCOMPARE(control.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(control.visualReference()->color(), QColor{"white"});

    auto seamError = new SeamError{this};
    control.setSeamError(seamError);
    control.setVisualReference(dataSet);
    QCOMPARE(visualReferenceChangedSpy.count(), 3);
    QCOMPARE(control.visualReference()->sampleCount(), 3);
    QCOMPARE(control.visualReference()->name(), QStringLiteral("Some Result"));
    QCOMPARE(control.visualReference()->color(), QColor{"blue"});

    seamError->setResultValue(15);
    QCOMPARE(visualReferenceChangedSpy.count(), 4);
    QVERIFY(control.visualReference()->isEmpty());
    QCOMPARE(control.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(control.visualReference()->color(), QColor{"white"});
}

void StaticErrorConfigControllerTest::testLowerBoundary()
{
    StaticErrorConfigController control{this};

    QCOMPARE(control.lowerBoundary()->name(), QStringLiteral("Min"));
    QCOMPARE(control.lowerBoundary()->color(), QColor("magenta"));

    QCOMPARE(control.shiftedLowerBoundary()->name(), QStringLiteral("Min Shift"));
    QCOMPARE(control.shiftedLowerBoundary()->color(), QColor("magenta"));

    auto seamError = new SeamError{this};
    control.setSeamError(seamError);

    QVERIFY(control.lowerBoundary()->isEnabled());
    QVERIFY(!control.shiftedLowerBoundary()->isEnabled());

    seamError->setMin(5.3);

    QVERIFY(control.lowerBoundary()->isEnabled());
    QVERIFY(!control.shiftedLowerBoundary()->isEnabled());
    QCOMPARE(control.lowerBoundary()->color(), QColor("magenta"));
    QCOMPARE(control.shiftedLowerBoundary()->color(), QColor("magenta"));
    QCOMPARE(control.lowerBoundary()->value(),5.3f);
    QCOMPARE(control.shiftedLowerBoundary()->value(), 5.3f);

    seamError->setShift(3.4);

    QVERIFY(control.lowerBoundary()->isEnabled());
    QVERIFY(control.shiftedLowerBoundary()->isEnabled());
    QCOMPARE(control.lowerBoundary()->color(), QColor("#32ff00ff"));
    QCOMPARE(control.shiftedLowerBoundary()->color(), QColor("magenta"));
    QCOMPARE(control.lowerBoundary()->value(), 5.3f);
    QCOMPARE(control.shiftedLowerBoundary()->value(), 5.3f + 3.4f);

    control.setSeamError(nullptr);
    QVERIFY(!control.lowerBoundary()->isEnabled());
    QVERIFY(!control.shiftedLowerBoundary()->isEnabled());
}

void StaticErrorConfigControllerTest::testUpperBoundary()
{
    StaticErrorConfigController control{this};

    QCOMPARE(control.upperBoundary()->name(), QStringLiteral("Max"));
    QCOMPARE(control.upperBoundary()->color(), QColor("magenta"));

    QCOMPARE(control.shiftedUpperBoundary()->name(), QStringLiteral("Max Shift"));
    QCOMPARE(control.shiftedUpperBoundary()->color(), QColor("magenta"));

    auto seamError = new SeamError{this};
    control.setSeamError(seamError);

    QVERIFY(control.upperBoundary()->isEnabled());
    QVERIFY(!control.shiftedUpperBoundary()->isEnabled());

    seamError->setMax(5.3);

    QVERIFY(control.upperBoundary()->isEnabled());
    QVERIFY(!control.shiftedUpperBoundary()->isEnabled());
    QCOMPARE(control.upperBoundary()->color(), QColor("magenta"));
    QCOMPARE(control.shiftedUpperBoundary()->color(), QColor("magenta"));
    QCOMPARE(control.upperBoundary()->value(), 5.3f);
    QCOMPARE(control.shiftedUpperBoundary()->value(), 5.3f);

    seamError->setShift(3.4);

    QVERIFY(control.upperBoundary()->isEnabled());
    QVERIFY(control.shiftedUpperBoundary()->isEnabled());
    QCOMPARE(control.upperBoundary()->color(), QColor("#32ff00ff"));
    QCOMPARE(control.shiftedUpperBoundary()->color(), QColor("magenta"));
    QCOMPARE(control.upperBoundary()->value(),5.3f);
    QCOMPARE(control.shiftedUpperBoundary()->value(), 5.3f + 3.4f);

    control.setSeamError(nullptr);
    QVERIFY(!control.upperBoundary()->isEnabled());
    QVERIFY(!control.shiftedUpperBoundary()->isEnabled());
}

void StaticErrorConfigControllerTest::testMinMaxFromReference()
{
    StaticErrorConfigController control{this};

    QSignalSpy visualReferenceChangedSpy(&control, &StaticErrorConfigController::visualReferenceChanged);
    QVERIFY(visualReferenceChangedSpy.isValid());

    auto error = new SeamError{this};
    QSignalSpy minChangedSpy(error, &SeamError::minChanged);
    QVERIFY(minChangedSpy.isValid());
    QSignalSpy maxChangedSpy(error, &SeamError::maxChanged);
    QVERIFY(maxChangedSpy.isValid());

    control.setSeamError(error);
    error->setShift(2.0);

    control.setMinFromReference();
    QCOMPARE(minChangedSpy.count(), 0);

    control.setMaxFromReference();
    QCOMPARE(maxChangedSpy.count(), 0);

    auto dataSet = new DataSet{this};
    dataSet->addSample(QVector2D{1.3, 3.6});
    dataSet->addSample(QVector2D{2.4, 7.5});
    dataSet->addSample(QVector2D{3.6,-2.3});

    control.setVisualReference(dataSet);

    control.setMinFromReference();
    QCOMPARE(minChangedSpy.count(), 1);
    QCOMPARE(error->min(), -2.3f);
    QCOMPARE(control.lowerBoundary()->value(), -2.3f);
    QCOMPARE(control.shiftedLowerBoundary()->value(), -0.3f);


    control.setMaxFromReference();
    QCOMPARE(maxChangedSpy.count(), 1);
    QCOMPARE(error->max(), 7.5f);
    QCOMPARE(control.upperBoundary()->value(), 7.5f);
    QCOMPARE(control.shiftedUpperBoundary()->value(), 9.5f);
}

QTEST_GUILESS_MAIN(StaticErrorConfigControllerTest)
#include "staticErrorConfigControllerTest.moc"
