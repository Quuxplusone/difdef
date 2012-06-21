cat >a <<EOF

void func1() {
    x += 1;
}

void func2() {
    x += 2;
}

EOF
cat >b <<EOF

void func1() {
    x += 1;
}

void threehalves() {
    x += 1.5;
}

void func2() {
    x += 2;
}

EOF

./diffn a b >out
cat >expected <<EOF
ab
abvoid func1() {
ab    x += 1;
ab}
ab
 bvoid threehalves() {
 b    x += 1.5;
 b}
 b
abvoid func2() {
ab    x += 2;
ab}
ab
EOF
diff expected out

./diffn --ifdefs a b >out
cat >expected <<EOF

void func1() {
    x += 1;
}

#if defined(V2)
void threehalves() {
    x += 1.5;
}
#endif /* V2 */

void func2() {
    x += 2;
}

EOF
diff expected out

rm -f a b expected out
