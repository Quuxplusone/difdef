cat >a <<EOF
foo
bar
EOF
cat >b <<EOF
bar
foo
EOF

./difdef a b -o out
cat >expected <<EOF
a foo
abbar
 bfoo
EOF
diff expected out

./difdef -DV1 -DV2 a b -o out
cat >expected <<EOF
#ifdef V1
foo
#endif /* V1 */
bar
#ifdef V2
foo
#endif /* V2 */
EOF
diff expected out

rm -f a b expected out
