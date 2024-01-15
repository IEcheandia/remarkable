#pragma once

#include <QuickQanava.h>
#include <QUuid>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterMacro : public qan::Node
{
    Q_OBJECT
    Q_PROPERTY (QUrl image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY (QString type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY (QUuid typeID READ typeID WRITE setTypeID NOTIFY typeIDChanged)
    Q_PROPERTY (QUuid ID READ ID WRITE setID NOTIFY IDChanged)
    Q_PROPERTY (int groupID READ groupID WRITE setGroupID NOTIFY groupIDChanged)
    Q_PROPERTY(QString contentName READ contentName WRITE setContentName NOTIFY contentNameChanged)
    Q_PROPERTY( bool matchingInPipeConnection READ isMatchingInPipeConnection NOTIFY matchingInPipeConnectionChanged)

    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)

    /**
     * Url to the qml file containing the configuration interface for the InstanceFilter this node represents.
     **/
    Q_PROPERTY(QUrl configurationInterface READ configurationInterface CONSTANT)

public:
    explicit FilterMacro( QObject* parent = nullptr);
    ~FilterMacro() override;

    QUrl image() const;
    void setImage(const QUrl &image);
    QString type() const;
    void setType(const QString &type);
    QUuid typeID() const;
    void setTypeID(const QUuid &id);
    QUuid ID() const;
    void setID(const QUuid &ID);
    int groupID() const;
    void setGroupID( int groupID);

    QString contentName() const;
    void setContentName(const QString &content);
    QString description() const
    {
        return m_description;
    }
    void setDescription(const QString &description);

    Q_INVOKABLE QRectF getItemGeometry() const;
    Q_INVOKABLE void setItemGeometry(int x, int y, int width = 160, int height = 160);

    bool isMatchingInPipeConnection() const
    {
        return m_matching;
    }
    void setMatchingInPipeConnection(bool matching);

    QUrl configurationInterface() const;

    static QQmlComponent* delegate(QQmlEngine &engine);

Q_SIGNALS:
    void imageChanged();
    void typeChanged();
    void typeIDChanged();
    void IDChanged();
    void groupIDChanged();
    void contentNameChanged();
    void matchingInPipeConnectionChanged();
    void descriptionChanged();

private:
    QUrl m_image;
    QString m_type;
    QUuid m_typeID;
    QUuid m_ID;
    int m_groupID;

    QString m_contentName;
    QString m_description;
    bool m_matching{true};

};

}
}
}
}
QML_DECLARE_TYPE(precitec::gui::components::grapheditor::FilterMacro)
