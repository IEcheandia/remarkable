#include <QTest>
#include <QSignalSpy>

#include "../overlayGroupModel.h"

Q_DECLARE_METATYPE(precitec::gui::components::image::OverlayGroupModel::OverlayGroup)
Q_DECLARE_METATYPE(std::vector<precitec::image::LayerType>)
using precitec::gui::components::image::OverlayGroupModel;
using namespace precitec::image;

class TestOverlayGroupModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testInvalidIndex();
    void testGroupToLayers_data();
    void testGroupToLayers();
    void testNames_data();
    void testNames();
    void testRoleNames_data();
    void testRoleNames();
    void testAvailable_data();
    void testAvailable();
    void testEnabled_data();
    void testEnabled();
};

void TestOverlayGroupModel::testCtor()
{
    OverlayGroupModel model;
    QCOMPARE(model.rowCount(QModelIndex()), 7);
    QCOMPARE(model.roleNames().count(), 3);
}

void TestOverlayGroupModel::testInvalidIndex()
{
    OverlayGroupModel model;
    const auto index = model.index(0, 1, QModelIndex());
    QCOMPARE(index.isValid(), false);
    QCOMPARE(model.data(index, Qt::DisplayRole).isValid(), false);
    QCOMPARE(model.data(index, Qt::UserRole).isValid(), false);
    QCOMPARE(model.data(index, Qt::UserRole + 1).isValid(), false);
    QCOMPARE(model.layers(index), std::vector<precitec::image::LayerType>{});
    QCOMPARE(model.setData(index, true, Qt::UserRole), false);
    QCOMPARE(model.setData(index, false, Qt::UserRole + 1), false);

    // some other invalid indices
    QCOMPARE(model.index(-1, 0, QModelIndex()).isValid(), false);
    QCOMPARE(model.index(9, 0, QModelIndex()).isValid(), false);
    const auto validIndex = model.index(0, 0, QModelIndex());
    QVERIFY(validIndex.isValid());
    QCOMPARE(model.index(0, 0, validIndex).isValid(), false);
}

void TestOverlayGroupModel::testGroupToLayers_data()
{
    QTest::addColumn<OverlayGroupModel::OverlayGroup>("group");
    QTest::addColumn<std::vector<precitec::image::LayerType>>("layers");

    QTest::newRow("Line")      << OverlayGroupModel::OverlayGroup::Line      << std::vector<LayerType>{eLayerLine, eLayerLineTransp};
    QTest::newRow("Contour")   << OverlayGroupModel::OverlayGroup::Contour   << std::vector<LayerType>{eLayerContour, eLayerContourTransp};
    QTest::newRow("Position")  << OverlayGroupModel::OverlayGroup::Position  << std::vector<LayerType>{eLayerPosition, eLayerPositionTransp};
    QTest::newRow("Text")      << OverlayGroupModel::OverlayGroup::Text      << std::vector<LayerType>{eLayerText, eLayerTextTransp};
    QTest::newRow("Grid")      << OverlayGroupModel::OverlayGroup::Grid      << std::vector<LayerType>{eLayerGridTransp};
    QTest::newRow("Image")     << OverlayGroupModel::OverlayGroup::Image     << std::vector<LayerType>{eLayerImage};
    QTest::newRow("LiveImage") << OverlayGroupModel::OverlayGroup::LiveImage << std::vector<LayerType>{};
}

void TestOverlayGroupModel::testGroupToLayers()
{
    QFETCH(OverlayGroupModel::OverlayGroup, group);
    QTEST(OverlayGroupModel::groupToLayers(group), "layers");
    OverlayGroupModel model;
    const auto index = model.index(group);
    QTEST(model.layers(index), "layers");
}

void TestOverlayGroupModel::testNames_data()
{
    QTest::addColumn<OverlayGroupModel::OverlayGroup>("group");
    QTest::addColumn<QString>("name");

    QTest::newRow("Line")      << OverlayGroupModel::OverlayGroup::Line      << QStringLiteral("Line");
    QTest::newRow("Contour")   << OverlayGroupModel::OverlayGroup::Contour   << QStringLiteral("Contour");
    QTest::newRow("Position")  << OverlayGroupModel::OverlayGroup::Position  << QStringLiteral("Position");
    QTest::newRow("Text")      << OverlayGroupModel::OverlayGroup::Text      << QStringLiteral("Text");
    QTest::newRow("Grid")      << OverlayGroupModel::OverlayGroup::Grid      << QStringLiteral("Grid");
    QTest::newRow("Image")     << OverlayGroupModel::OverlayGroup::Image     << QStringLiteral("Image");
    QTest::newRow("LiveImage") << OverlayGroupModel::OverlayGroup::LiveImage << QStringLiteral("Live image");
}

void TestOverlayGroupModel::testNames()
{
    QFETCH(OverlayGroupModel::OverlayGroup, group);
    OverlayGroupModel model;
    const auto index = model.index(group);
    QTEST(model.data(index, Qt::DisplayRole).toString(), "name");
}

void TestOverlayGroupModel::testRoleNames_data()
{
    QTest::addColumn<int>("role");
    QTest::addColumn<QByteArray>("name");

    QTest::newRow("display") << int(Qt::DisplayRole) << QByteArrayLiteral("display");
    QTest::newRow("available") << int(Qt::UserRole) << QByteArrayLiteral("available");
    QTest::newRow("enabled") << int(Qt::UserRole + 1) << QByteArrayLiteral("enabled");
    QTest::newRow("not a role") << int(Qt::UserRole + 2) << QByteArray();
}

void TestOverlayGroupModel::testRoleNames()
{
    OverlayGroupModel model;
    const auto roleNames = model.roleNames();
    QFETCH(int, role);
    QTEST(roleNames[role], "name");
}

void TestOverlayGroupModel::testAvailable_data()
{
    QTest::addColumn<OverlayGroupModel::OverlayGroup>("group");

    QTest::newRow("Line")      << OverlayGroupModel::OverlayGroup::Line;
    QTest::newRow("Contour")   << OverlayGroupModel::OverlayGroup::Contour;
    QTest::newRow("Position")  << OverlayGroupModel::OverlayGroup::Position;
    QTest::newRow("Text")      << OverlayGroupModel::OverlayGroup::Text;
    QTest::newRow("Grid")      << OverlayGroupModel::OverlayGroup::Grid;
    QTest::newRow("Image")     << OverlayGroupModel::OverlayGroup::Image;
    QTest::newRow("LiveImage") << OverlayGroupModel::OverlayGroup::LiveImage;
}

void TestOverlayGroupModel::testAvailable()
{
    QFETCH(OverlayGroupModel::OverlayGroup, group);
    OverlayGroupModel model;
    const auto index = model.index(group);
    const int role = model.roleNames().key(QByteArrayLiteral("available"));
    QCOMPARE(model.data(index, role).toBool(), false);
    QSignalSpy dataChangedSpy(&model, &OverlayGroupModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    model.setAvailable(index, true);
    QCOMPARE(model.data(index, role).toBool(), true);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), index);
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), index);
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>(), QVector<int>{role});

    // setting again to true should not emit the signal
    model.setAvailable(index, true);
    QCOMPARE(dataChangedSpy.count(), 1);

    // set back to false
    model.setAvailable(index, false);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(model.data(index, role).toBool(), false);
}

void TestOverlayGroupModel::testEnabled_data()
{
    QTest::addColumn<OverlayGroupModel::OverlayGroup>("group");

    QTest::newRow("Line")      << OverlayGroupModel::OverlayGroup::Line;
    QTest::newRow("Contour")   << OverlayGroupModel::OverlayGroup::Contour;
    QTest::newRow("Position")  << OverlayGroupModel::OverlayGroup::Position;
    QTest::newRow("Text")      << OverlayGroupModel::OverlayGroup::Text;
    QTest::newRow("Grid")      << OverlayGroupModel::OverlayGroup::Grid;
    QTest::newRow("Image")     << OverlayGroupModel::OverlayGroup::Image;
    QTest::newRow("LiveImage") << OverlayGroupModel::OverlayGroup::LiveImage;
}

void TestOverlayGroupModel::testEnabled()
{
    QFETCH(OverlayGroupModel::OverlayGroup, group);
    OverlayGroupModel model;
    const auto index = model.index(group);
    const int role = model.roleNames().key(QByteArrayLiteral("enabled"));
    QCOMPARE(model.data(index, role).toBool(), true);

    QSignalSpy dataChangedSpy(&model, &OverlayGroupModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    model.setData(index, false, role);
    QCOMPARE(model.data(index, role).toBool(), false);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), index);
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), index);
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>(), QVector<int>{role});

    // setting again to false should not emit the signal
    model.setAvailable(index, false);
    QCOMPARE(dataChangedSpy.count(), 1);

    // set back to true
    model.setData(index, true, role);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(model.data(index, role).toBool(), true);
}

QTEST_GUILESS_MAIN(TestOverlayGroupModel)
#include "testOverlayGroupModel.moc"
