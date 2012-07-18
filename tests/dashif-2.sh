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

./diffn --if=A --if=B a b -o out
cat >expected <<EOF
prefix
#if A
foo
#endif /* A */
bar
#if B
baz
#endif /* B */
EOF
diff expected out

./diffn --ifdef=A --ifdef=B a b -o out
cat >expected <<EOF
prefix
#ifdef A
foo
#endif /* A */
bar
#ifdef B
baz
#endif /* B */
EOF
diff expected out
./diffn -DA -DB a b -o out
diff expected out

rm -f a b expected out
