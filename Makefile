

all:
ifdef DIR
	@echo 'Start to compile the project'
	@make -C $(shell pwd)/study-linux-kernel SRCDIR=$(SRCDIR) SRCFILE=$(SRCFILE) OBJDIR=$(shell pwd)/obj BINDIR=$(shell pwd)/bin
else
	@echo 'not define DIR'
endif
