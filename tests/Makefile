.PHONY:
include ../common.mk
INCLUDE=-I../include
CPPFLAGS=$(_CPPFLAGS) $(INCLUDE)
SUBDIRS=
C_GEN_TEST_OBJS=c_gen_tests.o test_c_gen.o
OBJS=$(C_GEN_TEST_OBJS)
TARGETS=test_c_gen
all: $(SUBDIRS) $(OBJS) $(TARGETS)

test_c_gen: $(C_GEN_TEST_OBJS)
	$(CC) $(CPPFLAGS) -o $@ $^ ../src/line_gen.a
clean:
	$(RM) $(RM_FLAGS) $(OBJS) $(TARGETS)
