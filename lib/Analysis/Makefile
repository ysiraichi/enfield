
core = Parser
bisonf = $(core).yy
flexf = $(core).l

bisonO = $(core).cc


bison: $(bisonf)
	bison $(bisonf) -o $(bisonO)

flex: $(flexf)
	flex $(flexf)

clean:
	rm -rfv *.hh *.cc
