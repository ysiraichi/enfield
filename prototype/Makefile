
objects = Analyser.o Isomorphism.o
executable = enfield

input = input
tests = $(shell ls p_*)

CC = clang++
CXXFLAGS = -O0 -g -std=c++11

all: $(objects)
	$(CC) $(CXXFLAGS) $? -o $(executable)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $?

run: $(tests)
	@$(foreach test, $(tests), 								\
		echo "\n\n./$(executable) $(input) $(test)";		\
		./$(executable) $(input) $(test);)

clean:
	rm -vf $(objects) $(executable)
