cat >a <<EOF
void
threehalves()
{
    x += 1.5;
}
foo
EOF
cat >b <<EOF
void
func1()
{
    x += 1;
}
foo
void
threehalves()
{
    x += 1.5;
}
foo
void
func2()
{
    x += 2;
}

EOF

./difdef a b >out
cat >expected <<EOF
 bvoid
 bfunc1()
 b{
 b    x += 1;
 b}
 bfoo
abvoid
abthreehalves()
ab{
ab    x += 1.5;
ab}
abfoo
 bvoid
 bfunc2()
 b{
 b    x += 2;
 b}
 b
EOF
diff expected out

rm -f a b expected out
