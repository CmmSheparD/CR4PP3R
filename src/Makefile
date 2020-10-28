CPPC = g++
CPPFLAGS = -std=c++17 -c -Wall -Wextra -Wpedantic -g
EXE = l0n3r

all: $(EXE)

$(EXE): main.o view.o tile.o map.o
	$(CPPC) $^ -o $@ -lncurses

main.o: main.cpp map/map.hh view/view.hh
	$(CPPC) $(CPPFLAGS) $<

view.o: view/view.cpp view/view.hh
	$(CPPC) $(CPPFLAGS) $<

tile.o: map/tile.cpp map/tile.hh
	$(CPPC) $(CPPFLAGS) $<

map.o: map/map.cpp map/map.hh
	$(CPPC) $(CPPFLAGS) $<

clean:
	rm $(EXE) *.o
