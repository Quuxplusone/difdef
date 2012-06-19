
all: diff2 diff3 diffn

diff2: diff2_main.cc diff2.cc
	g++ -O3 -W -Wall -Wextra -pedantic -Wno-sign-compare $^ -o $@

diff3: diff3_main.cc diffn.cc diff2.cc
	g++ -O3 -W -Wall -Wextra -pedantic -Wno-sign-compare $^ -o $@

diffn: diffn_main.cc diffn.cc diff2.cc
	g++ -O3 -W -Wall -Wextra -pedantic -Wno-sign-compare $^ -o $@

clean:
	rm -f diff2 diff3 diffn
