cat >a <<EOF
common
prefix
#if TWO
  foo  // should not be merged
#elif THREE
  bar  // should not be merged
#else
  baz  // should not be merged
#endif  // match, but can't merge
common
#if HRAIR  // should be merged
still common  // should be merged
#endif
suffix
EOF
cat >b <<EOF
common
prefix
#if ONE
  roo
#elif TWO
  foo  // should not be merged
#elif THREE
  bar  // should not be merged
#else
  baz  // should not be merged
#endif  // match, but can't merge
common
#if HRAIR  // should be merged
still common  // should be merged
#endif
suffix
EOF

./difdef a b >out
cat >expected <<EOF
abcommon
abprefix
a #if TWO
 b#if ONE
 b  roo
 b#elif TWO
ab  foo  // should not be merged
ab#elif THREE
ab  bar  // should not be merged
ab#else
ab  baz  // should not be merged
ab#endif  // match, but can't merge
abcommon
ab#if HRAIR  // should be merged
abstill common  // should be merged
ab#endif
absuffix
EOF
diff expected out

./difdef -DV1 -DV2 a b >out
cat >expected <<EOF
common
prefix
#ifdef V1
#if TWO
  foo  // should not be merged
#elif THREE
  bar  // should not be merged
#else
  baz  // should not be merged
#endif  // match, but can't merge
#endif /* V1 */
#ifdef V2
#if ONE
  roo
#elif TWO
  foo  // should not be merged
#elif THREE
  bar  // should not be merged
#else
  baz  // should not be merged
#endif  // match, but can't merge
#endif /* V2 */
common
#if HRAIR  // should be merged
still common  // should be merged
#endif
suffix
EOF
diff expected out

./difdef -DV2 -DV1 b a >out
cat >expected <<EOF
common
prefix
#ifdef V2
#if ONE
  roo
#elif TWO
  foo  // should not be merged
#elif THREE
  bar  // should not be merged
#else
  baz  // should not be merged
#endif  // match, but can't merge
#endif /* V2 */
#ifdef V1
#if TWO
  foo  // should not be merged
#elif THREE
  bar  // should not be merged
#else
  baz  // should not be merged
#endif  // match, but can't merge
#endif /* V1 */
common
#if HRAIR  // should be merged
still common  // should be merged
#endif
suffix
EOF
diff expected out

rm -f a b expected out
