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

./diffn -DA -DB a b >out
cat >expected <<EOF
#if defined(A)
#if a
  foo  // should not be merged
#endif
#endif /* A */
#if defined(B)
#ifdef a
  foo  // should not be merged
#else
  bar
#endif
#endif /* B */
EOF
diff expected out

rm -f a b expected out
