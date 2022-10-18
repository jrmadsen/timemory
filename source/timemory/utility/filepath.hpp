// MIT License
//
// Copyright (c) 2020, The Regents of the University of California,
// through Lawrence Berkeley National Laboratory (subject to receipt of any
// required approvals from the U.S. Dept. of Energy).  All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

/** \file utility/filepath.hpp
 * \headerfile utility/filepath.hpp "timemory/utility/filepath.hpp"
 * Functions for converting OS filepaths
 *
 */

#pragma once

#include "timemory/log/macros.hpp"
#include "timemory/macros/os.hpp"
#include "timemory/utility/launch_process.hpp"
#include "timemory/utility/macros.hpp"
#include "timemory/utility/types.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#if defined(TIMEMORY_UNIX)
#    include <execinfo.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#elif defined(TIMEMORY_WINDOWS)
#    include <direct.h>
#    include <shlwapi.h>
#endif

#if defined(TIMEMORY_LINUX)
#    include <linux/limits.h>
#endif

#if defined(TIMEMORY_WINDOWS) && !defined(PATH_MAX)
#    if defined(MAX_PATH)
#        define PATH_MAX MAX_PATH
#    else
#        define PATH_MAX 260
#    endif
#endif

#if !defined(TIMEMORY_PATH_MAX)
#    if defined(PATH_MAX)
#        define TIMEMORY_PATH_MAX PATH_MAX
#    else
#        define TIMEMORY_PATH_MAX 4096
#    endif
#endif

namespace tim
{
namespace filepath
{
int
makedir(std::string _dir, int umask = TIMEMORY_DEFAULT_UMASK);

inline std::string&
replace(std::string& _path, char _c, const char* _v)
{
    auto _pos = std::string::npos;
    while((_pos = _path.find(_c)) != std::string::npos)
        _path.replace(_pos, 1, _v);
    return _path;
}

inline std::string&
replace(std::string& _path, const char* _c, const char* _v)
{
    auto _pos = std::string::npos;
    while((_pos = _path.find(_c)) != std::string::npos)
        _path.replace(_pos, strlen(_c), _v);
    return _path;
}

#if defined(TIMEMORY_WINDOWS)

inline std::string
os()
{
    return "\\";
}

inline std::string
inverse()
{
    return "/";
}

inline std::string
osrepr(std::string _path)
{
    // OS-dependent representation
    while(_path.find('/') != std::string::npos)
        _path.replace(_path.find('/'), 1, "\\");
    return _path;
}

#elif defined(TIMEMORY_UNIX)

inline std::string
os()
{
    return "/";
}

inline std::string
inverse()
{
    return "\\";
}

inline std::string
osrepr(std::string _path)
{
    // OS-dependent representation
    while(_path.find("\\\\") != std::string::npos)
        _path.replace(_path.find("\\\\"), 2, "/");
    while(_path.find('\\') != std::string::npos)
        _path.replace(_path.find('\\'), 1, "/");
    return _path;
}

#endif

inline std::string
canonical(std::string _path)
{
    replace(_path, '\\', "/");
    replace(_path, "//", "/");
    return _path;
}

inline int
makedir(std::string _dir, int umask)
{
#if defined(TIMEMORY_UNIX)
    while(_dir.find("\\\\") != std::string::npos)
        _dir.replace(_dir.find("\\\\"), 2, "/");
    while(_dir.find('\\') != std::string::npos)
        _dir.replace(_dir.find('\\'), 1, "/");

    if(_dir.length() == 0)
        return 0;

    int ret = mkdir(_dir.c_str(), umask);
    if(ret != 0)
    {
        int err = errno;
        if(err != EEXIST)
        {
            std::stringstream _mdir{};
            _mdir << "mkdir(" << _dir.c_str() << ", " << umask << ") returned: " << ret
                  << "\n";
            std::stringstream _sdir{};
            _sdir << "/bin/mkdir -p " << _dir;
            ret = (launch_process(_sdir.str().c_str())) ? 0 : 1;
            if(ret != 0)
            {
                std::cerr << _mdir.str() << _sdir.str() << " returned: " << ret
                          << std::endl;
            }
            return ret;
        }
    }
#elif defined(TIMEMORY_WINDOWS)
    consume_parameters(umask);
    while(_dir.find('/') != std::string::npos)
        _dir.replace(_dir.find('/'), 1, "\\");

    if(_dir.length() == 0)
        return 0;

    int ret = _mkdir(_dir.c_str());
    if(ret != 0)
    {
        int err = errno;
        if(err != EEXIST)
        {
            std::stringstream _mdir{};
            _mdir << "_mkdir(" << _dir.c_str() << ") returned: " << ret << "\n";
            std::stringstream _sdir{};
            _sdir << "mkdir " << _dir;
            ret = (launch_process(_sdir.str().c_str())) ? 0 : 1;
            if(ret != 0)
            {
                std::cerr << _mdir.str() << _sdir.str() << " returned: " << ret
                          << std::endl;
            }
            return ret;
        }
    }
#endif
    return 0;
}

//
template <typename... Args>
inline bool
open(std::ofstream& _ofs, std::string _fpath, Args&&... _args)
{
    auto _path = canonical(_fpath);
    auto _base = canonical(_fpath);
    auto _pos  = _path.find_last_of('/');
    if(_pos != std::string::npos)
    {
        _path = _path.substr(0, _pos);
        _base = _base.substr(_pos + 1);
    }
    else
    {
        _path  = {};
        _fpath = std::string{ "./" } + _base;
    }

    if(!_path.empty())
    {
        auto ret = makedir(osrepr(_path));
        if(ret != 0)
            _fpath = std::string{ "./" } + _base;
    }

    _ofs.open(osrepr(_fpath), std::forward<Args>(_args)...);
    return (_ofs && _ofs.is_open() && _ofs.good());
}

//
template <typename... Args>
inline bool
open(std::ifstream& _ofs, std::string _fpath, Args&&... _args)
{
    auto _path = canonical(_fpath);
    auto _base = canonical(_fpath);
    auto _pos  = _path.find_last_of('/');
    if(_pos != std::string::npos)
    {
        _path = _path.substr(0, _pos);
        _base = _base.substr(_pos + 1);
    }
    else
    {
        _path  = {};
        _fpath = std::string{ "./" } + _base;
    }

    _ofs.open(osrepr(_fpath), std::forward<Args>(_args)...);
    return (_ofs && _ofs.is_open() && _ofs.good());
}

//
template <typename... Args>
inline auto*
fopen(std::string _fpath, const char* _mode)
{
    auto _path = canonical(_fpath);
    auto _base = canonical(_fpath);
    auto _pos  = _path.find_last_of('/');
    if(_pos != std::string::npos)
    {
        _path = _path.substr(0, _pos);
        _base = _base.substr(_pos + 1);
    }
    else
    {
        _path  = {};
        _fpath = std::string{ "./" } + _base;
    }

    if(!_path.empty())
    {
        auto ret = makedir(osrepr(_path));
        if(ret != 0)
            _fpath = std::string{ "./" } + _base;
    }

    return ::fopen(osrepr(_fpath).c_str(), _mode);
}

//--------------------------------------------------------------------------------------//

inline bool
exists(std::string _fname)
{
    _fname = osrepr(_fname);
#if defined(TIMEMORY_UNIX)
    struct stat _buffer;
    if(stat(_fname.c_str(), &_buffer) == 0)
        return (S_ISREG(_buffer.st_mode) != 0 || S_ISLNK(_buffer.st_mode) != 0);
    return false;
#else
    return PathFileExists(_fname.c_str());
#endif
}

//--------------------------------------------------------------------------------------//

template <size_t MaxLen = TIMEMORY_PATH_MAX>
inline std::string
realpath(const std::string& _relpath, std::string* _resolved = nullptr)
{
    auto _len = std::min<size_t>(_relpath.length(), TIMEMORY_PATH_MAX);

    char _buffer[TIMEMORY_PATH_MAX];
    memset(_buffer, '\0', sizeof(_buffer) * sizeof(char));
    strncpy(_buffer, _relpath.data(), _len);

#if defined(TIMEMORY_UNIX)
    if(::realpath(_relpath.c_str(), _buffer) == nullptr)
    {
        auto _err = errno;
        TIMEMORY_PRINTF_WARNING(stderr, "realpath failed for '%s' :: %s\n", _relpath,
                                strerror(_err));
    }
#else
    if(_fullpath(_buffer, _relpath.c_str(), _len) == nullptr)
    {
        TIMEMORY_PRINTF_WARNING(stderr, "fullpath failed for '%s'\n");
    }
#endif

    if(_resolved)
    {
        _resolved->clear();
        _len = strnlen(_buffer, TIMEMORY_PATH_MAX);
        _resolved->resize(_len);
        for(size_t i = 0; i < _len; ++i)
            (*_resolved)[i] = _buffer[i];
    }

    return (_resolved) ? *_resolved : std::string{ _buffer };
}
}  // namespace filepath
}  // namespace tim

//--------------------------------------------------------------------------------------//
