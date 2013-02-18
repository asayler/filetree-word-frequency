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
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

namespace bt = boost;
namespace fs = bt::filesystem;
namespace po = bt::program_options;

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
#include <vector>

// *** Local Data Structures ***
#include "TS_Map.hpp"
#include "PC_Queue.hpp"

// *** Constants ***
#define FILE_EXT_DELIMINATOR "."
#define TYPE_DELIMINATORS    ","
#define TYPE_NONE            "."
#define TYPE_ANY             "*"

#define DEFAULT_TYPES        "txt"
#define DEFAULT_MOST          10
#define DEFAULT_MOST_STR     "10"
#define DEFAULT_LEAST         0
#define DEFAULT_LEAST_STR    "0"
#define DEFAULT_WORKERS       2 //Only used if not auto-detected

#define USAGE " [OPTION]... [PATH]..."
#define DESC "Counts word frequency in files of TYPE within PATH"

// *** Globals ***
PC_Queue<fs::path>       gFiles(fs::path("")); 
TS_Map<std::string, int> gWords; 

// *** Local Prototypes ***
static void findFiles(fs::path startp, std::set<fs::path> types);
static void processFile();

// *** Test and Compare Functions ***
bool compare_word_counts(std::pair<std::string, int> first, std::pair<std::string, int> second){
    return first.second > second.second;
}
inline bool legalChar(char c){
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

// *** Main Entry ***
int main(int argc, char* argv[]){
    
    // Local Vars
    std::vector<fs::path> searchPaths;
    std::set<fs::path> searchTypes;
    int mostN;
    int leastN;
    int workersN;
    bt::thread_group finders;
    bt::thread_group counters;

    // Parse Input
    try{
	po::options_description visible("Options");
        visible.add_options()
            ("help,h",
	     ": produce help message")
	    ("file-types,t", po::value<std::string>(),
	     ": comma-seperated list of file extensions to scan\n"
	     "  ('" TYPE_NONE  "' for no extension) (DEFAULT: '" DEFAULT_TYPES "')")
	    ("most,m", po::value<int>(),
	     ": list the N most common words (DEFAULT: '" DEFAULT_MOST_STR "')")
	    ("least,l", po::value<int>(),
	     ": list the N least common words (DEFAULT: '" DEFAULT_LEAST_STR "')")
	    ("worker-threads,w", po::value<int>(),
	     ": use N worker-threads to process files\n"
	     "  (DEFAULT: <System Optmized>)")
	    ;

	po::options_description hidden("Hidden Options");
	hidden.add_options()
	    ("input-paths", po::value< std::vector<std::string> >(), "input paths")
	    ;

	po::positional_options_description pos_opts;
        pos_opts.add("input-paths", -1);
	
	po::options_description cli_opts;
        cli_opts.add(visible).add(hidden);

	po::variables_map vm;        
	po::store(po::command_line_parser(argc, argv).options(cli_opts).positional(pos_opts).run(), vm);
	po::notify(vm);    

	// '--help' option
        if(vm.count("help")){
	    std::string name(argv[0]);
	    std::cerr << "Usage:" << std::endl;
	    std::cerr << "  " << name << USAGE << std::endl;
	    std::cerr << DESC << std::endl;
	    std::cerr << visible << std::endl;
            return EXIT_SUCCESS;
        }

	// 'files-types' option
	std::string strTypes;
	if(vm.count("file-types")){
	    strTypes = vm["file-types"].as< std::string >();
	}
	else{
	    strTypes = DEFAULT_TYPES;
	}
	std::vector<std::string> vecTypes;
	bt::split(vecTypes, strTypes, bt::is_any_of(TYPE_DELIMINATORS));
	for (std::vector<std::string>::iterator i = vecTypes.begin() ; i != vecTypes.end(); ++i){
	    std::string strExt;
	    if(*i == TYPE_NONE){
		strExt = "";
	    }
	    else if(*i == TYPE_ANY){
		strExt = "";
		//ToDo: Add Wildcard Handling
	    }
	    else{
		strExt = FILE_EXT_DELIMINATOR + *i;
	    }
	    fs::path pExt(strExt);
	    searchTypes.insert(pExt);
	}

	// 'most' option
	if(vm.count("most")){
	    mostN = vm["most"].as< int >();
	}
	else{
	    mostN = DEFAULT_MOST;
	}
	if(mostN < 0){
	    std::cerr << "Option 'most' must be greater than or equal to 0" << std::endl;
	    exit(EXIT_FAILURE);
	}

	// 'least' option
	if(vm.count("least")){
	    leastN = vm["least"].as< int >();
	}
	else{
	    leastN = DEFAULT_LEAST;
	}
	if(leastN < 0){
	    std::cerr << "Option 'least' must be greater than or equal to 0" << std::endl;
	    exit(EXIT_FAILURE);
	}

	// 'worker-threads' option
	if(vm.count("worker-threads")){
	    workersN = vm["worker-threads"].as< int >();
	}
	else{
	    workersN = bt::thread::hardware_concurrency();
	    if(workersN == 0){
		workersN = DEFAULT_WORKERS;
	    }
	}
	if(workersN <= 0){
	    std::cerr << "Option 'worker-threads' must be greater than 0" << std::endl;
	    exit(EXIT_FAILURE);
	}

	// input paths option
	if(vm.count("input-paths")){
	    std::vector<std::string> paths;
	    paths = vm["input-paths"].as< std::vector<std::string> >();
	    for (std::vector<std::string>::iterator i = paths.begin() ; i != paths.end(); ++i){
		fs::path tmpPath(*i);
		searchPaths.push_back(tmpPath);
	    }
	}
	else{
	    searchPaths.clear();
	    searchPaths.push_back(fs::current_path());
	}
    }
    catch(const std::exception &e) {
	std::cerr << "error: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    catch(...) {
	std::cerr << "Exception of unknown type!" << std::endl;
	exit(EXIT_FAILURE);
    }

    // Launch File Search Threads
    for (std::vector<fs::path>::iterator i = searchPaths.begin(); i != searchPaths.end(); ++i){
	if(fs::exists(*i)){
	    finders.add_thread(new bt::thread(findFiles, *i, searchTypes));
	}
	else{
	    std::cerr << "Path " << *i << " not found!" << std::endl;
	}	
    }

    // Launch File Processing Threads
    for(int i = 0; i < workersN; i++){
	counters.add_thread(new bt::thread(processFile));
    }
   
    // Wait on File Search Threads
    finders.join_all();
    gFiles.close();

    // Wait of File Processing Threads
    counters.join_all();

    // Process and Print Results
    std::list< std::pair<std::string, int> > wordCounts = gWords.getList();
    wordCounts.sort(compare_word_counts);
    for (std::list< std::pair<std::string, int> >::iterator i = wordCounts.begin();
	 i != wordCounts.end(); i++){
	if(std::distance(wordCounts.begin(), i) < mostN){
	    std::cout << i->second << " - " << i->first << std::endl;
	}
	else{
	    break;
	}
    }
    wordCounts.reverse();
    for (std::list< std::pair<std::string, int> >::iterator i = wordCounts.begin();
	 i != wordCounts.end(); i++){
	if(std::distance(wordCounts.begin(), i) < leastN){
	    std::cout << i->second << " - " << i->first << std::endl;
	}
	else{
	    break;
	}
    }
    
    return EXIT_SUCCESS;
}

// Function to find all files of a set of types in tree startp
static void findFiles(fs::path startp, std::set<fs::path> types){

    try{
	if(!fs::is_symlink(startp)){
	    if(fs::is_directory(startp)){
		fs::directory_iterator end_iter;
		for(fs::directory_iterator dir_itr(startp); dir_itr != end_iter; ++dir_itr){    
		    findFiles(dir_itr->path(), types);
		}
	    }
	    else if(fs::is_regular_file(startp)){
		fs::path ext = startp.extension();
		if(types.find(ext) != types.end()){
		    gFiles.push(startp);
		}
	    }
	    else{
		// Do Nothing
	    }
	}
    }
    catch(const std::exception & ex){
	std::cerr << startp.filename() << ": " << ex.what() << std::endl;
    }
    catch(...) {
	std::cerr << startp.filename() << ": Exception of unknown type!" << std::endl;
    }

    return;
}

// Function to read files from queue and record word frequency
static void processFile(){
    
    for(fs::path p = gFiles.pop(); !p.empty(); p = gFiles.pop()){
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
