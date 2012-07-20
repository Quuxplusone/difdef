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

./difdef a b >out
cat >expected <<EOF
a #if A
ab#if B
ab#endif B
a #endif A
EOF
diff expected out

./difdef -DV1 -DV2 a b >out
cat >expected <<EOF
#ifdef V1
#if A
#if B
#endif B
#endif A
#endif /* V1 */
#ifdef V2
#if B
#endif B
#endif /* V2 */
EOF
diff expected out

rm -f a b expected out
