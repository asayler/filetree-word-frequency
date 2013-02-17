# File: Makefile
# By: Andy Sayler <www.andysayler.com>
#
# Creation Date: 2013/02/16
#
# Description:
#	This is the Makefile for the main
#	filetree-word-frequency program

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
