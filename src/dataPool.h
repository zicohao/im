#ifndef INCLUDE_DATAPOOL_H
#define INCLUDE_DATAPOOL_H

#include <iostream>
#include <vector>
#include <semaphore.h>
#include <boost/noncopyable.hpp>

const int NUM = 256;

class DataPool : boost::noncopyable {
public:
    DataPool() {}
    
    DataPool(int size)
        :_pool(size)
        ,_capacity(size)
    {
        _productStep = 0;
        _consumeStep = 0;
        sem_init(&_blankSem, 0, size);
        sem_init(&_dataSem, 0, 0);
    }
/*
    DataPool(const DataPool& rhx) {
        this->_pool = rhx._pool;
        this->_capacity = rhx._capacity;
        this->_consumeStep = rhx._consumeStep;
        this->_productStep = rhx._productStep;
        this->_blankSem = rhx._blankSem;
        this->_dataSem = rhx._dataSem;
    }
*/
    /*
     * getMessage 函数功能是从数据队列中读数据
     * 返回值为 void
     * 
     * */

    void getMessage(std::string& inMessage) {
        sem_wait(&_dataSem);
        inMessage = _pool[_consumeStep];
        _consumeStep++;
        _consumeStep %= _capacity;
        sem_post(&_blankSem);
    }

    /*
     * putMessage 函数功能是向数据队列中写入数据
     * 返回值为 void
     * 
     * */

    void putMessage(const std::string& putMessage) {
        sem_wait(&_blankSem);
        _pool[_productStep] = putMessage;
        _productStep++;
        _productStep %= _capacity;
        sem_post(&_dataSem);
    }

    ~DataPool() {
        sem_destroy(&_blankSem);
        sem_destroy(&_dataSem);
    }

private:
    std::vector<std::string> _pool;
    int _capacity;  //队列容量
    int _consumeStep;   //生产者
    int _productStep;   //消费者
    sem_t _blankSem;    //信号量
    sem_t _dataSem;
};

#endif
