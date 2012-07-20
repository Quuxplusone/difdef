cat >a <<EOF
common
prefix
#if TWO
foo
baz
#endif // TWO
common
suffix
EOF
cat >b <<EOF
common
prefix
#if TWO
foo
bar
baz
#endif // TWO
common
suffix
EOF

./difdef -DV1 -DV2 a b >out
cat >expected <<EOF
common
prefix
#if TWO
foo
#ifdef V2
bar
#endif /* V2 */
baz
#endif // TWO
common
suffix
EOF
diff expected out

./difdef -DV2 -DV1 b a >out
diff expected out

rm -f a b expected out
