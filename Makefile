CXX = g++
CFLAGS = -Ilibsrc -O3 -W -Wall -Wextra -pedantic

all: diffn

diffn: main.o ifdefs.o unified.o verify.o difdef_impl.o
	$(CXX) $(CFLAGS) $^ -o $@

difdef_impl.o: libsrc/difdef_impl.cc libsrc/patience.cc libsrc/classical.cc
	$(CXX) $(CFLAGS) -c libsrc/difdef_impl.cc -o $@

%.o: src/%.cc
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.o diffn
