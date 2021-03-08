CXX:=g++
CXXFLAGS:= -g -Wall -c 
TARGETS=ipc-pipe

BIN_PATH=$(shell pwd)../bin
ROOT_PATH=$(shell pwd)
INC_PATH=$(ROOT_PATH)
SRC_PATH=$(ROOT_PATH)
# SRC_PATH+=$(ROOT_PATH)/src



rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard,$d/,$2) \
						$(filter $2, $d))


###############################################
# src
###############################################
SRC_CPP=$(call rwildcard,$(SRC_PATH),%.cpp)

###############################################
# objects
###############################################
OBJS_CPP=$(patsubst %.cpp, %.o, $(SRC_CPP))
$(info objects=$(OBJS_CPP))


###############################################
# targets
###############################################
TARGETS=$(patsubst %.cpp, %, $(notdir $(SRC_CPP)))
$(info targets=$(TARGETS))

###############################################
# build
###############################################
all:$(TARGETS)

$(TARGETS): %:%.o
	$(CXX) $< -o $@ 

$(OBJS_CPP): %.o:%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@


.PHONY: clean
clean: 
	rm $(TARGETS) $(OBJS_CPP)