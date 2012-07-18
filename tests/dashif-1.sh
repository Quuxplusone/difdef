cat >a <<EOF
prefix
foo
bar
EOF

cat >b <<EOF
prefix
bar
baz
EOF

./diffn --if=HOST==A --if=HOST==B a b -o out
cat >expected <<EOF
prefix
#if HOST==A
foo
#endif /* HOST==A */
bar
#if HOST==B
baz
#endif /* HOST==B */
EOF
diff expected out

rm -f a b expected out
