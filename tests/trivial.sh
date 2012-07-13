cat >a <<EOF
foo
bar
baz
EOF
cp a b

./diffn a b >out
cat >expected <<EOF
abfoo
abbar
abbaz
EOF
diff expected out

./diffn -DV1 -DV2 a b >out
diff a out

rm -f a b expected out
