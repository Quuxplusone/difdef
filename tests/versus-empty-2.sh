touch empty

./difdef empty empty >out
diff empty out

./difdef -DV1 -DV2 empty empty >out
diff empty out

rm -f empty out

mkdir a
mkdir b
touch a/empty
touch b/empty
echo "foo" > a/nonempty-v-empty
touch b/nonempty-v-empty
./difdef -r -DV1 -DV2 a b -o out
if [ ! -r out/empty ]; then
    echo "out/empty should exist, but doesn't"
fi
diff out/empty a/empty
cat >expected <<EOF
#ifdef V1
foo
#endif /* V1 */
EOF
diff out/nonempty-v-empty expected

rm -rf a b expected out
