for i in a b a/shared b/shared a/a-only b/b-only; do
    mkdir $i
done
cat >a/shared.txt <<EOF
shared by a and b
EOF
cp a/shared.txt b/shared.txt
cat >a/shared/also-shared.txt <<EOF
also shared by a and b
but
different
between them
EOF
cat >b/shared/also-shared.txt <<EOF
also shared by a and b
but
somewhat different
between them
EOF
cat >a/a-only/just-a.txt <<EOF
just in a
EOF

./diffn -r -DA -DB a b -o out

cp -r a expected
mkdir expected/b-only
cat >expected/shared/also-shared.txt <<EOF
also shared by a and b
but
#if defined(A)
different
#endif /* A */
#if defined(B)
somewhat different
#endif /* B */
between them
EOF
cat >expected/a-only/just-a.txt <<EOF
#if defined(A)
just in a
#endif /* A */
EOF

diff -r expected out
if [ ! -d out/b-only ]; then
    echo "out/b-only doesn't exist as a directory!"
fi

rm -rf a b expected out