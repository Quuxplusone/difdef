cat >a <<EOF
foo
EOF
cat >b <<EOF
foo
bar
EOF
cat >c <<EOF
baz
EOF

./diffn --simple -DA a -DB b -DC c >out
cat >expected <<EOF
#if defined(A) || defined(B)
foo
#ifdef B
bar
#endif /* B */
#endif /* A || B */
#ifdef C
baz
#endif /* C */
EOF
diff expected out

./diffn --complex -DA a -DB b -DC c >out
cat >expected <<EOF
#if defined(A) || defined(B)
foo
#ifdef B
bar
#endif /* B */
#else /* C */
baz
#endif /* A || B || C */
EOF
diff expected out

rm -f a b c expected out
