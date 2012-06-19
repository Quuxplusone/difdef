
diff2: diff2_main.cc diff2.cc
	g++ -O3 -W -Wall -Wextra -pedantic -Wno-sign-compare $^ -o $@

clean:
	rm diff2
