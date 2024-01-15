#pragma once

#include <QImage>
#include <QSharedPointer>
#include <QuickQanava.h>
#include <QStandardPaths>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterGraphImageSaver : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qan::GraphView* graphView READ graphView WRITE setGraphView NOTIFY graphViewChanged)
    Q_PROPERTY(QString pathForImages READ pathForImages WRITE setPathForImages NOTIFY pathForImagesChanged)
    Q_PROPERTY(QString imageName READ imageName NOTIFY imageNameChanged)

public:
    explicit FilterGraphImageSaver(QObject *parent = nullptr);
    ~FilterGraphImageSaver() override;

    qan::GraphView* graphView() const;
    void setGraphView(qan::GraphView* filterGraphView);
    QString pathForImages() const;
    void setPathForImages(const QString &path);
    QString imageName() const;

    Q_INVOKABLE void getDynamicImage();
    Q_INVOKABLE void focusGraph();

Q_SIGNALS:
        void graphViewChanged();
        void pathForImagesChanged();
        void imageNameChanged();

private:
    qan::GraphView* m_graphView = nullptr;
    QString m_path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString m_imageName;
    QSharedPointer<QQuickItemGrabResult> m_grabPointer = nullptr;

};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterGraphImageSaver*)
