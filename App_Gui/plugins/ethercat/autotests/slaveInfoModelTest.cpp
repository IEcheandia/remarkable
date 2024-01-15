#include <QTest>
#include <QSignalSpy>

#include "../slaveInfoModel.h"
#include "../../../src/serviceToGuiServer.h"

#include "event/ethercatDefines.h"
#include "event/viService.h"

using precitec::gui::ServiceToGuiServer;
using precitec::gui::components::ethercat::SlaveInfoModel;
using precitec::interface::SlaveInfo;

class SlaveInfoModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetService();
    void testType_data();
    void testType();
    void testProcessData();
};

void SlaveInfoModelTest::testCtor()
{
    SlaveInfoModel model;
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.service() == nullptr);
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("display"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("vendorId"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("productCode"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("instance"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("latestInputData"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("latestOutputData"));
    QCOMPARE(roleNames[Qt::UserRole + 5], QByteArrayLiteral("type"));
    QCOMPARE(roleNames[Qt::UserRole + 6], QByteArrayLiteral("inputSize"));
    QCOMPARE(roleNames[Qt::UserRole + 7], QByteArrayLiteral("outputSize"));
}

void SlaveInfoModelTest::testSetService()
{
    SlaveInfoModel model;
    QSignalSpy serviceChangedSpy{&model, &SlaveInfoModel::serviceChanged};
    QVERIFY(serviceChangedSpy.isValid());
    QSignalSpy modelResetSpy{&model, &QAbstractItemModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    auto service = new ServiceToGuiServer{{}, &model};
    model.setService(service);
    QCOMPARE(model.service(), service);
    QCOMPARE(serviceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // setting same should not change
    model.setService(service);
    QCOMPARE(serviceChangedSpy.count(), 1);

    // deleting the service should change
    service->deleteLater();
    QVERIFY(serviceChangedSpy.wait());
    QVERIFY(model.service() == nullptr);
    QCOMPARE(modelResetSpy.count(), 1);
}

void SlaveInfoModelTest::testType_data()
{
    QTest::addColumn<int>("vendor");
    QTest::addColumn<int>("product");
    QTest::addColumn<SlaveInfoModel::Type>("type");

    QTest::newRow("EK1100") << VENDORID_BECKHOFF << PRODUCTCODE_EK1100 << SlaveInfoModel::Type::Unknown;
    QTest::newRow("EL1018") << VENDORID_BECKHOFF << PRODUCTCODE_EL1018 << SlaveInfoModel::Type::DigitalIn;
    QTest::newRow("EL2008") << VENDORID_BECKHOFF << PRODUCTCODE_EL2008 << SlaveInfoModel::Type::DigitalOut;
    QTest::newRow("EL3102") << VENDORID_BECKHOFF << PRODUCTCODE_EL3102 << SlaveInfoModel::Type::AnalogIn;
    QTest::newRow("EL3162") << VENDORID_BECKHOFF << PRODUCTCODE_EL3162 << SlaveInfoModel::Type::Unknown;
    QTest::newRow("EL3702") << VENDORID_BECKHOFF << PRODUCTCODE_EL3702 << SlaveInfoModel::Type::AnalogOversamplingIn;
    QTest::newRow("EL4102") << VENDORID_BECKHOFF << PRODUCTCODE_EL4102 << SlaveInfoModel::Type::AnalogOut0To10;
    QTest::newRow("EL4132") << VENDORID_BECKHOFF << PRODUCTCODE_EL4132 << SlaveInfoModel::Type::AnalogOutPlusMinus10;
    QTest::newRow("EL5101") << VENDORID_BECKHOFF << PRODUCTCODE_EL5101 << SlaveInfoModel::Type::Unknown;
    QTest::newRow("kunbus") << VENDORID_KUNBUS << PRODUCTCODE_KUNBUS_GW << SlaveInfoModel::Type::Gateway;
    QTest::newRow("anybus") << VENDORID_HMS << PRODUCTCODE_ANYBUS_GW << SlaveInfoModel::Type::Gateway;
    QTest::newRow("fieldbus") << VENDORID_HILSCHER << PRODUCTCODE_FIELDBUS << SlaveInfoModel::Type::Gateway;
}

void SlaveInfoModelTest::testType()
{
    SlaveInfoModel model;
    ServiceToGuiServer service{{}};

    SlaveInfo info{1};
    EC_T_GET_SLAVE_INFO slaveInfo;
    QFETCH(int, vendor);
    QFETCH(int, product);
    slaveInfo.dwVendorId = vendor;
    slaveInfo.dwProductCode = product;
    strncpy(slaveInfo.abyDeviceName, "foo", sizeof(slaveInfo.abyDeviceName) - 1);
    slaveInfo.abyDeviceName[sizeof(slaveInfo.abyDeviceName) - 1] = '\0';  
    info.GetDataPtr()[0] = slaveInfo;
    service.SlaveInfoECAT(1, info);

    model.setService(&service);
    QCOMPARE(model.rowCount(), 1);
    QTEST(model.index(0, 0).data(Qt::UserRole + 5).value<SlaveInfoModel::Type>(), "type");
    QCOMPARE(model.index(0, 0).data().toByteArray(), QByteArrayLiteral("foo"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toInt(), vendor);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toInt(), product);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).toInt(), 1);
}

void SlaveInfoModelTest::testProcessData()
{
    SlaveInfoModel model;
    ServiceToGuiServer service{{}};

    SlaveInfo info{2};
    EC_T_GET_SLAVE_INFO slaveInfo1;
    slaveInfo1.dwVendorId = VENDORID_BECKHOFF;
    slaveInfo1.dwProductCode = PRODUCTCODE_EL1018;
    slaveInfo1.dwPdOffsIn = 0;
    slaveInfo1.dwPdSizeIn = 8;
    slaveInfo1.dwPdOffsOut = 8;
    slaveInfo1.dwPdSizeOut = 8;
    EC_T_GET_SLAVE_INFO slaveInfo2;
    slaveInfo2.dwVendorId = VENDORID_BECKHOFF;
    slaveInfo2.dwProductCode = PRODUCTCODE_EL1018;
    slaveInfo2.dwPdOffsIn = 16;
    slaveInfo2.dwPdSizeIn = 8;
    slaveInfo2.dwPdOffsOut = 24;
    slaveInfo2.dwPdSizeOut = 8;
    info.GetDataPtr()[0] = slaveInfo1;
    info.GetDataPtr()[1] = slaveInfo2;
    service.SlaveInfoECAT(2, info);

    model.setService(&service);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).toInt(), 1);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 2).toInt(), 2);

    QSignalSpy dataChangedSpy{&model, &SlaveInfoModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    std::vector<precitec::interface::ProcessData> data{{4, "abcd"}, {4, "fghi"}};
    service.ProcessImage(data, data);
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(1, 0));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 3).toByteArray(), QByteArrayLiteral("f"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 4).toByteArray(), QByteArrayLiteral("g"));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 3).toByteArray(), QByteArrayLiteral("h"));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 4).toByteArray(), QByteArrayLiteral("i"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 6).toUInt(), 8u);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 7).toUInt(), 8u);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 6).toUInt(), 8u);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 7).toUInt(), 8u);
}

QTEST_GUILESS_MAIN(SlaveInfoModelTest)
#include "slaveInfoModelTest.moc"
