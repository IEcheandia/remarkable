
/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2016
 *  @brief			http://stackoverflow.com/questions/17356258/correctly-implement-finally-block-using-c-lambda
 */


#ifndef FINALLY_H_20160727_INCLUDED
#define FINALLY_H_20160727_INCLUDED

// stdlib includes
#include <functional>

namespace precitec 
{


class Finally
{
public:
    Finally() = delete;
    Finally(const Finally&) = delete;
    Finally(std::function<void()> p_oFinalizerFunctor)    // Please check that the finally block cannot throw, and mark the lambda as noexcept.
        : 
        m_oFinalizerFunctor(p_oFinalizerFunctor)
    {
    }

    ~Finally() /*noexcept*/
    {
        m_oFinalizerFunctor();
    }

private:
    std::function<void()> m_oFinalizerFunctor;
};


} // namespace precitec

#endif // FINALLY_H_20160727_INCLUDED
