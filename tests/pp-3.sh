cat >a <<EOF
uncommon
prefix
#if common
  foo  // should be merged
#endif
common
suffix
EOF
cat >b <<EOF
uncommon2
prefix2
#if common
  foo  // should be merged
#endif
common
suffix
EOF

./diffn -DA -DB a b >out
cat >expected <<EOF
#if defined(A)
uncommon
prefix
#endif /* A */
#if defined(B)
uncommon2
prefix2
#endif /* B */
#if common
  foo  // should be merged
#endif
common
suffix
EOF
diff expected out

rm -f a b expected out
