cat >a <<EOF
foo
bar
baz
EOF
touch b

./difdef a b >out
cat >expected <<EOF
a foo
a bar
a baz
EOF
diff expected out

./difdef b a >out
cat >expected <<EOF
 bfoo
 bbar
 bbaz
EOF
diff expected out

./difdef -DV1 -DV2 a b >out
cat >expected <<EOF
#ifdef V1
foo
bar
baz
#endif /* V1 */
EOF
diff expected out

./difdef -DV1 -DV2 b a >out
cat >expected <<EOF
#ifdef V2
foo
bar
baz
#endif /* V2 */
EOF
diff expected out

rm -f a b expected out
