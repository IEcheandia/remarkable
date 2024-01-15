#include <QTest>
#include <QSignalSpy>

#include "../moduleModel.h"

using precitec::gui::components::logging::ModuleModel;

class TestModuleModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLogModelNotSet();
    void testWithModel();
    void testWithModelReset();
    void testWithEmptyModel();
};

class MockModel : public QAbstractListModel
{
    Q_OBJECT
public:
    QVariant data(const QModelIndex & index, int role) const override
    {
        if (index.row() > m_list.count())
        {
            return QVariant();
        }
        if (role == Qt::UserRole + 1)
        {
            return m_list.at(index.row());
        }
        return QVariant();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
        {
            return 0;
        }
        return m_list.count();
    }

    void setStringList(const QStringList &list)
    {
        beginResetModel();
        m_list = list;
        endResetModel();
    }

    void addString(const QString &string)
    {
        beginInsertRows(QModelIndex(), m_list.count(), m_list.count());
        m_list << string;
        endInsertRows();
    }

private:
    QStringList m_list;
};

void TestModuleModel::testLogModelNotSet()
{
    ModuleModel model;
    QCOMPARE(model.rowCount(), 1);
    auto index = model.index(0, 0);
    QVERIFY(index.isValid());
    QCOMPARE(model.data(index, Qt::DisplayRole).toString(), QStringLiteral("All"));
    QCOMPARE(model.property("longestItem").toString(), QStringLiteral("All"));
    QVERIFY(!model.data(index, Qt::UserRole).isValid());
    QCOMPARE(model.longestItem(), index.data().toString());
    index = model.index(1, 0);
    QVERIFY(!index.isValid());
    QVERIFY(!model.data(index, Qt::DisplayRole).isValid());
}

void TestModuleModel::testWithModel()
{
    ModuleModel model;
    QCOMPARE(model.rowCount(), 1);
    QSignalSpy longestItemChangedSpy(&model, &ModuleModel::longestItemChanged);
    QVERIFY(longestItemChangedSpy.isValid());
    QSignalSpy rowsInsertedSpy(&model, &ModuleModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    MockModel mockModel;
    mockModel.setStringList(QStringList{
        QStringLiteral("Test"),
        QStringLiteral("Long Caption"),
        QStringLiteral("Test"),
        QStringLiteral("Foo")
    });
    QCOMPARE(mockModel.rowCount(), 4);

    model.setLogModel(QPointer<QAbstractItemModel>(&mockModel));
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("All"));
    QCOMPARE(model.index(1, 0).data().toString(), QStringLiteral("Test"));
    QCOMPARE(model.index(2, 0).data().toString(), QStringLiteral("Long Caption"));
    QCOMPARE(model.index(3, 0).data().toString(), QStringLiteral("Foo"));
    QCOMPARE(longestItemChangedSpy.count(), 1);
    QCOMPARE(model.longestItem(), QStringLiteral("Long Caption"));
    QCOMPARE(rowsInsertedSpy.count(), 1);
    mockModel.addString(QStringLiteral("Test"));
    QCOMPARE(rowsInsertedSpy.count(), 1);
    mockModel.addString(QStringLiteral("An even longer caption"));
    QCOMPARE(rowsInsertedSpy.count(), 2);
    QCOMPARE(longestItemChangedSpy.count(), 2);
    QCOMPARE(model.longestItem(), QStringLiteral("An even longer caption"));
}

void TestModuleModel::testWithModelReset()
{
    ModuleModel model;
    QCOMPARE(model.rowCount(), 1);
    QSignalSpy longestItemChangedSpy(&model, &ModuleModel::longestItemChanged);
    QVERIFY(longestItemChangedSpy.isValid());
    QSignalSpy rowsInsertedSpy(&model, &ModuleModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    MockModel mockModel;

    model.setLogModel(QPointer<QAbstractItemModel>(&mockModel));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("All"));
    QCOMPARE(rowsInsertedSpy.count(), 0);

    mockModel.setStringList(QStringList{
        QStringLiteral("Test"),
        QStringLiteral("Long Caption"),
        QStringLiteral("Test"),
        QStringLiteral("Foo")
    });
    QCOMPARE(mockModel.rowCount(), 4);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.front().at(0).toModelIndex().isValid(), false);
    QCOMPARE(rowsInsertedSpy.front().at(1).toInt(), 1);
    QCOMPARE(rowsInsertedSpy.front().at(2).toInt(), 3);

    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("All"));
    QCOMPARE(model.index(1, 0).data().toString(), QStringLiteral("Test"));
    QCOMPARE(model.index(2, 0).data().toString(), QStringLiteral("Long Caption"));
    QCOMPARE(model.index(3, 0).data().toString(), QStringLiteral("Foo"));
}

void TestModuleModel::testWithEmptyModel()
{
    ModuleModel model;
    QCOMPARE(model.rowCount(), 1);
    MockModel mockModel;
    QCOMPARE(mockModel.rowCount(), 0);
    model.setLogModel(QPointer<QAbstractItemModel>(&mockModel));
    QCOMPARE(model.rowCount(), 1);
    mockModel.addString(QStringLiteral("Test"));
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.index(1, 0).data().toString(), QStringLiteral("Test"));
}

QTEST_GUILESS_MAIN(TestModuleModel)
#include "testModuleModel.moc"
