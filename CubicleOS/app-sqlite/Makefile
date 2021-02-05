UK_ROOT ?= $(PWD)/../unikraft
UK_LIBS ?= $(PWD)/../unikraft/lib
LIBS := $(UK_LIBS)/libsqlite:$(UK_LIBS)/newlib


all:
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L=$(LIBS)

$(MAKECMDGOALS):
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L=$(LIBS) $(MAKECMDGOALS)

