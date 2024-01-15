#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>

#include "../src/instanceResultModel.h"
#include "../src/instanceResultSortModel.h"
#include "productInstanceModel.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "precitec/dataSet.h"

using precitec::gui::InstanceResultModel;
using precitec::gui::InstanceResultSortModel;
using precitec::gui::components::plotter::DataSet;
using precitec::storage::ProductInstanceModel;
using precitec::storage::Product;
using precitec::storage::SeamSeries;
using precitec::storage::Seam;

class InstanceResultModelTest: public QObject
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
};

void InstanceResultModelTest::testCtor()
{
    auto model = new InstanceResultModel{this};

    QVERIFY(!model->productInstanceModel());
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
}

void InstanceResultModelTest::testRoleNames()
{
    auto model = new InstanceResultModel{this};

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

void InstanceResultModelTest::testSetProductInstanceModel()
{
    auto model = new InstanceResultModel{this};

    QVERIFY(!model->productInstanceModel());

    QSignalSpy productInstanceModelChangedSpy(model, &InstanceResultModel::productInstanceModelChanged);
    QVERIFY(productInstanceModelChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &InstanceResultModel::modelReset);
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

void InstanceResultModelTest::testSeam()
{
    auto model = new InstanceResultModel{this};

    QVERIFY(!model->seam());

    QSignalSpy seamChangedSpy{model, &InstanceResultModel::seamChanged};
    QVERIFY(seamChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &InstanceResultModel::modelReset);
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

void InstanceResultModelTest::testUpdateModel()
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

    auto model = new InstanceResultModel{this};

    QSignalSpy modelResetSpy(model, &InstanceResultModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    QSignalSpy dataChangedSpy(model, &InstanceResultModel::dataChanged);
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

void InstanceResultModelTest::testResultType()
{
    auto model = new InstanceResultModel{this};

    QCOMPARE(model->resultType(), -1);

    QSignalSpy resultTypeChangedSpy(model, &InstanceResultModel::resultTypeChanged);
    QVERIFY(resultTypeChangedSpy.isValid());

    model->setResultType(-1);
    QCOMPARE(resultTypeChangedSpy.count(), 0);

    model->setResultType(504);
    QCOMPARE(model->resultType(), 504);
    QCOMPARE(resultTypeChangedSpy.count(), 1);

    model->setResultType(504);
    QCOMPARE(resultTypeChangedSpy.count(), 1);
}

void InstanceResultModelTest::testTriggerType()
{
    auto model = new InstanceResultModel{this};

    QCOMPARE(model->triggerType(), -1);

    QSignalSpy triggerTypeChangedSpy(model, &InstanceResultModel::triggerTypeChanged);
    QVERIFY(triggerTypeChangedSpy.isValid());

    model->setTriggerType(-1);
    QCOMPARE(triggerTypeChangedSpy.count(), 0);

    model->setTriggerType(504);
    QCOMPARE(model->triggerType(), 504);
    QCOMPARE(triggerTypeChangedSpy.count(), 1);

    model->setTriggerType(504);
    QCOMPARE(triggerTypeChangedSpy.count(), 1);
}

void InstanceResultModelTest::testCurrentIndex()
{
    auto model = new InstanceResultModel{this};

    QVERIFY(!model->currentIndex().isValid());

    QSignalSpy currentIndexChangedSpy(model, &InstanceResultModel::currentIndexChanged);
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

    QSignalSpy modelResetSpy(model, &InstanceResultModel::modelReset);
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

void InstanceResultModelTest::testThreshold()
{
    auto model = new InstanceResultModel{this};

    QCOMPARE(model->threshold(), 0.0);

    QSignalSpy thresholdChangedSpy(model, &InstanceResultModel::thresholdChanged);
    QVERIFY(thresholdChangedSpy.isValid());

    model->setThreshold(0.0);
    QCOMPARE(thresholdChangedSpy.count(), 0);

    model->setThreshold(5.5);
    QCOMPARE(model->threshold(), 5.5);
    QCOMPARE(thresholdChangedSpy.count(), 1);

    model->setThreshold(5.5);
    QCOMPARE(thresholdChangedSpy.count(), 1);
}

void InstanceResultModelTest::testUpdateResult()
{
    auto model = new InstanceResultModel{this};

    QSignalSpy loadingChangedSpy(model, &InstanceResultModel::loadingChanged);
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

    QSignalSpy modelResetSpy(model, &InstanceResultModel::modelReset);
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

void InstanceResultModelTest::testLoading()
{
    auto model = new InstanceResultModel{this};

    QSignalSpy loadingChangedSpy(model, &InstanceResultModel::loadingChanged);
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

void InstanceResultModelTest::testSetData()
{
    auto model = new InstanceResultModel{this};

    QSignalSpy dataChangedSpy(model, &InstanceResultModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &InstanceResultModel::loadingChanged);
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

    QSignalSpy modelResetSpy(model, &InstanceResultModel::modelReset);
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

void InstanceResultModelTest::testSelectAll()
{
    auto model = new InstanceResultModel{this};

    QSignalSpy dataChangedSpy(model, &InstanceResultModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &InstanceResultModel::loadingChanged);
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

    QSignalSpy modelResetSpy(model, &InstanceResultModel::modelReset);
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

void InstanceResultModelTest::testSelectNone()
{
    auto model = new InstanceResultModel{this};

    QSignalSpy dataChangedSpy(model, &InstanceResultModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &InstanceResultModel::loadingChanged);
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

    QSignalSpy modelResetSpy(model, &InstanceResultModel::modelReset);
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

QTEST_GUILESS_MAIN(InstanceResultModelTest)
#include "instanceResultModelTest.moc"
