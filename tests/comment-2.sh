cat >a <<EOF

#include <stdio.h>

int main()
{
    puts("hello world");
}
EOF

cat >b <<EOF

#include <stdio.h>
#include <stdlib.h>  /* extra header */

int main()
{
    puts("hello \\
world");
    return 0;
}
EOF

cat >c <<EOF

#include <stdio.h>

int main()
{
    puts("hello \\
cruel\\
world");
    return 0;
}
EOF

./diffn -DA -DB -DC a b c -o out
cat >expected <<EOF

#include <stdio.h>
#ifdef B
#include <stdlib.h>  /* extra header */
#endif /* B */

int main()
{
#ifdef A
    puts("hello world");
#endif /* A */
#ifdef B
    puts("hello \\
world");
#endif /* B */
#ifdef C
    puts("hello \\
cruel\\
world");
#endif /* C */
#if defined(B) || defined(C)
    return 0;
#endif /* B || C */
}
EOF
diff expected out

rm -f a b c expected out
