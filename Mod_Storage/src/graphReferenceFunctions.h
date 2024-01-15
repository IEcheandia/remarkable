#pragma once
#include "product.h"
#include "seam.h"
#include "graphReference.h"

namespace precitec::storage::graphFunctions
{

/**
 * @brief Dereferences the given ref and performs an action on the MeasureTask to which ref refers.
 *
 * Dereferencing means in this context looking up the MeasureTask in the product.
 */
template<class ActionT>
auto doOnLinkedTarget(const LinkedGraphReference& ref, const Product* product, const ActionT& action)
{
    using ResultT = std::decay_t<decltype(action(std::declval<AbstractMeasureTask>()))>;
    if (!product)
    {
        return ResultT{};
    }

    auto* linkedGraphTask = product->findMeasureTask(ref.value);
    if (!linkedGraphTask)
    {
        return ResultT{};
    }
    return action(*linkedGraphTask);
}

/**
 * @brief Deferenrences the given ref and visits the reference of the target to which ref refers.
 */
template<class VisitorT>
auto visitRefOfLinkedTarget(const LinkedGraphReference& ref, const Product* product, VisitorT visitor)
{
    return doOnLinkedTarget(ref, product, [&](const AbstractMeasureTask& task)
                            { return std::visit(visitor, task.graphReference()); });
}
}