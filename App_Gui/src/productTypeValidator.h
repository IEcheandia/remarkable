#pragma once

#include <QValidator>

namespace precitec
{

namespace storage
{
class Product;
}

namespace gui
{

class ProductController;

class ProductTypeValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::ProductController *controller READ controller WRITE setController NOTIFY controllerChanged)
    Q_PROPERTY(precitec::storage::Product *product READ product WRITE setProduct NOTIFY productChanged)
public:
    ProductTypeValidator(QObject *parent = nullptr);
    ~ProductTypeValidator() override;

    QValidator::State validate(QString &input, int &pos) const override;
    void fixup(QString &input) const override;

    void setController(precitec::gui::ProductController *controller);
    precitec::gui::ProductController *controller() const
    {
        return m_controller;
    }

    void setProduct(precitec::storage::Product *product);
    precitec::storage::Product *product() const
    {
        return m_product;
    }

Q_SIGNALS:
    void controllerChanged();
    void productChanged();

private:
    ProductController *m_controller = nullptr;
    storage::Product *m_product = nullptr;
    QMetaObject::Connection m_controllerConnection;
    QMetaObject::Connection m_productConnection;
};

}
}
