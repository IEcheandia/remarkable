#include <QTest>
#include "image/image.h"

class TestImage: public QObject
{
    Q_OBJECT

    template <class ValueT>
    void testResizeT();
    template <class ValueT>
    void testShallowCopyConstructorT();
    template <class ValueT>
    void testSubROIFromSizeT();
    template <class ValueT>
    void testSubROIFromRectT();
    
private Q_SLOTS:
    void initTestCase_data();
    void testResize();
    void testShallowCopyConstructor();
    void testSubROIFromSize();
    void testSubROIFromRect();
};

Q_DECLARE_METATYPE(std::string);
Q_DECLARE_METATYPE(precitec::image::Size2d);
using namespace precitec::image;
using namespace precitec::geo2d;

void TestImage::initTestCase_data()
{
    QTest::addColumn<std::string>("pixeltype");
    QTest::addColumn<Size2d>("imageSize");
    QTest::addColumn<Size2d>("roiSize");
    
    for (auto pixeltype : {std::string{"byte"} , std::string{"double"}})
    {
        for (auto imageSize : { 
                Size2d{100,200}, 
                Size2d{1024,1024}, 
                Size2d{1,1}
                })
        {
            QTest::addRow("%s_Size_%d_%d_halfROI",          pixeltype.c_str(), imageSize.width, imageSize.height) << pixeltype << imageSize << Size2d(imageSize.width/2,imageSize.height/2);
            QTest::addRow("%s_Size_%d_%d_fullROI",          pixeltype.c_str(), imageSize.width, imageSize.height) << pixeltype << imageSize << imageSize;
            QTest::addRow("%s_Size_%d_%d_workaroundFullROI",pixeltype.c_str(), imageSize.width, imageSize.height) << pixeltype << imageSize << Size2d(imageSize.width+1,imageSize.height+1);
            QTest::addRow("%s_Size_%d_%d_NullROI",          pixeltype.c_str(), imageSize.width, imageSize.height) << pixeltype << imageSize << Size2d(0,0);
            QTest::addRow("%s_Size_%d_%d_SmallROI",         pixeltype.c_str(), imageSize.width, imageSize.height) << pixeltype << imageSize << Size2d(1,1);
        }
    }
}


void TestImage::testResize()
{
    QFETCH_GLOBAL(std::string, pixeltype);
    if (pixeltype == "byte")
    {
        testResizeT<unsigned char>();
    }
    else if (pixeltype == "double")
    {
        testResizeT<double>();
    }
    else
    {
        QVERIFY2(false, "wrong pixeltype in data function ");
    }
}

void TestImage::testShallowCopyConstructor()
{
    QFETCH_GLOBAL(std::string, pixeltype);
    if (pixeltype == "byte")
    {
        testShallowCopyConstructorT<unsigned char>();
    }
    else if (pixeltype == "double")
    {
        testShallowCopyConstructorT<double>();
    }
    else
    {
        QVERIFY2(false, "wrong pixeltype in data function ");
    }
}


void TestImage::testSubROIFromSize()
{
    QFETCH_GLOBAL(std::string, pixeltype);
    if (pixeltype == "byte")
    {
        testSubROIFromSizeT<unsigned char>();
    }
    else if (pixeltype == "double")
    {
        testSubROIFromSizeT<double>();
    }
    else
    {
        QVERIFY2(false, "wrong pixeltype in data function ");
    }
}

void TestImage::testSubROIFromRect()
{
    QFETCH_GLOBAL(std::string, pixeltype);
    if (pixeltype == "byte")
    {
        testSubROIFromRectT<unsigned char>();
    }
    else if (pixeltype == "double")
    {
        testSubROIFromRectT<double>();
    }
    else
    {
        QVERIFY2(false, "wrong pixeltype in data function ");
    }
}

template <class ValueT>
void TestImage::testResizeT()
{
        TLineImage<ValueT> testImage;
        QCOMPARE(testImage.isValid(), false);
                
        QFETCH_GLOBAL(Size2d, imageSize);
        QFETCH_GLOBAL(Size2d, roiSize);
        
        testImage.resize(roiSize);
        QCOMPARE(testImage.size(), roiSize);
        
        testImage.clear();
        QCOMPARE(testImage.size(), Size2d(0,0));
        
        testImage.resize(imageSize);
        QCOMPARE(testImage.size(), imageSize);
        
        testImage.resize(roiSize);
        QCOMPARE(testImage.size(), roiSize);
        
}

template <class ValueT>
void TestImage::testShallowCopyConstructorT()
{
    QFETCH_GLOBAL(Size2d, imageSize);
    QFETCH_GLOBAL(Size2d, roiSize);
    
    TLineImage<ValueT> testImage(imageSize);
    QCOMPARE(testImage.size(), imageSize);    
    
    TLineImage<ValueT> testImageShallowCopy(testImage);
    if (imageSize.area() == 0)
    {
        QCOMPARE(testImage.isValid(), false);
        QCOMPARE(testImageShallowCopy.isValid(), false);
        // nothing else to test
        return;
    }
    
    QCOMPARE(testImage.isValid(), true);
    QCOMPARE(testImageShallowCopy.isValid(), true);
    QCOMPARE(testImageShallowCopy.size(), imageSize);
    testImage.setValue(0, 0, 123);
    QCOMPARE(testImage.getValue(0,0), 123);
    QCOMPARE(testImageShallowCopy.getValue(0,0), 123);
    
    auto x = imageSize.width -1;
    auto y = imageSize.height -1;
    testImage.setValue(x, y, 222);
    QCOMPARE(testImage.getValue(x,y), 222);
    QCOMPARE(testImageShallowCopy.getValue(x,y), 222);
    
    testImageShallowCopy.resize(roiSize);
    QCOMPARE(testImage.size(), imageSize);    
    QCOMPARE(testImageShallowCopy.size(), roiSize);
    
}

template <class ValueT>
void TestImage::testSubROIFromSizeT()
{
    QFETCH_GLOBAL(Size2d, imageSize);
    QFETCH_GLOBAL(Size2d, roiSize);
    
    TLineImage<ValueT> testImage(imageSize);
    QCOMPARE(testImage.size(), imageSize);            
   
    if (roiSize.area() == 0)
    {
        //subROI constructors require valid size, nothing else to test
        return;
    }
    TLineImage<ValueT> testROI (testImage, roiSize);
    
    bool workaroundFullROI = roiSize.width == imageSize.width +  1 && roiSize.height == imageSize.height + 1;
    bool useSizeWithBorder = !workaroundFullROI; // using a bigger roi as a workaround would crash the constructor that uses SizeWithBorder
    TLineImage<ValueT> testROISizeWithBorder (testImage, roiSize, useSizeWithBorder);
    

    for (auto pointInROI : {Point{0,0}, 
                            Point{0, std::max(roiSize.height -2,0)}, 
                            Point{std::max(roiSize.width -2,0), 0 }, 
                            Point{std::max(roiSize.width -2,0), std::max(roiSize.height -2,0) }})
    {
        if (roiSize.area() > 1)
        {
            QVERIFY(testROI.rowBegin(pointInROI.y) + pointInROI.x == testImage.rowBegin(pointInROI.y + 0) + pointInROI.x + 0);
        }
        QVERIFY(testROISizeWithBorder.rowBegin(pointInROI.y) + pointInROI.x == testImage.rowBegin(pointInROI.y + 0) + pointInROI.x + 0);
    }

    if (!workaroundFullROI)
    {
        QVERIFY(testROISizeWithBorder.end() -1 == testImage.rowBegin(roiSize.height -1) + roiSize.width -1);
    }
    
    QVERIFY(testROI.width() >= roiSize.width -1);
    QVERIFY(testROI.height() >= roiSize.height -1);
    
    for (int y = 0; y < testROI.height() ; y++)
    {
        QVERIFY(testROI.rowBegin(y) == testImage.rowBegin(y));
        QVERIFY(testROISizeWithBorder.rowBegin(y) == testImage.rowBegin(y));
    }
    
    if (!workaroundFullROI)
    {
        for (int y = 0; y < roiSize.height ; y++)
        {
            QVERIFY(testROISizeWithBorder.rowBegin(y) == testImage.rowBegin(y));
        }
    }
    
    QCOMPARE(testROI.size(), Size2d(roiSize.width -1 , roiSize.height -1));
    if (!workaroundFullROI)
    {
        QCOMPARE(testROISizeWithBorder.size(), roiSize);
    }
    
    
}

template <class ValueT>
void TestImage::testSubROIFromRectT()
{
    QFETCH_GLOBAL(Size2d, imageSize);
    QFETCH_GLOBAL(Size2d, roiSize);
    
    TLineImage<ValueT> testImage(imageSize);
    QCOMPARE(testImage.size(), imageSize);      

    if (roiSize.area() == 0)
    {
        //subROI constructors require valid size, nothing else to test
        return;
    }
    
    for (int roiStartX : {0, imageSize.width/2})
    {
            
        for (int roiStartY : {0, imageSize.height/2})
        {
            Point roiStart {roiStartX, roiStartY};
            if (roiStart.x + roiSize.width > imageSize.width + 1 || roiStart.y + roiSize.height > imageSize.height +1)
            {
                continue;
            }
            
            TLineImage<ValueT> testROI (testImage, Rect(roiStart,roiSize));
            
            bool workaroundFullROI = roiStartX + roiSize.width == imageSize.width +  1 &&  roiStartY +roiSize.height == imageSize.height + 1;
            bool useSizeWithBorder = !workaroundFullROI; // using a bigger roi as a workaround would crash the constructor that uses SizeWithBorder
            TLineImage<ValueT> testROISizeWithBorder (testImage, Rect(roiStart,roiSize), useSizeWithBorder);


            
            for (auto pointInROI : {Point{0,0}, 
                                    Point{0, std::max(roiSize.height -2,0)}, 
                                    Point{std::max(roiSize.width -2,0), 0 }, 
                                    Point{std::max(roiSize.width -2,0), std::max(roiSize.height -2,0) }})
            {
                if (roiSize.area() > 1)
                {
                    QVERIFY(testROI.rowBegin(pointInROI.y) + pointInROI.x == testImage.rowBegin(pointInROI.y + roiStart.y) + pointInROI.x + roiStart.x);
                }
                QVERIFY(testROISizeWithBorder.rowBegin(pointInROI.y) + pointInROI.x == testImage.rowBegin(pointInROI.y + roiStart.y) + pointInROI.x + roiStart.x);
            }

            if (!workaroundFullROI)
            {
                QVERIFY(testROISizeWithBorder.end() -1 == testImage.rowBegin(roiStartY + roiSize.height -1) + roiStartX + roiSize.width -1);
            }

            QVERIFY(testROI.width() >= roiSize.width -1);
            QVERIFY(testROI.height() >= roiSize.height -1);
            
            for (int y = 0; y < testROI.height() ; y++)
            {
                QVERIFY(testROI.rowBegin(y) == testImage.rowBegin(y + roiStart.y) + roiStart.x);
                QVERIFY(testROISizeWithBorder.rowBegin(y) == testImage.rowBegin(y + roiStart.y) + roiStart.x);
            }
                    
            if (!workaroundFullROI)
            {
                for (int y = 0; y < roiSize.height ; y++)
                {
                    QVERIFY(testROISizeWithBorder.rowBegin(y) == testImage.rowBegin(y + roiStart.y) + roiStart.x);
                }
            }
        
            QCOMPARE(testROI.size(), Size2d(roiSize.width -1 , roiSize.height -1));
            if (!workaroundFullROI)
            {
                QCOMPARE(testROISizeWithBorder.size(), roiSize);
            }
        
        }
    }
}



QTEST_MAIN(TestImage)
#include "testImage.moc"
