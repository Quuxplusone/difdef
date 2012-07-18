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
#if defined(B)
#include <stdlib.h>  /* extra header */
#endif /* B */

int main()
{
#if defined(A)
    puts("hello world");
#endif /* A */
#if defined(B)
    puts("hello \\
world");
#endif /* B */
#if defined(C)
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
