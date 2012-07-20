cat >a <<EOF
#if A
#if B
#endif B
#endif A
EOF
cat >b <<EOF
#if A
#endif A
EOF

./difdef a b >out
cat >expected <<EOF
ab#if A
a #if B
a #endif B
ab#endif A
EOF
diff expected out

./difdef -DV1 -DV2 a b >out
cat >expected <<EOF
#if A
#ifdef V1
#if B
#endif B
#endif /* V1 */
#endif A
EOF
diff expected out

rm -f a b expected out
