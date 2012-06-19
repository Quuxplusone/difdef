
difdef: difdef.cc diff2.cc
	g++ -O3 -W -Wall -Wextra -pedantic -Wno-sign-compare $^ -o $@

clean:
	rm difdef
