// Copyright tang.  All rights reserved.
// https://github.com/tangyibo/libcppes
//
// Use of this source code is governed by a BSD-style license
//
// Author: tang (inrgihc@126.com)
// Data : 2018/8/2
// Location: beijing , china
/////////////////////////////////////////////////////////////
#include "testlib/lut.h"
#include "ElasticSearch.h"
#include <iostream>
#include <cstdlib>

using namespace cppes;

const char *address="http://172.16.13.63:9200/";

/*
 * 根据规划，Elastic 6.x 版只允许每个 Index 包含一个 Type，7.x 版将会彻底移除 Type。
 * 本程序测试的ElasticSearch为2.3.3
 */

TEST(ElasticSearch, TEST_1) 
{
    bool ret=false;
    
    try {
        ElasticSearch es(address,false,true);
        std::cout<<"[1]connect elasticsearch:"<<address<<std::endl;

        Json::Value doc;
        doc["user"] = "kimchy";
        doc["post_date"] = "2009-11-15T14:12:12";
        doc["message"] = "trying out Elasticsearch";

        std::cout<<"[2]create index by index/type/id"<<std::endl;
        ret = es.index("twitter", "tweet", "1", doc);
        ASSERT_TRUE(ret);

        std::cout<<"[3]read document by index/type/id"<<std::endl;
        doc.clear();
        ret = es.getDocument("twitter", "tweet", "1", doc);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(!doc.empty());

        std::cout<<"[4]check exist by index/type/id"<<std::endl;
        ret = es.exist("twitter", "tweet", "1");
        ASSERT_TRUE(ret);

        std::cout<<"[5]delete document by index/type/id"<<std::endl;
        doc.clear();
        ret = es.deleteDocument("twitter", "tweet", "1");
        ASSERT_TRUE(ret);

        ///////////////////////////////////////////////////////
        
        Json::Value jData;
        jData["user"] = "auto_id";
        jData["post_date"] = "2009-11-15T14:12:14";
        jData["message"] = "sssssssssssss";
        
        std::cout<<"[6]create index by index/type without id"<<std::endl;
        std::string id = es.index("twitter", "tweet", jData);
        ASSERT_TRUE(!id.empty());
        
        std::cout<<"[7]delete document by index/type/id"<<std::endl;
        ret = es.deleteDocument("twitter", "tweet", id.c_str());
        ASSERT_TRUE(ret);

        std::cout << "[8]create index by index/type without id" << std::endl;
        std::string nid = es.index("twitter", "tweet", jData);
        ASSERT_TRUE(!nid.empty());       
        
        std::cout<<"[9]delete index"<<std::endl;
        ret=es.deleteIndex("twitter");
        ASSERT_TRUE(ret);

    } catch (Exception &e) {
        std::cout << "Failed:" << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cout << "Failed:" << e.what() << std::endl;
    }
}

TEST(ElasticSearch, TEST_2)
{
    bool ret=false;
    try {
        ElasticSearch es(address,false,true);
        std::cout<<"[1]connect elasticsearch:"<<address<<std::endl;

        Json::Value doc;
        doc["user"] = 2;
        doc["name"] ="tang";
        doc["post_date"] = "2009-11-15T14:12:14";
        doc["message"] = "sssssssssssss";

        std::cout << "[2]create index by index/type without id" << std::endl;
        std::string id = es.index("facebook", "document", doc);
        ASSERT_TRUE(!id.empty());
        std::cout<<"   Return id="<<id<<std::endl;
        
        std::cout << "[3]check index exist" << std::endl;
        Json::Value indexInfo;
        ret=es.existIndex("facebook",indexInfo);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(!indexInfo.empty());

        std::cout << "[4]update by index/type/id and key ,set value" << std::endl;
        ret = es.update("facebook", "document", id.c_str(), "message", "XXXXXXXXXXXXX");
        ASSERT_TRUE(ret);

        std::cout << "[5]update by index/type/id and set new source value" << std::endl;
        doc["testsr"] = "study elasticsearch";
        ret = es.update("facebook", "document", id.c_str(), doc);
        ASSERT_TRUE(ret);

        std::cout << "[6]upsert by index/type/id and set or insert new source value" << std::endl;
        doc["testsr"] = "study elasticsearch";
        ret = es.upsert("facebook", "document", "newid", doc);
        ASSERT_TRUE(ret);
        
        sleep(2);
        es.refresh("facebook");
        
        std::cout << "[7]search by index/type use query string" << std::endl;
        Json::Value result;
        std::string query="{\"query\":{\"match\":{\"name\":\"tang\"}}}";
        int search_count=es.search("facebook", "document",query,result);
        ASSERT_GT(search_count,0);
        ASSERT_TRUE(!result.empty());
        
        std::cout << "[8]count document by index/type" << std::endl;
        int count = es.getDocumentCount("facebook", "document");
        ASSERT_GT(count, 0);
        
        std::cout<<"[9]read document by index/type and key and value"<<std::endl;
        doc.clear();
        ret = es.getDocument("facebook", "document", "user", "2", doc);
        ASSERT_TRUE(!doc.empty());

        std::cout << "[10]delete index" << std::endl;
        ret = es.deleteIndex("facebook");
        ASSERT_TRUE(ret);
        
    } catch (Exception &e) {
        std::cout << "Failed:" << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cout << "Failed:" << e.what() << std::endl;
        ASSERT_EQ(ret, false);
    }
}


TEST(ElasticSearch, TEST_3)
{
    bool ret=false;
    try {
        ElasticSearch es(address,false,true);
        std::cout<<"[1]connect elasticsearch:"<<address<<std::endl;
        
        BulkBuilder builder;

        for (int i = 0; i < 10; ++i) 
        {
            Json::Value doc;
            doc["user"] = i + 1;
            doc["name"] = "tang";
            doc["post_date"] = "2009-11-15T14:12:14";
            doc["message"] = "sssssssssssss";
            builder.index("hadoop", "hdfs", doc);
        }
        
        for (int i = 0; i < 5; ++i) 
        {
            Json::Value doc1;
            doc1["user"] = i + 101;
            doc1["name"] = "tang";
            doc1["post_date"] = "2009-11-15T14:12:14";
            doc1["message"] = "sssssssssssss";

            std::stringstream ss;
            ss << (i + 101);
            builder.upsert("hadoop", "hdfs", ss.str(), doc1);
        }

        std::cout << "[2]bulk index document" << std::endl;
        Json::Value result;
        bool ret = es.bulk(builder.str().c_str(), result);
        ASSERT_EQ(ret, true);
        ASSERT_TRUE(!result.empty());
        
        builder.clear();

        sleep(2);
        std::cout << "[3]full scan all match query documents by index/type" << std::endl;
        std::string query="{\"query\":{\"match\":{\"name\":\"tang\"}}}";
        Json::Value resultArray;
        int count = es.fullScan("hadoop", "hdfs", query, resultArray);
        ASSERT_GT(count, 0);
        ASSERT_TRUE(!resultArray.empty());
        
        std::cout << "[4]delete index" << std::endl;
        ret = es.deleteIndex("hadoop");
        ASSERT_TRUE(ret);

    } catch (Exception &e) {
        std::cout << "Failed:" << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cout << "Failed:" << e.what() << std::endl;
        ASSERT_EQ(ret, false);
    }
}

int main(int argc, char *argv[])
{
    return ::lut::RunAllTests();
}
