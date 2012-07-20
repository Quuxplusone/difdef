cat >a <<EOF
int common;
#pragma mark This isn't a character literal
int different1;
#pragma mark Something else
EOF

cat >b <<EOF
int common;
#pragma mark This isn't a character literal
int different2;
#pragma mark Something else
EOF

./difdef a b >out
cat >expected <<EOF
abint common;
ab#pragma mark This isn't a character literal
a int different1;
 bint different2;
ab#pragma mark Something else
EOF
diff expected out

./difdef -DA -DB a b >out
cat >expected <<EOF
int common;
#pragma mark This isn't a character literal
#ifdef A
int different1;
#endif /* A */
#ifdef B
int different2;
#endif /* B */
#pragma mark Something else
EOF
diff expected out