// Copyright tang.  All rights reserved.
// https://github.com/tangyibo/libcppes
//
// Use of this source code is governed by a BSD-style license
//
// Author: tang (inrgihc@126.com)
// Data : 2018/8/2
// Location: beijing , china
/////////////////////////////////////////////////////////////
#ifndef _ELASTICSEARCH_HEADER_H_
#define _ELASTICSEARCH_HEADER_H_
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <iostream>
#include "Exception.h"
#include "HttpClient.h"
#include "json/json.h"

namespace cppes {
    
/*
 * @brief: API class for elastic search server.
 * @Node: Instance of elastic search on server represented by:
 *    http://url:port
 */
class ElasticSearch
{
public:
    ElasticSearch ( const std::string& node, bool readOnly = false,bool debug=false );
    ~ElasticSearch ( );

    /*
     * @brief:Test connection with node.
     * @return: true if server is active to connect, other false
     */
    bool isActive();

    /*
     * @brief:Request document number of type T in index I.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @return: int , match document count
     */
    int getDocumentCount ( const char* index, const char* type );

    /*
     * @brief:Request the document by index/type/id.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @param: id, [in], string , id of document
     * @param: msg, [out], Json::Value , content of document
     * @return: true if find, other false
     */
    bool getDocument ( const char* index, const char* type, const char* id, Json::Value& msg );

    /*
     * @brief:Request the document by index/type/ query key:value.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @param: key, [in], string , keyword string
     * @param: value, [in], string , value string
     * @param: msg, [out], Json::Value , content of document
     * @return: true if find, other false
     */
    bool getDocument ( const std::string& index, const std::string& type, const std::string& key, const std::string& value, Json::Value& msg );

    /*
     * @brief: Delete the document by index/type/id.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @param: id, [in], string , id string
     * @return: true if delete success, other false
     */
    bool deleteDocument ( const char* index, const char* type, const char* id );

     /*
     * @brief: Delete the document by index/type.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @return: true if delete success, other false
     * @comment:  ElasticSearch在2.0 以上的不支持批量删除,
      *  如果需要必须安装插件 delete-by-query才可以
     */
    bool deleteAll ( const char* index, const char* type );

     /*
     * @brief:  Test if document exists
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @return: true if exist, other false
     */
    bool exist ( const std::string& index, const std::string& type, const std::string& id );

    /*
     * @brief: Index a document.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @param: id, [in], string , id of document
     * @param: jData, [in], Json::Value , content of document
     * @return: true if created, other false
     */
    bool index ( const std::string& index, const std::string& type, const std::string& id, const Json::Value& jData );

    /*
     * @brief: Index a document with automatic id creation
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @param: jData, [in], Json::Value , content of document
     * @return: id string if create success, other empty string
     */
    std::string index ( const std::string& index, const std::string& type, const Json::Value& jData );

    /*
     * @brief: Update a document field.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @param: id, [in], string , id of document
     * @param: key, [in], string , keyword string
     * @param: value, [in], string , value string
     * @return: true if update success, other false
     */
    bool update ( const std::string& index, const std::string& type, const std::string& id, const std::string& key, const std::string& value );

    /*
     * @brief: Update doccument fields.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @param: id, [in], string , id of document
     * @param: jData, [in], Json::Value , content of document
     * @return: true if update success, other false
     */
    bool update ( const std::string& index, const std::string& type, const std::string& id, const Json::Value& jData );

    /*
     * @brief: Update or insert if the document does not already exists.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @param: id, [in], string , id of document
     * @param: jData, [in], Json::Value , content of document
     * @return: true if update success, other false
     */
    bool upsert ( const std::string& index, const std::string& type, const std::string& id, const Json::Value& jData );

    /*
     * @brief: Search API of ElasticSearch. Specify the doc type.
     * @param: index, [in], string , index of document
     * @param: type, [in], string , type of document
     * @param: query, [in], string , query string using json format
     * @param: result, [out], Json::Value , result list of document
     * @return: result count which find .
     */
    int search ( const std::string& index, const std::string& type, const std::string& query, Json::Value& result );

    /*
     * @brief: Bulk API
     * @param: data, [in], string , content of data
     * @param: jResult, [out], Json::Value , result
     * @return: true if success, other false
     */
    bool bulk ( const char* data, Json::Value& jResult );

public:

    /*
     * @brief: Test if index exists
     * @param: index, [in], string , index of document
     * @param: result, [out], Json::Value , description of index using json format
     * @return: true if success, other false
     */
    bool existIndex ( const std::string& index, Json::Value& result );

    /*
     * @brief: Create index, optionally with data (settings, mappings etc)
     * @param: index, [in], string , index of document
     * @param: data, [in], string ,  settings, mappings for index which must in json format
     * @return: true if success, other false
     */
    bool createIndex ( const std::string& index, const char* data = NULL );

    /*
     * @brief: Delete given index (and all types, documents, mappings)
     * @param: index, [in], string , index of document
     * @return: true if success, other false
     */
    bool deleteIndex ( const std::string& index );

    /*
     * @brief: Refresh the index.
     * @param: index, [in], string , index of document
     */
    void refresh ( const std::string& index );

public:
    /// Initialize a scroll search. Use the returned scroll id when calling scrollNext. Size is based on shardSize. Returns false on error
    bool initScroll ( std::string& scrollId, const std::string& index, const std::string& type, const std::string& query, int scrollSize = 1000 );

    /// Scroll to next matches of an initialized scroll search. scroll_id may be updated. End is reached when resultArray.empty() is true (in which scroll is automatically cleared). Returns false on error.
    bool scrollNext ( std::string& scrollId, Json::Value& resultArray );

    /// Clear an initialized scroll search prior to its automatically 1 minute timeout
    void clearScroll ( const std::string& scrollId );

    /// Perform a scan to get all results from a query.
    int fullScan ( const std::string& index, const std::string& type, const std::string& query, Json::Value& resultArray, int scrollSize = 1000 );

private:
    
    ///append the result source content list into array var
    void appendHitsToArray ( const Json::Value& msg, Json::Value& resultArray );

private:
    
    /// Private constructor.
    ElasticSearch ();

    /// URI withn format http://host:port
    std::string _url_prefix;
    
    /// HTTP Connexion module which using libcurl.
    HttpClient _http;

    /// Read Only option, all index functions return false.
    bool _readOnly;
    
    /// Debug semphore is using if true
    bool _debug;
};

/*
 * @brief Bulk helper for function ElasticSearch::bulk()
 */
class BulkBuilder
{
public:
    BulkBuilder ( );
    void index ( const std::string &index, const std::string &type, const std::string &id, const Json::Value &fields );
    void create ( const std::string &index, const std::string &type, const std::string &id, const Json::Value &fields );
    void index ( const std::string &index, const std::string &type, const Json::Value &fields );
    void create ( const std::string &index, const std::string &type, const Json::Value &fields );
    void update ( const std::string &index, const std::string &type, const std::string &id, const Json::Value &body );
    void upsert ( const std::string &index, const std::string &type, const std::string &id, const Json::Value &fields, bool update = false );
    void del ( const std::string &index, const std::string &type, const std::string &id );
    void clear ( );
    std::string str ( );
    bool isEmpty ( );

protected:
    void createCommand(const std::string &op, const std::string &index, const std::string &type, const std::string &id);
    
private:
    std::vector<Json::Value> operations;
};

} // end namespace
#endif // _ELASTICSEARCH_HEADER_H_
