cat >a <<EOF
common
prefix
#if a
  foo  // should not be merged
#endif
common
suffix
EOF
cat >b <<EOF
common
prefix
#ifdef a
  foo  // should not be merged
#else
  bar
#endif
common
suffix
EOF

./diffn -DA -DB a b >out
cat >expected <<EOF
common
prefix
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
common
suffix
EOF
diff expected out

rm -f a b expected out
