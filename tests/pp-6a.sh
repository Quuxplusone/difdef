## This test case verifies that we don't accidentally merge two versions
## of an #if where one of them is unfinished at the point where another
## one is terminated by an #endif.
## It's the same as pp-6.sh except that the common prefix/suffix includes
## nested #ifs which *are* safe to merge.
cat >a <<EOF
#if COMMON  // should merge
common
#else
prefix
#if ONE  // can't merge
foo
#endif ONE
#endif /* COMMON SUFFIX */
more common suffix
EOF
cat >b <<EOF
#if COMMON  // should merge
common
#else
prefix
#if ONE  // can't merge
foo
bar
baz
#endif ONE
#endif /* COMMON SUFFIX */
more common suffix
EOF
cat >c <<EOF
#if COMMON  // should merge
common
#else
prefix
#if ONE  // can't merge
foo
#endif ONE_A
bar
#if TWO
quux
#endif TWO
baz
#endif /* COMMON SUFFIX */
more common suffix
EOF

./difdef -DV1 -DV2 -DV3 a b c >out
cat >expected <<EOF
#if COMMON  // should merge
common
#else
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
#endif /* COMMON SUFFIX */
more common suffix
EOF
diff expected out

rm -f a b c expected out
