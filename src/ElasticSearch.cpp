// Copyright tang.  All rights reserved.
// https://github.com/tangyibo/libcppes
//
// Use of this source code is governed by a BSD-style license
//
// Author: tang (inrgihc@126.com)
// Data : 2018/8/2
// Location: beijing , china
/////////////////////////////////////////////////////////////
#include "ElasticSearch.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cassert>
#include <locale>
#include <vector>
#include <stdio.h>

namespace cppes {

ElasticSearch::ElasticSearch(const std::string& node, bool readOnly, bool debug)
: _url_prefix(node)
, _http()
, _readOnly(readOnly)
, _debug(debug)
{
    if (_url_prefix[_url_prefix.length() - 1] == '/')
        _url_prefix = _url_prefix.substr(0, _url_prefix.length() - 1);

    if (!isActive())
        EXCEPTION("Cannot connect Elasticsearch Node, database is not active.");
}

ElasticSearch::~ElasticSearch()
{
}

// Test connection with node.

bool ElasticSearch::isActive()
{
    std::string output;
    if (0 != _http.get(_url_prefix, output))
        return false;

    Json::Value msg;
    if (Json::Reader().parse(output, msg) && !msg.empty())
        return true;

    if (_http.http_status_code() == 200)
        return true;

    return false;
}

// Request the document by index/type/id.

bool ElasticSearch::getDocument(const char* index, const char* type, const char* id, Json::Value& msg)
{
    std::ostringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/" << id;

    std::string output;
    int ret = _http.get(oss.str(), output);
    if (0 != ret)
        return false;

    if (!Json::Reader().parse(output, msg) || msg.empty())
        EXCEPTION(output);

    if( msg.isMember("found") && msg["found"].asBool())
        return true;
    
    EXCEPTION(output);
    return false;
}

// Request the document by index/type/ query key:value.

bool ElasticSearch::getDocument(const std::string& index, const std::string& type, const std::string& key, const std::string& value, Json::Value& msg)
{
    /*
     * 查询到的结果与ElasticSearch设置的field属性有关
     */
    std::stringstream query;
    query << "{\"query\":{\"match\":{\"" << key << "\":\"" << value << "\"}}}";

    return search(index, type, query.str(), msg);
}

/// Delete the document by index/type/id.

bool ElasticSearch::deleteDocument(const char* index, const char* type, const char* id)
{
    if (_readOnly)
        return false;

    std::ostringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/" << id;
    Json::Value msg;

    std::string output;
    int ret = _http.remove(oss.str(), "", output);
    if (0 != ret)
        return false;

    if (!Json::Reader().parse(output, msg) || msg.empty())
        EXCEPTION(output);

    if (msg.isMember("found") && msg["found"].asBool())
        return true;

    if (msg.isMember("result") && msg["result"].isString() && msg["result"].asString() == "deleted")
        return true;

    if (_debug)
    {
        std::cout << "[Request]:(GET)" << oss.str() << std::endl;
        std::cout << "[Response]:" << output << std::endl;
        
        EXCEPTION(output);
    }

    return false;
}

/// Delete the document by index/type.

bool ElasticSearch::deleteAll(const char* index, const char* type)
{
    if (_readOnly)
        return false;

    /*
     * ElasticSearch在2.0 以上的不支持批量删除
     * 数据了，必须安装插件 delete-by-query才能
     * 高效的批量删除.
     */

    std::ostringstream oss, data;
    oss << _url_prefix << "/" << index << "/" << type << "/_query";
    data << "{\"query\":{\"match_all\": {}}}";

    Json::Value msg;
    std::string output;
    int ret = _http.remove(oss.str(), data.str().c_str(), output);
    if (0 != ret)
        return false;

    if (!Json::Reader().parse(output, msg) || msg.empty())
        EXCEPTION(output);

    if(msg.isMember("found") && msg["found"].asBool())
        return true;
    
    EXCEPTION(output);
    return false;
}

// Request the document number of type T in index I.

int ElasticSearch::getDocumentCount(const char* index, const char* type)
{
    std::ostringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/_count";

    Json::Value msg;
    std::string output;
    int ret = _http.get(oss.str(), output);
    if (0 != ret)
        return 0;

    if (!Json::Reader().parse(output, msg) || msg.empty())
        EXCEPTION(output);

    size_t count = 0;
    if (msg.isMember("count") && msg["count"].isInt())
        count = msg["count"].asInt();

    if (0 == count && _debug)
    {
        std::cout << "[Request]:(GET)" << oss.str() << std::endl;
        std::cout << "[Response]:" << output << std::endl;
    }

    return count;
}

// Test if document exists

bool ElasticSearch::exist(const std::string& index, const std::string& type, const std::string& id)
{
    std::stringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/" << id;

    Json::Value result;
    std::string output;
    int ret = _http.get(oss.str(), output);
    if (0 != ret)
        return false;

    if (!Json::Reader().parse(output, result) || result.empty())
        EXCEPTION(output);

    if (!result.isMember("found"))
    {
        if (_debug)
        {
            std::cout << "[Request]:(GET)" << oss.str() << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION("Database exception, field \"found\" must exist.");
    }

    if(result["found"].asBool())
        return true;
    
    //EXCEPTION(output);
    return false;
}

/// Index a document.

bool ElasticSearch::index(const std::string& index, const std::string& type, const std::string& id, const Json::Value& jData)
{
    if (_readOnly)
        return false;

    std::stringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/" << id;

    std::string data = Json::FastWriter().write(jData);

    std::string output;
    int ret = _http.put(oss.str(), data.c_str(), output);
    if (0 != ret)
        return false;

    Json::Value result;
    if (!Json::Reader().parse(output, result) || result.empty())
        EXCEPTION(output);

    if (result.isMember("reason"))
        EXCEPTION(result["reason"].asString());

    if (result.isMember("error") && result["error"].isString())
        EXCEPTION(result["error"].asString());

    if (result.isMember("_version") || result.isMember("created"))
        return true;

    if (_debug)
    {
        std::cout << "[Request]:(PUT)" << oss.str() << std::endl;
        std::cout << "[Data]" << Json::FastWriter().write(jData) << std::endl;
        std::cout << "[Response]:" << output << std::endl;

        EXCEPTION("The index returns ok: false.");
    }

    return false;
}

/// Index a document with automatic id creation

std::string ElasticSearch::index(const std::string& index, const std::string& type, const Json::Value& jData)
{
    if (_readOnly)
        return "";

    std::stringstream oss;
    oss << _url_prefix << "/" << index << "/" << type;

    std::string data = Json::FastWriter().write(jData);

    Json::Value result;
    std::string output;
    int ret = _http.post(oss.str(), data, output);
    if (0 != ret)
        return "";

    if (!Json::Reader().parse(output, result) || result.empty())
        EXCEPTION(output);

    if (result.isMember("reason"))
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << Json::FastWriter().write(jData) << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION(result["reason"].asString());
    }

    if (result.isMember("error") && result["error"].isString())
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << Json::FastWriter().write(jData) << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION(result["error"].asString());
    }

    if (!result.isMember("_id") || !result["_id"].isString())
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << Json::FastWriter().write(jData) << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION("The index induces error.");
    }

    return result["_id"].asString();
}

// Update a document field.

bool ElasticSearch::update(const std::string& index, const std::string& type, const std::string& id, const std::string& key, const std::string& value)
{
    if (_readOnly)
        return false;

    std::stringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/" << id << "/_update";

    std::stringstream data;
    data << "{\"doc\":{\"" << key << "\":\"" << value << "\"}}";

    Json::Value result;
    std::string output;
    int ret = _http.post(oss.str(), data.str().c_str(), output);
    if (0 != ret)
        return false;

    if (!Json::Reader().parse(output, result) || result.empty())
        EXCEPTION(output);

    if (!result.isMember("_version"))
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << data.str() << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION("The update failed.");
    }

    return true;
}

// Update doccument fields.

bool ElasticSearch::update(const std::string& index, const std::string& type, const std::string& id, const Json::Value& jData)
{
    if (_readOnly)
        return false;

    std::stringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/" << id << "/_update";

    std::stringstream data;
    data << "{\"doc\":" << Json::FastWriter().write(jData) << "}";

    Json::Value result;
    std::string output;
    int ret = _http.post(oss.str(), data.str().c_str(), output);
    if (0 != ret)
        return false;

    if (!Json::Reader().parse(output, result) || result.empty())
        EXCEPTION(output);

    if (result.isMember("error"))
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << data.str() << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION("The update doccument fields failed.");
    }

    return true;
}

// Update or insert if the document does not already exists.

bool ElasticSearch::upsert(const std::string& index, const std::string& type, const std::string& id, const Json::Value& jData)
{
    if (_readOnly)
        return false;

    std::stringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/" << id << "/_update";

    std::stringstream data;
    data << "{\"doc\":" << Json::FastWriter().write(jData);
    data << ", \"doc_as_upsert\" : true}";

    Json::Value result;
    std::string output;
    int ret = _http.post(oss.str(), data.str().c_str(), output);
    if (0 != ret)
        return false;

    if (!Json::Reader().parse(output, result) || result.empty())
        EXCEPTION(output);

    if (result.isMember("error"))
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << data.str() << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION("The update doccument fields failed.");
    }

    return true;
}

/// Search API of ES.

int ElasticSearch::search(const std::string& index, const std::string& type, const std::string& query, Json::Value& result)
{
    std::stringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/_search";

    std::string output;
    int ret = _http.post(oss.str(), query.c_str(), output);
    if (0 != ret)
        return 0;

    if (!Json::Reader().parse(output, result) || result.empty())
        EXCEPTION(output);

    if (!result.isMember("timed_out"))
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << query << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION("Search failed.");
    }

    if (result["timed_out"].asBool())
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << query << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION("Search timed out.");
    }

    if (!result.isMember("hits") || !result["hits"].isMember("hits"))
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << query << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION("Search reuslt wrong format.");
    }

    return int(result["hits"]["hits"].size());
}

// Test if index exists

bool ElasticSearch::existIndex(const std::string& index, Json::Value& result)
{
    std::ostringstream oss;
    oss << _url_prefix << "/" << index;

    std::string output;
    int ret = _http.get(oss.str(), output);
    if (0 != ret)
        return false;

    if (!Json::Reader().parse(output, result) || result.empty())
    {
        if (_debug)
        {
            std::cout << "[Request]:(HEAD)" << oss.str() << std::endl;
            std::cout << "[Response]:" << output << std::endl;

            EXCEPTION(output);
        }
    }

    if (200 == _http.http_status_code())
        return true;
    
    EXCEPTION(output);
    return false;
}

// Create index, optionally with data (settings, mappings etc)

bool ElasticSearch::createIndex(const std::string& index, const char* data)
{
    std::ostringstream oss;
    oss << _url_prefix << "/" << index;

    std::string output;
    int ret = _http.put(oss.str(), std::string(data), output);
    if (0 != ret)
        return false;

    Json::Value result;
    if (!Json::Reader().parse(output, result) || result.empty())
        EXCEPTION(output);

    if( 200 == _http.http_status_code())
        return true;
    
    EXCEPTION(output);
    return false;
}

// Delete given index (and all types, documents, mappings)

bool ElasticSearch::deleteIndex(const std::string& index)
{
    std::ostringstream oss;
    oss << _url_prefix << "/" << index;

    std::string output;
    int ret = _http.remove(oss.str(), "", output);
    if (0 != ret)
        return false;

    Json::Value result;
    if (!Json::Reader().parse(output, result) || result.empty())
        EXCEPTION(output);

    if( 200 == _http.http_status_code())
        return true;
    
    EXCEPTION(output);
    return false;
}

// Refresh the index.

void ElasticSearch::refresh(const std::string& index)
{
    std::ostringstream oss;
    oss << _url_prefix << "/" << index << "/_refresh";

    Json::Value msg;
    std::string output;
    _http.get(oss.str(), output);
}

bool ElasticSearch::initScroll(std::string& scrollId, const std::string& index, const std::string& type, const std::string& query, int scrollSize)
{
    std::ostringstream oss;
    oss << _url_prefix << "/" << index << "/" << type << "/_search?scroll=1m&search_type=scan&size=" << scrollSize;

    Json::Value msg;
    std::string output;
    if (0 != _http.post(oss.str(), query, output))
        return false;

    if (!Json::Reader().parse(output, msg))
        EXCEPTION(output);

    if (msg.isMember("error") && msg["error"].isString())
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << query << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION(msg["error"].asString());
    }

    if (msg.isMember("error") && msg["error"].isObject() && msg["error"].isMember("reason") && msg["error"]["reason"].isString())
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << query << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION(msg["error"]["reason"].asString());
    }

    if (msg.isMember("_scroll_id") && msg["_scroll_id"].isString())
    {
        scrollId = msg["_scroll_id"].asString();
    }
    else
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << query << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION("scrool response json no filed [_scroll_id]!");
    }

    return true;
}

bool ElasticSearch::scrollNext(std::string& scrollId, Json::Value& resultArray)
{
    std::ostringstream oss;
    oss << _url_prefix << "/_search/scroll?scroll=1m";

    std::string output;
    if (0 != _http.post(oss.str(), scrollId.c_str(), output))
        return false;

    Json::Value msg;
    if (!Json::Reader().parse(output, msg))
        EXCEPTION(output);

    if (msg.isMember("error") && msg["error"].isString())
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << scrollId << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION(msg["error"].asString());
    }

    if (msg.isMember("error") && msg["error"].isObject() && msg["error"].isMember("reason") && msg["error"]["reason"].isString())
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]" << scrollId << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION(msg["error"]["reason"].asString());
    }


    if (msg.isMember("_scroll_id") && msg["_scroll_id"].isString())
        scrollId = msg["_scroll_id"].asString();
    else
        EXCEPTION("scrool response json no filed [_scroll_id]!");

    appendHitsToArray(msg, resultArray);
    return true;
}

void ElasticSearch::clearScroll(const std::string& scrollId)
{
    std::ostringstream oss;
    oss << _url_prefix << "/_search/scroll";

    std::string output;
    _http.remove(oss.str(), scrollId.c_str(), output);
}

int ElasticSearch::fullScan(const std::string& index, const std::string& type, const std::string& query, Json::Value& resultArray, int scrollSize)
{
    resultArray.clear();

    std::string scrollId;
    if (!initScroll(scrollId, index, type, query, scrollSize))
        return 0;

    size_t currentSize = 0, newSize;
    while (scrollNext(scrollId, resultArray))
    {
        newSize = resultArray.size();
        if (currentSize == newSize)
            break;

        currentSize = newSize;
    }
    
    clearScroll(scrollId);
    
    return currentSize;
}

void ElasticSearch::appendHitsToArray(const Json::Value& msg, Json::Value& resultArray)
{
    if (!msg.isMember("hits"))
        EXCEPTION("Result corrupted, no member \"hits\".");

    if (!msg["hits"].isMember("hits"))
        EXCEPTION("Result corrupted, no member \"hits\" nested in \"hits\".");

    const Json::Value& array = msg["hits"]["hits"];
    for (size_t i = 0; i < array.size(); ++i)
    {
        resultArray.append(array[i]);
    }
}

// Bulk API of ES.

bool ElasticSearch::bulk(const char* data, Json::Value& jResult)
{
    if (_readOnly)
        return false;

    std::ostringstream oss;
    oss << _url_prefix << "/_bulk";

    Json::Value msg;
    std::string output;
    if (0 != _http.post(oss.str(), std::string(data), output))
        return false;

    if (!Json::Reader().parse(output, jResult))
    {
        if (_debug)
        {
            std::cout << "[Request]:(POST)" << oss.str() << std::endl;
            std::cout << "[Data]:" << data << std::endl;
            std::cout << "[Response]:" << output << std::endl;
        }

        EXCEPTION(output);
    }

    if( 200 == _http.http_status_code())
        return true;
    
    EXCEPTION(output);
    return false;    
}

////////////////////////////////////////////////////////////////////////////////

BulkBuilder::BulkBuilder()
{
}

void BulkBuilder::createCommand(const std::string &op, const std::string &index, const std::string &type, const std::string &id = "")
{
    Json::Value command;
    Json::Value commandParams;

    if (id != "")
    {
        commandParams["_id"] = id;
    }

    commandParams["_index"] = index;
    commandParams["_type"] = type;

    command[op] = commandParams;
    operations.push_back(command);
}

void BulkBuilder::index(const std::string &index, const std::string &type, const std::string &id, const Json::Value &fields)
{
    createCommand("index", index, type, id);
    operations.push_back(fields);
}

void BulkBuilder::create(const std::string &index, const std::string &type, const std::string &id, const Json::Value &fields)
{
    createCommand("create", index, type, id);
    operations.push_back(fields);
}

void BulkBuilder::index(const std::string &index, const std::string &type, const Json::Value &fields)
{
    createCommand("index", index, type);
    operations.push_back(fields);
}

void BulkBuilder::create(const std::string &index, const std::string &type, const Json::Value &fields)
{
    createCommand("create", index, type);
    operations.push_back(fields);
}

void BulkBuilder::update(const std::string &index, const std::string &type, const std::string &id, const Json::Value &body)
{
    createCommand("update", index, type, id);
    operations.push_back(body);
}

void BulkBuilder::upsert(const std::string &index, const std::string &type, const std::string &id, const Json::Value &fields, bool upsert)
{
    createCommand("update", index, type, id);

    Json::Value updateFields;
    updateFields["doc"] = fields;
    updateFields["doc_as_upsert"] = upsert;

    operations.push_back(updateFields);
}

void BulkBuilder::del(const std::string &index, const std::string &type, const std::string &id)
{
    createCommand("delete", index, type, id);
}

std::string BulkBuilder::str()
{
    std::stringstream json;

    for (size_t i = 0; i < operations.size(); ++i)
    {
        std::string item = Json::FastWriter().write(operations[i]);
        json << item;

        if (item[item.length() - 1] != '\n')
            json << std::endl;
    }

    return json.str();
}

void BulkBuilder::clear()
{
    operations.clear();
}

bool BulkBuilder::isEmpty()
{
    return operations.empty();
}

}//end namespace
