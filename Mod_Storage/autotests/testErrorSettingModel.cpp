#include <QTest>
#include <QSignalSpy>

#include "../src/resultSetting.h"
#include "../src/errorSettingModel.h"
#include "../src/productModel.h"

using precitec::storage::ResultSetting;
using precitec::storage::ErrorSettingModel;
using precitec::storage::ProductModel;

class TestErrorSettingModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testRoleNames();
    void testWriteUpdatedList();
    void testCreateAndAddItem();
    void testCreateNewError();
    void testCreateFileWithDefaultErrors();
    void testLoadErrorsNoFileFound();
    void testLoadErrors();
    void testDeleteError();
    void testUpdateValue();
    void testCheckAndAddNewError();
    void testUpdateUsedFlag();
    void cleanupTestCase();

private:
    QTemporaryDir m_dir;
    QString m_tempErrorStorageFile;
};

void TestErrorSettingModel::initTestCase()
{
    QVERIFY(m_dir.isValid());
    m_tempErrorStorageFile = m_dir.filePath(QStringLiteral("testErrors.json"));
}

void TestErrorSettingModel::testCtor()
{
    ErrorSettingModel model;
    QCOMPARE(model.rowCount(), 0);
    QString errorStorageFile = QDir{QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QStringLiteral("/config/")}.filePath(QStringLiteral("errorsConfig.json"));
    QCOMPARE(model.m_errorStorageFile, errorStorageFile);
}

void TestErrorSettingModel::testRoleNames()
{
    ErrorSettingModel model;
    const auto roles = model.roleNames();
    QCOMPARE(roles.size(), 15);
    QCOMPARE(roles.value(Qt::DisplayRole), QByteArrayLiteral("enumType"));
    QCOMPARE(roles.value(Qt::UserRole), QByteArrayLiteral("uuid"));
    QCOMPARE(roles.value(Qt::UserRole+1), QByteArrayLiteral("name"));
    QCOMPARE(roles.value(Qt::UserRole+2), QByteArrayLiteral("plotterNumber"));
    QCOMPARE(roles.value(Qt::UserRole+3), QByteArrayLiteral("plottable"));
    QCOMPARE(roles.value(Qt::UserRole+4), QByteArrayLiteral("min"));
    QCOMPARE(roles.value(Qt::UserRole+5), QByteArrayLiteral("max"));
    QCOMPARE(roles.value(Qt::UserRole+6), QByteArrayLiteral("lineColor"));
    QCOMPARE(roles.value(Qt::UserRole+7), QByteArrayLiteral("visibleItem"));
    QCOMPARE(roles.value(Qt::UserRole+8), QByteArrayLiteral("visualization"));
    QCOMPARE(roles.value(Qt::UserRole+9), QByteArrayLiteral("disabled"));
    QCOMPARE(roles.value(Qt::UserRole+10), QByteArrayLiteral("hue"));
    QCOMPARE(roles.value(Qt::UserRole+11), QByteArrayLiteral("saturation"));
    QCOMPARE(roles.value(Qt::UserRole+12), QByteArrayLiteral("lightness"));
    QCOMPARE(roles.value(Qt::UserRole+13), QByteArrayLiteral("qualityName"));
}

void TestErrorSettingModel::testWriteUpdatedList()
{
    QVERIFY(m_dir.isValid());
    ErrorSettingModel model;
    model.m_errorStorageFile = m_tempErrorStorageFile;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.writeUpdatedList();;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.createAndAddItem(QUuid::createUuid(), 7777);
    QCOMPARE(model.m_errorItems.size(), 1);
    QCOMPARE(model.rowCount(), 1);
}

void TestErrorSettingModel::testCreateAndAddItem()
{
    QVERIFY(m_dir.isValid());
    ErrorSettingModel model;
    model.m_errorStorageFile = m_tempErrorStorageFile;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.createAndAddItem(QUuid::createUuid(), 7777);
    QCOMPARE(model.m_errorItems.size(), 1);
    QCOMPARE(model.rowCount(), 1);
}

void TestErrorSettingModel::testCreateNewError()
{
    QVERIFY(m_dir.isValid());
    ErrorSettingModel model;
    model.m_errorStorageFile = m_tempErrorStorageFile;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.createNewError(QStringLiteral("testError 111"), 1111);
    model.createNewError(QStringLiteral("testError 123"), 2222);
    model.createNewError(QStringLiteral("testError 444"), 3333);
    QCOMPARE(model.m_errorItems.size(), 3);
    QCOMPARE(model.rowCount(), 3);
}

void TestErrorSettingModel::testCreateFileWithDefaultErrors()
{
    QVERIFY(m_dir.isValid());
    ErrorSettingModel model;
    model.m_errorStorageFile = m_tempErrorStorageFile;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.createFileWithDefaultErrors();
    QCOMPARE(model.m_errorItems.size(), 14);
    QCOMPARE(model.rowCount(), 14);
}

void TestErrorSettingModel::testLoadErrorsNoFileFound()
{
    QVERIFY(m_dir.isValid());
    ErrorSettingModel model;
    model.m_errorStorageFile = m_tempErrorStorageFile;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.loadErrors();
    QCOMPARE(model.rowCount(), 14);
    QCOMPARE(model.m_errorItems.size(), 14);
}

void TestErrorSettingModel::testLoadErrors()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/errorsconfig.json"), dir.filePath(QStringLiteral("errorsconfig.json"))));
    QString tempErrorStorageFile = dir.filePath(QStringLiteral("errorsconfig.json"));
    QFINDTESTDATA(tempErrorStorageFile);
    ErrorSettingModel model;
    model.m_errorStorageFile = tempErrorStorageFile;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.loadErrors();
    QCOMPARE(model.rowCount(), 15);    
    QCOMPARE(model.m_errorItems.size(), 15);
    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("errorsconfig.json"))));
}

void TestErrorSettingModel::testDeleteError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/errorsconfig.json"), dir.filePath(QStringLiteral("errorsconfig.json"))));
    QString tempErrorStorageFile = dir.filePath(QStringLiteral("errorsconfig.json"));
    QFINDTESTDATA(tempErrorStorageFile);
    ErrorSettingModel model;
    model.m_errorStorageFile = tempErrorStorageFile;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.loadErrors();
    QCOMPARE(model.rowCount(), 15);    
    QCOMPARE(model.m_errorItems.size(), 15);
    model.deleteError(model.index(0), 5004);
    QCOMPARE(model.m_errorItems.size(), 14);
    QCOMPARE(model.rowCount(), 14);
    model.deleteError(model.index(0), 1001);
    QCOMPARE(model.m_errorItems.size(), 13);
    QCOMPARE(model.rowCount(), 13);
    model.deleteError(model.index(0), 1008);
    QCOMPARE(model.m_errorItems.size(), 12);
    QCOMPARE(model.rowCount(), 12);
    model.deleteError(model.index(0), 4711);
    QCOMPARE(model.m_errorItems.size(), 12);
    QCOMPARE(model.rowCount(), 12);
    model.deleteError(model.index(0), 5020);
    QCOMPARE(model.m_errorItems.size(), 11);
    QCOMPARE(model.rowCount(), 11);

    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("errorsconfig.json"))));
}

void TestErrorSettingModel::testUpdateValue()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/errorsconfig.json"), dir.filePath(QStringLiteral("errorsconfig.json"))));
    QString tempErrorStorageFile = dir.filePath(QStringLiteral("errorsconfig.json"));
    QFINDTESTDATA(tempErrorStorageFile);
    ErrorSettingModel model;
    QSignalSpy dataChangedSpy{&model, &ErrorSettingModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    model.m_errorStorageFile = tempErrorStorageFile;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.loadErrors();
    QCOMPARE(model.rowCount(), 15);    
    QCOMPARE(model.m_errorItems.size(), 15);
    QVERIFY(model.m_errorItems.at(0));
    QCOMPARE(model.m_errorItems.at(0)->name(), QStringLiteral("X position out of limits"));
    QVERIFY(model.m_errorItems.at(4));
    QCOMPARE(model.m_errorItems.at(4)->name(), QStringLiteral("RankViolation"));
    model.updateValue(model.index(0), QStringLiteral("new text for test"), ResultSetting::Type::Name);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(0));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(0));
    model.updateValue(model.index(4), QStringLiteral("new text for violation"), ResultSetting::Type::Name);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(4));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(4));
    QVERIFY(model.m_errorItems.at(0));
    QCOMPARE(model.m_errorItems.at(0)->name(), QStringLiteral("new text for test"));
    QVERIFY(model.m_errorItems.at(4));
    QCOMPARE(model.m_errorItems.at(4)->name(), QStringLiteral("new text for violation"));
   
    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("errorsconfig.json"))));

}

void TestErrorSettingModel::testCheckAndAddNewError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/errorsconfig.json"), dir.filePath(QStringLiteral("errorsconfig.json"))));
    QString tempErrorStorageFile = dir.filePath(QStringLiteral("errorsconfig.json"));
    QFINDTESTDATA(tempErrorStorageFile);
    ErrorSettingModel model;
    model.m_errorStorageFile = tempErrorStorageFile;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_errorItems.size(), 0);
    model.loadErrors();
    QCOMPARE(model.rowCount(), 15);    
    QCOMPARE(model.m_errorItems.size(), 15);
    QUuid newId = QUuid::createUuid();
    model.checkAndAddNewError(newId, 4711);
    QCOMPARE(model.rowCount(), 16);    
    QCOMPARE(model.m_errorItems.size(), 16);
    model.checkAndAddNewError(newId, 4711);
    QCOMPARE(model.rowCount(), 16);    
    QCOMPARE(model.m_errorItems.size(), 16);
    newId = QUuid::createUuid();
    model.checkAndAddNewError(newId, 4711);
    QCOMPARE(model.rowCount(), 16);    
    QCOMPARE(model.m_errorItems.size(), 16);
    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("errorsconfig.json"))));
}

void TestErrorSettingModel::testUpdateUsedFlag()
{
    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());
    ProductModel prodModel ;
    prodModel.loadProducts(dir);
    QVERIFY(m_dir.isValid());
    ErrorSettingModel model;
    model.m_errorStorageFile = m_tempErrorStorageFile;
    model.loadErrors();
    QCOMPARE(model.m_errorItems.size(), 14);
    model.setProductModel(&prodModel);
    model.updateUsedFlag();    
    
    std::vector<int> testEnums({1002, 1806, 5015, 5018, 5089, 5099});
    for (unsigned int index = 0; index < testEnums.size(); index++)
    {
        int data = testEnums[index];
        auto it = std::find_if(model.m_errorItems.begin(), model.m_errorItems.end(), [data] (auto p) { return p->enumType() == data; });
        if (it != model.m_errorItems.end())
        {
            QCOMPARE((*it)->disabled(), true);
        }  else
        {
            QFAIL("Test failed.");
        }
    }
}

void TestErrorSettingModel::cleanupTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QFile::remove(m_dir.filePath(QStringLiteral("testErrors.json"))));
}


QTEST_GUILESS_MAIN(TestErrorSettingModel)
#include "testErrorSettingModel.moc"

