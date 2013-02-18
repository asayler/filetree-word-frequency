// PC_Queue.hpp
// A basic multi-producer, multi-consumer, thread-safe queue

// Copyright (c) 2013 Andy Sayler (www.andysayler.com)
//
// This file is part of the File Tree Word Frequency Analysis Program.
//
//    The File Tree Word Frequency Analysis Program is free software:
//    you can redistribute it and/or modify it under the terms of the
//    GNU General Public License as published by the Free Software
//    Foundation, either version 3 of the License, or (at your option)
//    any later version.
//
//    The File Tree Word Frequency Analysis Program is distributed in
//    the hope that it will be useful, but WITHOUT ANY WARRANTY;
//    without even the implied warranty of MERCHANTABILITY or FITNESS
//    FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//    for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with the File Tree Word Frequency Analysis Program
//    (see accompanying file COPYING).  If not, see
//    <http://www.gnu.org/licenses/>.
//
// This program uses the Boost Software Libraires in compliance with
// the Boost Software License, Version 1.0. (See accompanying file
// BOOST_LICENSE_1_0 or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PCQUEUE_H
#define PCQUEUE_H

#include <iostream>
#include <queue>
#include <boost/thread.hpp>

// PC_Queue Definition

template <typename T> class PC_Queue{

private:

    std::queue<T> q;
    boost::mutex m;
    boost::condition_variable c;
    bool closed;
    T finished;

public:

    PC_Queue(T fin);

    void push(const T &data);
    T    pop();
    void close();

};

// PC_Queue Class Implmentation

template <typename T> PC_Queue<T>::PC_Queue(T fin){
    finished = fin;
    closed = false;
}

template <typename T> void PC_Queue<T>::push(const T &data){
    boost::mutex::scoped_lock l(m);
    if(!closed){
	q.push(data);
	c.notify_one();
    }
    else{
	throw;
    }
}

template <typename T> T PC_Queue<T>::pop(){
    T ret;
    boost::mutex::scoped_lock l(m);
    while(q.empty() && !closed){
	c.wait(l);
    }
    if(!q.empty()){
	ret = q.front();
	q.pop();
    }
    else if(closed){
	ret = finished;
    }
    else{
	//If locks are working, we should not get here
	std::cerr << "Unhandeld Case" << std::endl;
	std::cerr << "q.empty() = " << q.empty() << std::endl;
	std::cerr << "closed = " << closed << std::endl;
	throw;
    }
    return ret;
}

template <typename T> void PC_Queue<T>::close(){
    boost::mutex::scoped_lock l(m);
    closed = true;
    c.notify_all();
}

#endif
