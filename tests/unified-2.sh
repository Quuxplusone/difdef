cat >a <<EOF
the
quick
brown
fox
jumped
over
the
lazy
dogs
EOF

cat >b <<EOF
the
quick
red
fox
jumped
all
the
way
over
the
lazy
cats
EOF

diff -U1 a b | tail -n +3 >expected
./difdef -U1 a b | tail -n +3 >out
diff expected out

diff -U1 b a | tail -n +3 >expected
./difdef -U1 b a | tail -n +3 >out
diff expected out

diff -U2 a b | tail -n +3 >expected
./difdef -U2 a b | tail -n +3 >out
diff expected out

diff -U2 b a | tail -n +3 >expected
./difdef -U2 b a | tail -n +3 >out
diff expected out

diff -U3 a b | tail -n +3 >expected
./difdef -U3 a b | tail -n +3 >out
diff expected out

diff -U3 b a | tail -n +3 >expected
./difdef -U3 b a | tail -n +3 >out
diff expected out

diff -U99 a b | tail -n +3 >expected
./difdef -U99 a b | tail -n +3 >out
diff expected out

diff -U99 b a | tail -n +3 >expected
./difdef -U99 b a | tail -n +3 >out
diff expected out

diff -U0 a b | tail -n +3 >expected
./difdef -U0 a b | tail -n +3 >out
diff expected out

diff -U0 b a | tail -n +3 >expected
./difdef -U0 b a | tail -n +3 >out
diff expected out

rm -f a b expected out
