#pragma once

#include "array.h"
#include "point.h"
#include <map>

namespace precitec
{
namespace geo2d
{

    template <typename T>
    class TAnnotatedArray
    {
    public:
        enum class Scalar
        {
            LaserPower,
            LaserPowerRing,
            LaserVelocity
        };

        typedef std::vector<double> scalardata_t;
        typedef std::map<Scalar, scalardata_t> scalarmap_t;

        explicit TAnnotatedArray(std::size_t p_oSize = 0, T p_oVal = T(), int p_oRank = filter::eRankMin)
        : m_array(p_oSize, p_oVal, p_oRank)
        {}

        explicit TAnnotatedArray(const TArray<T> & p_oArray)
        : m_array(p_oArray)
        {}

        const std::vector<T> &getData () const
        {
            return m_array.getData();
        }

        std::vector<T> &getData ()
        {
            return m_array.getData();
        }

        const std::vector<int> &getRank () const
        {
            return m_array.getRank();
        }

        std::vector<int> &getRank ()
        {
            return m_array.getRank();
        }

        bool hasScalarData() const
        {
            return m_scalardata.size() > 0;
        }

        bool hasScalarData(Scalar name) const
        {
            return m_scalardata.find(name) != m_scalardata.end();
        }

        const scalardata_t & getScalarData(Scalar name) const
        {
            return m_scalardata.at(name); //exception if array does not exists
        }
        scalardata_t & getScalarData(Scalar name)
        {
            return m_scalardata[name]; //creates array if array does not exists
        }

        std::vector<Scalar> getScalarDataTypes() const
        {
            std::vector<Scalar> result;
            for ( auto & entry : m_scalardata)
            {
                result.push_back(entry.first);
            }
            return result;
        }

        std::size_t size() const {
            auto result = m_array.size();
            assert(std::all_of(m_scalardata.begin(), m_scalardata.end(), [result]( typename scalarmap_t::value_type v)
                            {
                                return v.second.size() == result;
                            } ) && "SIZE() Inconsistency between scalar and data array");
            return result;
        }
        void reinitialize(const T &rValue = T())
        {
            m_array.reinitialize(rValue);
            m_scalardata.clear();
        }
        void reinitializeScalar(Scalar name, double value = 0.0)
        {
            //creates array if array does not exists
            m_scalardata[name].assign(m_array.size(), value);
        }
        void assign( std::size_t p_oSize = 0u, T p_oVal = T(), int p_oRank = filter::eRankMin )
        {
            m_array.assign(p_oSize, p_oVal, p_oRank);
            m_scalardata.clear();
        }
        std::pair<typename scalarmap_t::iterator,bool>  insertScalar(Scalar name)
        {
            //inserts if not present, otherwise does not change value
            return m_scalardata.insert({name, scalardata_t{}});
        }

        void resize(std::size_t n)
        {
            m_array.resize(n);
            for (auto && entry : m_scalardata)
            {
                entry.second.resize(n);
            }
        }
        void reserve(std::size_t n)
        {
            m_array.reserve(n);
            for (auto && entry : m_scalardata)
            {
                entry.second.reserve(n);
            }
        }

        void clear()
        {
            m_array.clear();
            for (auto && entry : m_scalardata)
            {
                entry.second.clear();
            }
        }
    private:
        TArray<T> m_array;
        scalarmap_t m_scalardata;
    };

    typedef TAnnotatedArray< DPoint >		   AnnotatedDPointarray;

}
}

