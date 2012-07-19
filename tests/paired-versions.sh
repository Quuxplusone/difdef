cat >a <<EOF
#include <stdio.h>

void the_wheel();
void the_telegraph();

EOF
cp a b
cat >c <<EOF
#include <stdio.h>

void the_wheel();
#if BACKWARDS_COMPATIBILITY
void the_telegraph();
#else
void the_cell_phone();
#endif

EOF
cp c d

./diffn -Da a -Db b -Dc c -Dd d -o out
cat >expected <<EOF
#include <stdio.h>

void the_wheel();
#if defined(a) || defined(b)
void the_telegraph();
#endif /* a || b */
#if defined(c) || defined(d)
#if BACKWARDS_COMPATIBILITY
void the_telegraph();
#else
void the_cell_phone();
#endif
#endif /* c || d */

EOF
diff expected out

./diffn -Da a -Dc c -Db b -Dd d -o out
diff expected out

rm -f a b c d expected out
