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

// *** BOOST Filesystem ***
#define BOOST_FILESYSTEM_VERSION 3

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED 
#  define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#ifndef BOOST_SYSTEM_NO_DEPRECATED 
#  define BOOST_SYSTEM_NO_DEPRECATED
#endif

#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;


// *** Std Library ***
#include <stdlib.h>
#include <iostream>
#include <set>
#include <string>
#include <queue>

#define FILE_EXT_DELIM '.'

static void findFiles(fs::path startp, std::set<fs::path> types);

std::queue<fs::path> files; 

int main(int argc, char* argv[]){
    
    fs::path rootp("");
    std::set<fs::path> types;

    fs::path ext(".txt");
    types.insert(ext);

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

    findFiles(rootp, types);
    
    while(!files.empty()){
	std::cout << files.front() << std::endl;
	files.pop();
    }

    return 0;
}

static void findFiles(fs::path startp, std::set<fs::path> types){

    try{
	if(fs::is_directory(startp)){
	    fs::directory_iterator end_iter;
	    for(fs::directory_iterator dir_itr(startp); dir_itr != end_iter; ++dir_itr){    
		findFiles(dir_itr->path(), types);
	    }
	}
	else if(fs::is_regular_file(startp)){
	    //Add path if of proper type
	    fs::path ext = startp.extension();
	    if(types.find(ext) != types.end()){
		files.push(startp);
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
