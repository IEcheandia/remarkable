#include "productTypeValidator.h"
#include "productController.h"
#include "product.h"

namespace precitec
{
namespace gui
{

ProductTypeValidator::ProductTypeValidator(QObject* parent)
    : QValidator(parent)
{
}

ProductTypeValidator::~ProductTypeValidator() = default;

void ProductTypeValidator::setController(precitec::gui::ProductController* controller)
{
    if (m_controller == controller)
    {
        return;
    }
    m_controller = controller;
    disconnect(m_controllerConnection);
    if (m_product)
    {
        m_controllerConnection = connect(m_product, &QObject::destroyed, this, std::bind(&ProductTypeValidator::setController, this, nullptr));
    } else
    {
        m_controllerConnection = {};
    }
    emit controllerChanged();
}

void ProductTypeValidator::setProduct(precitec::storage::Product *product)
{
    if (m_product == product)
    {
        return;
    }
    m_product = product;
    disconnect(m_productConnection);
    if (m_product)
    {
        m_productConnection = connect(m_product, &QObject::destroyed, this, std::bind(&ProductTypeValidator::setProduct, this, nullptr));
    } else
    {
        m_productConnection = {};
    }
    emit productChanged();
}

QValidator::State ProductTypeValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos)
    if (!m_controller || !m_product)
    {
        return QValidator::Invalid;
    }
    if (input.isEmpty())
    {
        return QValidator::Intermediate;
    }
    bool ok = false;
    int number = input.toInt(&ok);
    if (!ok)
    {
        return QValidator::Invalid;
    }
    if (number < 0)
    {
        return QValidator::Invalid;
    }
    if (m_controller->isTypeValid(m_product, number))
    {
        return QValidator::Acceptable;
    }
    return QValidator::Intermediate;
}

void ProductTypeValidator::fixup(QString &input) const
{
    if (m_controller)
    {
        input = QString::number(m_controller->nextType());
    }
}

}
}
