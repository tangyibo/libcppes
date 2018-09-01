// Copyright tang.  All rights reserved.
// https://github.com/tangyibo/libcppes
//
// Use of this source code is governed by a BSD-style license
//
// Author: tang (inrgihc@126.com)
// Data : 2018/8/2
// Location: beijing , china
/////////////////////////////////////////////////////////////
#include "HttpClient.h"
#include "curl/curl.h"
#include <algorithm>

namespace cppes {

HttpClient::HttpClient() 
:_debug(false)
,_timeout(120)
,_ca_path(NULL)
,_header()
,_user_passwd("")
,_http_status_code(0)
{
}

HttpClient::~HttpClient() 
{
    _ca_path = NULL;
    _header.clear();
    _user_passwd.clear();
}

static int OnDebug(CURL *curl, curl_infotype itype, char * pData, size_t size, void *)
{
    if (itype == CURLINFO_TEXT)
    {
        //printf("[TEXT]%s\n", pData);
    }
    else if (itype == CURLINFO_HEADER_IN)
    {
        printf("[HEADER_IN]%s\n", pData);
    }
    else if (itype == CURLINFO_HEADER_OUT)
    {
        printf("[HEADER_OUT]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_IN)
    {
        printf("[DATA_IN]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_OUT)
    {
        printf("[DATA_OUT]%s\n", pData);
    }
    return 0;
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*> ((std::string *)lpVoid);
    if (NULL == str || NULL == buffer)
    {
        return -1;
    }

    char* pData = (char*) buffer;
    str->append(pData, size * nmemb);
    
    return nmemb;
}

int HttpClient::request(const std::string &method,const std::string &endurl,const std::string &data,std::string& output,const std::string &content_type)
{
    CURLcode res=CURLE_OK;
    CURL* curl = curl_easy_init();
    if (NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    
    if (_debug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }

    //set http request url
    curl_easy_setopt(curl, CURLOPT_URL, endurl.c_str());

    //set http request headers
    struct curl_slist *headers = NULL;
    std::string type_value=std::string("Content-Type: ")+content_type;
    headers = curl_slist_append(headers,type_value.c_str());
    if (_header.size() > 0)
    {
        header_type::const_iterator it;
        for (it = _header.begin(); it != _header.end(); it++)
            headers = curl_slist_append(headers, (it->first+":"+it->second).c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    //set http request method
    if ("GET" == method) {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    } else if ("HEAD" == method) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "HEAD");
    } else if ("PUT" == method) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    } else if ("POST" == method) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    } else if ("DELETE" == method) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        if(!data.empty())
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
        }
    } else {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    }
    
    //set recv data function callback
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &output);
    
    //set no signal
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    
    //set CA file path if use https protocol
    std::string temp=endurl;
    std::transform(temp.begin(),temp.end(),temp.begin(),::tolower);
    if(temp.find("https://")!=std::string::npos) 
    {
        if (NULL == _ca_path) 
        {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
        } 
        else 
        {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
            curl_easy_setopt(curl, CURLOPT_CAINFO, _ca_path);
        }
    }
    
    //set connect and read timeout second
    if (_timeout > 0) 
    {
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, _timeout);
    }
    
    //set authority user and password if isset
    if(!_user_passwd.empty())
    {
        curl_easy_setopt(curl, CURLOPT_USERPWD, _user_passwd.c_str()); 
    }
    
    //real send request to http server
    res = curl_easy_perform(curl);
    
    //get http status code if request success
    if(CURLE_OK==res)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &_http_status_code); 
    }
    
    //release and cleanup
    if(NULL!=headers)
    {
        curl_slist_free_all(headers);
        headers=NULL;
    }

    curl_easy_cleanup(curl);
    
    return res;
}

} // end namespace