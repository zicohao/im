#ifndef INCLUDE_INFO_H
#define INCLUDE_INFO_H

/*
 * ========================================================================
 * 
 *          fileName:           info.h
 *          description:        组织 C/S 通信间的消息传输格式以及对外提供序
 *                              列化和反序列化接口
 *          对外接口:           void Serialize();       序列化
 *                              void DeSerialize();     反序列化
 *          qq:                 1003625407
 *          email@:             13669186256@163.com
 *
 * ========================================================================
 * */

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>

#include <jsoncpp/myjson/myjson.h>
#include <jsoncpp/json/json.h>
#include <boost/noncopyable.hpp>

class Base : boost::noncopyable {
public:
    Base() {};
    void writeJsonToString() {}
    void readJsonToRhx() {}
    virtual ~Base() {};
};
class Info : Base {
public:
    Info() {}

    //序列化添加状态信息组织成 json 对象
    void Serialize(std::string& strInfo) {
        ValueObj root;
        root["_userName"] = _userName;
        root["_school"] = _school;
        root["_message"] = _message;
        root["_token"] = _token;
        
        Write writer;
        //将 json 对象写回 string 中
        //(void)strInfo;
        writer.writeObjToJson(root, strInfo);
    }

    //反序列化从 json 对象中得到消息内容
    void DeSerialize(std::string& strInfo) {
        ValueObj root;
        Read reader;
        
        //从 json 对象中读取并解析
        //(void)strInfo;
        
        reader.readJsonToRhx(strInfo, root);

        _userName = root["_userName"].getstring();
        _school = root["_school"].getstring();
        _message = root["_message"].getstring();
        _token = root["_token"].getstring();
        
        /*
        _userName = root["_userName"].getstring();
        _userId = root["_userId"].getstring();
        _school = root["_school"].getstring();
        _message = root["_message"].getstring();
        _token = root["_token"].getstring();
        */
        
    }

    ~Info() {}
public:
    std::string _userName;
    std::string _school;
    std::string _message;
    std::string _token;
};

#endif
