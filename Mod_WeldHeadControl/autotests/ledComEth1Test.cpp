#include <QTest>
#include <QSignalSpy>
#include <QNetworkDatagram>
#include <QtConcurrentRun>
#include <QUdpSocket>
#include <QFutureWatcher>

#include <sys/socket.h>

#include "../include/viWeldHead/LEDControl/LEDcomEth1.h"

class LEDcomEthernet1TTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRwNotInited();
    void testSetLocalAddress();
    void testSetLedAddress();
    void testInit();
    void testWrite();
    void testRead();
    void testWaitForEndFlag();
};

void LEDcomEthernet1TTest::testCtor()
{
    LEDcomEthernet1T ledCom;
    QCOMPARE(ledCom.errornum(), 0);
    QCOMPARE(ledCom.localIp(), "128.1.1.1");
    QCOMPARE(ledCom.localPort(), 30312);
    QCOMPARE(ledCom.ledIp(), "128.1.1.2");
    QCOMPARE(ledCom.ledPort(), 30313);
}

void LEDcomEthernet1TTest::testRwNotInited()
{
    LEDcomEthernet1T ledCom;
    QCOMPARE(ledCom.errornum(), 0);
    ledCom.rw(nullptr, nullptr);
    QCOMPARE(ledCom.errornum(), 4);

    // after init the error should be cleared
    LEDdriverParameterT parameter;
    ledCom.init(parameter);
    QEXPECT_FAIL("", "Error not reset on init", Continue);
    QCOMPARE(ledCom.errornum(), 0);
    ledCom.deinit();
}

void LEDcomEthernet1TTest::testSetLocalAddress()
{
    LEDcomEthernet1T ledCom;
    QCOMPARE(ledCom.localIp(), "128.1.1.1");
    QCOMPARE(ledCom.localPort(), 30312);

    ledCom.setLocalAddress("127.0.0.1", 1234);
    QCOMPARE(ledCom.localIp(), "127.0.0.1");
    QCOMPARE(ledCom.localPort(), 1234);
}

void LEDcomEthernet1TTest::testSetLedAddress()
{
    LEDcomEthernet1T ledCom;
    QCOMPARE(ledCom.ledIp(), "128.1.1.2");
    QCOMPARE(ledCom.ledPort(), 30313);

    ledCom.setLedAddress("127.0.0.1", 1234);
    QCOMPARE(ledCom.ledIp(), "127.0.0.1");
    QCOMPARE(ledCom.ledPort(), 1234);
}

void LEDcomEthernet1TTest::testInit()
{
    LEDcomEthernet1T ledCom;
    ledCom.setLocalAddress("127.0.0.1", 30312);

    LEDdriverParameterT parameter;
    ledCom.init(parameter);
    QCOMPARE(ledCom.errornum(), 0);

    sockaddr_in addr;
    socklen_t length = sizeof(addr);
    getsockname(ledCom.sock, (struct sockaddr *)&addr, &length);
    QCOMPARE(addr.sin_family, AF_INET);
    QCOMPARE(addr.sin_port, htons(30312));
    QCOMPARE(addr.sin_addr.s_addr, inet_addr("127.0.0.1"));

    ledCom.deinit();
}

void LEDcomEthernet1TTest::testWrite()
{
    // open a Udp socket to read data written by LEDcomEthernet1T
    QUdpSocket socket;
    QVERIFY(socket.bind(QHostAddress::LocalHost, 30313));
    // udp socket emits readyRead as soon as data is available
    // use signalspy to be able to async wait on it
    QSignalSpy readyReadSpy{&socket, &QUdpSocket::readyRead};
    QVERIFY(readyReadSpy.isValid());

    // other side of socket
    LEDcomEthernet1T ledCom;
    ledCom.setLocalAddress("127.0.0.1", 30312);
    ledCom.setLedAddress("127.0.0.1", 30313);

    LEDdriverParameterT parameter;
    ledCom.init(parameter);
    QCOMPARE(ledCom.errornum(), 0);

    // needs to be done from thread, it waits after sending and thus blocks processing
    // QFutureWatcher to be able to wait on timeout
    QFutureWatcher<void> watcher;
    QSignalSpy futureFinishedSpy{&watcher, &QFutureWatcher<void>::finished};
    QVERIFY(futureFinishedSpy.isValid());
    // start the async sending
    watcher.setFuture(QtConcurrent::run(
        [&ledCom]
        {
            // init the out message, argument of rw is char* instead of const char *
            // thus we need to create a temporary char array and strcpy into it
            // otherwise it would be a compile warning
            char testMessage[5];
            strncpy(testMessage, "test", 5);
            // 4000 bytes are read at maximum, method does not check for buffer overflows
            char buffer[4000];
            ledCom.rw(testMessage, buffer, false);
        }
        ));
    // wait for the data which is sent in async way
    // cannot just call wait, we might alrady got it
    if (readyReadSpy.isEmpty())
    {
        QVERIFY(readyReadSpy.wait());
    }
    QCOMPARE(readyReadSpy.count(), 1);

    // compare that we got the message
    QVERIFY(socket.hasPendingDatagrams());
    const auto datagram = socket.receiveDatagram();
    QCOMPARE(datagram.data(), QByteArrayLiteral("test"));

    // now wait for the timeout of the rw thread
    // again might already be called
    if (futureFinishedSpy.isEmpty())
    {
        QVERIFY(futureFinishedSpy.wait(10000));
    }

    // and now close the socket
    ledCom.deinit();
}

void LEDcomEthernet1TTest::testRead()
{
    // led controller side
    QUdpSocket socket;
    QVERIFY(socket.bind(QHostAddress::LocalHost, 30313));
    QSignalSpy readyReadSpy{&socket, &QUdpSocket::readyRead};
    QVERIFY(readyReadSpy.isValid());

    // other side
    LEDcomEthernet1T ledCom;
    ledCom.setLocalAddress("127.0.0.1", 30312);
    ledCom.setLedAddress("127.0.0.1", 30313);

    LEDdriverParameterT parameter;
    ledCom.init(parameter);
    QCOMPARE(ledCom.errornum(), 0);

    QFutureWatcher<QByteArray> watcher;
    QSignalSpy futureFinishedSpy{&watcher, &QFutureWatcher<QByteArray>::finished};
    QVERIFY(futureFinishedSpy.isValid());
    watcher.setFuture(QtConcurrent::run(
        [&ledCom]
        {
            // init the out message, argument of rw is char* instead of const char *
            // thus we need to create a temporary char array and strcpy into it
            // otherwise it would be a compile warning
            char testMessage[5];
            strncpy(testMessage, "test", 5);
            // 4000 bytes are read at maximum, method does not check for buffer overflows
            char buffer[4000];
            ledCom.rw(testMessage, buffer, false);
            return QByteArray{buffer};
        }
        ));
    if (readyReadSpy.isEmpty())
    {
        QVERIFY(readyReadSpy.wait());
    }
    QCOMPARE(readyReadSpy.count(), 1);
    // send a reply
    QCOMPARE(socket.writeDatagram("reply", 5, QHostAddress::LocalHost, 30312), 5);

    // now wait for reply
    if (futureFinishedSpy.isEmpty())
    {
        QVERIFY(futureFinishedSpy.wait());
    }
    QCOMPARE(watcher.result(), QByteArrayLiteral("reply"));

    // and now close the socket
    ledCom.deinit();
}

void LEDcomEthernet1TTest::testWaitForEndFlag()
{
    // led controller side
    QUdpSocket socket;
    QVERIFY(socket.bind(QHostAddress::LocalHost, 30313));
    QSignalSpy readyReadSpy{&socket, &QUdpSocket::readyRead};
    QVERIFY(readyReadSpy.isValid());

    // other side
    LEDcomEthernet1T ledCom;
    ledCom.setLocalAddress("127.0.0.1", 30312);
    ledCom.setLedAddress("127.0.0.1", 30313);

    LEDdriverParameterT parameter;
    ledCom.init(parameter);
    QCOMPARE(ledCom.errornum(), 0);

    QFutureWatcher<QByteArray> watcher;
    QSignalSpy futureFinishedSpy{&watcher, &QFutureWatcher<QByteArray>::finished};
    QVERIFY(futureFinishedSpy.isValid());
    watcher.setFuture(QtConcurrent::run(
        [&ledCom]
        {
            // init the out message, argument of rw is char* instead of const char *
            // thus we need to create a temporary char array and strcpy into it
            // otherwise it would be a compile warning
            char testMessage[5];
            strncpy(testMessage, "test", 5);
            // 4000 bytes are read at maximum, method does not check for buffer overflows
            char buffer[4000];
            ledCom.rw(testMessage, buffer);
            return QByteArray{buffer};
        }
        ));
    if (readyReadSpy.isEmpty())
    {
        QVERIFY(readyReadSpy.wait());
    }
    QCOMPARE(readyReadSpy.count(), 1);
    // send a reply
    QCOMPARE(socket.writeDatagram("reply", 5, QHostAddress::LocalHost, 30312), 5);
    // wait a little bit and send end flag
    QTest::qWait(300);
    QCOMPARE(socket.writeDatagram(">", 1, QHostAddress::LocalHost, 30312), 1);

    // now wait for reply
    if (futureFinishedSpy.isEmpty())
    {
        QVERIFY(futureFinishedSpy.wait());
    }
    QCOMPARE(watcher.result(), QByteArrayLiteral("reply>"));

    // and now close the socket
    ledCom.deinit();
}

QTEST_GUILESS_MAIN(LEDcomEthernet1TTest)
#include "ledComEth1Test.moc"
