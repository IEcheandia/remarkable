#include <QTest>
#include <QSignalSpy>

#include "../src/qualityNormModel.h"
#include "../src/qualityNorm.h"
#include "../src/qualityNormLevel.h"
#include "../src/qualityNormResult.h"
#include "../src/gaugeRange.h"

using precitec::storage::QualityNormModel;
using precitec::storage::QualityNorm;
using precitec::storage::QualityNormLevel;
using precitec::storage::QualityNormResult;
using precitec::storage::GaugeRange;

class TestQualityNormModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testConfigurationDirecory();
    void testLoad();
};

void TestQualityNormModel::testCtor()
{
    QualityNormModel model{this};
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.configurationDirectory().isEmpty());
}

void TestQualityNormModel::testRoleNames()
{
    QualityNormModel model;
    const auto roles = model.roleNames();
    QCOMPARE(roles.size(), 2);
    QCOMPARE(roles.value(Qt::DisplayRole), QByteArrayLiteral("name"));
    QCOMPARE(roles.value(Qt::UserRole), QByteArrayLiteral("uuid"));
}

void TestQualityNormModel::testConfigurationDirecory()
{
    QualityNormModel model;
    QSignalSpy configurationDirectoryChangedSpy(&model, &QualityNormModel::configurationDirectoryChanged);
    QVERIFY(configurationDirectoryChangedSpy.isValid());

    model.setConfigurationDirectory(QStringLiteral(""));
    QCOMPARE(configurationDirectoryChangedSpy.count(), 0);

    model.setConfigurationDirectory(QStringLiteral("someDir"));
    QCOMPARE(model.configurationDirectory(), QStringLiteral("someDir"));
    QCOMPARE(configurationDirectoryChangedSpy.count(), 1);

    model.setConfigurationDirectory(QStringLiteral("someDir"));
    QCOMPARE(configurationDirectoryChangedSpy.count(), 1);
}

void TestQualityNormModel::testLoad()
{
    QualityNormModel model;
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    QCOMPARE(model.rowCount(), 0);

    model.setConfigurationDirectory(QDir{QFINDTESTDATA("testdata/quality_norms/")}.path());
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(model.m_qualityNorms.at(0)->name(), QStringLiteral("None"));
    QCOMPARE(model.m_qualityNorms.at(1)->name(), QStringLiteral("ISO"));
    QCOMPARE(model.m_qualityNorms.at(1)->uuid(), QUuid("0477aa1d-168a-4437-80bf-d1da322232c3"));

    auto qn = model.qualityNorm(QUuid("0477aa1d-168a-4437-80bf-d1da322232c3"));
    QVERIFY(qn);
    QVERIFY(qn->qualityNormResult(113));
    QVERIFY(qn->qualityNormResult(114));
    QVERIFY(qn->qualityNormResult(115));
    QVERIFY(qn->qualityNormResult(116));
}

QTEST_GUILESS_MAIN(TestQualityNormModel)
#include "testQualityNormModel.moc"



