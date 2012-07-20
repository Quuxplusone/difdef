make difdef
cp ./difdef ./tests/
cd tests
SAW_ERRORS=false
for test in *.sh; do
  echo $test
  source $test >OUTPUT
  if ! diff --brief /dev/null OUTPUT ; then
    SAW_ERRORS=true
  fi
done
rm -f difdef OUTPUT
if $SAW_ERRORS; then
  echo "====Some tests failed!===="
fi
