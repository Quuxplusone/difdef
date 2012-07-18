cat >a <<EOF
common
prefix
/* This is a comment
   with a single quote ' in it
   but it doesn't open a character
   literal */
uncommon
suffix
EOF

cat >b <<EOF
common
prefix
/* This is a comment
   with a single quote ' in it
   but it doesn't open a character
   literal */
uncommon2
suffix
EOF

./diffn -DA -DB a b -o out
cat >expected <<EOF
common
prefix
/* This is a comment
   with a single quote ' in it
   but it doesn't open a character
   literal */
#if defined(A)
uncommon
#endif /* A */
#if defined(B)
uncommon2
#endif /* B */
suffix
EOF
diff expected out

rm -f a b expected out
