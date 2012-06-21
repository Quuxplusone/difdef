cat >a <<EOF
foo
bar
baz
EOF
touch b

./diffn a b >out
cat >expected <<EOF
a foo
a bar
a baz
EOF
diff expected out

./diffn b a >out
cat >expected <<EOF
 bfoo
 bbar
 bbaz
EOF
diff expected out

./diffn --ifdefs a b >out
cat >expected <<EOF
#if defined(V1)
foo
bar
baz
#endif /* V1 */
EOF
diff expected out

./diffn --ifdefs b a >out
cat >expected <<EOF
#if defined(V2)
foo
bar
baz
#endif /* V2 */
EOF
diff expected out

rm -f a b expected out
