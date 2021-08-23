CC:=gcc
CFLAGS:= -g -Wall -Wno-unused -Werror -c -std=c11

BIN_PATH=$(BINDIR)
ROOT_PATH=$(shell pwd)
INC_PATH=-I$(ROOT_PATH)/common
SRC_PATH=$(SRCDIR)
VPATH=$(SRC_PATH):$(OBJDIR)
LIB=-lpthread -lcrypt -lm

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
OBJS_C=$(patsubst %.c,$(OBJDIR)/%.o, $(notdir $(SRC_C)))
$(info objects=$(OBJS_C))


###############################################
# targets
###############################################
TARGETS=$(patsubst %.c,%, $(notdir $(SRC_C)))
$(info targets=$(TARGETS))

###############################################
# build
###############################################
all: $(TARGETS) clean

$(TARGETS): $(OBJS_C)
	$(CC) $< -o $(BINDIR)/$@  $(LIB)

# $(OBJS_C): %.o:%.c
$(OBJS_C): %.o:$(SRC_C)
	$(CC) $(CFLAGS) $(INC_PATH) $< -o $@

.PHONY: cleanall
cleanall:
	rm $(OBJDIR)/*.o || true
	rm $(BINDIR)/* || true

	
.PHONY: clean
clean: 
	rm $(OBJDIR)/*.o || true