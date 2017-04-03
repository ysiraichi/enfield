
objects = Analyser.o Isomorphism.o
executable = enfield

CC = clang++
CXXFLAGS = -O0 -g -std=c++11

all: $(objects)
	$(CC) $(CXXFLAGS) $? -o $(executable)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $<

clean:
	rm -vf $(objects) $(executable)
