#include <QTest>

#include "../src/topBarButtonFilterModel.h"
#include "../src/topBarButtonModel.h"

using precitec::gui::TopBarButtonFilterModel;
using precitec::gui::TopBarButtonModel;

class TopBarButtonFilterModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testGraphEditorAvailable_data();
    void testGraphEditorAvailable();
    void testHeadMonitorAvailable_data();
    void testHeadMonitorAvailable();
};

void TopBarButtonFilterModelTest::testCtor()
{
    auto filterModel = new TopBarButtonFilterModel{this};
    QCOMPARE(filterModel->showGraphEditor(), false);
    QCOMPARE(filterModel->isHeadMonitorAvailable(), false);
    QCOMPARE(filterModel->rowCount(), 0);
}

void TopBarButtonFilterModelTest::testGraphEditorAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<TopBarButtonModel::TopBarButton>>("components");

    QTest::newRow("enabled") << true << 9 << QVector<TopBarButtonModel::TopBarButton>{
        TopBarButtonModel::TopBarButton::Overview,
        TopBarButtonModel::TopBarButton::Login,
        TopBarButtonModel::TopBarButton::Results,
        TopBarButtonModel::TopBarButton::Statistics,
        TopBarButtonModel::TopBarButton::Simulation,
        TopBarButtonModel::TopBarButton::HeadMonitor,
        TopBarButtonModel::TopBarButton::Wizard,
        TopBarButtonModel::TopBarButton::Grapheditor,
        TopBarButtonModel::TopBarButton::Configuration
    };
    QTest::newRow("disabled") << false << 8 << QVector<TopBarButtonModel::TopBarButton>{
        TopBarButtonModel::TopBarButton::Overview,
        TopBarButtonModel::TopBarButton::Login,
        TopBarButtonModel::TopBarButton::Results,
        TopBarButtonModel::TopBarButton::Statistics,
        TopBarButtonModel::TopBarButton::Simulation,
        TopBarButtonModel::TopBarButton::HeadMonitor,
        TopBarButtonModel::TopBarButton::Wizard,
        TopBarButtonModel::TopBarButton::Configuration
    };
}

void TopBarButtonFilterModelTest::testGraphEditorAvailable()
{
    auto filterModel = new TopBarButtonFilterModel{this};
    auto model = new TopBarButtonModel{this};
    filterModel->setSourceModel(model);
    filterModel->setHeadMonitorAvailable(true);

    QFETCH(bool, enabled);
    filterModel->setShowGraphEditor(enabled);
    QCOMPARE(filterModel->showGraphEditor(), enabled);

    QTEST(filterModel->rowCount(), "count");

    QFETCH(QVector<TopBarButtonModel::TopBarButton>, components);
    QCOMPARE(components.count(), filterModel->rowCount());
    for (int i = 0; i < filterModel->rowCount(); i++)
    {
        QCOMPARE(filterModel->index(i, 0).data(Qt::UserRole + 4).value<TopBarButtonModel::TopBarButton>(), components.at(i));
    }
}

void TopBarButtonFilterModelTest::testHeadMonitorAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<TopBarButtonModel::TopBarButton>>("components");

    QTest::newRow("enabled") << true << 9 << QVector<TopBarButtonModel::TopBarButton>{
        TopBarButtonModel::TopBarButton::Overview,
        TopBarButtonModel::TopBarButton::Login,
        TopBarButtonModel::TopBarButton::Results,
        TopBarButtonModel::TopBarButton::Statistics,
        TopBarButtonModel::TopBarButton::Simulation,
        TopBarButtonModel::TopBarButton::HeadMonitor,
        TopBarButtonModel::TopBarButton::Wizard,
        TopBarButtonModel::TopBarButton::Grapheditor,
        TopBarButtonModel::TopBarButton::Configuration
    };
    QTest::newRow("disabled") << false << 8 << QVector<TopBarButtonModel::TopBarButton>{
        TopBarButtonModel::TopBarButton::Overview,
        TopBarButtonModel::TopBarButton::Login,
        TopBarButtonModel::TopBarButton::Results,
        TopBarButtonModel::TopBarButton::Statistics,
        TopBarButtonModel::TopBarButton::Simulation,
        TopBarButtonModel::TopBarButton::Wizard,
        TopBarButtonModel::TopBarButton::Grapheditor,
        TopBarButtonModel::TopBarButton::Configuration
    };
}

void TopBarButtonFilterModelTest::testHeadMonitorAvailable()
{
    auto filterModel = new TopBarButtonFilterModel{this};
    auto model = new TopBarButtonModel{this};
    filterModel->setSourceModel(model);
    filterModel->setShowGraphEditor(true);

    QFETCH(bool, enabled);
    filterModel->setHeadMonitorAvailable(enabled);
    QCOMPARE(filterModel->isHeadMonitorAvailable(), enabled);

    QTEST(filterModel->rowCount(), "count");

    QFETCH(QVector<TopBarButtonModel::TopBarButton>, components);
    QCOMPARE(components.count(), filterModel->rowCount());
    for (int i = 0; i < filterModel->rowCount(); i++)
    {
        QCOMPARE(filterModel->index(i, 0).data(Qt::UserRole + 4).value<TopBarButtonModel::TopBarButton>(), components.at(i));
    }
}

QTEST_GUILESS_MAIN(TopBarButtonFilterModelTest)
#include "topBarButtonFilterModelTest.moc"
