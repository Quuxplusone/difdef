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

./diffn -DHOSTA -DHOSTB a b -o out
cat >expected <<EOF
prefix
#ifdef HOSTA
foo
#endif /* HOSTA */
bar
#ifdef HOSTB
baz
#endif /* HOSTB */
EOF
diff expected out

./diffn --if HOST==A --if HOST==B a b -o expected
./diffn --if=HOST==A --if=HOST==B a b -o out
diff expected out
./diffn -DHOST=A -DHOST=B a b -o out
diff expected out
./diffn -D HOST=A -D HOST=B a b -o out
diff expected out

rm -f a b expected out
