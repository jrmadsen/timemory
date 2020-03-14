//  MIT License
//
//  Copyright (c) 2020, The Regents of the University of California,
//  through Lawrence Berkeley National Laboratory (subject to receipt of any
//  required approvals from the U.S. Dept. of Energy).  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

/**
 * \headerfile "timemory/components/base/declaration.hpp"
 * \brief Declares the static polymorphic base for the components
 *
 */

#pragma once

#include "timemory/components/base/types.hpp"
#include "timemory/mpl/types.hpp"
#include "timemory/storage/declaration.hpp"
#include "timemory/utility/serializer.hpp"

//======================================================================================//

namespace tim
{
namespace component
{
//
//======================================================================================//
//
//          base component class for all components with non-void types
//
//======================================================================================//
//
template <typename Tp, typename Value>
struct base
{
    using EmptyT = std::tuple<>;
    template <typename U>
    using vector_t = std::vector<U>;

public:
    static constexpr bool has_accum_v          = trait::base_has_accum<Tp>::value;
    static constexpr bool has_last_v           = trait::base_has_last<Tp>::value;
    static constexpr bool implements_storage_v = implements_storage<Tp, Value>::value;
    static constexpr bool has_secondary_data   = trait::secondary_data<Tp>::value;
    static constexpr bool is_sampler_v         = trait::sampler<Tp>::value;
    static constexpr bool is_component_type    = false;
    static constexpr bool is_auto_type         = false;
    static constexpr bool is_component         = true;

    using Type             = Tp;
    using value_type       = Value;
    using accum_type       = conditional_t<has_accum_v, value_type, EmptyT>;
    using last_type        = conditional_t<has_last_v, value_type, EmptyT>;
    using sample_type      = conditional_t<is_sampler_v, operation::sample<Tp>, EmptyT>;
    using sample_list_type = conditional_t<is_sampler_v, vector_t<sample_type>, EmptyT>;

    using this_type         = Tp;
    using base_type         = base<Tp, Value>;
    using unit_type         = typename trait::units<Tp>::type;
    using display_unit_type = typename trait::units<Tp>::display_type;
    using storage_type      = impl::storage<Tp, implements_storage_v>;
    using graph_iterator    = typename storage_type::iterator;
    using state_t           = state<this_type>;
    using statistics_policy = policy::record_statistics<Tp, Value>;
    using fmtflags          = std::ios_base::fmtflags;

private:
    friend class impl::storage<Tp, implements_storage_v>;
    friend class storage<Tp>;

    friend struct operation::init_storage<Tp>;
    friend struct operation::construct<Tp>;
    friend struct operation::set_prefix<Tp>;
    friend struct operation::insert_node<Tp>;
    friend struct operation::pop_node<Tp>;
    friend struct operation::record<Tp>;
    friend struct operation::reset<Tp>;
    friend struct operation::measure<Tp>;
    friend struct operation::start<Tp>;
    friend struct operation::stop<Tp>;
    friend struct operation::minus<Tp>;
    friend struct operation::plus<Tp>;
    friend struct operation::multiply<Tp>;
    friend struct operation::divide<Tp>;
    friend struct operation::base_printer<Tp>;
    friend struct operation::print<Tp>;
    friend struct operation::print_storage<Tp>;
    friend struct operation::copy<Tp>;
    friend struct operation::sample<Tp>;
    friend struct operation::serialization<Tp>;
    friend struct operation::finalize::get<Tp, true>;
    friend struct operation::finalize::get<Tp, false>;

    template <typename Ret, typename Lhs, typename Rhs>
    friend struct operation::compose;

    static_assert(std::is_pointer<Tp>::value == false, "Error pointer base type");

public:
    base();
    ~base() = default;

    explicit base(const base_type&) = default;
    explicit base(base_type&&)      = default;

    base& operator=(const base_type&) = default;
    base& operator=(base_type&&) = default;

public:
    static void global_init(storage_type*) {}
    static void thread_init(storage_type*) {}
    static void global_finalize(storage_type*) {}
    static void thread_finalize(storage_type*) {}
    template <typename Archive>
    static void extra_serialization(Archive&, const unsigned int)
    {}

public:
    template <typename... Args>
    static void configure(Args&&...)
    {}

    //----------------------------------------------------------------------------------//
    /// type contains secondary data resembling the original data
    /// but should be another node entry in the graph. These types
    /// must provide a get_secondary() member function and that member function
    /// must return a pair-wise iterable container, e.g. std::map, of types:
    ///     - std::string
    ///     - value_type
    ///
    template <typename T = Type>
    static void append(graph_iterator itr, const T& rhs);

public:
    void reset();     /// reset the values
    void measure();   /// just record a measurment
    void sample() {}  /// sample a measurement
    void start();     /// start measurement
    void stop();      /// stop measurement

    // void mark_begin() {}  // mark a begining point in the execution
    // void mark_end() {}    // mark a ending point in the execution
    // void store() {}       // store a value

    void set_started();  // store that start has been called
    void set_stopped();  // store that stop has been called

    void get(void*& ptr, size_t typeid_hash) const;    /// assign type to a pointer
    auto get() const { return this->load(); }          /// default get routine
    auto get_display() const { return this->load(); }  /// default display routine
    void print(std::ostream&) const;

    bool operator<(const base_type& rhs) const { return (load() < rhs.load()); }
    bool operator>(const base_type& rhs) const { return (load() > rhs.load()); }
    bool operator<=(const base_type& rhs) const { return !(*this > rhs); }
    bool operator>=(const base_type& rhs) const { return !(*this < rhs); }

    Type& operator+=(const base_type& rhs) { return plus_oper(rhs); }
    Type& operator-=(const base_type& rhs) { return minus_oper(rhs); }
    Type& operator*=(const base_type& rhs) { return multiply_oper(rhs); }
    Type& operator/=(const base_type& rhs) { return divide_oper(rhs); }

    Type& operator+=(const Type& rhs) { return plus_oper(rhs); }
    Type& operator-=(const Type& rhs) { return minus_oper(rhs); }
    Type& operator*=(const Type& rhs) { return multiply_oper(rhs); }
    Type& operator/=(const Type& rhs) { return divide_oper(rhs); }

    Type& operator+=(const Value& rhs) { return plus_oper(rhs); }
    Type& operator-=(const Value& rhs) { return minus_oper(rhs); }
    Type& operator*=(const Value& rhs) { return multiply_oper(rhs); }
    Type& operator/=(const Value& rhs) { return divide_oper(rhs); }

    friend std::ostream& operator<<(std::ostream& os, const base_type& obj)
    {
        obj.print(os);
        return os;
    }

    template <typename Up = Tp, enable_if_t<(Up::is_sampler_v), int> = 0>
    void add_sample(sample_type&&);  /// add a sample

    // serialization load (input)
    template <typename Archive, typename Up = Type,
              enable_if_t<!(trait::custom_serialization<Up>::value), int> = 0>
    void CEREAL_LOAD_FUNCTION_NAME(Archive& ar, const unsigned int);

    // serialization store (output)
    template <typename Archive, typename Up = Type,
              enable_if_t<!(trait::custom_serialization<Up>::value), int> = 0>
    void CEREAL_SAVE_FUNCTION_NAME(Archive& ar, const unsigned int version) const;

    int64_t           nlaps() const { return laps; }
    int64_t           get_laps() const { return laps; }
    const value_type& get_value() const { return value; }
    const accum_type& get_accum() const { return accum; }
    const last_type&  get_last() const { return last; }
    const bool&       get_is_transient() const { return is_transient; }
    sample_list_type  get_samples() const { return samples; }

protected:
    static storage_type*& get_storage();
    static void           cleanup() {}

    template <typename Up = Tp, enable_if_t<(Up::has_accum_v), int> = 0>
    const value_type& load() const;

    template <typename Up = Tp, enable_if_t<!(Up::has_accum_v), int> = 0>
    const value_type& load() const;

    Type& plus_oper(const base_type& rhs);
    Type& minus_oper(const base_type& rhs);
    Type& multiply_oper(const base_type& rhs);
    Type& divide_oper(const base_type& rhs);

    Type& plus_oper(const Type& rhs);
    Type& minus_oper(const Type& rhs);
    Type& multiply_oper(const Type& rhs);
    Type& divide_oper(const Type& rhs);

    Type& plus_oper(const Value& rhs);
    Type& minus_oper(const Value& rhs);
    Type& multiply_oper(const Value& rhs);
    Type& divide_oper(const Value& rhs);

    void plus(const base_type& rhs)
    {
        laps += rhs.laps;
        if(rhs.is_transient)
            is_transient = rhs.is_transient;
    }

    void minus(const base_type& rhs)
    {
        laps -= rhs.laps;
        if(rhs.is_transient)
            is_transient = rhs.is_transient;
    }

protected:
    //----------------------------------------------------------------------------------//
    // insert the node into the graph
    //
    template <typename Scope, typename Up = base_type,
              enable_if_t<(Up::implements_storage_v), int>                = 0,
              enable_if_t<(std::is_same<Scope, scope::tree>::value), int> = 0>
    void insert_node(Scope&&, const int64_t& _hash);

    template <typename Scope, typename Up = base_type,
              enable_if_t<(Up::implements_storage_v), int>                 = 0,
              enable_if_t<!(std::is_same<Scope, scope::tree>::value), int> = 0>
    void insert_node(Scope&&, const int64_t& _hash);

    template <typename Scope, typename Up = base_type,
              enable_if_t<!(Up::implements_storage_v), int> = 0>
    void insert_node(Scope&&, const int64_t&);

    // pop the node off the graph
    template <typename Up = base_type, enable_if_t<(Up::implements_storage_v), int> = 0>
    void pop_node();

    template <typename Up = base_type, enable_if_t<!(Up::implements_storage_v), int> = 0>
    void pop_node();

    // initialize the storage
    template <typename Up = Tp, typename Vp = Value,
              enable_if_t<(implements_storage<Up, Vp>::value), int> = 0>
    static bool init_storage(storage_type*& _instance);

    template <typename Up = Tp, typename Vp = Value,
              enable_if_t<!(implements_storage<Up, Vp>::value), int> = 0>
    static bool init_storage(storage_type*&);

    static Type dummy();  // create an instance

protected:
    bool             is_running   = false;
    bool             is_on_stack  = false;
    bool             is_transient = false;
    bool             is_flat      = false;
    bool             depth_change = false;
    int64_t          laps         = 0;
    value_type       value        = value_type{};
    accum_type       accum        = accum_type{};
    last_type        last         = last_type{};
    sample_list_type samples      = sample_type{};
    graph_iterator   graph_itr    = graph_iterator{ nullptr };

public:
    static constexpr bool timing_category_v = trait::is_timing_category<Type>::value;
    static constexpr bool memory_category_v = trait::is_memory_category<Type>::value;
    static constexpr bool timing_units_v    = trait::uses_timing_units<Type>::value;
    static constexpr bool memory_units_v    = trait::uses_memory_units<Type>::value;
    static constexpr bool percent_units_v   = trait::uses_percent_units<Type>::value;
    static constexpr auto ios_fixed         = std::ios_base::fixed;
    static constexpr auto ios_decimal       = std::ios_base::dec;
    static constexpr auto ios_showpoint     = std::ios_base::showpoint;
    static const short    precision         = (percent_units_v) ? 1 : 3;
    static const short    width             = (percent_units_v) ? 6 : 8;
    static const fmtflags format_flags      = ios_fixed | ios_decimal | ios_showpoint;

    template <typename Up = Type, typename _Unit = typename trait::units<Up>::type,
              enable_if_t<(std::is_same<_Unit, int64_t>::value), int> = 0>
    static int64_t unit();

    template <typename Up = Type, typename _Unit = typename Up::display_unit_type,
              enable_if_t<(std::is_same<_Unit, std::string>::value), int> = 0>
    static std::string display_unit();

    template <typename Up = Type, typename _Unit = typename trait::units<Up>::type,
              enable_if_t<(std::is_same<_Unit, int64_t>::value), int> = 0>
    static int64_t get_unit();

    template <typename Up = Type, typename _Unit = typename Up::display_unit_type,
              enable_if_t<(std::is_same<_Unit, std::string>::value), int> = 0>

    static std::string             get_display_unit();
    static short                   get_width();
    static short                   get_precision();
    static std::ios_base::fmtflags get_format_flags();
    static std::string             label();
    static std::string             description();
    static std::string             get_label();
    static std::string             get_description();
};
//
//======================================================================================//
//
//          base component class for all components with void types
//
//======================================================================================//
//
template <typename Tp>
struct base<Tp, void>
{
    using EmptyT = std::tuple<>;

public:
    static constexpr bool implements_storage_v = false;
    static constexpr bool has_secondary_data   = false;
    static constexpr bool is_sampler_v         = trait::sampler<Tp>::value;
    static constexpr bool is_component_type    = false;
    static constexpr bool is_auto_type         = false;
    static constexpr bool is_component         = true;

    using Type             = Tp;
    using value_type       = void;
    using sample_type      = EmptyT;
    using sample_list_type = EmptyT;

    using this_type    = Tp;
    using base_type    = base<Tp, value_type>;
    using storage_type = impl::storage<Tp, implements_storage_v>;

private:
    friend class impl::storage<Tp, implements_storage_v>;

    friend struct operation::init_storage<Tp>;
    friend struct operation::construct<Tp>;
    friend struct operation::set_prefix<Tp>;
    friend struct operation::insert_node<Tp>;
    friend struct operation::pop_node<Tp>;
    friend struct operation::record<Tp>;
    friend struct operation::reset<Tp>;
    friend struct operation::measure<Tp>;
    friend struct operation::start<Tp>;
    friend struct operation::stop<Tp>;
    friend struct operation::minus<Tp>;
    friend struct operation::plus<Tp>;
    friend struct operation::multiply<Tp>;
    friend struct operation::divide<Tp>;
    friend struct operation::print<Tp>;
    friend struct operation::print_storage<Tp>;
    friend struct operation::copy<Tp>;
    friend struct operation::serialization<Tp>;

    template <typename Ret, typename Lhs, typename Rhs>
    friend struct operation::compose;

public:
    base();
    ~base()                         = default;
    explicit base(const base_type&) = default;
    explicit base(base_type&&)      = default;
    base& operator=(const base_type&) = default;
    base& operator=(base_type&&) = default;

public:
    static void global_init(storage_type*) {}
    static void thread_init(storage_type*) {}
    static void global_finalize(storage_type*) {}
    static void thread_finalize(storage_type*) {}
    template <typename Archive>
    static void extra_serialization(Archive&, const unsigned int)
    {}

public:
    template <typename... Args>
    static void configure(Args&&...)
    {}

    template <typename GraphItr>
    static void append(GraphItr, const Type&)
    {}

public:
    void reset();     // reset the values
    void measure();   // just record a measurment
    void sample() {}  // perform a sample
    void start();
    void stop();

    void mark_begin() {}
    void mark_end() {}
    void set_started();
    void set_stopped();

    friend std::ostream& operator<<(std::ostream& os, const base_type&) { return os; }

    void get() {}
    void get(void*& ptr, size_t typeid_hash) const;

    int64_t nlaps() const { return 0; }
    int64_t get_laps() const { return 0; }

private:
    template <typename Scope = scope::tree, typename... Args>
    void insert_node(Scope&&, Args&&...);

    void pop_node();

protected:
    static void cleanup() {}

    void plus(const base_type& rhs)
    {
        if(rhs.is_transient)
            is_transient = rhs.is_transient;
    }

    void minus(const base_type& rhs)
    {
        if(rhs.is_transient)
            is_transient = rhs.is_transient;
    }

protected:
    bool is_running   = false;
    bool is_on_stack  = false;
    bool is_transient = false;

public:
    //
    // components with void data types do not use label()/get_label()
    // to generate an output filename so provide a default one from
    // (potentially demangled) typeid(Type).name() and strip out
    // namespace and any template parameters + replace any spaces
    // with underscores
    //
    static std::string label();
    static std::string description();
    static std::string get_label();
    static std::string get_description();
};
//
//----------------------------------------------------------------------------------//
//
}  // namespace component
//
//----------------------------------------------------------------------------------//
//
}  // namespace tim
