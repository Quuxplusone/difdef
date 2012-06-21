cat >a <<EOF
foo
baz
EOF
cat >b <<EOF
foo
bats
EOF

./diffn -DRATS -DBATS a b >out
cat >expected <<EOF
foo
#if defined(RATS)
baz
#endif /* RATS */
#if defined(BATS)
bats
#endif /* BATS */
EOF
diff expected out

./diffn -DRATS a b >/dev/null 2>&1
if [ $? == 0 ]; then
  echo "Didn't report failure from too few -D options"
fi

./diffn -DRATS -DBATS -DCATS a b >/dev/null 2>&1
if [ $? == 0 ]; then
  echo "Didn't report failure from too many -D options"
fi

rm -f a b expected out
