#include <QTest>
#include <QObject>

#include "Scheduler/timeFolderCleaner.h"
#include <fstream>
#include <iostream>

#include <chrono>


using precitec::scheduler::TimeFolderCleaner;
using namespace std::chrono_literals;

class TimeFolderCleanerTest : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testIsTimeToDieFunction();
private:
    unsigned int itemFolderNumber(const std::string &dir);
};

void TimeFolderCleanerTest::testCtor()
{
    TimeFolderCleaner folderCleaner;
}

void TimeFolderCleanerTest::testIsTimeToDieFunction()
{
    QTemporaryDir dir;

    // create many new sub-folders
    int newSubFolderNumber = 5;
    for (int index = 0; index <= newSubFolderNumber; index++)
    {
        fs::create_directory(dir.path().toStdString() + "/" + "update" + std::to_string(index));
        std::ofstream ofs(dir.path().toStdString() + "/" + "update" + std::to_string(index) + "/" + "testFile.txt");
        ofs << "this is some text in the new file\n";
        ofs.close();
    }

    // make one folder as an old folder
    std::string oldFolder = dir.path().toStdString() + "/" + "update" + std::to_string(0);
    std::filesystem::last_write_time(oldFolder, std::filesystem::last_write_time(oldFolder) - 1h);
    TimeFolderCleaner timeFolderCleaner;
    timeFolderCleaner.setFolder(dir.path().toStdString());
    timeFolderCleaner.setTimeToLive(3*60);

    // before clean
    QCOMPARE(itemFolderNumber(dir.path().toStdString()), newSubFolderNumber + 1);
    timeFolderCleaner.cleanOldSubFolders();
    // after clean
    QCOMPARE(itemFolderNumber(dir.path().toStdString()), newSubFolderNumber);
}

unsigned int TimeFolderCleanerTest::itemFolderNumber(const std::string &dir)
{
    unsigned int count = 0;
    for (auto &item : fs::directory_iterator(dir))
    {
        if (item.is_directory())
        {
            count++;
        }
    }
    return count;
}

QTEST_GUILESS_MAIN(TimeFolderCleanerTest)
#include "timeFolderCleanerTest.moc"
