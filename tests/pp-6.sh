## This test case verifies that we don't accidentally merge two versions
## of an #if where one of them is unfinished at the point where another
## one is terminated by an #endif.
cat >a <<EOF
common
prefix
#if ONE  // can't merge
foo
#endif ONE
EOF
cat >b <<EOF
common
prefix
#if ONE  // can't merge
foo
bar
baz
#endif ONE
EOF
cat >c <<EOF
common
prefix
#if ONE  // can't merge
foo
#endif ONE_A
bar
#if TWO
quux
#endif TWO
baz
EOF

./diffn -DV1 -DV2 -DV3 a b c >out
cat >expected <<EOF
common
prefix
#ifdef V1
#if ONE  // can't merge
foo
#endif ONE
#endif /* V1 */
#ifdef V2
#if ONE  // can't merge
foo
bar
baz
#endif ONE
#endif /* V2 */
#ifdef V3
#if ONE  // can't merge
foo
#endif ONE_A
bar
#if TWO
quux
#endif TWO
baz
#endif /* V3 */
EOF
diff expected out

rm -f a b c expected out
