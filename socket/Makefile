CXX:=g++
CXXFLAGS:= -g -Wall -c 
TARGETS=ipc-posix-queue

BIN_PATH=$(BINDIR)
ROOT_PATH=$(shell pwd)
INC_PATH=-I$(ROOT_PATH)
SRC_PATH=$(ROOT_PATH)
# SRC_PATH+=$(ROOT_PATH)/src

LIB=-lpthread


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
all:$(TARGETS) clean_intermediate

$(TARGETS): %:%.o
	$(CXX) $< -o $(BINDIR)/$@  $(LIB)

$(OBJS_CPP): %.o:%.cpp
	$(CXX) $(CXXFLAGS) $(INC_PATH) $< -o $@


.PHONY: clean_intermediate
clean_intermediate:
	rm ./*.o

.PHONY: clean
clean: 
	rm $(TARGETS) $(OBJS_CPP)