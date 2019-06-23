#ifndef INCLUDE_LOG_H
#define INCLUDE_LOG_H

#include<stdio.h>
#include<iostream>

#define NORMAL  0
#define NOTICE  1
#define WARNING 2
#define FATAL   4

void print_log(std::string msg,int line);

#endif // INCLUDE_LOG_H
