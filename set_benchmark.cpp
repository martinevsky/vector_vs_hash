#include <set>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_set>

#include <boost/range.hpp>
#include <boost/intrusive/unordered_set.hpp>

#include <benchmark/benchmark.h>

using ValueType = int;
using DataType = std::vector<ValueType>;
using DataConstRange = boost::iterator_range<DataType::const_iterator>;

class SortedVector
{
public:
    SortedVector (DataConstRange data):
        m_data (data.begin(), data.end())
    {
        std::sort (m_data.begin(), m_data.end());
    }

    bool Find (ValueType value) const noexcept
    {
        return std::binary_search (m_data.begin(), m_data.end(), value);
    }

private:
    DataType m_data;
};

class UnsortedVector
{
public:
    UnsortedVector (DataConstRange data):
        m_data (data.begin(), data.end())
    {
        std::sort (m_data.begin(), m_data.end());
    }

    bool Find (ValueType value) const noexcept
    {
        return std::find (m_data.begin(), m_data.end(), value) != m_data.end();
    }

private:
    DataType m_data;
};

class StdSet
{
public:
    StdSet (DataConstRange data):
        m_data (data.begin(), data.end())
    {
    }

    bool Find (ValueType value) const noexcept
    {
        return m_data.find (value) != m_data.end();
    }

private:
    const std::set<ValueType> m_data;
};

class StdUnorderedSet
{
public:
    StdUnorderedSet (DataConstRange data):
        m_data (data.begin(), data.end())
    {
    }

    bool Find (ValueType value) const noexcept
    {
        return m_data.find (value) != m_data.end();
    }

private:
    const std::unordered_set<ValueType> m_data;
};

class BoostIntrusiveUnorderedSet
{
public:
    BoostIntrusiveUnorderedSet (DataConstRange data):
        m_buckets (std::max (100ul, data.size()/4)),
        m_set (Set::bucket_traits (m_buckets.data(), std::size (m_buckets)))
    {
        m_storage.reserve (data.size());
        for (auto a : data)
            m_storage.emplace_back (a);

        for (auto& a : m_storage)
            m_set.insert (a);  
    }

    bool Find (ValueType value) const noexcept
    {
        return m_set.find (Data (value)) != m_set.end();
    }

private:
    struct Data : public boost::intrusive::unordered_set_base_hook<>
    {
        ValueType m_value;

        explicit Data (ValueType value) noexcept:
            m_value (value)
        {
        }

        friend bool operator== (const Data& a, const Data &b) noexcept
        {
            return a.m_value == b.m_value;
        }

        friend std::size_t hash_value (const Data& value) noexcept
        {
            return std::size_t (value.m_value);
        }
    };

    using Set = boost::intrusive::unordered_set<Data>;

    std::vector<Data>             m_storage;
    std::vector<Set::bucket_type> m_buckets;
    Set                           m_set;
};

template <class Set>
void BM_Find (benchmark::State& state) 
{
    std::mt19937 gen (42);
    std::uniform_int_distribution<ValueType> dist;

    const auto generator = [&gen, &dist]() {return dist (gen); };

    std::vector<ValueType> data (state.range (0));
    std::generate (data.begin(), data.end(), generator);
    Set set {boost::make_iterator_range (data)};

	for (auto _ : state)
	{
		const constexpr size_t iterations = 100'000;
        const auto val = generator();
		for (size_t ii = 0; ii < iterations; ++ii)
            benchmark::DoNotOptimize (set.Find (val));
	
		state.SetItemsProcessed (state.items_processed() + iterations);
	}
    state.SetComplexityN (state.range (0));
}

const constexpr size_t Step   = 2;
const constexpr size_t Start  = 16;
const constexpr size_t Finish = 1<<21;

BENCHMARK_TEMPLATE (BM_Find, SortedVector)->RangeMultiplier (Step)->Range (Start, Finish)->Complexity();
BENCHMARK_TEMPLATE (BM_Find, UnsortedVector)->RangeMultiplier (Step)->Range (Start, 65536)->Complexity();
BENCHMARK_TEMPLATE (BM_Find, StdSet)->RangeMultiplier (Step)->Range (Start, Finish)->Complexity();
BENCHMARK_TEMPLATE (BM_Find, StdUnorderedSet)->RangeMultiplier (Step)->Range (Start, Finish)->Complexity();
BENCHMARK_TEMPLATE (BM_Find, BoostIntrusiveUnorderedSet)->RangeMultiplier (Step)->Range (Start, Finish)->Complexity();

BENCHMARK_MAIN();