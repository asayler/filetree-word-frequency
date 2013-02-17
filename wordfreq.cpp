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
#define HEAD_PRINT_CNT 5

// *** Local Prototypes ***
static void findFiles(fs::path startp, std::set<fs::path> types, bool top);
static void processFile();

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
	std::cerr << "fin = " << fin << std::endl;
    }

    void push(const T &data){
	std::cerr << "push locking" << std::endl;
	bt::mutex::scoped_lock l(m);
	if(!closed){
	    std::cerr << "pushing " << data << std::endl;
	    q.push(data);
	    std::cerr << "signaling one..." << std::endl;
	    c.notify_one();
	}
	else{
	    std::cerr << "ERROR can't push to closed queue" << std::endl;
	    throw;
	}
	std::cerr << "push done" << std::endl;
    }
    
    T pop(){
	bt::mutex::scoped_lock l(m);
	std::cerr << "poping..." << std::endl;
	if(q.empty() && !closed){
	    std::cerr << "queue empty: waiting..." << std::endl;
	    c.wait(l);
	    std::cerr << "done waiting..." << std::endl;
	}
	T ret;
	if(!q.empty()){
	    ret = q.front();
	    q.pop();
	    std::cerr << "poped " << ret << std::endl;
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
	//bt::mutex::scoped_lock l(m);
	std::cerr << "closing..." << std::endl;
	closed = true;
	std::cerr << "signaling all..." << std::endl;
	c.notify_all();
    }
};

// *** Globals ***
PC_Queue<fs::path>         gFiles(fs::path("")); 
std::map<std::string, int> gWords; 
bool producersFinished = false;

struct WordCount{
    std::string word;
    int count;

    WordCount(std::string w="", int c=0):word(w), count(c){}
};

bool compare_WordCount(WordCount first, WordCount second){
    return first.count > second.count;
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

    if(!fs::exists(rootp)){
	std::cerr << "Path " << rootp << " not found!" << std::endl;
	exit(EXIT_FAILURE);
    }

    // File Search
    bt::thread tFind(findFiles, rootp, types, true);
    //findFiles(rootp, types);
    
    // File Processing
    bt::thread tProcess(processFile);
   
    // Wait on Producers
    std::cerr << "waiting on tFind join" << std::endl;
    tFind.join();
    std::cerr << "tFind joined, calling close" << std::endl;
    gFiles.close();

    // Wait of Consumers
    std::cerr << "waiting on tProcess join" << std::endl;
    tProcess.join();
    std::cerr << "tProcess joined" << std::endl;

    // Process Map
    std::list<WordCount> wordCounts;
    for(std::map<std::string, int>::iterator i = gWords.begin(); i != gWords.end(); ++i){
	//std::cout << i->first << " - " << i->second << std::endl;
	WordCount wc(i->first, i->second);
	wordCounts.push_back(wc);
    }
    wordCounts.sort(compare_WordCount);
    for (std::list<WordCount>::iterator i = wordCounts.begin(); i != wordCounts.end(); i++){
	if(std::distance(wordCounts.begin(), i) < HEAD_PRINT_CNT){
	    std::cout << i->word << " - " << i->count << std::endl;
	}
	else{
	    break;
	}
    }

    return 0;
}

// Function to find all files of a set of types in the tree rooted
// at startp
static void findFiles(fs::path startp, std::set<fs::path> types, bool top){

    try{
	if(fs::is_directory(startp)){
	    fs::directory_iterator end_iter;
	    for(fs::directory_iterator dir_itr(startp); dir_itr != end_iter; ++dir_itr){    
		findFiles(dir_itr->path(), types, false);
	    }
	}
	else if(fs::is_regular_file(startp)){
	    //Add path if of proper type
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
 
    if(top){
	std::cerr << "find top done running" << std::endl;
	//gFiles.close();
    }

    //std::cerr << "find returning" << std::endl;
    return;
}

// Function to read file and record word frequency
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
		while(start != word.end()){
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
