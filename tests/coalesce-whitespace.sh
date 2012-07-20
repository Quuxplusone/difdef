cat >a <<EOF

void foo();


void bar();

EOF
cat >b <<EOF

void foo();



void bar();

EOF

./difdef a b >out
cat >expected <<EOF
ab
abvoid foo();
ab
ab
 b
abvoid bar();
ab
EOF
diff --brief expected out

./difdef -DV1 -DV2 a b >out
diff --brief a out

cat >c <<EOF
void foo();


void baz();
void bar();
EOF

./difdef -DV1 -DV2 -DV3 a b c >out
cat >expected <<EOF
void foo();

#ifdef V3
void baz();
#endif /* V3 */
void bar();
EOF
diff --brief expected out

rm -f a b c expected out
