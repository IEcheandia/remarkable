#pragma once

#include <QObject>
#include <vector>

namespace precitec
{
namespace gui
{

class ProductInstancesCacheEditor : public QObject
{
    Q_OBJECT
public:
    ProductInstancesCacheEditor(bool liveMode = false, QObject *parent = nullptr);
    ~ProductInstancesCacheEditor() = default;

    void setProductInstanceMaxNumber(std::size_t maxNumber);

    void add(const std::vector<QString> &productInstances);

    // use only for testing reasons
    void setAbsoluteCacheFileName(const QString &fileName);

private:
    QString m_absoluteCacheFileName;
    bool m_liveMode = false;
    std::size_t m_maxProductInstancesNumber = 1000;
};

}
}