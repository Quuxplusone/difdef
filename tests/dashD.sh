cat >a <<EOF
foo
baz
EOF
cat >b <<EOF
foo
bats
EOF

./difdef -DRATS -DBATS a b >out
cat >expected <<EOF
foo
#ifdef RATS
baz
#endif /* RATS */
#ifdef BATS
bats
#endif /* BATS */
EOF
diff expected out

./difdef -DRATS a b >/dev/null 2>&1
if [ $? == 0 ]; then
  echo "Didn't report failure from too few -D options"
fi

./difdef -DRATS -DBATS -DCATS a b >/dev/null 2>&1
if [ $? == 0 ]; then
  echo "Didn't report failure from too many -D options"
fi

rm -f a b expected out
