#include <QTest>

#include "memory/memoryFile.h"

#include <unistd.h>
#include <sys/syscall.h>

namespace
{
static const int SYSCALL_FILE_START = 0;
static const int SYSCALL_ERROR = -1;
static const int SYSCALL_END_OF_FILE = 0;
}

class TestMemoryFile : public QObject
{
    Q_OBJECT

    using MemoryFile = precitec::system::MemoryFile;

private Q_SLOTS:

    void testConstructor()
    {
        MemoryFile noFile;
        QVERIFY(!noFile.hasFileDescriptor());
        
        MemoryFile file{"someFilename.txt"};
        QVERIFY(file.hasFileDescriptor());
    }

    void testFilepath()
    {
        MemoryFile file{"SomeFile.txt"};
        const std::string actual = file.filepath();
        const std::string expected = "/proc/self/fd/" + std::to_string(file.descriptor());
        QCOMPARE(actual, expected);
    }

    void testTwoFilesWithIdenticalName()
    {
        MemoryFile firstMemFile{"SomeFile.txt"};
        QVERIFY(firstMemFile.hasFileDescriptor());

        MemoryFile secondMemFile{"SomeFile.txt"};
        QVERIFY(secondMemFile.hasFileDescriptor());
        QVERIFY(firstMemFile != secondMemFile);

        std::string dataString{"Hello World!"};
        auto truncateSecond = secondMemFile.setSize(1024);
        QCOMPARE(truncateSecond, 0);

        auto writeFirst = firstMemFile.write(dataString);
        QCOMPARE(writeFirst, dataString.size());
        auto writeSecond = secondMemFile.write(dataString);
        QCOMPARE(writeSecond, dataString.size());

        auto readFirst = firstMemFile.read(dataString.size());
        auto readSecond = secondMemFile.read(100);
        QCOMPARE(readFirst.second, SYSCALL_END_OF_FILE);
        QCOMPARE(readSecond.second, 100); //End of file is at 1024
        QCOMPARE(QString::fromStdString(readFirst.first), QStringLiteral(""));
        QCOMPARE(QString::fromStdString(readSecond.first), QStringLiteral(""));

        //Set file offset
        auto firstFileStart = firstMemFile.seekStart();
        QCOMPARE(firstFileStart, SYSCALL_FILE_START);
        auto secondFileStart = secondMemFile.seekStart();
        QCOMPARE(secondFileStart, SYSCALL_FILE_START);

        auto readFirst2 = firstMemFile.read(dataString.size());
        auto readSecond2 = secondMemFile.read(dataString.size());
        QCOMPARE(readFirst2.second, dataString.size());
        QCOMPARE(readSecond2.second, dataString.size());
    }
};

QTEST_APPLESS_MAIN(TestMemoryFile)
#include "testMemoryFile.moc"