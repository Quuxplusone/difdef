cat >a <<EOF
match1
#if bar
match2
#endif bar
EOF
cat >b <<EOF
#if foo
match1
#endif foo
match2
EOF

./diffn a b >out
cat >expected <<EOF
 b#if foo
abmatch1
a #if bar
 b#endif foo
abmatch2
a #endif bar
EOF
diff expected out

./diffn -DV1 -DV2 a b >out
cat >expected <<EOF
#if defined(V1)
match1
#if bar
match2
#endif bar
#endif /* V1 */
#if defined(V2)
#if foo
match1
#endif foo
match2
#endif /* V2 */
EOF
diff expected out

rm -f a b expected out
