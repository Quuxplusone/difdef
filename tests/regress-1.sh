cat >a <<EOF
foo
bar
EOF
cat >b <<EOF
bar
foo
EOF

./diffn a b -o out
cat >expected <<EOF
a foo
abbar
 bfoo
EOF
diff expected out

./diffn -DV1 -DV2 a b -o out
cat >expected <<EOF
#if defined(V1)
foo
#endif /* V1 */
bar
#if defined(V2)
foo
#endif /* V2 */
EOF
diff expected out

rm -f a b expected out
