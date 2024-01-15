#pragma once

#include <QObject>
#include <QVariant>

class QQuickItem;

namespace precitec
{
namespace storage
{
class ResultSettingModel;
}

namespace gui
{

class ProductController;

/**
 * Performs a click on the Object identified by @link{objectName}.
 **/
class ClickAction
{
    Q_GADGET
    /**
     * The Object to click
     **/
    Q_PROPERTY(QString objectName READ objectName WRITE setObjectName)
    /**
     * A hint for the parent to search for @link{objectName}
     **/
    Q_PROPERTY(QString parentObjectName READ parentObjectName WRITE setParentObjectName)
public:
    ClickAction() {}
    ClickAction(const QString &objectName, const QString &parentObjectName = {})
        : m_objectName(objectName)
        , m_parentObjectName(parentObjectName)
    {
    }

    QString objectName() const
    {
        return m_objectName;
    }
    void setObjectName(const QString &objectName)
    {
        m_objectName = objectName;
    }

    QString parentObjectName() const
    {
        return m_parentObjectName;
    }
    void setParentObjectName(const QString &objectName)
    {
        m_parentObjectName = objectName;
    }

private:
    QString m_objectName;
    QString m_parentObjectName;
};

/**
 * Waits for @link{timeout} before performing next action.
 **/
class TimerAction
{
    Q_GADGET
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
public:
    TimerAction() {}
    TimerAction(int timeout)
        : m_timeout(timeout)
    {
    }

    int timeout() const
    {
        return m_timeout;
    }
    void setTimeout(int timeout)
    {
        m_timeout = timeout;
    }

private:
    int m_timeout = 0;
};

/**
 * Performs a screenshot.
 **/
class ScreenshotAction
{
    Q_GADGET
public:
    ScreenshotAction() {}
};

/**
 * WaitForChangeAction waits till the @link{property} of the object with @link{objectName} changes to @link{value}.
 *
 * Once it changed another 100 msec delay is added to give the user interface time to repaint after the change.
 **/
class WaitForChangeAction
{
    Q_GADGET
    Q_PROPERTY(QString objectName READ objectName WRITE setObjectName)
    Q_PROPERTY(QString property READ property WRITE setProperty)
    Q_PROPERTY(QVariant value READ value WRITE setValue)
public:
    WaitForChangeAction() {}

    QString objectName() const
    {
        return m_objectName;
    }
    QString property() const
    {
        return m_property;
    }
    QVariant value() const
    {
        return m_value;
    }
    void setObjectName(const QString &name)
    {
        m_objectName = name;
    }
    void setProperty(const QString &property)
    {
        m_property = property;
    }
    void setValue(const QVariant &value)
    {
        m_value = value;
    }

private:
    QString m_objectName;
    QString m_property;
    QVariant m_value;
};

/**
 * The VerificationController is intented to support running a verification.
 * It can install a dedicated verification product, graph and result configuration.
 **/
class VerificationController : public QObject
{
    Q_OBJECT
    /**
     * ProductController, required for importing the verification product.
     **/
    Q_PROPERTY(precitec::gui::ProductController *productController READ productController WRITE setProductController NOTIFY productControllerChanged)
    /**
     * ResultsConfigModel, required for importing verification result configuration.
     **/
    Q_PROPERTY(precitec::storage::ResultSettingModel *resultsConfigModel READ resultsConfigModel WRITE setResultsConfigModel NOTIFY resultsConfigModelChanged)
    /**
     * Directory containing the graphs. Required for importing the verification graph.
     **/
    Q_PROPERTY(QString graphDir READ graphDir WRITE setGraphDir NOTIFY graphDirChanged)
public:
    VerificationController(QObject *parent = nullptr);
    ~VerificationController() override;

    void setProductController(ProductController *controller);
    ProductController *productController() const
    {
        return m_productController;
    }

    storage::ResultSettingModel *resultsConfigModel() const
    {
        return m_resultsConfigModel;
    }
    void setResultsConfigModel(storage::ResultSettingModel *model);

    QString graphDir() const
    {
        return m_graphDir;
    }
    void setGraphDir(const QString &graphDir);


    /**
     * Prepares verification by installing the appropriate product, graph and results configuration.
     **/
    Q_INVOKABLE void prepare();

    /**
     * Removes verification product and graph. Result configuration is not reverted.
     **/
    Q_INVOKABLE void remove();

    /**
     * Performs the @p actions
     **/
    Q_INVOKABLE void performTest(const QVariantList &actions);

    /**
     * Creates a ClickAction with the provided @p properties.
     **/
    Q_INVOKABLE precitec::gui::ClickAction createClickAction(const QVariantMap &properties = {}) const;

    /**
     * Creates a TimerAction with the provided @p properties.
     **/
    Q_INVOKABLE precitec::gui::TimerAction createTimerAction(const QVariantMap &properties = {}) const;

    /**
     * Creates a ScreenshotAction with the provided @p properties.
     **/
    Q_INVOKABLE precitec::gui::ScreenshotAction createScreenshotAction(const QVariantMap &properties = {}) const;

    /**
     * Creates a WaitForChangeAction with the provided @p properties.
     **/
    Q_INVOKABLE precitec::gui::WaitForChangeAction createWaitForChangeAction(const QVariantMap &properties = {}) const;

Q_SIGNALS:
    void productControllerChanged();
    void graphDirChanged();
    void resultsConfigModelChanged();
    /**
     * Internal signal for handling WaitForChangeActions
     **/
    void internalHandleChange();

private:
    void importGraph();
    void importProduct();
    void importResultConfig();
    void handleChange();

    void performNextAction();
    void clickItem(const QString &objectName, const QString &parentHint = QString{});
    QQuickItem *findItem(const QString &objectName, const QString &parentHint);
    QQuickItem *findItemRecursively(QQuickItem *item, const QString &objectName);

    ProductController *m_productController = nullptr;
    QMetaObject::Connection m_productControllerDestroyed;
    storage::ResultSettingModel *m_resultsConfigModel = nullptr;
    QMetaObject::Connection m_resultsConfigModelDestroyed;
    QString m_graphDir;
    QVariantList m_actions;

    QQuickItem *m_waitForChangeItem = nullptr;
    WaitForChangeAction m_waitForChange;
    QMetaObject::Connection m_waitForChangeConnection;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ClickAction)
Q_DECLARE_METATYPE(precitec::gui::TimerAction)
Q_DECLARE_METATYPE(precitec::gui::ScreenshotAction)
Q_DECLARE_METATYPE(precitec::gui::WaitForChangeAction)
