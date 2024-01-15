#pragma once

#include <QuickQanava.h>
#include <QString>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterComment : public qan::Node
{
    Q_OBJECT
    Q_PROPERTY (QUuid ID READ ID WRITE setID NOTIFY IDChanged)
    Q_PROPERTY (QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY (int groupID READ groupID WRITE setGroupID NOTIFY groupIDChanged)
    /**
     * Url to the qml file containing the configuration interface for the Comment this node represents.
     **/
    Q_PROPERTY(QUrl configurationInterface READ configurationInterface CONSTANT)

public:
    explicit FilterComment( QObject* parent = nullptr);
    ~FilterComment() override;

    QUuid ID() const;
    void setID(const QUuid &id);
    QString text() const;
    void setText(const QString &comment);
    int groupID() const;
    void setGroupID(int groupID);
    Q_INVOKABLE QRectF getItemGeometry() const;
    Q_INVOKABLE void setItemGeometry(int x, int y, int width, int height);

    //Group interaction stuff
    void nodeGroupInteraction();

    QUrl configurationInterface() const;

    static QQmlComponent* delegate(QQmlEngine &engine);

Q_SIGNALS:
    void textChanged();
    void groupIDChanged();
    void IDChanged();

private:
    QString m_text;
    int m_groupID;
    QUuid m_ID;
};

}
}
}
}
QML_DECLARE_TYPE(precitec::gui::components::grapheditor::FilterComment)
