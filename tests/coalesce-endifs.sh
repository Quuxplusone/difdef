cat >a <<EOF
#if SOMETHING
  foo
#endif // foo
  baz
EOF
cat >b <<EOF
#if SOMETHING
  foo
  bar
#endif // foo bar
  baz
EOF
./diffn -DA -DB a b >out
cat >expected <<EOF
#if SOMETHING
  foo
#if defined(B)
  bar
#endif /* B */
#endif // foo bar
  baz
EOF
diff expected out

rm -f a b expected out
