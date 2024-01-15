#include "verificationController.h"
#include "productController.h"
#include "resultSettingModel.h"
#include "product.h"

#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickWindow>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>

using precitec::storage::ResultSettingModel;

namespace precitec
{
namespace gui
{

VerificationController::VerificationController(QObject *parent)
    : QObject(parent)
{
    connect(this, &VerificationController::internalHandleChange, this, &VerificationController::handleChange);
}

VerificationController::~VerificationController() = default;

void VerificationController::setProductController(ProductController *controller)
{
    if (m_productController == controller)
    {
        return;
    }
    disconnect(m_productControllerDestroyed);
    m_productControllerDestroyed = {};
    m_productController = controller;
    if (m_productController)
    {
        m_productControllerDestroyed = connect(m_productController, &QObject::destroyed, std::bind(&VerificationController::setProductController, this, nullptr));
    }
    emit productControllerChanged();
}

void VerificationController::setResultsConfigModel(ResultSettingModel *model)
{
    if (m_resultsConfigModel == model)
    {
        return;
    }
    disconnect(m_resultsConfigModelDestroyed);
    m_resultsConfigModelDestroyed = {};
    m_resultsConfigModel = model;
    if (m_resultsConfigModel)
    {
        m_resultsConfigModelDestroyed = connect(m_resultsConfigModel, &QObject::destroyed, std::bind(&VerificationController::setResultsConfigModel, this, nullptr));
    }
    emit resultsConfigModelChanged();
}

void VerificationController::setGraphDir(const QString& graphDir)
{
    if (m_graphDir == graphDir)
    {
        return;
    }
    m_graphDir = graphDir;
    emit graphDirChanged();
}

void VerificationController::prepare()
{
    importGraph();
    QTimer::singleShot(500, this, &VerificationController::importResultConfig);
    QTimer::singleShot(1000, this, &VerificationController::importProduct);
}

void VerificationController::remove()
{
    if (m_productController)
    {
        m_productController->deleteProduct({QByteArrayLiteral("07ed57c7-1a82-485c-b109-2f58fae7a3ef")});
    }
    if (!m_graphDir.isEmpty())
    {
        QDir graphDir{m_graphDir};
        if (!graphDir.exists())
        {
            return;
        }
        QFile::remove(graphDir.filePath(QStringLiteral("d969550b-35b6-4902-9b27-b677e31eab77.xml")));
    }
}

void VerificationController::importGraph()
{
    if (m_graphDir.isEmpty())
    {
        return;
    }
    QDir graphDir{m_graphDir};
    if (!graphDir.exists())
    {
        return;
    }
    QFile::copy(QStringLiteral(":/resources/graphs/verification.xml"), graphDir.filePath(QStringLiteral("d969550b-35b6-4902-9b27-b677e31eab77.xml")));
}

void VerificationController::importProduct()
{
    if (!m_productController)
    {
        return;
    }
    m_productController->importProduct(QStringLiteral(":/resources/products/verification.json"));
    m_productController->saveChanges();
}

void VerificationController::importResultConfig()
{
    if (!m_resultsConfigModel)
    {
        return;
    }
    m_resultsConfigModel->import(QStringLiteral(":/resources/resultsConfig/verification.json"));
}

template <typename T>
static T createAction(const QVariantMap &properties)
{
    T action;
    for (auto it = properties.begin(); it != properties.end(); it++)
    {
        for (int i = 0; i < action.staticMetaObject.propertyCount(); i++)
        {
            auto property = action.staticMetaObject.property(i);
            if (qstrcmp(property.name(), it.key().toUtf8()) == 0)
            {
                property.writeOnGadget(&action, it.value());
            }
        }
    }
    return action;
}

ClickAction VerificationController::createClickAction(const QVariantMap &properties) const
{
    return createAction<ClickAction>(properties);
}

TimerAction VerificationController::createTimerAction(const QVariantMap &properties) const
{
    return createAction<TimerAction>(properties);
}

ScreenshotAction VerificationController::createScreenshotAction(const QVariantMap &properties) const
{
    return createAction<ScreenshotAction>(properties);
}

WaitForChangeAction VerificationController::createWaitForChangeAction(const QVariantMap &properties) const
{
    return createAction<WaitForChangeAction>(properties);
}

void VerificationController::performTest(const QVariantList &actions)
{
    if (!m_actions.empty())
    {
        return;
    }
    m_actions = actions;

    performNextAction();
}

void VerificationController::performNextAction()
{
    if (m_actions.empty())
    {
        return;
    }
    auto action = m_actions.front();
    m_actions.erase(m_actions.begin());
    int timer = 0;
    if (action.canConvert<ClickAction>())
    {
        const auto clickAction = action.value<ClickAction>();
        clickItem(clickAction.objectName(), clickAction.parentObjectName());
    }
    if (action.canConvert<ScreenshotAction>())
    {
        if (auto window = qobject_cast<QQuickWindow*>(qApp->topLevelWindows().front()))
        {
            const auto name = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
            const auto image = window->grabWindow();
            image.save(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QStringLiteral("/%1-verification-screenshot.png").arg(name));
        }
    }
    if (action.canConvert<TimerAction>())
    {
        timer = action.value<TimerAction>().timeout();
    }
    if (action.canConvert<WaitForChangeAction>())
    {
        const auto waitAction = action.value<WaitForChangeAction>();
        if (auto item = findItem(waitAction.objectName(), {}))
        {
            for (int i = 0; i < item->metaObject()->propertyCount(); i++)
            {
                auto property = item->metaObject()->property(i);
                if (qstrcmp(property.name(), waitAction.property().toUtf8()) == 0)
                {
                    m_waitForChangeItem = item;
                    m_waitForChange = waitAction;
                    m_waitForChangeConnection = connect(item, property.notifySignal(), this, QMetaMethod::fromSignal(&VerificationController::internalHandleChange));
                    return;
                }
            }
        }
    }
    QTimer::singleShot(timer, this, &VerificationController::performNextAction);
}

void VerificationController::handleChange()
{
    if (m_waitForChangeItem)
    {
        if (m_waitForChangeItem->property(m_waitForChange.property().toUtf8().constData()) == m_waitForChange.value())
        {
            disconnect(m_waitForChangeConnection);
            m_waitForChangeItem = nullptr;
            QTimer::singleShot(100, this, &VerificationController::performNextAction);
        }
    }
}

void VerificationController::clickItem(const QString &objectName, const QString &parentHint)
{
    static int s_lastMouseTimestamp = 0;
    if (auto item = findItem(objectName, parentHint))
    {
        // perform click on a QQuickControl 2 AbstractButton which is checkable by invoking the toggle slot
        if (item->property("checkable").toBool())
        {
            const int slot = item->metaObject()->indexOfSlot("toggle()");
            if (slot != -1)
            {
                item->metaObject()->method(slot).invoke(item);
                return;
            }
        }
        // perform click by mouse press followed by mouse release on the center of the item
        const auto itemCenter = QRectF{0, 0, item->width(), item->height()}.center();
        const auto windowPos = item->mapToScene(itemCenter).toPoint();
        const auto globalPos = item->window()->mapToGlobal(windowPos);

        QMouseEvent press{QEvent::MouseButtonPress, windowPos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
        press.setTimestamp(++s_lastMouseTimestamp);
        press.setAccepted(false);
        qApp->notify(item->window(), &press);

        // and release
        QMouseEvent release{QEvent::MouseButtonRelease, windowPos, globalPos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier};
        release.setTimestamp(++s_lastMouseTimestamp);
        release.setAccepted(false);
        qApp->notify(item->window(), &release);
    }
}

QQuickItem *VerificationController::findItem(const QString &objectName, const QString &parentHint)
{
    auto window = qobject_cast<QQuickWindow*>(qApp->topLevelWindows().front());
    if (!window)
    {
        return nullptr;
    }
    // check whether we find the item as child of the QQuickWindow
    if (auto item = window->findChild<QQuickItem*>(objectName))
    {
        return item;
    }
    // not found - do we have a parent hint?
    if (parentHint.isEmpty())
    {
        // if not, we couldn't find it
        return nullptr;
    }
    // find the parent item and check children recursively
    return findItemRecursively(window->findChild<QQuickItem*>(parentHint), objectName);
}

QQuickItem *VerificationController::findItemRecursively(QQuickItem *item, const QString &objectName)
{
    if (!item)
    {
        // not found
        return nullptr;
    }
    if (item->objectName() == objectName)
    {
        // it's our item
        return item;
    }
    for (auto child : item->childItems())
    {
        // recursion step, go over all childItems
        if (auto foundChild = findItemRecursively(child, objectName))
        {
            return foundChild;
        }
    }
    return nullptr;
}

}
}
