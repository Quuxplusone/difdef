cat >a <<EOF
#if A
#if B
#endif B
#endif A
EOF
cat >b <<EOF
#if B
#endif B
EOF

./diffn a b >out
cat >expected <<EOF
a #if A
ab#if B
ab#endif B
a #endif A
EOF
diff expected out

./diffn -DV1 -DV2 a b >out
cat >expected <<EOF
#if defined(V1)
#if A
#if B
#endif B
#endif A
#endif /* V1 */
#if defined(V2)
#if B
#endif B
#endif /* V2 */
EOF
diff expected out

rm a b expected out
