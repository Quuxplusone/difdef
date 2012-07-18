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

./diffn --if=A --if=B a b >out
cat >expected <<EOF
#if A
uncommon
prefix
#endif /* A */
#if B
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
