make diffn
cp ./diffn ./tests/
cd tests
for test in *.sh; do
  echo $test
  source $test >OUTPUT
  diff --brief /dev/null OUTPUT
done
rm -f diffn OUTPUT
