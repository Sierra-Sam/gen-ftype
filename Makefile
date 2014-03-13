
PROGRAM = gen-ftype
SRCS = gen-ftype.c

TEST_PROGRAM = test-gen-ftype
TEST_SRCS = test-gen-ftype.c

.PHONY: gen test clean all

all: $(PROGRAM) $(TEST_PROGRAM)

gen: $(PROGRAM)

test: $(TEST_PROGRAM)
	./$(TEST_PROGRAM)

$(PROGRAM): $(SRCS)
	gcc -o $(PROGRAM) -Wall -Wextra $(SRCS)

$(TEST_PROGRAM): $(TEST_SRCS)
	gcc -o $(TEST_PROGRAM) -Wall -Wextra $(TEST_SRCS)

clean:
	rm -f $(PROGRAM) $(TEST_PROGRAM) *.o

#END

