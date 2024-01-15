#include <QTest>
#include <QSignalSpy>

#include "../gatewayFilterModel.h"
#include "../gatewayModel.h"
#include "../viConfigService.h"

using precitec::gui::components::ethercat::GatewayFilterModel;
using precitec::gui::components::ethercat::GatewayModel;
using precitec::gui::components::ethercat::ViConfigService;

class EthercatGatewayModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetInputData();
    void testSetOutputData();
    void testSetViConfigService();
    void testInvalidInputPosition();
};

void EthercatGatewayModelTest::testCtor()
{
    GatewayModel model;
    QVERIFY(!model.viConfig());
    QCOMPARE(model.inputData(), QByteArray());
    QCOMPARE(model.outputData(), QByteArray());

    QCOMPARE(model.rowCount(), 0);

    model.setGateway(0, 20 * 8, 20 * 8);

    QCOMPARE(model.rowCount(), 20 * 8 * 2);

    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            const auto index = model.index(i * 8 + j, 0);
            QCOMPARE(index.data(), QStringLiteral("Reserved"));
            QCOMPARE(index.data(Qt::UserRole).toInt(), i);
            QCOMPARE(index.data(Qt::UserRole + 1).toInt(), j);
            QCOMPARE(index.data(Qt::UserRole + 2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Input);
            QCOMPARE(index.data(Qt::UserRole + 3).toBool(), false);
        }
    }

    for (int i = 20; i < 40; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            const auto index = model.index(i * 8 + j, 0);
            QCOMPARE(index.data(), QStringLiteral("Reserved"));
            QCOMPARE(index.data(Qt::UserRole).toInt(), i - 20);
            QCOMPARE(index.data(Qt::UserRole + 1).toInt(), j);
            QCOMPARE(index.data(Qt::UserRole + 2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Output);
            QCOMPARE(index.data(Qt::UserRole + 3).toBool(), false);
        }
    }
}

void EthercatGatewayModelTest::testSetInputData()
{
    GatewayModel model;
    model.setGateway(0, 20 * 8, 20 * 8);

    GatewayFilterModel inputModel;
    inputModel.setSignalType(ViConfigService::SignalType::Input);
    inputModel.setSourceModel(&model);
    QCOMPARE(inputModel.rowCount(), 20 * 8);

    GatewayFilterModel outputModel;
    outputModel.setSignalType(ViConfigService::SignalType::Output);
    outputModel.setSourceModel(&model);
    QCOMPARE(outputModel.rowCount(), 20 * 8);

    QSignalSpy inputDataChangedSpy{&model, &GatewayModel::inputDataChanged};
    QVERIFY(inputDataChangedSpy.isValid());
    QSignalSpy outputDataChangedSpy{&model, &GatewayModel::outputDataChanged};
    QVERIFY(outputDataChangedSpy.isValid());
    QSignalSpy dataChangedSpy{&model, &GatewayModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QByteArray testData(5, 0);
    testData[0] = 0xFF;
    testData[1] = 0x1;
    testData[2] = 0x2;
    testData[3] = 0x3;
    testData[4] = 0x4;
    model.setInputData(testData);
    QCOMPARE(model.inputData(), testData);
    QCOMPARE(model.outputData(), QByteArray());
    QCOMPARE(inputDataChangedSpy.count(), 1);
    QCOMPARE(outputDataChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(20 * 8 -1, 0));
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>(), QVector<int>() << Qt::UserRole + 3);

    // setting same data should not change again
    model.setInputData(testData);
    QCOMPARE(inputDataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    // test the data
    // first byte is all true
    for (int i = 0; i < 8; i++)
    {
        QCOMPARE(inputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), true);
    }
    // second byte has first bit true, all other false
    QCOMPARE(inputModel.index(8, 0).data(Qt::UserRole + 3).toBool(), true);
    for (int i = 9; i < 16; i++)
    {
        QCOMPARE(inputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    // second byte has first bit false, second true all other false
    QCOMPARE(inputModel.index(16, 0).data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(inputModel.index(17, 0).data(Qt::UserRole + 3).toBool(), true);
    for (int i = 18; i < 24; i++)
    {
        QCOMPARE(inputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    // third byte has first two bits true, all other false
    QCOMPARE(inputModel.index(24, 0).data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(inputModel.index(25, 0).data(Qt::UserRole + 3).toBool(), true);
    for (int i = 26; i < 32; i++)
    {
        QCOMPARE(inputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    // fourth byte has first two bits false, one true, remaining false
    for (int i = 32; i < 34; i++)
    {
        QCOMPARE(inputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    QCOMPARE(inputModel.index(34, 0).data(Qt::UserRole + 3).toBool(), true);
    for (int i = 35; i < 40; i++)
    {
        QCOMPARE(inputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    // all remaining bits including output are false
    for (int i = 40; i < 20 * 8; i++)
    {
        QCOMPARE(inputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    for (int i = 0; i < 20 * 8; i++)
    {
        QCOMPARE(outputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
}

void EthercatGatewayModelTest::testSetOutputData()
{
    GatewayModel model;
    model.setGateway(0, 20 * 8, 20 * 8);

    GatewayFilterModel inputModel;
    inputModel.setSignalType(ViConfigService::SignalType::Input);
    inputModel.setSourceModel(&model);
    QCOMPARE(inputModel.rowCount(), 20 * 8);

    GatewayFilterModel outputModel;
    outputModel.setSignalType(ViConfigService::SignalType::Output);
    outputModel.setSourceModel(&model);
    QCOMPARE(outputModel.rowCount(), 20 * 8);

    QSignalSpy inputDataChangedSpy{&model, &GatewayModel::inputDataChanged};
    QVERIFY(inputDataChangedSpy.isValid());
    QSignalSpy outputDataChangedSpy{&model, &GatewayModel::outputDataChanged};
    QVERIFY(outputDataChangedSpy.isValid());
    QSignalSpy dataChangedSpy{&model, &GatewayModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QByteArray testData(5, 0);
    testData[0] = 0xFF;
    testData[1] = 0x1;
    testData[2] = 0x2;
    testData[3] = 0x3;
    testData[4] = 0x4;
    model.setOutputData(testData);
    QCOMPARE(model.outputData(), testData);
    QCOMPARE(model.inputData(), QByteArray());
    QCOMPARE(outputDataChangedSpy.count(), 1);
    QCOMPARE(inputDataChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(20 * 8, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(40 * 8 -1, 0));
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>(), QVector<int>() << Qt::UserRole + 3);

    // setting same data should not change again
    model.setOutputData(testData);
    QCOMPARE(outputDataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    // test the data
    // first byte is all true
    for (int i = 0; i < 8; i++)
    {
        QCOMPARE(outputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), true);
    }
    // second byte has first bit true, all other false
    QCOMPARE(outputModel.index(8, 0).data(Qt::UserRole + 3).toBool(), true);
    for (int i = 9; i < 16; i++)
    {
        QCOMPARE(outputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    // second byte has first bit false, second true all other false
    QCOMPARE(outputModel.index(16, 0).data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(outputModel.index(17, 0).data(Qt::UserRole + 3).toBool(), true);
    for (int i = 18; i < 24; i++)
    {
        QCOMPARE(outputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    // third byte has first two bits true, all other false
    QCOMPARE(outputModel.index(24, 0).data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(outputModel.index(25, 0).data(Qt::UserRole + 3).toBool(), true);
    for (int i = 26; i < 32; i++)
    {
        QCOMPARE(outputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    // fourth byte has first two bits false, one true, remaining false
    for (int i = 32; i < 34; i++)
    {
        QCOMPARE(outputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    QCOMPARE(outputModel.index(34, 0).data(Qt::UserRole + 3).toBool(), true);
    for (int i = 35; i < 40; i++)
    {
        QCOMPARE(outputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    // all remaining bits including output are false
    for (int i = 40; i < 20 * 8; i++)
    {
        QCOMPARE(outputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
    for (int i = 0; i < 20 * 8; i++)
    {
        QCOMPARE(inputModel.index(i, 0).data(Qt::UserRole + 3).toBool(), false);
    }
}

void EthercatGatewayModelTest::testSetViConfigService()
{
    GatewayModel model;
    model.setGateway(0, 20 * 8, 20 * 8);
    QSignalSpy modelResetSpy{&model, &GatewayModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy viConfigServiceChangedSpy{&model, &GatewayModel::viConfigChanged};
    QVERIFY(viConfigServiceChangedSpy.isValid());

    ViConfigService service;
    model.setViConfig(&service);
    QCOMPARE(model.viConfig(), &service);
    QCOMPARE(viConfigServiceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // setting same should not change
    model.setViConfig(&service);
    QCOMPARE(viConfigServiceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // but setting the test data should emit again
    QString path = QFINDTESTDATA("../../../../wm_inst/config_templates");
    QVERIFY(!path.isEmpty());
    service.setConfigurationDir(path);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 2);

    // we haven't inited the gateway yet, it's still all reserved
    for (int i = 0; i < 40; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            const auto index = model.index(i * 8 + j, 0);
            QCOMPARE(index.data(), QStringLiteral("Reserved"));
        }
    }

    // let's init the gateway
    model.setGateway(1, 20 * 8, 20 * 8);
    QCOMPARE(modelResetSpy.count(), 3);
    // let's test some interesting examples
    QCOMPARE(model.index(20 * 8 + 1, 0).data().toString(), QStringLiteral("Part OK (OK=1)"));
    QCOMPARE(model.index(20 * 8 + 41, 0).data().toString(), QStringLiteral("System Fault Image Acquisition"));
    QCOMPARE(model.index(97, 0).data().toString(), QStringLiteral("Part No Bit 1"));
    QCOMPARE(model.index(20 * 8 + 37, 0).data().toString(), QStringLiteral("Option: Head Monitor: GlassDirty"));
}

void EthercatGatewayModelTest::testInvalidInputPosition()
{
    GatewayModel model;
    model.setGateway(0, 20 * 8, 20 * 8);
    QSignalSpy modelResetSpy{&model, &GatewayModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy viConfigServiceChangedSpy{&model, &GatewayModel::viConfigChanged};
    QVERIFY(viConfigServiceChangedSpy.isValid());

    ViConfigService service;
    model.setViConfig(&service);
    QCOMPARE(model.viConfig(), &service);
    QCOMPARE(viConfigServiceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // setting same should not change
    model.setViConfig(&service);
    QCOMPARE(viConfigServiceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // but setting the test data should emit again
    QString path = QFINDTESTDATA("../../../../wm_inst/config_templates");
    QVERIFY(!path.isEmpty());
    service.setConfigurationDir(path);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 2);

    // let's init the gateway
    model.setGateway(1, 10 * 8, 20 * 8);
    QCOMPARE(model.index(10 * 8 + 5, 0).data().toString(), QStringLiteral("Reserved"));
}

QTEST_GUILESS_MAIN(EthercatGatewayModelTest)
#include "ethercatGatewayModelTest.moc"
