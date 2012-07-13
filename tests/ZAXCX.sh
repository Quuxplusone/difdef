cat >a <<EOF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void func1() {
    x += 1;
}

void func2() {
    x += 2;
}

EOF
cat >b <<EOF
#include <stdio.h>
#include <stdlib.h>

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
ab#include <stdio.h>
ab#include <stdlib.h>
a #include <string.h>
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

./diffn -DV1 -DV2 a b >out
cat >expected <<EOF
#include <stdio.h>
#include <stdlib.h>
#if defined(V1)
#include <string.h>
#endif /* V1 */

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
