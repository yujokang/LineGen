.PHONY:
include ../common.mk
INCLUDE=-I../include
CPPFLAGS=$(_CPPFLAGS) $(INCLUDE)
SUBDIRS=
OBJS=c_gen.o compare_files.o
TARGETS=line_gen.a
all: $(SUBDIRS) $(OBJS) $(TARGETS)
line_gen.a: c_gen.o compare_files.o
	$(AR) $(AR_FLAGS) $@ $^
clean:
	$(RM) $(RM_FLAGS) $(OBJS) $(TARGETS)
