#pragma once

#include <QObject>
#include <QProcess>


namespace precitec
{
namespace gui
{

class Process : public QProcess
{
    Q_OBJECT
public:
    explicit Process(QObject *parent = nullptr);
    ~Process() override;

protected:
    void setupChildProcess() override;
};


class RemoteDesktopController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)
public:
    RemoteDesktopController(QObject *parent = nullptr);
    ~RemoteDesktopController() override;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    bool isEnabled() const;

Q_SIGNALS:
    void enabledChanged();

private:
    void performStart();
    QProcess *m_process;
};

}
}
