// TS_Map.hpp
// A basic thread-safe associative map

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

#ifndef TSMAP_H
#define TSMAP_H

#include <iterator>
#include <map>
#include <boost/thread.hpp>

// TS_Map Class Definition

template <typename K, typename V> class TS_Map{

private:

    std::map<K, V> map;
    boost::mutex m;

public:

    V& operator[] (const K &key);
    std::list< std::pair<K, V> > getList();

};

// TS_Map Class Implementation

template <typename K, typename V> V& TS_Map<K, V>::operator[] (const K &key){
    boost::mutex::scoped_lock l(m);
    return map[key];
}

template <typename K, typename V> std::list< std::pair<K, V> > TS_Map<K, V>::getList(){
    boost::mutex::scoped_lock l(m);
    std::list< std::pair<K, V> > lst;
    for(typename std::map<K, V>::iterator i = map.begin(); i != map.end(); ++i){
	lst.push_back(*i);
    }
    return lst;
}

#endif
