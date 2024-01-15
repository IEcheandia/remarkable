#include <QTest>
#include <QSignalSpy>

#include "../logFilterModel.h"
#include "../logModel.h"

using precitec::gui::components::logging::LogModel;
using precitec::gui::components::logging::LogFilterModel;

class TestLogFilterModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFilterElements();
    void testFilter();
};

class MockModel : public QAbstractListModel
{
    Q_OBJECT
public:
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    struct MockData {
        LogModel::LogLevel level;
        QByteArray moduleName;
        QByteArray message;
    };

    void addData(std::initializer_list<MockData> data);

private:
    std::vector<MockData> m_data;
};

QVariant MockModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= int(m_data.size())) {
        return QVariant();
    }
    const auto &data = m_data.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return data.message;
    case Qt::UserRole + 1:
        return data.moduleName;
    case Qt::UserRole + 2:
        return QVariant::fromValue(data.level);
    }
    return QVariant();
}

int MockModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_data.size();
}

void MockModel::addData(std::initializer_list<MockData> data)
{
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size() + data.size() - 1);
    m_data.insert(m_data.end(), data.begin(), data.end());
    endInsertRows();
}

void TestLogFilterModel::testFilterElements()
{
    LogFilterModel model;
    QCOMPARE(model.property("includeInfo").toBool(), true);
    QCOMPARE(model.property("includeWarning").toBool(), true);
    QCOMPARE(model.property("includeError").toBool(), true);
    QCOMPARE(model.property("includeDebug").toBool(), false);
    QCOMPARE(model.property("includeTracker").toBool(), false);
    QCOMPARE(model.property("moduleNameFilter").toByteArray(), QByteArray());

    QSignalSpy includeInfoChangedSpy(&model, &LogFilterModel::includeInfoChanged);
    QVERIFY(includeInfoChangedSpy.isValid());
    QSignalSpy includeWarningChangedSpy(&model, &LogFilterModel::includeWarningChanged);
    QVERIFY(includeWarningChangedSpy.isValid());
    QSignalSpy includeErrorChangedSpy(&model, &LogFilterModel::includeErrorChanged);
    QVERIFY(includeErrorChangedSpy.isValid());
    QSignalSpy includeDebugChangedSpy(&model, &LogFilterModel::includeDebugChanged);
    QVERIFY(includeDebugChangedSpy.isValid());
    QSignalSpy includeTrackerChangedSpy{&model, &LogFilterModel::includeTrackerChanged};
    QVERIFY(includeTrackerChangedSpy.isValid());
    QSignalSpy moduleNameFilterChangedSpy(&model, &LogFilterModel::moduleNameFilterChanged);
    QVERIFY(moduleNameFilterChangedSpy.isValid());

    model.setProperty("includeInfo", false);
    QCOMPARE(includeInfoChangedSpy.count(), 1);
    QCOMPARE(includeWarningChangedSpy.count(), 0);
    QCOMPARE(includeErrorChangedSpy.count(), 0);
    QCOMPARE(includeDebugChangedSpy.count(), 0);
    QCOMPARE(includeTrackerChangedSpy.count(), 0);
    QCOMPARE(model.property("includeInfo").toBool(), false);
    model.setProperty("includeWarning", false);
    QCOMPARE(includeInfoChangedSpy.count(), 1);
    QCOMPARE(includeWarningChangedSpy.count(), 1);
    QCOMPARE(includeErrorChangedSpy.count(), 0);
    QCOMPARE(includeDebugChangedSpy.count(), 0);
    QCOMPARE(includeTrackerChangedSpy.count(), 0);
    QCOMPARE(model.property("includeWarning").toBool(), false);
    model.setProperty("includeError", false);
    QCOMPARE(includeInfoChangedSpy.count(), 1);
    QCOMPARE(includeWarningChangedSpy.count(), 1);
    QCOMPARE(includeErrorChangedSpy.count(), 1);
    QCOMPARE(includeDebugChangedSpy.count(), 0);
    QCOMPARE(includeTrackerChangedSpy.count(), 0);
    QCOMPARE(model.property("includeError").toBool(), false);
    model.setProperty("includeDebug", true);
    QCOMPARE(includeInfoChangedSpy.count(), 1);
    QCOMPARE(includeWarningChangedSpy.count(), 1);
    QCOMPARE(includeErrorChangedSpy.count(), 1);
    QCOMPARE(includeDebugChangedSpy.count(), 1);
    QCOMPARE(includeTrackerChangedSpy.count(), 0);
    QCOMPARE(model.property("includeDebug").toBool(), true);
    model.setProperty("includeTracker", true);
    QCOMPARE(includeInfoChangedSpy.count(), 1);
    QCOMPARE(includeWarningChangedSpy.count(), 1);
    QCOMPARE(includeErrorChangedSpy.count(), 1);
    QCOMPARE(includeDebugChangedSpy.count(), 1);
    QCOMPARE(includeTrackerChangedSpy.count(), 1);
    QCOMPARE(model.property("includeTracker").toBool(), true);

    QCOMPARE(moduleNameFilterChangedSpy.count(), 0);
    model.setProperty("moduleNameFilter", QByteArrayLiteral("Test"));
    QCOMPARE(moduleNameFilterChangedSpy.count(), 1);
    QCOMPARE(includeInfoChangedSpy.count(), 1);
    QCOMPARE(includeWarningChangedSpy.count(), 1);
    QCOMPARE(includeErrorChangedSpy.count(), 1);
    QCOMPARE(model.property("moduleNameFilter").toByteArray(), QByteArrayLiteral("Test"));
}

void TestLogFilterModel::testFilter()
{
    LogFilterModel model;
    MockModel mockModel;

    model.setSourceModel(&mockModel);
    QCOMPARE(model.rowCount(), 0);

    // let's add some data
    mockModel.addData({
        {LogModel::LogLevel::Info, QByteArrayLiteral("Foo"), QByteArrayLiteral("Info")},
        {LogModel::LogLevel::Info, QByteArrayLiteral("Bar"), QByteArrayLiteral("Info2")},
        {LogModel::LogLevel::Startup, QByteArrayLiteral("Foo"), QByteArrayLiteral("Startup")}
    });
    QCOMPARE(mockModel.rowCount(), 3);
    QCOMPARE(model.rowCount(), mockModel.rowCount());
    model.setProperty("includeInfo", false);
    QCOMPARE(model.rowCount(), 0);

    // some warnings
    mockModel.addData({
        {LogModel::LogLevel::Warning, QByteArrayLiteral("Foo"), QByteArrayLiteral("Warning")},
        {LogModel::LogLevel::Warning, QByteArrayLiteral("Foo"), QByteArrayLiteral("Warning2")}
    });
    QCOMPARE(mockModel.rowCount(), 5);
    QCOMPARE(model.rowCount(), 2);
    model.setProperty("includeWarning", false);
    QCOMPARE(model.rowCount(), 0);
    // errors
    mockModel.addData({
        {LogModel::LogLevel::Error, QByteArrayLiteral("Foo"), QByteArrayLiteral("Error")},
        {LogModel::LogLevel::Fatal, QByteArrayLiteral("Bar"), QByteArrayLiteral("Fatal")},
        {LogModel::LogLevel::Error, QByteArrayLiteral("Foo"), QByteArrayLiteral("Error2")}
    });
    QCOMPARE(mockModel.rowCount(), 8);
    QCOMPARE(model.rowCount(), 3);
    // let's filter on Bar
    model.setProperty("moduleNameFilter", QByteArrayLiteral("Bar"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).data().toByteArray(), QByteArrayLiteral("Fatal"));
    model.setProperty("includeError", false);
    QCOMPARE(model.rowCount(), 0);

    model.setProperty("includeInfo", true);
    QCOMPARE(model.rowCount(), 1);
    model.setProperty("includeWarning", true);
    QCOMPARE(model.rowCount(), 1);
    model.setProperty("includeError", true);
    QCOMPARE(model.rowCount(), 2);
    model.setProperty("moduleNameFilter", QByteArrayLiteral("Foo"));
    QCOMPARE(model.rowCount(), 6);

    // debug
    mockModel.addData({
        {LogModel::LogLevel::Debug, QByteArrayLiteral("Foo"), QByteArrayLiteral("Debug")}
    });
    QCOMPARE(mockModel.rowCount(), 9);
    QCOMPARE(model.rowCount(), 6);
    model.setProperty("includeDebug", true);
    QCOMPARE(model.rowCount(), 7);
    model.setProperty("includeError", false);
    model.setProperty("includeWarning", false);
    model.setProperty("includeInfo", false);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).data().toByteArray(), QByteArrayLiteral("Debug"));

    // tracker
    mockModel.addData({
        {LogModel::LogLevel::Tracker, QByteArrayLiteral("Foo"), QByteArrayLiteral("Tracker")}
    });
    QCOMPARE(mockModel.rowCount(), 10);
    QCOMPARE(model.rowCount(), 1);
    model.setProperty("includeTracker", true);
    QCOMPARE(model.rowCount(), 2);
    model.setProperty("includeDebug", false);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).data().toByteArray(), QByteArrayLiteral("Tracker"));
}

QTEST_GUILESS_MAIN(TestLogFilterModel)
#include "testLogFilterModel.moc"
