#ifndef RVECTOR_H
#define RVECTOR_H

#include<iostream>
#include<queue>
#include<vector>
#include<string>
#include<boost/function_output_iterator.hpp>
#include"LockFreeStack.h"

#define underline "\033[4m"
#define reset   "\033[0m"

// Inspired by Alan Talbot's Lecture
template<typename T>
class RecyclableVectorContainer{
public:

    static RecyclableVectorContainer<T>& getInstance(){
        static RecyclableVectorContainer<T> instance; // singleton
        return instance;
    }

    std::vector<T> request(){
        if(data.empty()){
            return std::vector<T>();
        }
        return data.pop();
    }

    void recycle(std::vector<T>& vec){
        data.push(move(vec));
    }

private:
    LockFreeStack<std::vector<T>> data; // Use lock-free stack for benefit of LRU and concurrent request
};

/**
 * rvector is a template vector can be recyclable
 */
template<typename T>
class rvector{
public:
    rvector(): data{move(RecyclableVectorContainer<T>::getInstance().request())} {}
    rvector(int size): data{move(RecyclableVectorContainer<T>::getInstance().request())} { data.resize(size); }
    ~rvector(){
        data.clear();
        RecyclableVectorContainer<T>::getInstance().recycle(data); // recycle vector
    }
    std::vector<T> data;
};

/** 
 * Use Boost::Iterator’s function_output_iterator to create a rvector_ostream_joiner that acts just like 
 * ostream_iterator except that the delimiter only goes between elements (and not after the final element).
 * Use this to easily create an “operator<<“ to print rvectors in ostreams.
 * */
template <class DelimT, class charT = char, class traits = std::char_traits<charT> >
class rvector_ostream_joiner{
public:

    rvector_ostream_joiner(std::basic_ostream<charT,traits>& os, DelimT delimiter)
    : os{&os}, delimiter{delimiter}, first{true} {}

    template<typename T>
    void operator()(const T& x)
    {
        if(first){
            *os << underline <<  delimiter << x << delimiter;
            first = false;
        }else{
            *os << underline << x << delimiter;
        }
        *os << reset;
    }

private:
    std::basic_ostream<charT, traits>* os;
    DelimT delimiter;
    bool first;
};

template <class T, class charT = char, class traits = std::char_traits<charT> >
std::basic_ostream<charT,traits>& operator << (std::basic_ostream<charT,traits> &out, rvector<T> x){
    copy(x.data.begin(), x.data.end(), boost::make_function_output_iterator(rvector_ostream_joiner(out, u8"│")));
    return out;
}

#endif