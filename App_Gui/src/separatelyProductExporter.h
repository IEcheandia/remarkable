#pragma once

#include <QString>
#include <QObject>

namespace precitec
{
namespace gui
{
namespace components
{
namespace removableDevices
{

class CopyService;

}
}
}
}

namespace precitec
{

namespace storage
{
class Product;
}

namespace gui
{

using precitec::storage::Product;

class SeparatelyProductExporter: public QObject
{
    Q_OBJECT

public:
    explicit SeparatelyProductExporter(precitec::gui::components::removableDevices::CopyService* copyService, QObject *parent = nullptr);

    void start(const QString& path, const QString& scanfieldPath, Product* product);

signals:
    void copyInProgressChanged(bool isCopying);
    void finished();

private:
    void setCopyInProgress(bool isCopying);

    void exportAssemblyImage(const QString& path, const Product* product) const;

    precitec::gui::components::removableDevices::CopyService* m_copyService = nullptr;
    bool m_isCopying{false};
    std::atomic<bool> m_copyServiceInProgress{false};
};
}
}
