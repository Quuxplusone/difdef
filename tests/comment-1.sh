cat >a <<EOF
/* the quick
   brown fox */
EOF
cat >b <<EOF
/* the quick
   brown fox */
EOF

./diffn a b -o out
cat >expected <<EOF
ab/* the quick
ab   brown fox */
EOF
diff expected out

./diffn -DA -DB a b -o out
diff a out

cat >b <<EOF
/* the fast
   brown fox */
EOF

./diffn a b -o out
cat >expected <<EOF
a /* the quick
 b/* the fast
ab   brown fox */
EOF
diff expected out

./diffn -DA -DB a b -o out
cat >expected <<EOF
#ifdef A
/* the quick
   brown fox */
#endif /* A */
#ifdef B
/* the fast
   brown fox */
#endif /* B */
EOF
diff expected out

cat >b <<EOF
/* the quick
   red fox */
EOF

./diffn a b -o out
cat >expected <<EOF
ab/* the quick
a    brown fox */
 b   red fox */
EOF
diff expected out

./diffn -DA -DB a b -o out
cat >expected <<EOF
#ifdef A
/* the quick
   brown fox */
#endif /* A */
#ifdef B
/* the quick
   red fox */
#endif /* B */
EOF
diff expected out

rm -f a b expected out
