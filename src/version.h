#ifndef INCLUDE_VERSION_H
#define INCLUDE_VERSION_H

#include <iostream>

#include <boost/noncopyable.hpp>
#include <ncurses.h>

const int SIZE = 1024;

class Version : boost::noncopyable {
public:
    Version() {
        initscr();
    }

    Version(const Version& rhx) {
        this->_header = rhx._header;
        this->_output = rhx._output;
        this->_input = rhx._input;
        this->_online = rhx._online;
    }
    
    //绘制聊天室 title 的函数
    void drawHeader() {
        int y = 0;
        int x = 0;
        int h = LINES / 5;
        int w = COLS;
        _header = newwin(h, w, y, x);
        box(_header, '$', '=');
        wrefresh(_header);
    }/*
    void drawFiles() {
        int y = LINES / 5;
        int x = 0;
        int h = LINES * 3 / 5;
        int w = COLS * 1 / 5;
        _output = newwin(h, w, y, x);
        box(_output, '-', '-');
        wrefresh(_output);
    }
*/
    //绘制聊天室消息输出框的函数
    void drawOutput() {
        int y = LINES / 5;
        int x = 0;
        int h = LINES * 3 / 5;
        int w = COLS * 3 / 4;
        _output = newwin(h, w, y, x);
        box(_output, '$', '=');
        wrefresh(_output);
    }

    //绘制聊天室消息输入框的函数
    void drawInput() {
        int y = LINES * 4 / 5;
        int x = 0;
        int h = LINES / 5;
        //int w = COLS;
        int w = COLS * 3 / 4;
        _input = newwin(h, w, y, x);
        box(_input, '$', '=');
        wrefresh(_input);
    }

    //绘制在线列表的函数
    void drawOnline() {
        int y = LINES / 5;
        int x = COLS * 3 / 4;
        int h = LINES * 4 / 5;
        int w = COLS * 1 / 4;
        _online = newwin(h, w, y, x);
        box(_online, '$', '=');
        wrefresh(_online);
    }

    //向窗口中放置数据信息
    void putStringOfVersion(WINDOW* w, int y, int x, std::string& strInfo) {
        mvwaddstr(w, y, x, strInfo.c_str());
        wrefresh(w);
    }
    
    //从窗口中获取信息
    void getStringOfVersion(WINDOW* w, std::string& getData) {
        char buffer[SIZE];
        wgetnstr(w, buffer, SIZE);
        getData = buffer;
    }

    //获取 Header 窗口的函数
    WINDOW* getHeader() {
        return _header;
    }

    WINDOW* getOutput() {
        return _output;
    }

    //获取消息窗口的函数
    WINDOW* getInput() {
        return _input;
    }

    //获取 onlineList 窗口的函数
    WINDOW* getOnline() {
        return _online;
    }

    ~Version() {
        endwin();
    }

private:
    WINDOW* _header;
    WINDOW* _output;
    WINDOW* _input;
    WINDOW* _online;
};

#endif
