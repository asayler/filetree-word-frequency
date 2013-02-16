# File: Makefile
# By: Andy Sayler <www.andysayler.com>
#
# Creation Date: 2013/02/16
#
# Description:
#	This is the Makefile for the main
#	filetree-word-frequency program

CPP = g++

CPPFLAGS = -c -g -Wall -Wextra
LPPFLAGS = -g -Wall -Wextra

MAINEXECUTABLES = 
TESTEXECUTABLES = 

.PHONY: all clean install

all: $(MAINEXECUTABLES) $(TESTEXECUTABLES)

clean:
	$(RM) $(MAINEXECUTABLES)
	$(RM) $(TESTEXECUTABLES)
	$(RM) *.o
	$(RM) *~
