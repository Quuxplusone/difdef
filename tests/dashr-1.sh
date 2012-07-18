mkdir a
cat >a/a.txt <<EOF
foo
bar
EOF
mkdir b
cat >b/a.txt <<EOF
foo
bar
EOF
./diffn -r -DA -DB a b -o c
diff -r a c

cat >b/b-only.txt <<EOF
baz
EOF
./diffn -r --if=A --if=B a b -o out
cp -r a expected
cat >expected/b-only.txt <<EOF
#if B
baz
#endif /* B */
EOF
diff -r expected out

rm -rf a b c expected out