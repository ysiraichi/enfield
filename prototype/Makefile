
objects = Analyser.o Isomorphism.o DynSolver.o
executable = enfield

input = input
tests = $(shell ls p_*)

CC = clang++
CXXFLAGS = -O0 -g -std=c++11

M = -dyn

all: $(objects)
	$(CC) $(CXXFLAGS) $? -o $(executable)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $?

run: $(tests)
	@$(foreach test, $(tests), 								\
		echo "\n\n./$(executable) $(input) $(test) $(M)";   \
		./$(executable) $(input) $(test) $(M) 2> /dev/null;)

clean:
	rm -vf $(objects) $(executable)
