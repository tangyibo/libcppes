// Copyright tang.  All rights reserved.
// https://github.com/tangyibo/libcppes
//
// Use of this source code is governed by a BSD-style license
//
// Author: tang (inrgihc@126.com)
// Data : 2018/8/2
// Location: beijing , china
/////////////////////////////////////////////////////////////
#ifndef __HTTP_CLIENT_HEADER_H__
#define __HTTP_CLIENT_HEADER_H__
#include <string>
#include <vector>
#include "json/json.h"

#define _TEXT_PLAIN "text/plain"
#define _APPLICATION_JSON "application/json"
#define _APPLICATION_URLENCODED "application/x-www-form-urlencoded"

namespace cppes {

class HttpClient
{
public:
    typedef std::map<std::string,std::string> header_type;
    
    HttpClient ();
    virtual ~HttpClient ();

public:
    /* 
     * @brief Generic get request to http server
     * @param url, 输入参数,请求的Url地址,如:http://www.sina.com.cn
     * @param output, 输出参数,HTTP响应的body,
     * @return int,CURL状态码，成功为0
     */
    inline int get ( const std::string &url, std::string &output )
    {
        return request ( "GET", url, "", output, _APPLICATION_JSON);
    }
   
    /* 
     * @brief Generic head request to http server
     * @param url, 输入参数,请求的Url地址,如:http://www.sina.com.cn
     * @param output, 输出参数,HTTP响应的body,
     * @return int, CURL状态码，成功为0
     */
    inline int head ( const std::string &url, std::string &output )
    {
        return request ( "HEAD", url, "", output , _APPLICATION_JSON);
    }

    /* 
     * @brief Generic put request to http server
     * @param url, 输入参数,请求的Url地址,如:http://www.sina.com.cn
     * @param data, 输入参数,HTTP请求的body,通常为如下格式:
     *  (1)key1=value1&key2=value2&...;
     *  (2)JSON格式
     *  (3)XML格式
     * @param output, 输出参数,HTTP响应的body,
     * @return int, CURL状态码，成功为0
     */
    inline int put ( const std::string &url, const std::string &data, std::string &output )
    {
        return request ( "PUT", url, data, output , _APPLICATION_JSON );
    } 

    /* 
     * @brief Generic post request to http server
     * @param url, 输入参数,请求的Url地址,如:http://www.sina.com.cn
     * @param data, 输入参数,HTTP请求的body,通常为如下格式:
     *  (1)key1=value1&key2=value2&...;
     *  (2)JSON格式
     *  (3)XML格式
     * @param output, 输出参数,HTTP响应的body,
     * @return int, CURL状态码，成功为0
     */
    inline int post ( const std::string &url, const std::string &data, std::string &output )
    {
        return request ( "POST", url, data, output, _APPLICATION_JSON );
    }

    /* 
     * @brief Generic delete request to http server
     * @param url, 输入参数,请求的Url地址,如:http://www.sina.com.cn
     * @param data, 输入参数,HTTP请求的body,通常为如下格式:
     *  (1)key1=value1&key2=value2&...;
     *  (2)JSON格式
     *  (3)XML格式
     * @param output, 输出参数,HTTP响应的body,
     * @return int, CURL状态码，成功为0
     */
    inline int remove ( const std::string &url,const std::string &data, std::string &output )
    {
        return request ( "DELETE", url, data, output ,_APPLICATION_JSON );
    }

    /* 
     * @brief Generic post request to http server using url is encoded.
     * @param url, 输入参数,请求的Url地址,如:http://www.sina.com.cn
     * @param data, 输入参数,HTTP请求的body,通常为如下格式:
     *  (1)key1=value1&key2=value2&...;
     *  (2)JSON格式
     *  (3)XML格式
     * @param output, 输出参数,HTTP响应的body,
     * @return int, CURL状态码，成功为0
     */
    inline unsigned int rawpost ( const std::string &url, const std::string &data, std::string &output )
    {
        return request ( "POST", url, data, output, _APPLICATION_URLENCODED );
    }

public:
    void timeout( int seconds)                          {    _timeout=seconds;      }
    void debug ( bool use_debug )                       {    _debug=use_debug;      }
    void set_ca_path(const char *path)                  {    _ca_path=(char *)path; }
    void set_headers( const header_type &header)        {    _header=header;        }
    void set_user_passwd( const std::string &userpass)  {    _user_passwd=userpass; }
    
    const long http_status_code()                        { return _http_status_code; }
    const long http_status_code() const                  { return _http_status_code; }
    
protected:
    int request(const std::string &method,
            const std::string &endurl,
            const std::string &data,
            std::string& output,
            const std::string &content_type);

private:
    bool _debug;
    int _timeout;
    char* _ca_path;
    header_type _header;
    std::string _user_passwd;
    
    long _http_status_code;
};

}//end namespace

#endif
