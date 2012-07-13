cat >a <<EOF
void foo() {
    return 0;
}
EOF
cat >b <<EOF
void foo() {
    printf("Hello world!");

    return 0;
}
EOF
cat >c <<EOF
void foo() {
    printf("Hello world!");
    printf("\n");  /* oh yeah, need a newline */

    return 0;
}
EOF

./diffn -DV1 -DV2 -DV3 a b c >out
cat >expected <<EOF
void foo() {
#if defined(V2) || defined(V3)
    printf("Hello world!");
#if defined(V3)
    printf("\n");  /* oh yeah, need a newline */
#endif /* V3 */
#endif /* V2 || V3 */

    return 0;
}
EOF
diff expected out

rm -f a b c expected out
