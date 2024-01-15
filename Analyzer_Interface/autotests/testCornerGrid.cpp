#include <QTest>
#include "util/camGridData.h"
#include <calibration3DCoordsInterpolator.h>
#include "math/UniformMapBuilder.h"
#include "math/Calibration3DCoordsLoader.h"
#include <common/bitmap.h>
#include <fstream>
 #include <string>

class TestCornerGrid: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoadCornerGrid();
    void testAverageMap();
};

using precitec::system::CamGridData;
using precitec::math::Calibration3DCoordsInterpolator;
using precitec::math::Calibration3DCoords;
using precitec::math::CalibrationCornerGrid;
using precitec::math::Calibration3DCoordsTransformer;


void TestCornerGrid::testLoadCornerGrid() 
{
    std::string testdatafolder = QFINDTESTDATA("testdata/scheimpflug/").toStdString();
    CamGridData oCamGridData;
    std::string p_oCamGridImageFilename = testdatafolder + "/config/calibImgData0fallback.csv";
    std::string oMsgError = oCamGridData.loadFromCSV(p_oCamGridImageFilename);
    QVERIFY2( oMsgError.empty(), oMsgError.c_str() );
    
    //test functionality similar to rCalibrationData.load3DFieldFromCamGridData(oCamGridData);
    
    //auto res = loadCamGridData(o3DCoords, oCamGridData);
    //QVERIFY(res);
    
    
    //load the cells from camgriddata: now we have the correspondence between pixel and mm for each chessboard corner
	CalibrationCornerGrid oCornerGrid({oCamGridData.sensorWidth(),  oCamGridData.sensorHeight()}); 
	loadCamGridDataToCornerGrid(oCornerGrid, oCamGridData, /*linearize*/ true);
	QVERIFY( oCornerGrid.getGrid2D3D().size() != 0 );
    
    Calibration3DCoords o3DCoords;
    
    Calibration3DCoordsInterpolator oInterpolator(o3DCoords);
	oInterpolator.resetGridCellData(oCamGridData.sensorWidth(), oCamGridData.sensorHeight());
    assert(o3DCoords.getSensorSize().area() == oCamGridData.sensorWidth() * oCamGridData.sensorHeight());
	//precomputecalib3dcoords
	bool ok = oInterpolator.allCellsTo3D(oCornerGrid, /*extrapolate*/ true, /*rectify*/ true);
	QVERIFY(ok);
	
}


void TestCornerGrid::testAverageMap() 
{
    std::vector<std::string> testdatafolders = { 
        QFINDTESTDATA("testdata/scanner_1/").toStdString(), //0
        QFINDTESTDATA("testdata/scanner_2/").toStdString(), //1
        QFINDTESTDATA("testdata/scanner_3/").toStdString(), //2
        QFINDTESTDATA("testdata/scanner_5/").toStdString(), //3       
    };
    
    if (std::find(testdatafolders.begin(),  testdatafolders.end(),  "") !=  testdatafolders.end())
    {
        QSKIP("System does not have test data ");
    }
    
    int width = 1024; int height = 1024;
    std::vector<CalibrationCornerGrid> cornergrids(testdatafolders.size(),  CalibrationCornerGrid{{width, height}});
    for (unsigned int i = 0; i < testdatafolders.size(); i++)
    {
        CamGridData oCamGridData;
        std::string p_oCamGridImageFilename = testdatafolders[i] + "/config/calibImgData0fallback.csv";
        std::string oMsgError = oCamGridData.loadFromCSV(p_oCamGridImageFilename);
        std::cout << oMsgError;
        QVERIFY( oMsgError.empty() );
        QVERIFY2(oCamGridData.sensorWidth() == width, "unexpected value in test data");
        QVERIFY2(oCamGridData.sensorHeight() ==  height, "unexpected value in test data");
        
        //load the cells from camgriddata: now we have the correspondence between pixel and mm for each chessboard corner
        CalibrationCornerGrid & rCornerGrid = cornergrids[i]; 
        loadCamGridDataToCornerGrid(rCornerGrid, oCamGridData, /*linearize*/ true);
        QVERIFY( rCornerGrid.getGrid2D3D().size() != 0 );
        {
            std::string outfile(testdatafolders[i] +"/avg.txt");
            std::ofstream outMap(outfile);
            rCornerGrid.printGrid(outMap);
        }
        Calibration3DCoords o3DCoords;
        Calibration3DCoordsInterpolator oInterpolator(o3DCoords);
        oInterpolator.resetGridCellData(width, height);
        //precomputecalib3dcoords
        bool ok = oInterpolator.allCellsTo3D(rCornerGrid, /*extrapolate*/ true, /*rectify*/ true);
        QVERIFY(ok);
        
        float xmm, ymm;
        Calibration3DCoordsTransformer T(o3DCoords);
        T.translateToOffsetOrigin(512,512);

        o3DCoords.getCoordinates(xmm,ymm,512,512);
        QCOMPARE(xmm, 0.0f);
        QCOMPARE(ymm, 0.0f);
        
        precitec::image::BImage equivalentCheckerBoard;
        o3DCoords.coordsToCheckerBoardImage(equivalentCheckerBoard, {0,0}, o3DCoords.getSensorSize(), 2.0, 
                                              precitec::math::Calibration3DCoords::CoordinatePlaneMode::InternalPlane);
    
        
        std::string oFilename = testdatafolders[i] +"/chessboard.bmp";
        fileio::Bitmap oBitmap(oFilename.c_str(), equivalentCheckerBoard.width(), equivalentCheckerBoard.height(), false); //default value of topdown gives mirrored images
        ok = oBitmap.isValid() && oBitmap.save(equivalentCheckerBoard.data());
    }
    
    using precitec::math::UniformGridMapBuilder;
    UniformGridMapBuilder builder({ {cornergrids[0], UniformGridMapBuilder::t_map_center{0,0}},
                                    {cornergrids[2], UniformGridMapBuilder::t_map_center{1024,0}}  ,
                                    {cornergrids[3], UniformGridMapBuilder::t_map_center{768,768}}
                                    }, 
                                  UniformGridMapBuilder::t_map_center{768,0});

    auto oAverageMap = builder.getAverageMap();
    {
        std::cout << "Average Map" << std::endl;
        std::string outfile(testdatafolders[1] +"/avg_map.txt");
        std::ofstream outAverageMap(outfile);
        oAverageMap.printGrid(outAverageMap);
        
    }
    {
        Calibration3DCoords o3DCoords;
        Calibration3DCoordsInterpolator oInterpolator(o3DCoords);
        oInterpolator.resetGridCellData(width, height);
        //precomputecalib3dcoords
        bool ok = oInterpolator.allCellsTo3D(oAverageMap, /*extrapolate*/ true, /*rectify*/ true);
        QVERIFY(ok);
        
        
        float xmm, ymm;
        o3DCoords.getCoordinates(xmm,ymm,512,512);
        Calibration3DCoordsTransformer T(o3DCoords);
        T.applyTranslationToInternalPlane(-xmm, -ymm);
        T.applyScaleToInternalPlane( 1.0, -1.0);
        
        o3DCoords.getCoordinates(xmm,ymm,512,512);
        QCOMPARE(xmm, 0.0f);
        QCOMPARE(ymm, 0.0f);
        
        precitec::image::BImage equivalentCheckerBoard;
        o3DCoords.coordsToCheckerBoardImage(equivalentCheckerBoard, {0,0}, o3DCoords.getSensorSize(), 2.0, 
                                              precitec::math::Calibration3DCoords::CoordinatePlaneMode::InternalPlane);
    
        std::string oFilename = testdatafolders[1] +"/avg_chessboard.bmp";
        fileio::Bitmap oBitmap(oFilename.c_str(), equivalentCheckerBoard.width(), equivalentCheckerBoard.height(), false); //default value of topdown gives mirrored images
        ok = oBitmap.isValid() && oBitmap.save(equivalentCheckerBoard.data());
    }
}



QTEST_MAIN(TestCornerGrid)
#include "testCornerGrid.moc"
