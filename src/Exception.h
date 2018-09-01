// Copyright tang.  All rights reserved.
// https://github.com/tangyibo/libcppes
//
// Use of this source code is governed by a BSD-style license
//
// Author: tang (inrgihc@126.com)
// Data : 2018/8/2
// Location: beijing , china
/////////////////////////////////////////////////////////////
#ifndef _EXCEPTION_HEADER_H_
#define _EXCEPTION_HEADER_H_
#include <string>
#include <exception>

namespace cppes {

class Exception : public std::exception
{
public:
    explicit Exception ( const char *file,const int line,const char* what );
    explicit Exception (  const char *file,const int line,const std::string& what );
    virtual ~Exception ( ) throw ( );
    virtual const char* what ( ) const throw ( );
    const char* stackTrace ( ) const throw ( );

private:
    void fillStackTrace ( const char *file,const int line ); //填充栈痕迹
    std::string demangle ( const char* symbol );

    std::string _message;
    std::string _stack;
};

#define EXCEPTION(...) throw Exception(__FILE__, __LINE__, __VA_ARGS__)

} //end namespace

#endif  // EXCEPTION_H_
