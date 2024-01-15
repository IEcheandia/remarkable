#include <QTest>
#include <QSignalSpy>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <precitec/dataSet.h>

#include "../recordedSignalAnalyzerController.h"

using precitec::gui::components::ethercat::RecordedSignalAnalyzerController;
using precitec::gui::components::plotter::DataSet;

class RecordedSignalAnalyzerControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetStorageDir();
    void testEmptyFile();
    void testEmptyCompressedFile();
    void testLoading();
};

void RecordedSignalAnalyzerControllerTest::testCtor()
{
    RecordedSignalAnalyzerController controller;
    QCOMPARE(controller.storageDir(), QString());
    QCOMPARE(controller.isLoading(), false);
    QCOMPARE(controller.recordingDate(), QDateTime());
    QCOMPARE(controller.dataSets(), QVariantList());
    QCOMPARE(controller.hasData(), false);
    QCOMPARE(controller.xRange(), 10.0);
}

void RecordedSignalAnalyzerControllerTest::testSetStorageDir()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    RecordedSignalAnalyzerController controller;
    QSignalSpy storageDirChangedSpy{&controller, &RecordedSignalAnalyzerController::storageDirChanged};
    QVERIFY(storageDirChangedSpy.isValid());
    QSignalSpy dataChangedSpy{&controller, &RecordedSignalAnalyzerController::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy loadingChangedSpy{&controller, &RecordedSignalAnalyzerController::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());

    controller.setStorageDir(dir.path());
    QCOMPARE(controller.storageDir(), dir.path());
    QCOMPARE(storageDirChangedSpy.count(), 1);

    // setting same dir again should not emit changed
    controller.setStorageDir(dir.path());
    QCOMPARE(storageDirChangedSpy.count(), 1);

    QCOMPARE(controller.isLoading(), true);
    QCOMPARE(loadingChangedSpy.count(), 1);
    // everything else happens in thread
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(controller.isLoading(), false);
    QCOMPARE(controller.dataSets(), QVariantList());
}

void RecordedSignalAnalyzerControllerTest::testEmptyFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile file{dir.filePath(QStringLiteral("signalAnalyzer.json"))};
    QCOMPARE(file.exists(), false);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    QCOMPARE(file.exists(), true);

    RecordedSignalAnalyzerController controller;
    QSignalSpy dataChangedSpy{&controller, &RecordedSignalAnalyzerController::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    controller.setStorageDir(dir.path());
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(controller.dataSets(), QVariantList());
    QCOMPARE(controller.hasData(), false);
}

void RecordedSignalAnalyzerControllerTest::testEmptyCompressedFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile file{dir.filePath(QStringLiteral("signalAnalyzer.json"))};
    QCOMPARE(file.exists(), false);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(qCompress(QByteArray()));
    file.close();
    QCOMPARE(file.exists(), true);

    RecordedSignalAnalyzerController controller;
    QSignalSpy dataChangedSpy{&controller, &RecordedSignalAnalyzerController::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    controller.setStorageDir(dir.path());
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(controller.dataSets(), QVariantList());
    QCOMPARE(controller.hasData(), false);
}

void RecordedSignalAnalyzerControllerTest::testLoading()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile file{dir.filePath(QStringLiteral("signalAnalyzer.json"))};
    QCOMPARE(file.exists(), false);
    QVERIFY(file.open(QIODevice::WriteOnly));

    QJsonObject rootObject{
        qMakePair(QStringLiteral("date"), QDateTime::fromMSecsSinceEpoch(0).toString(Qt::ISODate)),
        qMakePair(QStringLiteral("samples"), QJsonArray{
            QJsonObject{
                qMakePair(QStringLiteral("name"), QStringLiteral("foo")),
                qMakePair(QStringLiteral("color"), QJsonObject{
                    qMakePair(QStringLiteral("r"), 255),
                    qMakePair(QStringLiteral("g"), 0),
                    qMakePair(QStringLiteral("b"), 0)
                }),
                qMakePair(QStringLiteral("samples"), QJsonArray{
                    {0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0}
                })
            },
            QJsonObject{
                qMakePair(QStringLiteral("name"), QStringLiteral("bar")),
                qMakePair(QStringLiteral("color"), QJsonObject{
                    qMakePair(QStringLiteral("r"), 0),
                    qMakePair(QStringLiteral("g"), 255),
                    qMakePair(QStringLiteral("b"), 0)
                }),
                qMakePair(QStringLiteral("samples"), QJsonArray{
                    {1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0}
                })
            },
            QJsonObject{
                qMakePair(QStringLiteral("name"), QStringLiteral("foobar")),
                qMakePair(QStringLiteral("color"), QJsonObject{
                    qMakePair(QStringLiteral("r"), 0),
                    qMakePair(QStringLiteral("g"), 0),
                    qMakePair(QStringLiteral("b"), 255)
                }),
                qMakePair(QStringLiteral("samples"), QJsonArray{
                    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0}
                })
            }
        })
    };

    file.write(qCompress(QCborValue(QJsonDocument{rootObject}.toJson()).toCbor()));
    file.close();
    QCOMPARE(file.exists(), true);

    RecordedSignalAnalyzerController controller;
    QSignalSpy dataChangedSpy{&controller, &RecordedSignalAnalyzerController::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    controller.setStorageDir(dir.path());
    QVERIFY(dataChangedSpy.wait());

    QCOMPARE(controller.recordingDate(), QDateTime::fromMSecsSinceEpoch(0));
    QCOMPARE(controller.hasData(), true);

    auto dataSets = controller.dataSets();
    QCOMPARE(dataSets.count(), 3);
    QCOMPARE(controller.xRange(), 0.007f);

    auto dataSet = dataSets.at(0).value<DataSet*>();
    QVERIFY(dataSet);
    QCOMPARE(dataSet->name(), QStringLiteral("foo"));
    QCOMPARE(dataSet->isSmooth(), false);
    QCOMPARE(dataSet->color(), Qt::red);
    QCOMPARE(dataSet->sampleCount(), 8);
    auto samples = dataSet->samples();
    auto it = samples.begin();
    QCOMPARE(*it, QVector2D(0, 0.0));
    QCOMPARE(*(++it), QVector2D(0.001, 1.0));
    QCOMPARE(*(++it), QVector2D(0.002, 0.0));
    QCOMPARE(*(++it), QVector2D(0.003, 1.0));
    QCOMPARE(*(++it), QVector2D(0.004, 0.0));
    QCOMPARE(*(++it), QVector2D(0.005, 1.0));
    QCOMPARE(*(++it), QVector2D(0.006, 0.0));
    QCOMPARE(*(++it), QVector2D(0.007, 1.0));
    QCOMPARE(++it, samples.end());

    dataSet = dataSets.at(1).value<DataSet*>();
    QVERIFY(dataSet);
    QCOMPARE(dataSet->name(), QStringLiteral("bar"));
    QCOMPARE(dataSet->isSmooth(), false);
    QCOMPARE(dataSet->color(), Qt::green);
    QCOMPARE(dataSet->sampleCount(), 8);
    samples = dataSet->samples();
    it = samples.begin();
    QCOMPARE(*it, QVector2D(0, 1.0));
    QCOMPARE(*(++it), QVector2D(0.001, 0.0));
    QCOMPARE(*(++it), QVector2D(0.002, 1.0));
    QCOMPARE(*(++it), QVector2D(0.003, 0.0));
    QCOMPARE(*(++it), QVector2D(0.004, 1.0));
    QCOMPARE(*(++it), QVector2D(0.005, 0.0));
    QCOMPARE(*(++it), QVector2D(0.006, 1.0));
    QCOMPARE(*(++it), QVector2D(0.007, 0.0));
    QCOMPARE(++it, samples.end());

    dataSet = dataSets.at(2).value<DataSet*>();
    QVERIFY(dataSet);
    QCOMPARE(dataSet->name(), QStringLiteral("foobar"));
    QCOMPARE(dataSet->isSmooth(), false);
    QCOMPARE(dataSet->color(), Qt::blue);
    QCOMPARE(dataSet->sampleCount(), 8);
    samples = dataSet->samples();
    it = samples.begin();
    QCOMPARE(*it, QVector2D(0, 0.0));
    QCOMPARE(*(++it), QVector2D(0.001, 0.0));
    QCOMPARE(*(++it), QVector2D(0.002, 0.0));
    QCOMPARE(*(++it), QVector2D(0.003, 0.0));
    QCOMPARE(*(++it), QVector2D(0.004, 0.0));
    QCOMPARE(*(++it), QVector2D(0.005, 0.0));
    QCOMPARE(*(++it), QVector2D(0.006, 0.0));
    QCOMPARE(*(++it), QVector2D(0.007, 1.0));
    QCOMPARE(++it, samples.end());
}

QTEST_GUILESS_MAIN(RecordedSignalAnalyzerControllerTest)
#include "recordedSignalAnalyzerControllerTest.moc"
