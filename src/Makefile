# COPYRIGHT Team S69_STRINGS

LIB = s21_string.a
TEST_LIB = $(addprefix test_, $(LIB))

CC = gcc
GCOV = gcov

ifeq ($(shell uname -p), arm)
		CC = gcc-12
		GCOV = gcov-12
endif

EXTERNAL = external/check_s21_string.c
FLAGS = -Wall -Werror -Wextra -std=c11
OPTFLAGS = -g -pedantic
GFLAGS = -fprofile-arcs -ftest-coverage

IN_FLAGS = $(shell pkg-config --cflags check)
CHFLAGS = $(shell pkg-config --cflags --libs check)

GOFLAGS = -lgcov

TEST_DIR = tests
BUILD_DIR = build
BUILD_TEST_DIR = build_tests
TEST_BUILD_DIR = test_build
GCOV_DIR = gcov_report

OPENOS = vi
ifeq ($(shell uname -s), Linux)
		OPENOS = xdg-open
endif
ifeq ($(shell uname -s), Darwin)
		OPENOS = open
endif

S21_SOURCES = $(wildcard s21_*.c)
TESTS = $(wildcard $(TEST_DIR)/test_*.c)
S21_OBJECTS=$(addprefix $(BUILD_DIR)/, $(S21_SOURCES:.c=.o))
TEST_S21_OBJECTS=$(addprefix $(TEST_BUILD_DIR)/, $(S21_SOURCES:.c=.o))
TEST_OBJECTS=$(addprefix $(BUILD_TEST_DIR)/, $(notdir $(TESTS:.c=.o)))

.PHONY : all clean gcov_report style style-fix check  \
				rebuild docker gcov valgrind run external

all : $(LIB) test gcov_report

run :
	./test

###############################################
# FORM AND RUN TESTS
###############################################

test: $(TEST_OBJECTS) $(TEST_LIB)
	$(CC) $(FLAGS) $^ -o $@ $(CHFLAGS) $(GOFLAGS)

$(TEST_OBJECTS) : $(BUILD_TEST_DIR)/%.o : $(TEST_DIR)/%.c $(TEST_DIR)/test_me.h
	@mkdir -p $(BUILD_TEST_DIR)
	$(CC) $(FLAGS) $(IN_FLAGS) -c $< -o $@

###############################################
# FORM AND RUN EXTERNAL TESTS
###############################################

external : $(EXTERNAL) $(LIB)
	$(CC) -w -g $^ -o external/external $(CHFLAGS)
	./external/external

###############################################
# CREATING A TEST LIBRARY
###############################################

$(TEST_LIB): $(TEST_S21_OBJECTS)
	ar -rcs $@ $^

$(TEST_S21_OBJECTS): $(TEST_BUILD_DIR)/%.o : %.c $(LIB:.a=.h)
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(FLAGS) $(OPTFLAGS) $(GFLAGS) -c $< -o $@

###############################################
# CREATING A LIBRARY
###############################################

$(LIB): $(S21_OBJECTS)
	ar -rcs $@ $^

$(S21_OBJECTS): $(BUILD_DIR)/%.o : %.c $(LIB:.a=.h)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(FLAGS) -c $< -o $@

###############################################
# GCOV_REPORT
###############################################

gcov: gcov_report
	@$(OPENOS) ./$(GCOV_DIR)/gcov_report.html

gcov_report : $(LIB) test run $(GCOV_DIR)/gcov_report.html

$(GCOV_DIR)/gcov_report.html : $(LIB) test
	@mkdir -p gcov_report
	gcovr -r . --gcov-executable $(GCOV) --html --html-details -o $(GCOV_DIR)/gcov_report.html

###############################################
# CHECKS AND FIXES
###############################################

valgrind: test
	valgrind --leak-check=full --show-leak-kinds=all \
	--track-origins=yes --verbose ./test

style: 
	@cp ../materials/linters/.clang-format ./
	@echo =COPY DONE!=
	@clang-format -n *.c *.h
	@clang-format -n tests/*.c tests/*.h
	@echo =CHECK COMPLETED=
	@rm -rf .clang-format
	@echo =REMOVE DONE!=

style-fix: 
	@cp ../materials/linters/.clang-format ./
	@echo =COPY DONE!=
	@clang-format -i *.c *.h
	@clang-format -i tests/*.c tests/*.h
	@echo =CHECK COMPLETED=
	@rm -rf .clang-format
	@echo =REMOVE DONE!=

check:
	cppcheck --enable=all --suppress=missingIncludeSystem ./

docker: clean
	@cd ../materials/build && chmod +x ./run.sh && ./run.sh

###############################################
# CLEAN AND REBUILD
###############################################

clean:
	@echo ==Cleaning...==
	@rm -rf $(GCOV_DIR) $(BUILD_DIR) $(BUILD_TEST_DIR) $(TEST_BUILD_DIR)
	@rm -rf *.html
	@rm -rf a.out test
	@rm -rf *.a
	@rm -rf *.css
	@rm -rf *.gch $(TEST_DIR)/*.gch
	@rm -rf ./external/external
	@echo ==EVERYTHING REMOVED!==
	@echo ==Done!==

rebuild : 
	$(MAKE) clean
	$(MAKE) all
