TESTS = test_cpy test_ref

PLOT = plot_ref
PLOT_EXEC = ./$(PLOT)

TEST_DATA = s Tai

CFLAGS = -O0 -Wall -Werror -g

VPLOT = gnuplot
PLOT_SRCIPT = scripts/plot_ins_time.gp scripts/plot_search_time.gp

# Control the build verbosity                                                   
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

GIT_HOOKS := .git/hooks/applied

.PHONY: all clean

all: $(GIT_HOOKS) $(TESTS) $(PLOT)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

OBJS_LIB = \
    tst.o bloom.o

OBJS := \
    $(OBJS_LIB) \
    test_cpy.o \
    test_ref.o \
	plot_ref.o

deps := $(OBJS:%.o=.%.o.d)

test_%: test_%.o $(OBJS_LIB)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS)  -o $@ $^ -lm

plot_%: plot_%.o $(OBJS_LIB)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS)  -o $@ $^ -lm

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

test:  $(TESTS)
	echo 3 | sudo tee /proc/sys/vm/drop_caches;
	perf stat --repeat 100 \
                -e cache-misses,cache-references,instructions,cycles \
                ./test_cpy --bench $(TEST_DATA)
	perf stat --repeat 100 \
                -e cache-misses,cache-references,instructions,cycles \
				./test_ref --bench $(TEST_DATA)

bench: $(TESTS)
	@for test in $(TESTS); do\
		./$$test --bench $(TEST_DATA); \
	done

plot: $(TESTS)
	$(VECHO) "  PLOT\t$^\n"
	$(PLOT_EXEC)
	$(VPLOT) $(PLOT_SRCIPT)

CLEAN_OBJ = \
	bench_cpy.txt bench_ref.txt ref.txt cpy.txt caculate \
	ins_time_bloom.txt ins_time_wobloom.txt search_time_bloom.txt search_time_wobloom.txt

clean:
	$(RM) $(TESTS) $(OBJS)
	$(RM) $(deps)
	rm -f  $(CLEAN_OBJ)

-include $(deps)
