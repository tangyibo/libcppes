// Copyright tang.  All rights reserved.
// https://github.com/tangyibo/libcppes
//
// Use of this source code is governed by a BSD-style license
//
// Author: tang (inrgihc@126.com)
// Data : 2018/8/2
// Location: beijing , china
/////////////////////////////////////////////////////////////
#include "Exception.h"
#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

namespace cppes {
    
using namespace std;

Exception::Exception( const char *file,const int line,const char* msg)
: _message(msg)
{
    fillStackTrace(file,line);
}

Exception::Exception( const char *file,const int line,const string& msg)
: _message(msg)
{
    fillStackTrace(file,line);
}

Exception::~Exception() throw ()
{
    _message.clear();
    _stack.clear();
}

const char* Exception::what() const throw ()
{
    return _message.c_str();
}

const char* Exception::stackTrace() const throw ()
{
    return _stack.c_str();
}

void Exception::fillStackTrace( const char *file,const int line)
{
    const int len = 200;
    void* buffer[len];
    int nptrs = ::backtrace(buffer, len); 
    char** strings = ::backtrace_symbols(buffer, nptrs);

    std::stringstream stream;
    stream << "Exception in file[" << file << ":" << line << ":\n";
    _stack.append(stream.str());    

    if (strings)
    {
	    for (int i = 0; i < nptrs; ++i)
	    {
		    std::stringstream ss;
		    ss << "\n >>>[" << i << "] ";
		    _stack.append(ss.str());
		    _stack.append(demangle(strings[i]));
	    }

	    free(strings);
    }
}

string Exception::demangle(const char* symbol)
{
    size_t size;
    int status;
    char temp[128];
    char* demangled;
    if (1 == sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp))
    {
        if (NULL != (demangled = abi::__cxa_demangle(temp, NULL, &size, &status)))
        {
            string result(demangled);
            free(demangled);
            return result;
        }
    }
    if (1 == sscanf(symbol, "%127s", temp))
    {
        return temp;
    }

    return symbol;
}

} // end namespace