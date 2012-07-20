cat >a <<EOF
#if a
  foo  // should not be merged
#endif
EOF
cat >b <<EOF
#ifdef a
  foo  // should not be merged
#else
  bar
#endif
EOF

./difdef -DA -DB a b >out
cat >expected <<EOF
#ifdef A
#if a
  foo  // should not be merged
#endif
#endif /* A */
#ifdef B
#ifdef a
  foo  // should not be merged
#else
  bar
#endif
#endif /* B */
EOF
diff expected out

rm -f a b expected out
