// MIT License
//
// Copyright (c) 2018 Jonathan R. Madsen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

/** \file timer.hpp
 * Primary timer class
 * Inherits from base_timer
 */

#ifndef timer_hpp_
#define timer_hpp_

//----------------------------------------------------------------------------//

#include "timemory/namespace.hpp"
#include "timemory/base_timer.hpp"

namespace NAME_TIM
{
namespace util
{

//============================================================================//
// Main timer class
//============================================================================//

class timer : public details::base_timer
{
public:
    typedef base_timer      base_type;
    typedef timer           this_type;
    typedef std::string     string_t;
    typedef void (*clone_function_t)(const this_type&);
    typedef std::unique_ptr<this_type>  unique_ptr_type;
    typedef std::shared_ptr<this_type>  shared_ptr_type;

public:
    timer(const string_t& _begin = "[ ",
          const string_t& _close = " ]",
          bool _use_static_width = false,
          uint16_t prec = default_precision);
    timer(const string_t& _begin,
          const string_t& _close,
          const string_t& _fmt,
          bool _use_static_width = false,
          uint16_t prec = default_precision);
    virtual ~timer();

public:
    static string_t default_format;
    static uint16_t default_precision;
    static void propose_output_width(uint64_t);

public:
    timer& stop_and_return() { this->stop(); return *this; }
    string_t begin() const { return m_begin; }
    string_t close() const { return m_close; }
    std::string as_string() const
    {
        std::stringstream ss;
        this->report(ss, false, true);
        return ss.str();
    }

    void print() const
    {
        std::cout << this->as_string() << std::endl;
    }

    this_type clone() const;
    unique_ptr_type clone_to_unique_ptr() const;
    shared_ptr_type clone_to_shared_ptr() const;
    this_type* clone_to_pointer() const;

    this_type& operator+=(const this_type& rhs)
    {
        auto_lock_t l(m_mutex);
        m_accum += rhs.get_accum();
        return *this;
    }

protected:
    virtual void compose() final;
    void set_parent(this_type* parent) { m_parent = parent; }

protected:
    bool        m_use_static_width;
    this_type*  m_parent;
    string_t    m_begin;
    string_t    m_close;

private:
    static uint64_t f_output_width;

public:
    template <typename Archive> void
    serialize(Archive& ar, const unsigned int version)
    {
        details::base_timer::serialize(ar, version);
    }

};

//----------------------------------------------------------------------------//

inline timer::unique_ptr_type
timer::clone_to_unique_ptr() const
{
    return std::unique_ptr<timer>(new timer(this->clone()));
}

//----------------------------------------------------------------------------//

inline timer::shared_ptr_type
timer::clone_to_shared_ptr() const
{
    return std::shared_ptr<timer>(new timer(this->clone()));
}

//----------------------------------------------------------------------------//

inline timer*
timer::clone_to_pointer() const
{
    return new timer(this->clone());
}

//----------------------------------------------------------------------------//

} // namespace util

} // namespace NAME_TIM

//----------------------------------------------------------------------------//

#endif // timer_hpp_
