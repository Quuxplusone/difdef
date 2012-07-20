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

./difdef -DHOSTA -DHOSTB a b -o out
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

./difdef --if HOST==A --if HOST==B a b -o expected
./difdef --if=HOST==A --if=HOST==B a b -o out
diff expected out
./difdef -DHOST=A -DHOST=B a b -o out
diff expected out
./difdef -D HOST=A -D HOST=B a b -o out
diff expected out

rm -f a b expected out
