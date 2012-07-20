CXX = g++
CFLAGS = -Ilibsrc -O -g -pg -W -Wall -Wextra -pedantic

all: difdef

difdef: main.o ifdefs.o recurse.o unified.o verify.o difdef_impl.o getline.o
	$(CXX) $(CFLAGS) $^ -o $@

difdef_impl.o: libsrc/difdef_impl.cc libsrc/patience.cc libsrc/classical.cc
	$(CXX) $(CFLAGS) -c libsrc/difdef_impl.cc -o $@

getline.o: libsrc/getline.cc
	$(CXX) $(CFLAGS) -c $^ -o $@

%.o: src/%.cc
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.o difdef
