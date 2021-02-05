UK_ROOT ?= $(PWD)/../unikraft
UK_LIBS ?= $(PWD)/../unikraft/lib
LIBS := $(UK_LIBS)/libpthread-embedded:$(UK_LIBS)/newlib:$(UK_LIBS)/lwip:$(UK_LIBS)/libnginx


all:
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L=$(LIBS)

$(MAKECMDGOALS):
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L=$(LIBS) $(MAKECMDGOALS)

