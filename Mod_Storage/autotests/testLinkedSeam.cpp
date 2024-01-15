#include <QTest>
#include <QSignalSpy>

#include "../src/copyMode.h"
#include "../src/product.h"
#include "../src/seamSeries.h"
#include "../src/seam.h"
#include "../src/linkedSeam.h"

using precitec::storage::CopyMode;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;
using precitec::storage::LinkedSeam;

class TestLinkedSeam : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreateLinkedSeam();
    void testDuplicateLinkedSeam_data();
    void testDuplicateLinkedSeam();
    void testDestroySeam();
    void testSaveAndLoad();
};

void TestLinkedSeam::testCreateLinkedSeam()
{
    // first create a Product with one SeamSeries and two Seams
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto *seamSeries = p.seamSeries().front();
    auto *seam1 = p.createSeam();
    auto *seam2 = p.createSeam();

    QCOMPARE(seamSeries->seams().size(), 2u);

    QSignalSpy seamsChangedSpy{seamSeries, &SeamSeries::seamsChanged};
    QVERIFY(seamsChangedSpy.isValid());
    QSignalSpy linkedSeamsChangedSpy{seam1, &Seam::linkedSeamsChanged};
    QVERIFY(linkedSeamsChangedSpy.isValid());

    auto *firstLink = dynamic_cast<LinkedSeam*>(seamSeries->createSeamLink(seam1, QStringLiteral("3")));
    QVERIFY(firstLink);
    QCOMPARE(seamsChangedSpy.count(), 1);
    QCOMPARE(linkedSeamsChangedSpy.count(), 1);
    QCOMPARE(seamSeries->seams().size(), 3u);
    QCOMPARE(seamSeries->seams().at(2), firstLink);
    QCOMPARE(firstLink->label(), QStringLiteral("3"));
    QCOMPARE(firstLink->name(), QStringLiteral("4"));
    QCOMPARE(firstLink->linkTo(), seam1);
    QCOMPARE(firstLink->number(), 3);
    QCOMPARE(firstLink->parent(), seamSeries);
    QCOMPARE(seam1->linkedSeams().size(), 1u);
    QCOMPARE(seam1->linkedSeams().front(), firstLink);
    QVERIFY(seam2->linkedSeams().empty());

    // create link with same label again
    QVERIFY(!seamSeries->createSeamLink(seam1, QStringLiteral("3")));
    QCOMPARE(seamsChangedSpy.count(), 1);
    QCOMPARE(linkedSeamsChangedSpy.count(), 1);
    // create link for existing seam number
    QVERIFY(!seamSeries->createSeamLink(seam1, QStringLiteral("1")));
    QCOMPARE(seamsChangedSpy.count(), 1);
    QCOMPARE(linkedSeamsChangedSpy.count(), 1);

    // create another link to seam1
    auto *secondLink = dynamic_cast<LinkedSeam*>(seamSeries->createSeamLink(seam1, QStringLiteral("4")));
    QVERIFY(secondLink);
    QCOMPARE(seamsChangedSpy.count(), 2);
    QCOMPARE(seamSeries->seams().size(), 4u);
    QCOMPARE(seamSeries->seams().at(3), secondLink);
    QCOMPARE(secondLink->label(), QStringLiteral("4"));
    QCOMPARE(secondLink->name(), QStringLiteral("5"));
    QCOMPARE(secondLink->linkTo(), seam1);
    QCOMPARE(secondLink->number(), 4);
    QCOMPARE(seam1->linkedSeams().size(), 2u);
    QCOMPARE(seam1->linkedSeams().at(1), secondLink);
    QVERIFY(seam2->linkedSeams().empty());
    QCOMPARE(linkedSeamsChangedSpy.count(), 2);

    // and a link to seam 2
    auto *thirdLink = dynamic_cast<LinkedSeam*>(seamSeries->createSeamLink(seam2, QStringLiteral("5")));
    QVERIFY(thirdLink);
    QCOMPARE(seamsChangedSpy.count(), 3);
    QCOMPARE(seamSeries->seams().size(), 5u);
    QCOMPARE(seamSeries->seams().at(4), thirdLink);
    QCOMPARE(thirdLink->label(), QStringLiteral("5"));
    QCOMPARE(thirdLink->name(), QStringLiteral("6"));
    QCOMPARE(thirdLink->linkTo(), seam2);
    QCOMPARE(thirdLink->number(), 5);
    QCOMPARE(seam1->linkedSeams().size(), 2u);
    QCOMPARE(seam2->linkedSeams().size(), 1u);
    QCOMPARE(seam2->linkedSeams().at(0), thirdLink);
    QCOMPARE(linkedSeamsChangedSpy.count(), 2);

    // some things that should fail
    QVERIFY(!seamSeries->createSeamLink(nullptr, QStringLiteral("6")));
    QCOMPARE(seamsChangedSpy.count(), 3);
    QCOMPARE(linkedSeamsChangedSpy.count(), 2);

    // another seam series and creating a link to wrong series
    auto secondSeamSeries = p.createSeamSeries();
    auto thirdSeam = secondSeamSeries->createSeam();
    QVERIFY(!seamSeries->createSeamLink(thirdSeam, QStringLiteral("6")));
    QCOMPARE(seamsChangedSpy.count(), 3);
    QCOMPARE(linkedSeamsChangedSpy.count(), 2);

    // test finding links
    QCOMPARE(seam1->findLink(QStringLiteral("3")), firstLink);
    QCOMPARE(seam1->findLink(QStringLiteral("4")), secondLink);
    QCOMPARE(seam2->findLink(QStringLiteral("5")), thirdLink);
    QVERIFY(!seam1->findLink(QStringLiteral("5")));
}

void TestLinkedSeam::testDuplicateLinkedSeam_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void TestLinkedSeam::testDuplicateLinkedSeam()
{
    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto *seamSeries = p.seamSeries().front();
    auto *seam = p.createSeam();
    QCOMPARE(seamSeries->seams().size(), 1u);

    auto *initialLink = dynamic_cast<LinkedSeam*>(seamSeries->createSeamLink(seam, QStringLiteral("3")));
    QVERIFY(initialLink);
    initialLink->setPositionInAssemblyImage({3.7, 5.2});

    LinkedSeam * clone = initialLink->clone(copyMode, seam);
    QVERIFY(clone);
    QCOMPARE(clone->name(), initialLink->name());
    QCOMPARE(clone->label(), initialLink->label());
    QCOMPARE(clone->uuid() != initialLink->uuid(), changeUuid);
    QCOMPARE(clone->positionInAssemblyImage(), initialLink->positionInAssemblyImage());
}

void TestLinkedSeam::testDestroySeam()
{
    // setup product with one seam series, 2 seams and 3 seam links
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto *seamSeries = p.seamSeries().front();
    auto *seam1 = p.createSeam();
    auto *seam2 = p.createSeam();
    auto *firstLink = seamSeries->createSeamLink(seam1, QStringLiteral("3"));
    auto *secondLink = seamSeries->createSeamLink(seam1, QStringLiteral("4"));
    auto *thirdLink = seamSeries->createSeamLink(seam1, QStringLiteral("5"));
    QCOMPARE(seamSeries->seams().size(), 5u);

    QSignalSpy seamsChangedSpy{seamSeries, &SeamSeries::seamsChanged};
    QVERIFY(seamsChangedSpy.isValid());
    QSignalSpy seamDestroyedSpy{seam1, &QObject::destroyed};
    QVERIFY(seamDestroyedSpy.isValid());
    QSignalSpy firstLinkDestroyedSpy{firstLink, &QObject::destroyed};
    QVERIFY(firstLinkDestroyedSpy.isValid());
    QSignalSpy secondLinkDestroyedSpy{secondLink, &QObject::destroyed};
    QVERIFY(secondLinkDestroyedSpy.isValid());
    QSignalSpy thirdLinkDestroyedSpy{thirdLink, &QObject::destroyed};
    QVERIFY(thirdLinkDestroyedSpy.isValid());
    QSignalSpy linkedSeamsChangedSpy{seam1, &Seam::linkedSeamsChanged};
    QVERIFY(linkedSeamsChangedSpy.isValid());

    QCOMPARE(seam1->linkedSeams().size(), 3u);
    seamSeries->destroySeam(secondLink);
    QCOMPARE(seamsChangedSpy.count(), 1);
    QCOMPARE(linkedSeamsChangedSpy.count(), 0);
    QCOMPARE(seamSeries->seams().size(), 4u);
    QVERIFY(secondLinkDestroyedSpy.wait());
    QCOMPARE(linkedSeamsChangedSpy.count(), 1);
    QCOMPARE(seam1->linkedSeams().size(), 2u);
    QCOMPARE(seam1->linkedSeams().at(0), firstLink);
    QCOMPARE(seam1->linkedSeams().at(1), thirdLink);

    seamSeries->destroySeam(seam1);
    QCOMPARE(seamsChangedSpy.count(), 2);
    QCOMPARE(seamSeries->seams().size(), 1u);
    QCOMPARE(seamSeries->seams().at(0), seam2);
    QVERIFY(seamDestroyedSpy.wait());

    QCOMPARE(seamDestroyedSpy.count(), 1);
    QCOMPARE(firstLinkDestroyedSpy.count(), 1);
    QCOMPARE(secondLinkDestroyedSpy.count(), 1);
    QCOMPARE(thirdLinkDestroyedSpy.count(), 1);

    seamSeries->destroySeam(seam2);
    QCOMPARE(seamsChangedSpy.count(), 3);
}

void TestLinkedSeam::testSaveAndLoad()
{
    // setup product with one seam series, 2 seams and 3 seam links
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto *seamSeries = p.seamSeries().front();
    auto *seam1 = p.createSeam();
    auto *seam2 = p.createSeam();
    auto *firstLink = seamSeries->createSeamLink(seam1, QStringLiteral("3"));
    auto *secondLink = seamSeries->createSeamLink(seam1, QStringLiteral("4"));
    auto *thirdLink = seamSeries->createSeamLink(seam1, QStringLiteral("5"));
    QCOMPARE(seamSeries->seams().size(), 5u);

    thirdLink->setPositionInAssemblyImage({1, 2});

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    p.setFilePath(dir.filePath(QStringLiteral("test.json")));
    QVERIFY(p.save());

    std::unique_ptr<Product> loaded{Product::fromJson(dir.filePath(QStringLiteral("test.json")))};

    auto loadedSeamSeries = loaded->seamSeries().front();
    QVERIFY(loadedSeamSeries);
    QCOMPARE(loadedSeamSeries->seams().size(), 5u);
    QCOMPARE(loadedSeamSeries->uuid(), seamSeries->uuid());

    QCOMPARE(loadedSeamSeries->seams().at(0)->uuid(), seam1->uuid());
    QCOMPARE(loadedSeamSeries->seams().at(1)->uuid(), seam2->uuid());
    QCOMPARE(loadedSeamSeries->seams().at(2)->uuid(), firstLink->uuid());
    QCOMPARE(loadedSeamSeries->seams().at(3)->uuid(), secondLink->uuid());
    QCOMPARE(loadedSeamSeries->seams().at(4)->uuid(), thirdLink->uuid());

    auto *loadedSeam1 = loadedSeamSeries->seams().at(0);
    auto *loadedSeam2 = loadedSeamSeries->seams().at(1);
    auto *loadedSeam3 = dynamic_cast<LinkedSeam*>(loadedSeamSeries->seams().at(2));
    auto *loadedSeam4 = dynamic_cast<LinkedSeam*>(loadedSeamSeries->seams().at(3));
    auto *loadedSeam5 = dynamic_cast<LinkedSeam*>(loadedSeamSeries->seams().at(4));
    QSignalSpy linkedSeamsChangedSpy{loadedSeam1, &Seam::linkedSeamsChanged};
    QVERIFY(linkedSeamsChangedSpy.isValid());

    QVERIFY(loadedSeam1);
    QVERIFY(loadedSeam2);
    QVERIFY(loadedSeam3);
    QVERIFY(loadedSeam4);
    QVERIFY(loadedSeam5);

    QCOMPARE(loadedSeam3->linkTo(), loadedSeam1);
    QCOMPARE(loadedSeam4->linkTo(), loadedSeam1);
    QCOMPARE(loadedSeam5->linkTo(), loadedSeam1);

    QCOMPARE(loadedSeam3->number(), 3);
    QCOMPARE(loadedSeam4->number(), 4);
    QCOMPARE(loadedSeam5->number(), 5);

    QCOMPARE(loadedSeam3->label(), QStringLiteral("3"));
    QCOMPARE(loadedSeam4->label(), QStringLiteral("4"));
    QCOMPARE(loadedSeam5->label(), QStringLiteral("5"));

    QCOMPARE(loadedSeam3->name(), QStringLiteral("4"));
    QCOMPARE(loadedSeam4->name(), QStringLiteral("5"));
    QCOMPARE(loadedSeam5->name(), QStringLiteral("6"));

    QCOMPARE(loadedSeam1->linkedSeams().size(), 3u);
    QCOMPARE(loadedSeam1->linkedSeams().at(0), loadedSeam3);
    QCOMPARE(loadedSeam1->linkedSeams().at(1), loadedSeam4);
    QCOMPARE(loadedSeam1->linkedSeams().at(2), loadedSeam5);

    QCOMPARE(loadedSeam3->parent(), loadedSeamSeries);
    QCOMPARE(loadedSeam4->parent(), loadedSeamSeries);
    QCOMPARE(loadedSeam5->parent(), loadedSeamSeries);

    QCOMPARE(loadedSeam5->positionInAssemblyImage(), QPointF(1, 2));
    QVERIFY(!loadedSeam4->positionInAssemblyImage().isNull());
    QVERIFY(!loadedSeam3->positionInAssemblyImage().isNull());
    QVERIFY(!loadedSeam2->positionInAssemblyImage().isNull());
    QVERIFY(!loadedSeam1->positionInAssemblyImage().isNull());

    QCOMPARE(linkedSeamsChangedSpy.count(), 0);
    delete loadedSeam5;
    QCOMPARE(loadedSeam1->linkedSeams().size(), 2u);
    QCOMPARE(linkedSeamsChangedSpy.count(), 1);
}

QTEST_GUILESS_MAIN(TestLinkedSeam)
#include "testLinkedSeam.moc"
