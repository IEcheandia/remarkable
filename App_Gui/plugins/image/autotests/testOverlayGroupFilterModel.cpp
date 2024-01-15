#include <QTest>
#include <QSignalSpy>

#include "../overlayGroupModel.h"
#include "../overlayGroupFilterModel.h"

Q_DECLARE_METATYPE(precitec::gui::components::image::OverlayGroupModel::OverlayGroup)
Q_DECLARE_METATYPE(std::vector<precitec::image::LayerType>)
using precitec::gui::components::image::OverlayGroupModel;
using precitec::gui::components::image::OverlayGroupFilterModel;
using namespace precitec::image;

class TestOverlayGroupFilterModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testAcceptsRow_data();
    void testAcceptsRow();
    void testSwapEnabled_data();
    void testSwapEnabled();
    void testSwapEnabledInvalidIndex();
};

void TestOverlayGroupFilterModel::testCtor()
{
    OverlayGroupModel model;
    QCOMPARE(model.rowCount(QModelIndex()), 7);
    OverlayGroupFilterModel filterModel;
    filterModel.setSourceModel(&model);
    QCOMPARE(filterModel.rowCount(), 0);
}

void TestOverlayGroupFilterModel::testAcceptsRow_data()
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

void TestOverlayGroupFilterModel::testAcceptsRow()
{
    OverlayGroupModel model;
    QFETCH(OverlayGroupModel::OverlayGroup, group);
    const auto sourceIndex = model.index(group);

    OverlayGroupFilterModel filterModel;
    filterModel.setSourceModel(&model);

    model.setAvailable(sourceIndex, true);
    QCOMPARE(filterModel.rowCount(QModelIndex()), 1);
    const auto filterIndex = filterModel.index(0, 0);
    QTEST(filterModel.data(filterIndex, Qt::DisplayRole).toString(), "name");
    QCOMPARE(filterModel.data(filterIndex, Qt::UserRole).toBool(), true);
    QCOMPARE(filterModel.data(filterIndex, Qt::UserRole + 1).toBool(), true);
}

void TestOverlayGroupFilterModel::testSwapEnabled_data()
{
    QTest::addColumn<OverlayGroupModel::OverlayGroup>("group");
    QTest::addColumn<QString>("name");

    QTest::newRow("Line")      << OverlayGroupModel::OverlayGroup::Line;
    QTest::newRow("Contour")   << OverlayGroupModel::OverlayGroup::Contour;
    QTest::newRow("Position")  << OverlayGroupModel::OverlayGroup::Position;
    QTest::newRow("Text")      << OverlayGroupModel::OverlayGroup::Text;
    QTest::newRow("Grid")      << OverlayGroupModel::OverlayGroup::Grid;
    QTest::newRow("Image")     << OverlayGroupModel::OverlayGroup::Image;
    QTest::newRow("LiveImage") << OverlayGroupModel::OverlayGroup::LiveImage;
}

void TestOverlayGroupFilterModel::testSwapEnabled()
{
    OverlayGroupModel model;
    QFETCH(OverlayGroupModel::OverlayGroup, group);
    const auto sourceIndex = model.index(group);
    model.setAvailable(sourceIndex, true);

    OverlayGroupFilterModel filterModel;
    filterModel.setSourceModel(&model);
    const auto filterIndex = filterModel.index(0, 0);
    QSignalSpy dataChangedSpy(&filterModel, &QAbstractItemModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    filterModel.swapEnabled(0);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), filterIndex);
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), filterIndex);
    QCOMPARE(filterModel.data(filterIndex, Qt::UserRole + 1).toBool(), false);
    filterModel.swapEnabled(0);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(filterModel.data(filterIndex, Qt::UserRole + 1).toBool(), true);
}

void TestOverlayGroupFilterModel::testSwapEnabledInvalidIndex()
{
    OverlayGroupModel model;
    OverlayGroupFilterModel filterModel;
    filterModel.setSourceModel(&model);
    QSignalSpy dataChangedSpy(&filterModel, &QAbstractItemModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    filterModel.swapEnabled(0);
    QCOMPARE(dataChangedSpy.count(), 0);
}

QTEST_GUILESS_MAIN(TestOverlayGroupFilterModel)

#include "testOverlayGroupFilterModel.moc"
