

all:
ifdef DIR
	@echo 'Start to compile the project'
	@make -C $(DIR) BINDIR=$(shell pwd)/bin OBJDIR=$(shell pwd)/obj
else
	@echo 'not define DIR'
endif
