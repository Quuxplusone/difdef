cat >a <<EOF
void foo() {
    return 0;
}
EOF
cat >b <<EOF
void foo() {
    printf("ello world!\n");
    return 0;
}
EOF
cat >c <<EOF
void foo() {
    printf("H");  /* oh yeah, need an H */
    printf("ello world!\n");
    return 0;
}
EOF

./diffn --ifdefs a b c >out
cat >expected <<EOF
void foo() {
#if defined(V3)
    printf("H");  /* oh yeah, need an H */
#endif /* V3 */
#if defined(V2) || defined(V3)
    printf("ello world!\n");
#endif /* V2 || V3 */
    return 0;
}
EOF
## This is not very attractive output, but I'm not sure how to
## objectively define "improvement" in this case.
diff expected out

rm -f a b c expected out
