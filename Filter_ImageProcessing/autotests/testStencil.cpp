#include "../stencil.h"
#include <geo/geo.h>
#include <fliplib/NullSourceFilter.h>
#include <common/bitmap.h>
#include <QTest>
#include <overlay/overlayCanvas.h>
#include <overlay/overlayPrimitive.h>
#include <QtGui/qimage.h>
#include <QImage>

using namespace precitec::interface;
using precitec::interface::ImageContext;
using precitec::geo2d::Size;
using precitec::image::BImage;
using precitec::filter::Stencil;

class StencilTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();

private:
    void createMask(int widthMask, int heightMask, int border);
    QString m_maskFileName{"config/mask.bmp"};
    int m_widthFrame{50};
    QTemporaryDir m_dir;


};

struct DummyInput
{

private:
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<ImageFrame> pipeInImageFrame{&sourceFilter, "ImageIn"};
    ImageFrame m_imageFrameIn;

public:
    DummyInput()
    {
        pipeInImageFrame.setTag("ImageIn");
    }

    bool connectToFilter(fliplib::BaseFilter* pFilter)
    {
        //connect pipes
        const int group = 1;
        bool ok = pFilter->connectPipe(&(pipeInImageFrame), group);
        return ok;
    }


    void fillDataAndSignal(int imageNumber, int width, int height)
    {
        ImageContext context;
        context.setImageNumber(imageNumber);

        BImage image{Size{width, height}};
        image.fill(255);
        m_imageFrameIn = ImageFrame{0, context, image, AnalysisOK};
    }

    void signal()
    {
        pipeInImageFrame.signal(m_imageFrameIn);
    }
};


class DummyOutFilter: public fliplib::BaseFilter
{
public:

    DummyOutFilter(): fliplib::BaseFilter("dummy") {}
    void proceed(const void *sender, fliplib::PipeEventArgs &e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }

    void proceedGroup (const void * sender, fliplib::PipeGroupEventArgs& e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }

    int getFilterType() const override
    {
        return BaseFilterInterface::SINK;
    }

    bool isProceedCalled() const
    {
        return m_proceedCalled;
    }

    void resetProceedCalled()
    {
        m_proceedCalled = false;
    }

private:
    bool m_proceedCalled = false;
};


void StencilTest::testCtor()
{
    Stencil filter;
    QCOMPARE(filter.name(), std::string("Stencil"));
    QVERIFY(filter.findPipe("ImageOut") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);

    QVERIFY(filter.getParameters().exists(std::string("MaskFileName")));
    QCOMPARE(filter.getParameters().findParameter(std::string("MaskFileName")).getType(), fliplib::Parameter::TYPE_string);
    QCOMPARE(filter.getParameters().findParameter(std::string("MaskFileName")).getValue().convert<std::string>(), "config/mask.bmp");
}

void StencilTest::testProceed_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("widthMask");
    QTest::addColumn<int>("heightMask");
    QTest::addColumn<int>("border");
    QTest::addColumn<bool>("isValid");
    QTest::addRow("valid") << "config/mask.bmp" << 50 << 50 << 5 << true;
    QTest::addRow("maskNotFound") << "config/mask1.bmp" << 50 << 50 << 5 << false;
    QTest::addRow("fileSizeIncorrect") << "config/mask.bmp" << 60 << 60 << 5 << false;
}

void StencilTest::createMask(int widthMask, int heightMask, int border)
{
    QVERIFY(m_dir.isValid());
    QString configDir("config/");
    QDir(m_dir.path()).mkdir(configDir);
    QVERIFY(QDir(m_dir.path()).exists("config/"));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());

    // create mask
    QImage mask{widthMask, heightMask, QImage::Format_Indexed8};
    mask.setColor(0, 0);
    mask.setColor(255, 255);
    mask.fill(255);
    for (int i = 0; i < widthMask; i++)
    {
        for (int j = 0; j < heightMask; j++)
        {
            if (i > border && i < widthMask - border)
            {
                if (j > border && j < heightMask - border)
                {
                    mask.setPixel(i, j, 0);
                }
            }
        }
    }
    mask.save(m_dir.path() + "/" + m_maskFileName);
}

void StencilTest::testProceed()
{
    Stencil filter;
    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());

    // create and connect pipes
    DummyInput dummyInput;
    DummyOutFilter dummyOutFilter;
    QFETCH(QString, filename);
    filter.getParameters().update(std::string("MaskFileName"), fliplib::Parameter::TYPE_string, filename.toStdString());
    filter.setParameter();

    QVERIFY(dummyInput.connectToFilter(&filter));
    QVERIFY(dummyOutFilter.connectPipe(filter.findPipe("ImageOut"), 0));

    QFETCH(int, widthMask);
    QFETCH(int, heightMask);
    QFETCH(int, border);
    QFETCH(bool, isValid);

    createMask(widthMask, heightMask, border);

    const int imageNumber = 0;
    dummyInput.fillDataAndSignal(imageNumber, m_widthFrame, m_widthFrame);

    QCOMPARE(dummyOutFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyOutFilter.isProceedCalled(), true);
    dummyOutFilter.resetProceedCalled();

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<ImageFrame>*>(filter.findPipe("ImageOut"));
    QVERIFY(outPipe);
    const auto result = outPipe->read(0);
    const auto frame = result.data();
    QCOMPARE(isValid, frame.isValid());

    const auto filePath = m_dir.path() + "/" + filename;

    const auto mask = QImage(filePath);

    for (int i = 0; i < frame.width(); i++)
    {
        for (int j = 0; j < frame.height(); j++)
        {
            QCOMPARE(mask.pixelColor(i, j).blue(), frame.getValue(i, j));
        }
    }
}

QTEST_GUILESS_MAIN(StencilTest);
#include "testStencil.moc"

