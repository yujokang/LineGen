.PHONY:src tests
include common.mk
INCLUDE=-Iinclude
CPPFLAGS=$(_CPPFLAGS) $(INCLUDE)
SUBDIRS=src tests
OBJS=
TARGETS=line_gen.a
all: $(SUBDIRS) $(OBJS) $(TARGETS)
line_gen.a: src/line_gen.a
	cp $^ $@
src:
	$(MAKE) -C src
tests:
	$(MAKE) -C tests
clean:
	$(RM) $(RM_FLAGS) $(OBJS) $(TARGETS)
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
