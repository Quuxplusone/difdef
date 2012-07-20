cat >a <<EOF
int prefix;
#pragma unknown /* This is
                   a fake pragma */
int suffix;
EOF

cat >b <<EOF
int prefix;
#pragma unknown /* This is
                   a faker pragma */
int suffix;
EOF

./diffn -DA a -DB b >out
cat >expected <<EOF
int prefix;
#ifdef A
#pragma unknown /* This is
                   a fake pragma */
#endif /* A */
#ifdef B
#pragma unknown /* This is
                   a faker pragma */
#endif /* B */
int suffix;
EOF
diff expected out

rm -f a b expected out
