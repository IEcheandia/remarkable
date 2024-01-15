#pragma once

#include <QObject>

namespace precitec
{

namespace gui
{

class ScrollBarSynchController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal position READ position NOTIFY positionChanged)

public:
    explicit ScrollBarSynchController(QObject* parent = nullptr);
    ~ScrollBarSynchController() override;

    qreal position() const
    {
        return m_position;
    }

    Q_INVOKABLE void setPosition(qreal position);

Q_SIGNALS:
    void positionChanged();

private:
    qreal m_position = 0.0;
};

}
}


