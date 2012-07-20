cat >a <<EOF
void
one() { }

void
two() { }
EOF
cat >b <<EOF
void
one() { }

void
threehalves() { }

void
two() { }
EOF

cat >expected <<EOF
void
one() { }

#ifdef B
void
threehalves() { }
#endif /* B */

void
two() { }
EOF

./difdef -DA -DB a b >out
diff expected out

./difdef -DB -DA b a >out
diff expected out

rm -f a b expected out
