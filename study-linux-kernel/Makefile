CC=gcc
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
finddll=$(findstring dll,$(TARGETS))
$(info finddll=$(finddll))

ifeq ($(finddll),dll) 
TARGETS:=lib$(TARGETS).so
CFLAGS+=-fPIC
$(info Will be compiled into a dynamic link library)
$(info CFLAG = $(CFLAGS))
endif

$(info targets=$(TARGETS))


###############################################
# build
###############################################
all: $(TARGETS) 

# 添加选项 --verbose 打印编译详情
$(TARGETS): $(OBJS_C)
	@if [ $(finddll) = dll ];\
	then \
	$(CC) --verbose $< -shared -fPIC -o $(BINDIR)/$@;\
	else \
	$(CC) --verbose $< -o $(BINDIR)/$@  $(LIB);\
	fi

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