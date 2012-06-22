cat >a <<EOF
#ifndef FOO
 #define FOO

#include <stdio.h>

int foo(void);

#endif /* FOO */
EOF
cat >b <<EOF

#ifndef FOO
 #define FOO

#pragma once

#include <stdio.h>

int foo(void);
int bar(void);

#endif /* FOO */
EOF

./diffn -DA -DB a b >out
cat >expected <<EOF
#ifndef FOO
 #define FOO

#if defined(B)
#pragma once
#endif /* B */

#include <stdio.h>

int foo(void);
#if defined(B)
int bar(void);
#endif /* B */

#endif /* FOO */
EOF
diff expected out

./diffn -DB -DA b a >out
diff expected out

rm -f a b expected out
