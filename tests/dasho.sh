cat >a <<EOF
foo
bar
baz
EOF
cp a b

cat >expected <<EOF
abfoo
abbar
abbaz
EOF
./difdef a b -o - >out
diff expected out
rm -f out
./difdef a b -o out
diff expected out

rm -f out
./difdef -DV1 -DV2 a b -o out
diff a out

rm -f a b expected out
