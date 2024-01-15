#include <QTest>

#include "../include/calibration/chessboardRecognition.h"
#include <common/bitmap.h>

class TestChessboardRecognition: public QObject
{    
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testChessboard_data();
    void testChessboard();
};


void TestChessboardRecognition::testCtor()
{
    precitec::calibration_algorithm::ChessboardRecognitionAlgorithm testRecognition( precitec::image::BImage{}, -1);
    QVERIFY(! testRecognition.isValid());
    
}

void TestChessboardRecognition::testChessboard_data()
{
    QTest::addColumn<QString>("filename");
    
    std::string inputFolder = QFINDTESTDATA("/opt/wm_inst/data/scanfieldimage/second_test/20200213_152030/").toStdString();
    if (inputFolder.empty())
    {
        QSKIP("System does not have test data " );
    }
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            std::ostringstream oStreamFilename;
            oStreamFilename << inputFolder << "row_" << i << "_col_" << j << ".bmp";
            QTest::addRow( "row_%d_col_%d", i,j) << QString::fromStdString(oStreamFilename.str()) ;
        }
    }
}

void TestChessboardRecognition::testChessboard()
{
    using namespace precitec::image;

    QFETCH(QString, filename);
    
    
    fileio::Bitmap oBitmap(filename.toStdString());
    QVERIFY2(oBitmap.validSize(), filename.toStdString().c_str());
    
    BImage oChessboard{Size2d{oBitmap.width(), oBitmap.height()}};
    std::vector<unsigned char> oAdditionalData;
    bool ok = oBitmap.load(oChessboard.begin(), oAdditionalData);
    QVERIFY(ok);
    
    precitec::calibration_algorithm::ChessboardRecognitionAlgorithm testRecognition(oChessboard, 46);
   
    precitec::calibration_algorithm::ChessboardRecognitionAlgorithm testRecognitionAutomaticThreshold(oChessboard, -1);
    
    QVERIFY2(testRecognition.isValid(), filename.toStdString().c_str());
    QVERIFY2(testRecognitionAutomaticThreshold.isValid(), filename.toStdString().c_str());
       
    auto & rCornerGrid = testRecognitionAutomaticThreshold.getCornerGrid();
    std::cout <<  filename.toStdString() << " pix / square " << rCornerGrid.factor_real_to_pix() << std::endl;
    QCOMPARE(std::floor(rCornerGrid.factor_real_to_pix()) , 54.0);
}

QTEST_MAIN(TestChessboardRecognition)
#include "testChessboardRecognition.moc"
