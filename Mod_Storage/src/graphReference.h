#pragma once
#include <QMetaType>
#include <QStringLiteral>
#include <QUuid>

#include <variant>
#include <vector>

namespace precitec::storage
{

/**
 * @brief Refers to a single graph.
 */
struct SingleGraphReference
{
    QUuid value; ///< An id of a graph.
};

[[nodiscard]] inline bool operator==(const SingleGraphReference& lhs, const SingleGraphReference& rhs)
{
    return lhs.value == rhs.value;
}

[[nodiscard]] inline bool operator!=(const SingleGraphReference& lhs, const SingleGraphReference& rhs)
{
    return !(lhs == rhs);
}

/**
 * @brief Refers to a list of subGraphs.
 */
struct SubGraphReference
{
    std::vector<QUuid> value; ///< A list of ids of subGraphs
};

[[nodiscard]] inline bool operator==(const SubGraphReference& lhs, const SubGraphReference& rhs)
{
    return lhs.value == rhs.value;
}

[[nodiscard]] inline bool operator!=(const SubGraphReference& lhs, const SubGraphReference& rhs)
{
    return !(lhs == rhs);
}

/**
 * @brief Refers to another seam which provides the graph or subGraph to use.
 */
struct LinkedGraphReference
{
    QUuid value; ///< An id of another seam in the same product.
};

[[nodiscard]] inline bool operator==(const LinkedGraphReference& lhs, const LinkedGraphReference& rhs)
{
    return lhs.value == rhs.value;
}

[[nodiscard]] inline bool operator!=(const LinkedGraphReference& lhs, const LinkedGraphReference& rhs)
{
    return !(lhs == rhs);
}

/**
 * @brief a generic reference to a graph
 */
using GraphReference = std::variant<SubGraphReference, SingleGraphReference, LinkedGraphReference>;

/**
 * @brief checks if the given reference refers to subGraphs
 */
[[nodiscard]] inline bool hasSubGraphs(const GraphReference& ref)
{
    const auto* ref_ptr = std::get_if<SubGraphReference>(&ref);
    return ref_ptr != nullptr;
}

/**
 * @brief checks if the given reference refers to a single graph
 */
[[nodiscard]] inline bool hasGraph(const GraphReference& ref)
{
    const auto* ref_ptr = std::get_if<SingleGraphReference>(&ref);
    return ref_ptr != nullptr && !ref_ptr->value.isNull();
}

/**
 * @brief checks if the given reference refers to another link to get the graph from
 */
[[nodiscard]] inline bool hasLinkedGraph(const GraphReference& ref)
{
    return std::holds_alternative<LinkedGraphReference>(ref);
}

/**
 * @brief returns the value of the given reference if the type matches, or an empty default of the given type.
 */
template<class RefType>
[[nodiscard]] const auto& valueOrDefault(const GraphReference& ref)
{
    if (const auto* ref_ptr = std::get_if<RefType>(&ref))
    {
        return ref_ptr->value;
    }
    static decltype(RefType{}.value) defaultValue{};
    return defaultValue;
}

/**
 * @brief helper type for variant visitors to be used with multiple lambdas
 *
 * taken from https://en.cppreference.com/w/cpp/utility/variant/visit
 *
 * usage:
 * std::visit(overloaded{
 *   [](SingleGraphReference const &){...}},
 *   [](SubGraphReference const &){...}},
 *   [](LinkedGraphReference const &){...}},
 * });
 */
template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

/**
 * @brief returns a human-readable string used in the ChangeTracker.
 */
[[nodiscard]] inline QString toStringForChangeTracking(const GraphReference& ref)
{
    return std::visit(overloaded{[](const SingleGraphReference& ref)
                                 {
                                     return ref.value.toString();
                                 },
                                 [](const SubGraphReference& ref)
                                 {
                                     QString ret;
                                     for (const auto& uuid : ref.value)
                                     {
                                         ret += uuid.toString();
                                     }
                                     return ret;
                                 },
                                 [](const LinkedGraphReference& ref)
                                 {
                                     return QStringLiteral("Linked to Seam ") + ref.value.toString();
                                 }},
                      ref);
}
}

Q_DECLARE_METATYPE(precitec::storage::SingleGraphReference);
Q_DECLARE_METATYPE(precitec::storage::SubGraphReference);
Q_DECLARE_METATYPE(precitec::storage::GraphReference);