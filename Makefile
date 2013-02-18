# File: Makefile
# By: Andy Sayler <www.andysayler.com>
#
# Creation Date: 2013/02/16
#
# Description:
#	This is the Makefile for the main
#	filetree-word-frequency program
#
# Copyright (c) 2013 Andy Sayler (www.andysayler.com)
#
# This file is part of the File Tree Word Frequency Analysis Program.
#
#    The File Tree Word Frequency Analysis Program is free software:
#    you can redistribute it and/or modify it under the terms of the
#    GNU General Public License as published by the Free Software
#    Foundation, either version 3 of the License, or (at your option)
#    any later version.
#
#    The File Tree Word Frequency Analysis Program is distributed in
#    the hope that it will be useful, but WITHOUT ANY WARRANTY;
#    without even the implied warranty of MERCHANTABILITY or FITNESS
#    FOR A PARTICULAR PURPOSE.  See the GNU General Public License
#    for more details.
#
#    You should have received a copy of the GNU General Public
#    License along with the File Tree Word Frequency Analysis Program
#    (see accompanying file COPYING).  If not, see
#    <http://www.gnu.org/licenses/>.
#
# This program uses the Boost Software Libraires in compliance with
# the Boost Software License, Version 1.0. (See accompanying file
# BOOST_LICENSE_1_0 or copy at http://www.boost.org/LICENSE_1_0.txt)

CPPC = g++
CPPL = g++

CPPCFLAGS = -c -g -Wall -Wextra
CPPLFLAGS = -g -Wall -Wextra

MAINEXECUTABLES    = wordfreq
EXAMPLEEXECUTABLES = simple_ls
TESTEXECUTABLES    = 

BOOSTLIBS = -lboost_filesystem -lboost_thread -lboost_system -lboost_program_options

.PHONY: all clean install

all: $(MAINEXECUTABLES) $(EXAMPLEEXECUTABLES) $(TESTEXECUTABLES)

wordfreq: wordfreq.o
	$(CPPL) $(CPPLFLAGS) $^ $(BOOSTLIBS) -o $@

simple_ls: simple_ls.o
	$(CPPL) $(CPPLFLAGS) $^ $(BOOSTLIBS) -o $@

wordfreq.o: wordfreq.cpp
	$(CPPC) $(CPPCFLAGS) $< -o $@

simple_ls.o: simple_ls.cpp
	$(CPPC) $(CPPCFLAGS) $< -o $@

clean:
	$(RM) $(MAINEXECUTABLES)
	$(RM) $(EXAMPLEEXECUTABLES)
	$(RM) $(TESTEXECUTABLES)
	$(RM) *.o
	$(RM) *~
