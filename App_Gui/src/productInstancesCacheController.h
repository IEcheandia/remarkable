#pragma once

#include <QObject>

#include <vector>

class ProductInstancesCacheControllerTest;

namespace precitec
{
namespace gui
{

class ProductInstancesCacheController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool liveMode READ liveMode WRITE setLiveMode NOTIFY liveModeChanged)
public:
    ProductInstancesCacheController(QObject *parent = nullptr);
    ~ProductInstancesCacheController() = default;

    bool liveMode() const;
    void setLiveMode(bool liveMode);
    Q_INVOKABLE void addProductInstancePathToBuffer(const QString &pathToProductInstance);
    Q_INVOKABLE void cacheBuffer();
    Q_INVOKABLE void clearCacheBuffer();
    Q_INVOKABLE bool exists(const QString &directory);

    std::vector<QString> buffer() const
    {
        return m_buffer;
    }
Q_SIGNALS:
    void liveModeChanged();

private:
    std::vector<QString> m_buffer;
    bool m_liveMode = false;

    friend ProductInstancesCacheControllerTest;
    void setAbsoluteCacheFileName(const QString &fileName);
    QString m_absoluteCacheFileName;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ProductInstancesCacheController *)
