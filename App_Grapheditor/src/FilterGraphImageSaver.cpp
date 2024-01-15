#include "FilterGraphImageSaver.h"
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QtSvg/QSvgGenerator>

using namespace precitec::gui::components::grapheditor;

FilterGraphImageSaver::FilterGraphImageSaver(QObject* parent) : QObject(parent)
{

}

FilterGraphImageSaver::~FilterGraphImageSaver() = default;

qan::GraphView * FilterGraphImageSaver::graphView() const
{
    return m_graphView;
}

void FilterGraphImageSaver::setGraphView ( qan::GraphView* filterGraphView )
{
    if (m_graphView != filterGraphView)
    {
        m_graphView = filterGraphView;
        emit graphViewChanged();
    }
}

QString FilterGraphImageSaver::pathForImages() const
{
    return m_path;
}

void FilterGraphImageSaver::setPathForImages(const QString& path)
{
    if (m_path != path)
    {
        m_path = path;
        emit pathForImagesChanged();
    }
}

QString FilterGraphImageSaver::imageName() const
{
    return m_imageName;
}

void FilterGraphImageSaver::getDynamicImage()
{
    //const auto name = QDir::homePath() + "/" + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")) + ".ppm";
    m_imageName = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")) + ".png";
    emit imageNameChanged();
    const auto name = m_path + "/" + m_imageName;

    m_graphView->getGrid()->setVisible(false);
    m_graphView->setTransformOrigin(QQuickItem::Center);
    m_graphView->getContainerItem()->setTransformOrigin(QQuickItem::Center);
    m_graphView->fitInView();       //Maybe a oqn fitInView function, which makes the graph in the center of the graphView
    QSize imageSize{m_graphView->size().toSize()};
    imageSize.setWidth(imageSize.width()*5);    //10000
    imageSize.setHeight(imageSize.height()*5);  //10000
    auto oldScale = m_graphView->getContainerItem()->scale();
    auto oldZoom = m_graphView->getZoom();
    m_grabPointer = m_graphView->grabToImage(imageSize);
    connect(m_grabPointer.data(), &QQuickItemGrabResult::ready, [=](){
        qDebug() << "Image will be saved!";
        qDebug() << "Result of Image: " << m_grabPointer->image().isNull();
        const auto *grabPointer = m_grabPointer.data();
        qDebug() << "Result: " << grabPointer->saveToFile(name);
        m_grabPointer.clear();
        m_graphView->getGrid()->setVisible(true);
        m_graphView->getContainerItem()->setScale(oldScale);
        m_graphView->setZoom(oldZoom);
    });           //Need to disconnect?    //Tested image formats are png, jpeg, tiff, bmp
    m_graphView->getContainerItem()->setTransformOrigin(QQuickItem::TopLeft);
    m_graphView->setTransformOrigin(QQuickItem::TopLeft);
}

void FilterGraphImageSaver::focusGraph()
{
    m_graphView->fitInView();
}
