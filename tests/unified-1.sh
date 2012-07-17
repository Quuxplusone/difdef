cat >a <<EOF
the
quick
brown
fox
jumped
quickly
over
the
lazy
dogs
EOF

cat >b <<EOF
first
the
quick
brown
fox
jumped
over
the
lazy
dogs
and
then
went
home
EOF

diff -U1 a b | tail -n +3 >expected
./diffn -U1 a b | tail -n +3 >out
diff expected out

diff -U1 b a | tail -n +3 >expected
./diffn -U1 b a | tail -n +3 >out
diff expected out

diff -U2 a b | tail -n +3 >expected
./diffn -U2 a b | tail -n +3 >out
diff expected out

diff -U2 b a | tail -n +3 >expected
./diffn -U2 b a | tail -n +3 >out
diff expected out

diff -U3 a b | tail -n +3 >expected
./diffn -U3 a b | tail -n +3 >out
diff expected out

diff -U3 b a | tail -n +3 >expected
./diffn -U3 b a | tail -n +3 >out
diff expected out

diff -U99 a b | tail -n +3 >expected
./diffn -U99 a b | tail -n +3 >out
diff expected out

diff -U99 b a | tail -n +3 >expected
./diffn -U99 b a | tail -n +3 >out
diff expected out

diff -U0 a b | tail -n +3 >expected
./diffn -U0 a b | tail -n +3 >out
diff expected out

diff -U0 b a | tail -n +3 >expected
./diffn -U0 b a | tail -n +3 >out
diff expected out

rm -f a b expected out
