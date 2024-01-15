#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>

#include "../src/referenceCurveConstructor.h"
#include "../src/instanceResultSortModel.h"
#include "productInstanceModel.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "referenceCurve.h"
#include "referenceCurveData.h"
#include "precitec/dataSet.h"

using precitec::gui::ReferenceCurveConstructor;
using precitec::gui::InstanceResultSortModel;
using precitec::gui::components::plotter::DataSet;
using precitec::storage::ProductInstanceModel;
using precitec::storage::Product;
using precitec::storage::SeamSeries;
using precitec::storage::Seam;
using precitec::storage::ReferenceCurve;

class ReferenceCurveConstructorTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testSetProductInstanceModel();
    void testSeam();
    void testUpdateModel();
    void testResultType();
    void testTriggerType();
    void testCurrentIndex();
    void testThreshold();
    void testUpdateResult();
    void testLoading();
    void testSetData();
    void testSelectAll();
    void testSelectNone();
    void testReferenceCurve();
    void testCurrentProduct();
    void testJitter();
    void testReferenceType();
    void testUpdateReferences();
    void testAverageCurve();
    void testMedianCurve();
    void testMinMaxCurve();
    void testSave();
};

void ReferenceCurveConstructorTest::testCtor()
{
    auto model = new ReferenceCurveConstructor{this};

    QVERIFY(!model->productInstanceModel());
    QVERIFY(!model->currentProduct());
    QVERIFY(!model->seam());
    QCOMPARE(model->resultType(), -1);
    QCOMPARE(model->triggerType(), -1);
    QCOMPARE(model->threshold(), 0.0);
    QVERIFY(!model->currentIndex().isValid());
    QVERIFY(model->result());
    QVERIFY(model->trigger());
    QVERIFY(!model->loading());
    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->roleNames().size(), 8);
    QVERIFY(!model->referenceCurve());
    QCOMPARE(model->jitter(), 0.0);
    QCOMPARE(model->referenceType(), ReferenceCurve::ReferenceType::Average);
    QVERIFY(model->lower());
    QCOMPARE(model->lower()->color(), Qt::magenta);
    QCOMPARE(model->lower()->drawingMode(), DataSet::DrawingMode::Line);
    QCOMPARE(model->lower()->name(), QStringLiteral("Lower Boundary"));
    QVERIFY(model->middle());
    QCOMPARE(model->middle()->color(), Qt::darkMagenta);
    QCOMPARE(model->middle()->drawingOrder(), DataSet::DrawingOrder::OnTop);
    QCOMPARE(model->middle()->name(), QStringLiteral("Reference Curve"));
    QVERIFY(model->upper());
    QCOMPARE(model->upper()->color(), Qt::magenta);
    QCOMPARE(model->upper()->drawingMode(), DataSet::DrawingMode::Line);
    QCOMPARE(model->upper()->name(), QStringLiteral("Upper Boundary"));
    QVERIFY(!model->updating());
    QVERIFY(!model->hasChanges());
}

void ReferenceCurveConstructorTest::testRoleNames()
{
    auto model = new ReferenceCurveConstructor{this};

    const auto& roleNames = model->roleNames();
    QCOMPARE(roleNames.size(), 8);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("serialNumber"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("date"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("state"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("nioColor"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("linkedSeam"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("visualSeamNumber"));
    QCOMPARE(roleNames[Qt::UserRole + 5], QByteArrayLiteral("selected"));
    QCOMPARE(roleNames[Qt::UserRole + 6], QByteArrayLiteral("result"));
}

void ReferenceCurveConstructorTest::testSetProductInstanceModel()
{
    auto model = new ReferenceCurveConstructor{this};

    QVERIFY(!model->productInstanceModel());

    QSignalSpy productInstanceModelChangedSpy(model, &ReferenceCurveConstructor::productInstanceModelChanged);
    QVERIFY(productInstanceModelChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    model->setProductInstanceModel(nullptr);
    QCOMPARE(productInstanceModelChangedSpy.count(), 0);

    const auto instanceModel = new ProductInstanceModel(this);

    model->setProductInstanceModel(instanceModel);
    QCOMPARE(model->productInstanceModel(), instanceModel);
    QCOMPARE(productInstanceModelChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 0);

    model->setProductInstanceModel(instanceModel);
    QCOMPARE(productInstanceModelChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 0);

    instanceModel->deleteLater();
    QTRY_COMPARE(productInstanceModelChangedSpy.count(), 2);
    QVERIFY(!model->productInstanceModel());
    QCOMPARE(modelResetSpy.count(), 0);
}

void ReferenceCurveConstructorTest::testSeam()
{
    auto model = new ReferenceCurveConstructor{this};

    QVERIFY(!model->seam());

    QSignalSpy seamChangedSpy{model, &ReferenceCurveConstructor::seamChanged};
    QVERIFY(seamChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    model->setSeam(nullptr);
    QCOMPARE(seamChangedSpy.count(), 0);

    auto product = new Product{QUuid::createUuid(), this};
    auto series = product->createSeamSeries();
    QVERIFY(series);
    auto seam = series->createSeam();
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 0);

    model->setSeam(seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 0);

    seam->deleteLater();
    QTRY_COMPARE(seamChangedSpy.count(), 2);
    QVERIFY(!model->seam());
    QCOMPARE(modelResetSpy.count(), 0);
}

void ReferenceCurveConstructorTest::testUpdateModel()
{
    QTemporaryDir dir;
    QString productDir = dir.path() + QDir::separator() + QStringLiteral("1f086211-fbd4-4493-a580-6ff11e4925de") + QDir::separator();
    QString subDir = QStringLiteral("/seam_series0002/seam0002/");

    const auto instanceCount = 5;
    const auto resultType = 504;

    const QString resultFileName = QFINDTESTDATA("testdata/reference_data/504.result");
    QVERIFY(!resultFileName.isEmpty());
    QFile result{resultFileName};
    QVERIFY(result.exists());

    for (int i = 0; i < instanceCount; i++)
    {
        const auto serialId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto serialNumber = QString::number(i);

        const auto instanceId = serialId + QStringLiteral("-SN-") + serialNumber;

        const auto folderPath = productDir + instanceId + QDir::separator() + subDir;

        QVERIFY(QDir{}.mkpath(folderPath));
        const auto name = folderPath + QString::number(resultType) + QStringLiteral(".result");
        QVERIFY(result.copy(name));
        QFile file{name};
        QVERIFY(file.exists());

        // mark some as nio
        QFile metadata{productDir + instanceId + QDir::separator() + QStringLiteral("metadata.json")};
        if (metadata.open(QIODevice::ReadWrite))
        {
            metadata.write(QJsonDocument {{
                {QStringLiteral("date"), QStringLiteral("2022-01-10T11:04:39.207Z")},
                {QStringLiteral("date"), QStringLiteral("2022-01-10T11:04:39.207Z")},
                {QStringLiteral("nio"), i % 2 != 0},
                {QStringLiteral("nioSwitchedOff"), true}
            }}.toJson());
        }
        QVERIFY(metadata.exists());
    }

    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    QSignalSpy dataChangedSpy(model, &ReferenceCurveConstructor::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    auto linkedSeam = seam->seamSeries()->createSeamLink(seam, QStringLiteral("4"));
    QVERIFY(linkedSeam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);
    QCOMPARE(modelResetSpy.count(), 0);

    auto instanceModel = new ProductInstanceModel{this};
    QSignalSpy loadingChangedSpy(instanceModel, &ProductInstanceModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());
    QSignalSpy instanceDataChangedSpy(instanceModel, &ProductInstanceModel::dataChanged);
    QVERIFY(instanceDataChangedSpy.isValid());

    model->setProductInstanceModel(instanceModel);
    QCOMPARE(model->productInstanceModel(), instanceModel);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model->rowCount(), 0);

    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());
    QCOMPARE(loadingChangedSpy.count(), 1);

    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(instanceModel->isLoading(), false);
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 2);

    QCOMPARE(instanceModel->rowCount(), instanceCount);
    QCOMPARE(model->rowCount(), 2 * instanceCount);

    auto sortModel = new InstanceResultSortModel{this};
    sortModel->setSourceModel(model);
    sortModel->setSortOrder(Qt::AscendingOrder);
    QCOMPARE(sortModel->rowCount(), instanceCount * 2);

    for (auto i = 0; i < instanceCount; i++)
    {
        const auto index = sortModel->index(i * 2, 0);

        QCOMPARE(index.data(Qt::DisplayRole).toString(), QString::number(i));
        QCOMPARE(index.data(Qt::UserRole + 1).value<precitec::storage::ProductInstanceModel::State>(), precitec::storage::ProductInstanceModel::State::Unknown);
        QCOMPARE(index.data(Qt::UserRole + 2).value<QColor>(), QColor(0,0,0,0));
        QVERIFY(!index.data(Qt::UserRole + 3).toBool());
        QCOMPARE(index.data(Qt::UserRole + 4).toInt(), seam->visualNumber());
        QVERIFY(!index.data(Qt::UserRole + 5).toBool());
        QVERIFY(!index.data(Qt::UserRole + 6).value<DataSet*>());

        const auto index_linked = sortModel->index(i * 2 + 1, 0);

        QCOMPARE(index_linked.data(Qt::DisplayRole).toString(), QString::number(i));
        QCOMPARE(index_linked.data(Qt::UserRole + 1).value<precitec::storage::ProductInstanceModel::State>(), precitec::storage::ProductInstanceModel::State::Unknown);
        QCOMPARE(index_linked.data(Qt::UserRole + 2).value<QColor>(), QColor(0,0,0,0));
        QVERIFY(index_linked.data(Qt::UserRole + 3).toBool());
        QCOMPARE(index_linked.data(Qt::UserRole + 4).toInt(), linkedSeam->visualNumber());
        QVERIFY(!index_linked.data(Qt::UserRole + 5).toBool());
        QVERIFY(!index_linked.data(Qt::UserRole + 6).value<DataSet*>());
    }

    instanceModel->ensureAllMetaDataLoaded();

    QTRY_COMPARE(instanceDataChangedSpy.count(), 5);
    // x2 for linked seams
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 3);

    for (auto i = 0; i < instanceCount; i++)
    {
        const auto index = sortModel->index(i * 2, 0);

        QCOMPARE(index.data(Qt::DisplayRole).toString(), QString::number(i));
        QCOMPARE(index.data(Qt::UserRole + 1).value<precitec::storage::ProductInstanceModel::State>(), i % 2 == 0 ? precitec::storage::ProductInstanceModel::State::Io : precitec::storage::ProductInstanceModel::State::Nio);
        QCOMPARE(index.data(Qt::UserRole + 2).value<QColor>(), i % 2 == 0 ? Qt::green : Qt::red);
        QVERIFY(!index.data(Qt::UserRole + 3).toBool());
        QCOMPARE(index.data(Qt::UserRole + 4).toInt(), seam->visualNumber());
        QVERIFY(!index.data(Qt::UserRole + 5).toBool());
        QVERIFY(!index.data(Qt::UserRole + 6).value<DataSet*>());

        const auto index_linked = sortModel->index(i * 2 + 1, 0);

        QCOMPARE(index_linked.data(Qt::DisplayRole).toString(), QString::number(i));
        QCOMPARE(index_linked.data(Qt::UserRole + 1).value<precitec::storage::ProductInstanceModel::State>(), i % 2 == 0 ? precitec::storage::ProductInstanceModel::State::Io : precitec::storage::ProductInstanceModel::State::Nio);
        QCOMPARE(index.data(Qt::UserRole + 2).value<QColor>(), i % 2 == 0 ? Qt::green : Qt::red);
        QVERIFY(index_linked.data(Qt::UserRole + 3).toBool());
        QCOMPARE(index_linked.data(Qt::UserRole + 4).toInt(), linkedSeam->visualNumber());
        QVERIFY(!index_linked.data(Qt::UserRole + 5).toBool());
        QVERIFY(!index_linked.data(Qt::UserRole + 6).value<DataSet*>());
    }

    sortModel->setIncludeLinkedSeams(false);
    QTRY_COMPARE(sortModel->rowCount(), instanceCount);

    for (auto i = 0; i < instanceCount; i++)
    {
        const auto index = sortModel->index(i, 0);

        QCOMPARE(index.data(Qt::DisplayRole).toString(), QString::number(i));
        QCOMPARE(index.data(Qt::UserRole + 1).value<precitec::storage::ProductInstanceModel::State>(), i % 2 == 0 ? precitec::storage::ProductInstanceModel::State::Io : precitec::storage::ProductInstanceModel::State::Nio);
        QCOMPARE(index.data(Qt::UserRole + 2).value<QColor>(), i % 2 == 0 ? Qt::green : Qt::red);
        QVERIFY(!index.data(Qt::UserRole + 3).toBool());
        QCOMPARE(index.data(Qt::UserRole + 4).toInt(), seam->visualNumber());
        QVERIFY(!index.data(Qt::UserRole + 5).toBool());
        QVERIFY(!index.data(Qt::UserRole + 6).value<DataSet*>());
    }
}

void ReferenceCurveConstructorTest::testResultType()
{
    auto model = new ReferenceCurveConstructor{this};

    QCOMPARE(model->resultType(), -1);

    QSignalSpy resultTypeChangedSpy(model, &ReferenceCurveConstructor::resultTypeChanged);
    QVERIFY(resultTypeChangedSpy.isValid());

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    model->setResultType(-1);
    QCOMPARE(resultTypeChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    model->setResultType(504);
    QCOMPARE(model->resultType(), 504);
    QCOMPARE(resultTypeChangedSpy.count(), 1);
    QCOMPARE(updatingChangedSpy.count(), 0);

    model->setResultType(504);
    QCOMPARE(resultTypeChangedSpy.count(), 1);

    auto refCurve = new ReferenceCurve(this);
    model->setReferenceCurve(refCurve);

    model->setResultType(202);
    QCOMPARE(model->resultType(), 202);
    QCOMPARE(resultTypeChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 2);
}

void ReferenceCurveConstructorTest::testTriggerType()
{
    auto model = new ReferenceCurveConstructor{this};

    QCOMPARE(model->triggerType(), -1);

    QSignalSpy triggerTypeChangedSpy(model, &ReferenceCurveConstructor::triggerTypeChanged);
    QVERIFY(triggerTypeChangedSpy.isValid());

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    model->setTriggerType(-1);
    QCOMPARE(triggerTypeChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    model->setTriggerType(504);
    QCOMPARE(model->triggerType(), 504);
    QCOMPARE(triggerTypeChangedSpy.count(), 1);
    QCOMPARE(updatingChangedSpy.count(), 0);

    model->setTriggerType(504);
    QCOMPARE(triggerTypeChangedSpy.count(), 1);

    auto refCurve = new ReferenceCurve(this);
    model->setReferenceCurve(refCurve);

    model->setTriggerType(202);
    QCOMPARE(model->triggerType(), 202);
    QCOMPARE(triggerTypeChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 2);
}

void ReferenceCurveConstructorTest::testCurrentIndex()
{
    auto model = new ReferenceCurveConstructor{this};

    QVERIFY(!model->currentIndex().isValid());

    QSignalSpy currentIndexChangedSpy(model, &ReferenceCurveConstructor::currentIndexChanged);
    QVERIFY(currentIndexChangedSpy.isValid());

    model->setCurrentIndex({});
    QCOMPARE(currentIndexChangedSpy.count(), 0);

    // setup some data to load
    QTemporaryDir dir;
    QString productDir = dir.path() + QDir::separator() + QStringLiteral("1f086211-fbd4-4493-a580-6ff11e4925de") + QDir::separator();
    QString subDir = QStringLiteral("/seam_series0002/seam0002/");

    const auto instanceCount = 5;
    const auto resultType = 504;

    const QString resultFileName = QFINDTESTDATA("testdata/reference_data/504.result");
    QVERIFY(!resultFileName.isEmpty());
    QFile result{resultFileName};
    QVERIFY(result.exists());

    for (int i = 0; i < instanceCount; i++)
    {
        const auto serialId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto serialNumber = QString::number(i);

        const auto instanceId = serialId + QStringLiteral("-SN-") + serialNumber;

        const auto folderPath = productDir + instanceId + QDir::separator() + subDir;

        QVERIFY(QDir{}.mkpath(folderPath));
        const auto name = folderPath + QString::number(resultType) + QStringLiteral(".result");
        QVERIFY(result.copy(name));
        QFile file{name};
        QVERIFY(file.exists());
    }

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);

    model->setCurrentIndex(model->index(3, 0));
    QCOMPARE(model->currentIndex(), model->index(3, 0));
    QCOMPARE(currentIndexChangedSpy.count(), 1);

    model->setCurrentIndex(model->index(3, 0));
    QCOMPARE(currentIndexChangedSpy.count(), 1);

    model->resetCurrentIndex();
    QVERIFY(!model->currentIndex().isValid());
    QCOMPARE(currentIndexChangedSpy.count(), 2);
}

void ReferenceCurveConstructorTest::testThreshold()
{
    auto model = new ReferenceCurveConstructor{this};

    QCOMPARE(model->threshold(), 0.0);

    QSignalSpy thresholdChangedSpy(model, &ReferenceCurveConstructor::thresholdChanged);
    QVERIFY(thresholdChangedSpy.isValid());

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    model->setThreshold(0.0);
    QCOMPARE(thresholdChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    model->setThreshold(5.5);
    QCOMPARE(model->threshold(), 5.5);
    QCOMPARE(thresholdChangedSpy.count(), 1);
    QCOMPARE(updatingChangedSpy.count(), 0);

    model->setThreshold(5.5);
    QCOMPARE(thresholdChangedSpy.count(), 1);

    auto refCurve = new ReferenceCurve(this);
    model->setReferenceCurve(refCurve);

    model->setThreshold(2);
    QCOMPARE(model->threshold(), 2);
    QCOMPARE(thresholdChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 2);
}

void ReferenceCurveConstructorTest::testUpdateResult()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy loadingChangedSpy(model, &ReferenceCurveConstructor::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    // setup some data to load
    QTemporaryDir dir;
    QString productDir = dir.path() + QDir::separator() + QStringLiteral("1f086211-fbd4-4493-a580-6ff11e4925de") + QDir::separator();
    QString subDir = QStringLiteral("/seam_series0002/seam0002/");

    const auto instanceCount = 5;
    const auto resultType = 504;

    const QString resultFileName = QFINDTESTDATA("testdata/reference_data/504.result");
    QVERIFY(!resultFileName.isEmpty());
    QFile result{resultFileName};
    QVERIFY(result.exists());

    for (int i = 0; i < instanceCount; i++)
    {
        const auto serialId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto serialNumber = QString::number(i);

        const auto instanceId = serialId + QStringLiteral("-SN-") + serialNumber;

        const auto folderPath = productDir + instanceId + QDir::separator() + subDir;

        QVERIFY(QDir{}.mkpath(folderPath));
        const auto name = folderPath + QString::number(resultType) + QStringLiteral(".result");
        QVERIFY(result.copy(name));
        QFile file{name};
        QVERIFY(file.exists());
    }

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);
    QCOMPARE(loadingChangedSpy.count(), 0);

    model->setResultType(504);
    QCOMPARE(loadingChangedSpy.count(), 0);

    QCOMPARE(model->result()->sampleCount(), 0);
    QCOMPARE(model->trigger()->sampleCount(), 0);

    model->setCurrentIndex(model->index(3, 0));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);

    QCOMPARE(model->result()->sampleCount(), 29);
    QCOMPARE(model->trigger()->sampleCount(), 0);

    model->setThreshold(1.5);
    QCOMPARE(loadingChangedSpy.count(), 3);
    QTRY_COMPARE(loadingChangedSpy.count(), 4);

    QCOMPARE(model->result()->sampleCount(), 29);
    QCOMPARE(model->trigger()->sampleCount(), 0);

    model->setTriggerType(504);
    QCOMPARE(loadingChangedSpy.count(), 5);
    QTRY_COMPARE(loadingChangedSpy.count(), 6);

    QCOMPARE(model->result()->sampleCount(), 22);
    QCOMPARE(model->trigger()->sampleCount(), 29);
}

void ReferenceCurveConstructorTest::testLoading()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy loadingChangedSpy(model, &ReferenceCurveConstructor::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QVERIFY(!model->loading());

    model->incrementLoading();
    QVERIFY(model->loading());
    QCOMPARE(loadingChangedSpy.count(), 1);

    model->incrementLoading();
    QVERIFY(model->loading());
    QCOMPARE(loadingChangedSpy.count(), 1);

    model->decrementLoading();
    QVERIFY(model->loading());
    QCOMPARE(loadingChangedSpy.count(), 1);

    model->decrementLoading();
    QVERIFY(!model->loading());
    QCOMPARE(loadingChangedSpy.count(), 2);
}

void ReferenceCurveConstructorTest::testSetData()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy dataChangedSpy(model, &ReferenceCurveConstructor::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ReferenceCurveConstructor::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    // setup some data to load
    QTemporaryDir dir;
    QString productDir = dir.path() + QDir::separator() + QStringLiteral("1f086211-fbd4-4493-a580-6ff11e4925de") + QDir::separator();
    QString subDir = QStringLiteral("/seam_series0002/seam0002/");

    const auto instanceCount = 5;
    const auto resultType = 504;

    const QString resultFileName = QFINDTESTDATA("testdata/reference_data/504.result");
    QVERIFY(!resultFileName.isEmpty());
    QFile result{resultFileName};
    QVERIFY(result.exists());

    for (int i = 0; i < instanceCount; i++)
    {
        const auto serialId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto serialNumber = QString::number(i);

        const auto instanceId = serialId + QStringLiteral("-SN-") + serialNumber;

        const auto folderPath = productDir + instanceId + QDir::separator() + subDir;

        QVERIFY(QDir{}.mkpath(folderPath));
        const auto name = folderPath + QString::number(resultType) + QStringLiteral(".result");
        QVERIFY(result.copy(name));
        QFile file{name};
        QVERIFY(file.exists());
    }

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 0);

    model->setResultType(504);
    model->setData(model->index(3, 0), true, Qt::UserRole + 5);
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 2);

    QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), model->index(3, 0));
    QCOMPARE(dataChangedSpy.at(0).at(1).toModelIndex(), model->index(3, 0));
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 6});

    QCOMPARE(dataChangedSpy.at(1).at(0).toModelIndex(), model->index(3, 0));
    QCOMPARE(dataChangedSpy.at(1).at(1).toModelIndex(), model->index(3, 0));
    QCOMPARE(dataChangedSpy.at(1).at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 5});

    QVERIFY(model->index(3, 0).data(Qt::UserRole + 5).toBool());
    QVERIFY(model->index(3, 0).data(Qt::UserRole + 6).value<DataSet*>());
    QCOMPARE(model->index(3, 0).data(Qt::UserRole + 6).value<DataSet*>()->sampleCount(), 29);

    model->setTriggerType(504);
    model->setThreshold(1.5);

    model->setData(model->index(2, 0), true, Qt::UserRole + 5);
    QCOMPARE(loadingChangedSpy.count(), 3);
    QTRY_COMPARE(loadingChangedSpy.count(), 4);
    QCOMPARE(dataChangedSpy.count(), 4);

    QCOMPARE(dataChangedSpy.at(2).at(0).toModelIndex(), model->index(2, 0));
    QCOMPARE(dataChangedSpy.at(2).at(1).toModelIndex(), model->index(2, 0));
    QCOMPARE(dataChangedSpy.at(2).at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 6});

    QCOMPARE(dataChangedSpy.at(3).at(0).toModelIndex(), model->index(2, 0));
    QCOMPARE(dataChangedSpy.at(3).at(1).toModelIndex(), model->index(2, 0));
    QCOMPARE(dataChangedSpy.at(3).at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 5});

    QVERIFY(model->index(2, 0).data(Qt::UserRole + 5).toBool());
    QVERIFY(model->index(2, 0).data(Qt::UserRole + 6).value<DataSet*>());
    QCOMPARE(model->index(2, 0).data(Qt::UserRole + 6).value<DataSet*>()->sampleCount(), 22);
}

void ReferenceCurveConstructorTest::testSelectAll()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy dataChangedSpy(model, &ReferenceCurveConstructor::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ReferenceCurveConstructor::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    // setup some data to load
    QTemporaryDir dir;
    QString productDir = dir.path() + QDir::separator() + QStringLiteral("1f086211-fbd4-4493-a580-6ff11e4925de") + QDir::separator();
    QString subDir = QStringLiteral("/seam_series0002/seam0002/");

    const auto instanceCount = 5;
    const auto resultType = 504;

    const QString resultFileName = QFINDTESTDATA("testdata/reference_data/504.result");
    QVERIFY(!resultFileName.isEmpty());
    QFile result{resultFileName};
    QVERIFY(result.exists());

    for (int i = 0; i < instanceCount; i++)
    {
        const auto serialId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto serialNumber = QString::number(i);

        const auto instanceId = serialId + QStringLiteral("-SN-") + serialNumber;

        const auto folderPath = productDir + instanceId + QDir::separator() + subDir;

        QVERIFY(QDir{}.mkpath(folderPath));
        const auto name = folderPath + QString::number(resultType) + QStringLiteral(".result");
        QVERIFY(result.copy(name));
        QFile file{name};
        QVERIFY(file.exists());
    }

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 0);

    for (auto i = 0; i < instanceCount; i++)
    {
        const auto index = model->index(i, 0);
        QVERIFY(!index.data(Qt::UserRole + 5).toBool());
    }

    model->setResultType(504);
    model->selectAll();

    QTRY_COMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->m_loadedInstances.size(), 5);
    QCOMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), model->index(0, 0));
    QCOMPARE(dataChangedSpy.at(0).at(1).toModelIndex(), model->index(4, 0));
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 6));

    for (auto i = 0; i < instanceCount; i++)
    {
        const auto index = model->index(i, 0);
        QVERIFY(index.data(Qt::UserRole + 5).toBool());
        QVERIFY(index.data(Qt::UserRole + 6).value<DataSet*>());
        QCOMPARE(index.data(Qt::UserRole + 6).value<DataSet*>()->sampleCount(), 29);
    }
}

void ReferenceCurveConstructorTest::testSelectNone()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy dataChangedSpy(model, &ReferenceCurveConstructor::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ReferenceCurveConstructor::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    // setup some data to load
    QTemporaryDir dir;
    QString productDir = dir.path() + QDir::separator() + QStringLiteral("1f086211-fbd4-4493-a580-6ff11e4925de") + QDir::separator();
    QString subDir = QStringLiteral("/seam_series0002/seam0002/");

    const auto instanceCount = 5;
    const auto resultType = 504;

    const QString resultFileName = QFINDTESTDATA("testdata/reference_data/504.result");
    QVERIFY(!resultFileName.isEmpty());
    QFile result{resultFileName};
    QVERIFY(result.exists());

    for (int i = 0; i < instanceCount; i++)
    {
        const auto serialId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto serialNumber = QString::number(i);

        const auto instanceId = serialId + QStringLiteral("-SN-") + serialNumber;

        const auto folderPath = productDir + instanceId + QDir::separator() + subDir;

        QVERIFY(QDir{}.mkpath(folderPath));
        const auto name = folderPath + QString::number(resultType) + QStringLiteral(".result");
        QVERIFY(result.copy(name));
        QFile file{name};
        QVERIFY(file.exists());
    }

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 0);

    for (auto i = 0; i < instanceCount; i++)
    {
        const auto index = model->index(i, 0);
        QVERIFY(!index.data(Qt::UserRole + 5).toBool());
    }

    model->setResultType(504);
    model->selectAll();

    QTRY_COMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->m_loadedInstances.size(), 5);
    QCOMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), model->index(0, 0));
    QCOMPARE(dataChangedSpy.at(0).at(1).toModelIndex(), model->index(4, 0));
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 6));

    for (auto i = 0; i < instanceCount; i++)
    {
        const auto index = model->index(i, 0);
        QVERIFY(index.data(Qt::UserRole + 5).toBool());
        QVERIFY(index.data(Qt::UserRole + 6).value<DataSet*>());
        QCOMPARE(index.data(Qt::UserRole + 6).value<DataSet*>()->sampleCount(), 29);
    }

    model->selectNone();

    QTRY_COMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->m_loadedInstances.size(), 0);
    QCOMPARE(dataChangedSpy.count(), 2);

    QCOMPARE(dataChangedSpy.at(1).at(0).toModelIndex(), model->index(0, 0));
    QCOMPARE(dataChangedSpy.at(1).at(1).toModelIndex(), model->index(4, 0));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 6));

    for (auto i = 0; i < instanceCount; i++)
    {
        const auto index = model->index(i, 0);
        QVERIFY(!index.data(Qt::UserRole + 5).toBool());
        QVERIFY(!index.data(Qt::UserRole + 6).value<DataSet*>());
    }
}

void ReferenceCurveConstructorTest::testReferenceCurve()
{
    auto model = new ReferenceCurveConstructor{this};
    QVERIFY(!model->referenceCurve());

    QSignalSpy referenceCurveChangedSpy(model, &ReferenceCurveConstructor::referenceCurveChanged);
    QVERIFY(referenceCurveChangedSpy.isValid());

    model->setReferenceCurve(nullptr);
    QCOMPARE(referenceCurveChangedSpy.count(), 0);

    auto refCurve = new ReferenceCurve(this);
    model->setReferenceCurve(refCurve);
    QCOMPARE(model->referenceCurve(), refCurve);
    QCOMPARE(referenceCurveChangedSpy.count(), 1);

    model->setReferenceCurve(refCurve);
    QCOMPARE(referenceCurveChangedSpy.count(), 1);

    refCurve->deleteLater();
    QTRY_COMPARE(referenceCurveChangedSpy.count(), 2);
    QVERIFY(!model->referenceCurve());
}

void ReferenceCurveConstructorTest::testCurrentProduct()
{
    auto model = new ReferenceCurveConstructor{this};
    QVERIFY(!model->currentProduct());

    QSignalSpy currentProductChangedSpy(model, &ReferenceCurveConstructor::currentProductChanged);
    QVERIFY(currentProductChangedSpy.isValid());

    model->setCurrentProduct(nullptr);
    QCOMPARE(currentProductChangedSpy.count(), 0);

    auto product = new Product{QUuid::createUuid(), this};

    model->setCurrentProduct(product);
    QCOMPARE(model->currentProduct(), product);
    QCOMPARE(currentProductChangedSpy.count(), 1);

    model->setCurrentProduct(product);
    QCOMPARE(currentProductChangedSpy.count(), 1);

    product->deleteLater();
    QTRY_COMPARE(currentProductChangedSpy.count(), 2);
    QVERIFY(!model->currentProduct());
}

void ReferenceCurveConstructorTest::testJitter()
{
    auto model = new ReferenceCurveConstructor{this};

    QCOMPARE(model->jitter(), 0.0);
    QVERIFY(!model->hasChanges());

    QSignalSpy jitterChangedSpy(model, &ReferenceCurveConstructor::jitterChanged);
    QVERIFY(jitterChangedSpy.isValid());

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    QSignalSpy hasChangedSpy(model, &ReferenceCurveConstructor::hasChanged);
    QVERIFY(hasChangedSpy.isValid());

    model->setJitter(0.0);
    QCOMPARE(jitterChangedSpy.count(), 0);

    model->setJitter(5.5);
    QCOMPARE(model->jitter(), 5.5);
    QVERIFY(model->hasChanges());
    QCOMPARE(jitterChangedSpy.count(), 1);
    QCOMPARE(updatingChangedSpy.count(), 0);
    QCOMPARE(hasChangedSpy.count(), 1);

    model->setJitter(5.5);
    QCOMPARE(jitterChangedSpy.count(), 1);

    auto refCurve = new ReferenceCurve(this);
    refCurve->setJitter(2.0);
    model->setReferenceCurve(refCurve);

    QCOMPARE(model->jitter(), 2.0);
    QVERIFY(!model->hasChanges());
    QCOMPARE(jitterChangedSpy.count(), 2);
    QCOMPARE(hasChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 2);
}

void ReferenceCurveConstructorTest::testReferenceType()
{
    auto model = new ReferenceCurveConstructor{this};

    QCOMPARE(model->referenceType(), ReferenceCurve::ReferenceType::Average);
    QVERIFY(!model->hasChanges());

    QSignalSpy referenceTypeChangedSpy(model, &ReferenceCurveConstructor::referenceTypeChanged);
    QVERIFY(referenceTypeChangedSpy.isValid());

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    QSignalSpy hasChangedSpy(model, &ReferenceCurveConstructor::hasChanged);
    QVERIFY(hasChangedSpy.isValid());

    model->setReferenceType(ReferenceCurve::ReferenceType::Average);
    QCOMPARE(referenceTypeChangedSpy.count(), 0);

    model->setReferenceType(ReferenceCurve::ReferenceType::Median);
    QCOMPARE(model->referenceType(), ReferenceCurve::ReferenceType::Median);
    QVERIFY(model->hasChanges());
    QCOMPARE(referenceTypeChangedSpy.count(), 1);
    QCOMPARE(updatingChangedSpy.count(), 0);
    QCOMPARE(hasChangedSpy.count(), 1);

    model->setReferenceType(ReferenceCurve::ReferenceType::Median);
    QCOMPARE(referenceTypeChangedSpy.count(), 1);

    auto refCurve = new ReferenceCurve(this);
    refCurve->setReferenceType(ReferenceCurve::ReferenceType::MinMax);
    model->setReferenceCurve(refCurve);

    QCOMPARE(model->referenceType(), ReferenceCurve::ReferenceType::MinMax);
    QVERIFY(!model->hasChanges());
    QCOMPARE(referenceTypeChangedSpy.count(), 2);
    QCOMPARE(hasChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 2);
}

void ReferenceCurveConstructorTest::testUpdateReferences()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy dataChangedSpy(model, &ReferenceCurveConstructor::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ReferenceCurveConstructor::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    QSignalSpy hasChangedSpy(model, &ReferenceCurveConstructor::hasChanged);
    QVERIFY(hasChangedSpy.isValid());

    // setup some data to load
    QTemporaryDir dir;
    QString productDir = dir.path() + QDir::separator() + QStringLiteral("1f086211-fbd4-4493-a580-6ff11e4925de") + QDir::separator();
    QString subDir = QStringLiteral("/seam_series0002/seam0002/");

    const auto instanceCount = 5;
    const auto resultType = 504;

    const QString resultFileName = QFINDTESTDATA("testdata/reference_data/504.result");
    QVERIFY(!resultFileName.isEmpty());
    QFile result{resultFileName};
    QVERIFY(result.exists());

    for (int i = 0; i < instanceCount; i++)
    {
        const auto serialId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto serialNumber = QString::number(i);

        const auto instanceId = serialId + QStringLiteral("-SN-") + serialNumber;

        const auto folderPath = productDir + instanceId + QDir::separator() + subDir;

        QVERIFY(QDir{}.mkpath(folderPath));
        const auto name = folderPath + QString::number(resultType) + QStringLiteral(".result");
        QVERIFY(result.copy(name));
        QFile file{name};
        QVERIFY(file.exists());
    }

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 0);

    for (auto i = 0; i < instanceCount; i++)
    {
        const auto index = model->index(i, 0);
        QVERIFY(!index.data(Qt::UserRole + 5).toBool());
    }

    model->setResultType(504);

    auto refCurve = new ReferenceCurve{this};
    QVERIFY(refCurve);
    refCurve->setSourceOfCurve({QString::number(2), QString::number(1)});

    model->setReferenceCurve(refCurve);
    QVERIFY(model->hasChanges());

    QCOMPARE(hasChangedSpy.count(), 1);
    QCOMPARE(updatingChangedSpy.count(), 1);
    QTRY_COMPARE(updatingChangedSpy.count(), 2);

    auto sortModel = new InstanceResultSortModel{this};
    sortModel->setSourceModel(model);
    sortModel->setSortOrder(Qt::AscendingOrder);
    QCOMPARE(sortModel->rowCount(), instanceCount);

    QVERIFY(!sortModel->index(0, 0).data(Qt::UserRole + 5).toBool());
    QVERIFY(sortModel->index(1, 0).data(Qt::UserRole + 5).toBool());
    QVERIFY(sortModel->index(2, 0).data(Qt::UserRole + 5).toBool());
    QVERIFY(!sortModel->index(3, 0).data(Qt::UserRole + 5).toBool());
    QVERIFY(!sortModel->index(4, 0).data(Qt::UserRole + 5).toBool());
}

void ReferenceCurveConstructorTest::testAverageCurve()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy dataChangedSpy(model, &ReferenceCurveConstructor::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ReferenceCurveConstructor::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QString productDir = QFINDTESTDATA("testdata/reference_data/average");
    QDir dir{productDir};
    QVERIFY(dir.exists());

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);
    QCOMPARE(loadingChangedSpy.count(), 0);

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    QSignalSpy hasChangedSpy(model, &ReferenceCurveConstructor::hasChanged);
    QVERIFY(hasChangedSpy.isValid());

    auto refCurve = new ReferenceCurve{this};
    QVERIFY(refCurve);

    model->setReferenceCurve(refCurve);
    QVERIFY(!model->hasChanges());

    QCOMPARE(hasChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    QVERIFY(model->upper()->isEmpty());
    QVERIFY(model->middle()->isEmpty());
    QVERIFY(model->lower()->isEmpty());

    model->setResultType(504);
    QCOMPARE(updatingChangedSpy.count(), 2);

    auto sortModel = new InstanceResultSortModel{this};
    sortModel->setSourceModel(model);
    sortModel->setSortOrder(Qt::AscendingOrder);
    QCOMPARE(sortModel->rowCount(), 3);

    model->setData(sortModel->mapToSource(sortModel->index(0, 0)), true, Qt::UserRole + 5);

    QCOMPARE(updatingChangedSpy.count(), 3);
    QTRY_COMPARE(updatingChangedSpy.count(), 4);

    QCOMPARE(hasChangedSpy.count(), 1);
    QVERIFY(model->hasChanges());

    // reference curve is identical with the only selected result
    std::forward_list<QVector2D> samples{{0, 0}, {1, 1}, {2, 1}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), samples);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), samples);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), samples);

    model->selectAll();

    QCOMPARE(updatingChangedSpy.count(), 5);
    QTRY_COMPARE(updatingChangedSpy.count(), 6);

    std::forward_list<QVector2D> averageCurve{{0, 1}, {1, 2}, {2, 1}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), averageCurve);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), averageCurve);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), averageCurve);

    model->setJitter(1.5);

    QCOMPARE(updatingChangedSpy.count(), 7);
    QTRY_COMPARE(updatingChangedSpy.count(), 8);

    std::forward_list<QVector2D> upperCurve{{0, 2}, {1, 2}, {2, 2}};
    std::forward_list<QVector2D> lowerCurve{{0, 1}, {1, 1}, {2, 1}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), upperCurve);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), averageCurve);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), lowerCurve);

    model->setTriggerType(504);

    QCOMPARE(updatingChangedSpy.count(), 9);
    QTRY_COMPARE(updatingChangedSpy.count(), 10);

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), upperCurve);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), averageCurve);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), lowerCurve);

    model->setThreshold(1.0);

    QCOMPARE(updatingChangedSpy.count(), 11);
    QTRY_COMPARE(updatingChangedSpy.count(), 12);

    std::forward_list<QVector2D> upperCurveWithLwmThreshold{{0, 2}, {1, 2}, {2, 2}};
    std::forward_list<QVector2D> averageCurveWithLwmThreshold{{0, 4.0f / 3.0f}, {1, 2}, {2, 2}};
    std::forward_list<QVector2D> lowerCurveWithLwmThreshold{{0, 4.0f / 3.0f}, {1, 4.0f / 3.0f}, {2, 2}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), upperCurveWithLwmThreshold);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), averageCurveWithLwmThreshold);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), lowerCurveWithLwmThreshold);
}

void ReferenceCurveConstructorTest::testMedianCurve()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy dataChangedSpy(model, &ReferenceCurveConstructor::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ReferenceCurveConstructor::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QString productDir = QFINDTESTDATA("testdata/reference_data/median");
    QDir dir{productDir};
    QVERIFY(dir.exists());

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);
    QCOMPARE(loadingChangedSpy.count(), 0);

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    QSignalSpy hasChangedSpy(model, &ReferenceCurveConstructor::hasChanged);
    QVERIFY(hasChangedSpy.isValid());

    auto refCurve = new ReferenceCurve{this};
    QVERIFY(refCurve);

    model->setReferenceCurve(refCurve);
    QVERIFY(!model->hasChanges());

    QCOMPARE(hasChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    QVERIFY(model->upper()->isEmpty());
    QVERIFY(model->middle()->isEmpty());
    QVERIFY(model->lower()->isEmpty());

    model->setResultType(504);
    QCOMPARE(updatingChangedSpy.count(), 2);

    model->setReferenceType(ReferenceCurve::ReferenceType::Median);

    QCOMPARE(updatingChangedSpy.count(), 4);

    QCOMPARE(hasChangedSpy.count(), 1);
    QVERIFY(model->hasChanges());

    auto sortModel = new InstanceResultSortModel{this};
    sortModel->setSourceModel(model);
    sortModel->setSortOrder(Qt::AscendingOrder);
    QCOMPARE(sortModel->rowCount(), 3);

    model->setData(sortModel->mapToSource(sortModel->index(0, 0)), true, Qt::UserRole + 5);

    QCOMPARE(updatingChangedSpy.count(), 5);
    QTRY_COMPARE(updatingChangedSpy.count(), 6);

    // reference curve is identical with the only selected result
    std::forward_list<QVector2D> samples{{0, 0}, {1, 1}, {2, 0}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), samples);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), samples);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), samples);

    model->selectAll();

    QCOMPARE(updatingChangedSpy.count(), 7);
    QTRY_COMPARE(updatingChangedSpy.count(), 8);

    std::forward_list<QVector2D> medianCurve{{0, 0}, {1, 1}, {2, 0}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), medianCurve);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), medianCurve);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), medianCurve);

    model->setJitter(1.5);

    QCOMPARE(updatingChangedSpy.count(), 9);
    QTRY_COMPARE(updatingChangedSpy.count(), 10);

    std::forward_list<QVector2D> upperCurve{{0, 1}, {1, 1}, {2, 1}};
    std::forward_list<QVector2D> lowerCurve{{0, 0}, {1, 0}, {2, 0}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), upperCurve);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), medianCurve);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), lowerCurve);

    model->setTriggerType(504);

    QCOMPARE(updatingChangedSpy.count(), 11);
    QTRY_COMPARE(updatingChangedSpy.count(), 12);

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), upperCurve);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), medianCurve);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), lowerCurve);

    model->setThreshold(1.0);

    QCOMPARE(updatingChangedSpy.count(), 13);
    QTRY_COMPARE(updatingChangedSpy.count(), 14);

    std::forward_list<QVector2D> upperCurveWithLwmThreshold{{0, 4}, {1, 4}, {2, 4}};
    std::forward_list<QVector2D> medianCurveWithLwmThreshold{{0, 1}, {1, 4}, {2, 2}};
    std::forward_list<QVector2D> lowerCurveWithLwmThreshold{{0, 1}, {1, 1}, {2, 2}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), upperCurveWithLwmThreshold);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), medianCurveWithLwmThreshold);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), lowerCurveWithLwmThreshold);
}

void ReferenceCurveConstructorTest::testMinMaxCurve()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy dataChangedSpy(model, &ReferenceCurveConstructor::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ReferenceCurveConstructor::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QString productDir = QFINDTESTDATA("testdata/reference_data/minmax");
    QDir dir{productDir};
    QVERIFY(dir.exists());

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);
    QCOMPARE(loadingChangedSpy.count(), 0);

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    QSignalSpy hasChangedSpy(model, &ReferenceCurveConstructor::hasChanged);
    QVERIFY(hasChangedSpy.isValid());

    auto refCurve = new ReferenceCurve{this};
    QVERIFY(refCurve);

    model->setReferenceCurve(refCurve);
    QVERIFY(!model->hasChanges());

    QCOMPARE(hasChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    QVERIFY(model->upper()->isEmpty());
    QVERIFY(model->middle()->isEmpty());
    QVERIFY(model->lower()->isEmpty());

    model->setResultType(504);
    QCOMPARE(updatingChangedSpy.count(), 2);

    model->setReferenceType(ReferenceCurve::ReferenceType::MinMax);
    QCOMPARE(updatingChangedSpy.count(), 4);

    QCOMPARE(hasChangedSpy.count(), 1);
    QVERIFY(model->hasChanges());

    auto sortModel = new InstanceResultSortModel{this};
    sortModel->setSourceModel(model);
    sortModel->setSortOrder(Qt::AscendingOrder);
    QCOMPARE(sortModel->rowCount(), 3);

    model->setData(sortModel->mapToSource(sortModel->index(0, 0)), true, Qt::UserRole + 5);

    QCOMPARE(updatingChangedSpy.count(), 5);
    QTRY_COMPARE(updatingChangedSpy.count(), 6);

    // reference curve is identical with the only selected result
    std::forward_list<QVector2D> samples{{0, 0}, {1, 1}, {2, 0}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), samples);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), samples);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), samples);

    model->selectAll();

    QCOMPARE(updatingChangedSpy.count(), 7);
    QTRY_COMPARE(updatingChangedSpy.count(), 8);

    std::forward_list<QVector2D> maxCurve{{0, 2}, {1, 4}, {2, 2}};
    std::forward_list<QVector2D> middleCurve{{0, 1}, {1, 2.5}, {2, 1}};
    std::forward_list<QVector2D> minCurve{{0, 0}, {1, 1}, {2, 0}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), maxCurve);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), middleCurve);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), minCurve);

    model->setJitter(1.5);

    QCOMPARE(updatingChangedSpy.count(), 9);
    QTRY_COMPARE(updatingChangedSpy.count(), 10);

    // min-max curve is not affected by jitter
    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), maxCurve);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), middleCurve);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), minCurve);

    model->setTriggerType(504);

    QCOMPARE(updatingChangedSpy.count(), 11);
    QTRY_COMPARE(updatingChangedSpy.count(), 12);

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), maxCurve);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), middleCurve);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), minCurve);

    model->setThreshold(1.0);

    QCOMPARE(updatingChangedSpy.count(), 13);
    QTRY_COMPARE(updatingChangedSpy.count(), 14);

    std::forward_list<QVector2D> maxCurveWithLwmThreshold{{0, 2}, {1, 4}, {2, 2}};
    std::forward_list<QVector2D> middleCurveWithLwmThreshold{{0, 1.5}, {1, 3.5}, {2, 2}};
    std::forward_list<QVector2D> minCurveWithLwmThreshold{{0, 1}, {1, 3}, {2, 2}};

    QCOMPARE(model->upper()->sampleCount(), 3);
    QCOMPARE(model->upper()->samples(), maxCurveWithLwmThreshold);
    QCOMPARE(model->middle()->sampleCount(), 3);
    QCOMPARE(model->middle()->samples(), middleCurveWithLwmThreshold);
    QCOMPARE(model->lower()->sampleCount(), 3);
    QCOMPARE(model->lower()->samples(), minCurveWithLwmThreshold);
}

void ReferenceCurveConstructorTest::testSave()
{
    auto model = new ReferenceCurveConstructor{this};

    QSignalSpy dataChangedSpy(model, &ReferenceCurveConstructor::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QString productDir = QFINDTESTDATA("testdata/reference_data/median");
    QDir dir{productDir};
    QVERIFY(dir.exists());

    QSignalSpy modelResetSpy(model, &ReferenceCurveConstructor::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const QString fileName = QFINDTESTDATA("testdata/reference_data/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    QSignalSpy referenceCurveDataChangedSpy(product, &Product::referenceCurveDataChanged);
    QVERIFY(referenceCurveDataChangedSpy.isValid());

    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    model->setSeam(seam);
    QCOMPARE(model->seam(), seam);

    auto instanceModel = new ProductInstanceModel{this};
    model->setProductInstanceModel(instanceModel);
    instanceModel->setProduct(product);
    instanceModel->setDirectory(dir.path());

    QTRY_COMPARE(modelResetSpy.count(), 2);

    QSignalSpy updatingChangedSpy(model, &ReferenceCurveConstructor::updatingChanged);
    QVERIFY(updatingChangedSpy.isValid());

    QSignalSpy hasChangedSpy(model, &ReferenceCurveConstructor::hasChanged);
    QVERIFY(hasChangedSpy.isValid());

    auto refCurve = new ReferenceCurve{this};
    QVERIFY(refCurve);

    model->setReferenceCurve(refCurve);

    model->setResultType(504);
    QCOMPARE(updatingChangedSpy.count(), 2);

    model->setJitter(1.5);
    QCOMPARE(updatingChangedSpy.count(), 4);

    model->setReferenceType(ReferenceCurve::ReferenceType::Median);
    QCOMPARE(updatingChangedSpy.count(), 6);

    auto sortModel = new InstanceResultSortModel{this};
    sortModel->setSourceModel(model);
    sortModel->setSortOrder(Qt::AscendingOrder);
    QCOMPARE(sortModel->rowCount(), 3);

    model->selectAll();

    QCOMPARE(hasChangedSpy.count(), 1);

    QCOMPARE(updatingChangedSpy.count(), 7);
    QTRY_COMPARE(updatingChangedSpy.count(), 8);

    QVERIFY(model->hasChanges());

    // current product not set
    model->save();
    QVERIFY(model->hasChanges());

    QVERIFY(!product->referenceCurveData(refCurve->upper()));
    QVERIFY(!product->referenceCurveData(refCurve->middle()));
    QVERIFY(!product->referenceCurveData(refCurve->lower()));

    model->setCurrentProduct(product);

    QCOMPARE(referenceCurveDataChangedSpy.count(), 0);

    model->save();
    QVERIFY(!model->hasChanges());

    QCOMPARE(hasChangedSpy.count(), 2);

    QCOMPARE(refCurve->jitter(), 1.5f);
    QCOMPARE(refCurve->referenceType(), ReferenceCurve::ReferenceType::Median);
    QCOMPARE(refCurve->sourceOfCurve().size(), 3);
    QVERIFY(refCurve->isSource(QStringLiteral("0")));
    QVERIFY(refCurve->isSource(QStringLiteral("1")));
    QVERIFY(refCurve->isSource(QStringLiteral("2")));

    std::vector<QVector2D> upperCurve{{0, 1}, {1, 1}, {2, 1}};
    std::vector<QVector2D> medianCurve{{0, 0}, {1, 1}, {2, 0}};
    std::vector<QVector2D> lowerCurve{{0, 0}, {1, 0}, {2, 0}};

    QVERIFY(product->referenceCurveData(refCurve->upper()));
    QCOMPARE(product->referenceCurveData(refCurve->upper())->samples(), upperCurve);
    QVERIFY(product->referenceCurveData(refCurve->middle()));
    QCOMPARE(product->referenceCurveData(refCurve->middle())->samples(), medianCurve);
    QVERIFY(product->referenceCurveData(refCurve->lower()));
    QCOMPARE(product->referenceCurveData(refCurve->lower())->samples(), lowerCurve);

    QCOMPARE(referenceCurveDataChangedSpy.count(), 3);
}

QTEST_GUILESS_MAIN(ReferenceCurveConstructorTest)
#include "referenceCurveConstructorTest.moc"
