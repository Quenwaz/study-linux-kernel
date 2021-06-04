CC:=gcc
CFLAGS:= -g -Wall -c -std=c11

BIN_PATH=$(BINDIR)
ROOT_PATH=$(shell pwd)
INC_PATH=-I$(ROOT_PATH)/common
SRC_PATH=$(SRCDIR)
VPATH=$(SRC_PATH):$(OBJDIR)
LIB=-lpthread -lcrypt

# SRC_PATH+=$(ROOT_PATH)/src
$(info BIN_PATH=$(BIN_PATH))
$(info OBJDIR=$(OBJDIR))

rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard,$d/,$2) \
						$(filter $2,$d))


###############################################
# src
###############################################
SRC_C=$(call rwildcard,$(SRC_PATH),%.c)
SRC_C:=$(SRCFILE)

# SRC_C:=$(notdir $(SRC_C))
$(info src=$(SRC_C))

###############################################
# objects
###############################################
OBJS_C=$(patsubst %.c,%.o, $(notdir $(SRC_C)))
$(info objects=$(OBJS_C))


###############################################
# targets
###############################################
TARGETS=$(patsubst %.c,%, $(notdir $(SRC_C)))
$(info targets=$(TARGETS))

###############################################
# build
###############################################
all:$(TARGETS)

$(TARGETS): %:%.o
	$(CC) $< -o $(BINDIR)/$@  $(LIB)

$(OBJS_C): %.o:%.c
	$(CC) $(CFLAGS) $(INC_PATH) $< -o $(OBJDIR)/$@


.PHONY: clean
clean: 
	rm $(OBJDIR)/*.o
	rm $(BINDIR)/*