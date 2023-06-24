CC=gcc


BIN_PATH=$(BINDIR)
ROOT_PATH=$(shell pwd)
INC_PATH=-I$(ROOT_PATH)/common
SRC_PATH=$(SRCDIR)

CFLAGS:= -g -Wall -Wno-unused -Werror -c -std=c11 -I$(SRC_PATH) -I$(SRC_PATH)/../
VPATH=$(SRC_PATH):$(OBJDIR)
LDLIBS=-lpthread -lcrypt -lm
LDFLAGS=-L$(BIN_PATH) -Wl,-rpath=. #$(BIN_PATH)

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

dynamic_link_library:=$(filter %dll,$(TARGETS))
$(info dynamic_link_library=$(dynamic_link_library))
# finddll=$(findstring dll,$(TARGETS))
# $(info finddll=$(finddll))

ifneq ($(dynamic_link_library),)
TARGETS:=lib$(TARGETS).so
CFLAGS+=-fPIC
$(info Will be compiled into a dynamic link library)
$(info CFLAG = $(CFLAGS))
endif

$(info targets=$(TARGETS))


###############################################
# dependent dynamic link library 
###############################################
ifeq ($(dynamic_link_library),)
DEP_DLL:=$(call wildcard,$(BIN_PATH)/libcommon*.so)
DEP_DLL:=$(patsubst lib%.so,-l%,$(notdir $(DEP_DLL)))
$(info DEP_DLL=$(DEP_DLL))
endif

###############################################
# build
###############################################
all: $(TARGETS) clean

# 添加选项 --verbose 打印编译详情
$(TARGETS): $(OBJS_C)
	@if [ "$(dynamic_link_library)" = "" ];then \
	$(CC) --verbose $< -o $(BINDIR)/$@ $(LDFLAGS) $(LDLIBS) $(DEP_DLL);\
	else \
	$(CC) --verbose $< -shared -fPIC -o $(BINDIR)/$@;\
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