// A Basic Word Frequency Analysis Program

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


// *** BOOST Includes ***
#define BOOST_FILESYSTEM_VERSION 3

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED 
#  define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#ifndef BOOST_SYSTEM_NO_DEPRECATED 
#  define BOOST_SYSTEM_NO_DEPRECATED
#endif

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/thread.hpp>

namespace bt = boost;
namespace fs = bt::filesystem;

// *** Std Library Includes ***
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <string>
#include <queue>

// *** Constants ***
#define FILE_EXT_DELIM '.'
#define HEAD_PRINT_CNT 10

// *** Local Prototypes ***
static void findFiles(fs::path startp, std::set<fs::path> types);
static void processFile();

template <typename K, typename V>
class TS_Map{
private:
    std::map<K, V> map;
    bt::mutex m;

public:
    V& operator[] (const K &key){
	bt::mutex::scoped_lock l(m);
	return map[key];
    }

    std::list< std::pair<K, V> > getList(){
	bt::mutex::scoped_lock l(m);
	std::list< std::pair<K, V> > lst;
	for(typename std::map<K, V>::iterator i = map.begin(); i != map.end(); ++i){
	    lst.push_back(*i);
	}
	return lst;
    }

};

template <typename T>
class PC_Queue{
private:
    std::queue<T> q;
    bt::mutex m;
    bt::condition_variable c;
    bool closed;
    T finished;

public:
    PC_Queue(T fin){
	finished = fin;
	closed = false;
    }

    void push(const T &data){
	bt::mutex::scoped_lock l(m);
	if(!closed){
	    q.push(data);
	    c.notify_one();
	}
	else{
	    throw;
	}
    }
    
    T pop(){
	bt::mutex::scoped_lock l(m);
	if(q.empty() && !closed){
	    c.wait(l);
	}
	T ret;
	if(!q.empty()){
	    ret = q.front();
	    q.pop();
	}
	else if(closed){
	    ret = finished;
	}
	else{
	    //Should not get here
	    throw;
	}
	return ret;
    }

    void close(){
	bt::mutex::scoped_lock l(m);
	closed = true;
	c.notify_all();
    }
};

// *** Globals ***
PC_Queue<fs::path>       gFiles(fs::path("")); 
TS_Map<std::string, int> gWords; 


// *** Test Functions ***
bool compare_word_counts(std::pair<std::string, int> first, std::pair<std::string, int> second){
    return first.second > second.second;
}
bool legalChar(char c){
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

// *** Main Entry ***
int main(int argc, char* argv[]){
    
    fs::path rootp("");
    std::set<fs::path> types;

    fs::path ext(".txt");
    types.insert(ext);

    // Input Processing
    if(argc == 1){
	rootp = fs::current_path();
    }
    else if(argc == 2){
	rootp = fs::system_complete(argv[1]);
    }
    else{
	std::cerr << "usage:   simple_ls"        << std::endl;
	std::cerr << "         simple_ls <path>" << std::endl;
	exit(EXIT_FAILURE);
    }

    // Input Validation
    if(!fs::exists(rootp)){
	std::cerr << "Path " << rootp << " not found!" << std::endl;
	exit(EXIT_FAILURE);
    }

    // File Search
    bt::thread tFind(findFiles, rootp, types);
    //findFiles(rootp, types);
    
    // File Processing
    bt::thread tProcess(processFile);
   
    // Wait on Producers
    tFind.join();
    gFiles.close();

    // Wait of Consumers
    tProcess.join();

    // Process and Print Map
    std::list< std::pair<std::string, int> > wordCounts = gWords.getList();
    wordCounts.sort(compare_word_counts);
    for (std::list< std::pair<std::string, int> >::iterator i = wordCounts.begin();
	 i != wordCounts.end(); i++){
	if(std::distance(wordCounts.begin(), i) < HEAD_PRINT_CNT){
	    std::cout << i->second << " - " << i->first << std::endl;
	}
	else{
	    break;
	}
    }
    
    return 0;
}

// Function to find all files of a set of types in tree startp
static void findFiles(fs::path startp, std::set<fs::path> types){

    try{
	if(fs::is_directory(startp)){
	    fs::directory_iterator end_iter;
	    for(fs::directory_iterator dir_itr(startp); dir_itr != end_iter; ++dir_itr){    
		findFiles(dir_itr->path(), types);
	    }
	}
	else if(fs::is_regular_file(startp)){
	    //Add path to queue if of proper type
	    fs::path ext = startp.extension();
	    if(types.find(ext) != types.end()){
		gFiles.push(startp);
	    }
	}
	else{
	    // Do Nothing
	}
    }
    catch(const std::exception & ex){
	std::cerr << startp.filename() << " " << ex.what() << std::endl;
    }

    return;
}

// Function to read files from queue and record word frequency
static void processFile(){
    
    for(fs::path p = gFiles.pop(); !p.empty(); p = gFiles.pop()){
	std::cout << p << std::endl;

	fs::ifstream file(p);

	if(file.is_open()){
	
	    std::string word;
	
	    while(file >> word){
		std::transform(word.begin(), word.end(), word.begin(), ::tolower);
		std::string::iterator start = word.begin();
		std::string::iterator end   = word.begin();
		while(sytart != word.end()){
		    while(start != word.end() && !legalChar(*start)){
			start++;
		    }
		    end = start;
		    while(end != word.end() && legalChar(*end)){
			end++;
		    }
		    if(start < end){
			std::string subWord(start, end);
			++gWords[subWord];
			start = end;
		    }
		}
	    }
    
	    file.close();	
	}
	else{
	    std::cerr << "Could not open file " << p << std::endl;
	}
    }    

    return;
}
