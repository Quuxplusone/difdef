
all: diffn

diffn: diffn_main.cc diffn.cc patience.cc
	g++ -O3 -W -Wall -Wextra -pedantic -Wno-sign-compare $^ -o $@

clean:
	rm -f diffn
