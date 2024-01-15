#include <QTest>
#include <QSignalSpy>

#include "../src/referenceErrorConfigController.h"
#include "seamError.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "referenceCurve.h"
#include "referenceCurveData.h"
#include "precitec/dataSet.h"


using precitec::gui::ReferenceErrorConfigController;
using precitec::gui::components::plotter::DataSet;
using precitec::storage::SeamError;
using precitec::storage::Product;

class ReferenceErrorConfigControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSeamError();
    void testVisualReference();
    void testReferenceCurve();
    void testBoundary();
};

void ReferenceErrorConfigControllerTest::testCtor()
{
    ReferenceErrorConfigController control{this};

    QVERIFY(!control.seamError());
    QVERIFY(control.visualReference());
    QVERIFY(control.visualReference()->isEmpty());
    QCOMPARE(control.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(control.visualReference()->drawingOrder(), DataSet::DrawingOrder::OnTop);

    QCOMPARE(control.lowerShadow()->name(), QStringLiteral("Min"));
    QCOMPARE(control.lowerShadow()->color(), QColor("darkturquoise"));

    QCOMPARE(control.upperShadow()->name(), QStringLiteral("Max"));
    QCOMPARE(control.upperShadow()->color(), QColor("darkturquoise"));

    QCOMPARE(control.lowerReference()->name(), QStringLiteral("Lower Reference"));
    QCOMPARE(control.lowerReference()->color(), QColor("magenta"));

    QCOMPARE(control.middleReference()->name(), QStringLiteral("Middle Reference"));
    QCOMPARE(control.middleReference()->color(), QColor("darkmagenta"));

    QCOMPARE(control.upperReference()->name(), QStringLiteral("Upper Reference"));
    QCOMPARE(control.upperReference()->color(), QColor("magenta"));
}

void ReferenceErrorConfigControllerTest::testSeamError()
{
    ReferenceErrorConfigController control{this};
    QVERIFY(!control.seamError());

    QSignalSpy seamErrorChangedSpy(&control, &ReferenceErrorConfigController::seamErrorChanged);
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

void ReferenceErrorConfigControllerTest::testVisualReference()
{
    ReferenceErrorConfigController control{this};
    QVERIFY(control.visualReference());
    QVERIFY(control.visualReference()->isEmpty());
    QCOMPARE(control.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(control.visualReference()->drawingOrder(), DataSet::DrawingOrder::OnTop);

    QSignalSpy visualReferenceChangedSpy(&control, &ReferenceErrorConfigController::visualReferenceChanged);
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

void ReferenceErrorConfigControllerTest::testReferenceCurve()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    const auto seam = seamSeries->createSeam();

    const auto reference = seam->createReferenceCurve(15);
    product->setReferenceCurveData(reference->lower(), std::vector<QVector2D>{{0, -1}, {1, 2.3f}});
    product->setReferenceCurveData(reference->middle(), std::vector<QVector2D>{{0, 0}, {1, 1.4f}, {2, 2.3f}});
    product->setReferenceCurveData(reference->upper(), std::vector<QVector2D>{{0, 1}, {1, -0.2f}, {2, 1.3f}, {4, 0.3}});

    SeamError error;
    error.setMeasureTask(seam);
    error.setResultValue(15);

    ReferenceErrorConfigController control{this};
    QSignalSpy referenceChangedSpy(&control, &ReferenceErrorConfigController::referenceChanged);
    QVERIFY(referenceChangedSpy.isValid());
    control.setCurrentProduct(product);
    QCOMPARE(referenceChangedSpy.count(), 0);
    control.setSeamError(&error);
    QCOMPARE(referenceChangedSpy.count(), 1);

    QCOMPARE(control.lowerReference()->name(), QStringLiteral("Lower Reference"));
    QCOMPARE(control.lowerReference()->color(), QColor("magenta"));
    QVERIFY(control.lowerReference()->isEmpty());

    QCOMPARE(control.middleReference()->name(), QStringLiteral("Middle Reference"));
    QCOMPARE(control.middleReference()->color(), QColor("darkmagenta"));
    QVERIFY(control.middleReference()->isEmpty());

    QCOMPARE(control.upperReference()->name(), QStringLiteral("Upper Reference"));
    QCOMPARE(control.upperReference()->color(), QColor("magenta"));
    QVERIFY(control.upperReference()->isEmpty());

    error.setEnvelope(reference->uuid());
    QCOMPARE(error.envelope(), reference->uuid());
    QCOMPARE(referenceChangedSpy.count(), 2);

    QCOMPARE(control.lowerReference()->sampleCount(), 2);
    QVERIFY(!control.lowerReference()->isEnabled());
    QCOMPARE(control.middleReference()->sampleCount(), 3);
    QVERIFY(control.middleReference()->isEnabled());
    QCOMPARE(control.upperReference()->sampleCount(), 4);
    QVERIFY(!control.upperReference()->isEnabled());

    error.setUseMiddleCurveAsReference(false);
    QVERIFY(control.lowerReference()->isEnabled());
    QVERIFY(!control.middleReference()->isEnabled());
    QVERIFY(control.upperReference()->isEnabled());

    error.setEnvelope(QUuid::createUuid());
    QCOMPARE(referenceChangedSpy.count(), 3);
    QVERIFY(control.lowerReference()->isEmpty());
    QVERIFY(control.middleReference()->isEmpty());
    QVERIFY(control.upperReference()->isEmpty());
}

void ReferenceErrorConfigControllerTest::testBoundary()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    const auto seam = seamSeries->createSeam();

    const auto reference = seam->createReferenceCurve(15);
    product->setReferenceCurveData(reference->lower(), std::vector<QVector2D>{{0, -1}, {1, 2.3f}});
    product->setReferenceCurveData(reference->middle(), std::vector<QVector2D>{{0, 0}, {1, 1.4f}, {2, 2.3f}});
    product->setReferenceCurveData(reference->upper(), std::vector<QVector2D>{{0, 1}, {1, -0.2f}, {2, 1.3f}, {4, 0.3}});

    SeamError error;
    error.setMeasureTask(seam);
    error.setResultValue(15);

    ReferenceErrorConfigController control{this};
    QSignalSpy referenceChangedSpy(&control, &ReferenceErrorConfigController::referenceChanged);
    QVERIFY(referenceChangedSpy.isValid());
    control.setCurrentProduct(product);
    QCOMPARE(referenceChangedSpy.count(), 0);
    control.setSeamError(&error);
    QCOMPARE(referenceChangedSpy.count(), 1);

    QCOMPARE(control.lowerReference()->name(), QStringLiteral("Lower Reference"));
    QCOMPARE(control.lowerReference()->color(), QColor("magenta"));
    QVERIFY(control.lowerReference()->isEmpty());

    QCOMPARE(control.middleReference()->name(), QStringLiteral("Middle Reference"));
    QCOMPARE(control.middleReference()->color(), QColor("darkmagenta"));
    QVERIFY(control.middleReference()->isEmpty());

    QCOMPARE(control.upperReference()->name(), QStringLiteral("Upper Reference"));
    QCOMPARE(control.upperReference()->color(), QColor("magenta"));
    QVERIFY(control.upperReference()->isEmpty());

    error.setEnvelope(reference->uuid());
    QCOMPARE(error.envelope(), reference->uuid());
    QCOMPARE(referenceChangedSpy.count(), 2);

    QCOMPARE(control.lowerReference()->sampleCount(), 2);
    QVERIFY(!control.lowerReference()->isEnabled());
    QCOMPARE(control.middleReference()->sampleCount(), 3);
    QVERIFY(control.middleReference()->isEnabled());
    QCOMPARE(control.upperReference()->sampleCount(), 4);
    QVERIFY(!control.upperReference()->isEnabled());

    QCOMPARE(control.lowerShadow()->sampleCount(), 3);
    QCOMPARE(control.upperShadow()->sampleCount(), 3);

    error.setUseMiddleCurveAsReference(false);

    QCOMPARE(control.lowerShadow()->sampleCount(), 2);
    QCOMPARE(control.upperShadow()->sampleCount(), 4);

    error.setMin(3);
    QCOMPARE(control.lowerShadow()->offset().y(), -3.0f);

    error.setMax(2);
    QCOMPARE(control.upperShadow()->offset().y(), 2.0f);

    control.setSeamError(nullptr);
    QVERIFY(control.lowerReference()->isEmpty());
    QVERIFY(control.middleReference()->isEmpty());
    QVERIFY(control.upperReference()->isEmpty());
    QVERIFY(control.lowerShadow()->isEmpty());
    QVERIFY(control.upperShadow()->isEmpty());
}

QTEST_GUILESS_MAIN(ReferenceErrorConfigControllerTest)
#include "referenceErrorConfigControllerTest.moc"
