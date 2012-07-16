CXX = g++
CFLAGS = -Ilibsrc -O3 -W -Wall -Wextra -pedantic

all: diffn

diffn: main.o difdef_impl.o
	$(CXX) $(CFLAGS) $^ -o $@

difdef_impl.o: libsrc/difdef_impl.cc libsrc/patience.cc libsrc/classical.cc
	$(CXX) $(CFLAGS) -c libsrc/difdef_impl.cc -o $@

main.o: src/main.cc
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.o diffn
