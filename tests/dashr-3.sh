mkdir a
mkdir b
if [ ./diffn -r -DA -DB a b >/dev/null 2>&1 ]; then
    echo "Failed to give error for -r -D without -o"
fi
mkdir out
if [ ./diffn -r -DA -DB a b -o out >/dev/null 2>&1 ]; then
    echo "Failed to give error for -r -D with already-existing -o directory"
fi
rmdir out ; echo "hello" > out
if [ ./diffn -r -DA -DB a b -o out >/dev/null 2>&1 ]; then
    echo "Failed to give error for -r -D with already-existing -o file"
fi
if [ $(cat out) != "hello" ]; then
    echo "Failed to suppress output where one path is not a directory"
fi

mkdir a/shared ; echo "hello" > b/shared
if [ ./diffn -r -DA -DB a b -o out >/dev/null 2>&1 ]; then
    echo "Failed to give error when shared file is both regular and directory"
fi
rm -rf out

rm -rf b ; echo "hello" > b
if [ ./diffn -r -DA -DB a b -o out >/dev/null 2>&1 ]; then
    echo "Failed to give error for -r -D where one path is not a directory"
fi
if [ -r out ]; then
    echo "Failed to suppress output where one path is not a directory"
fi

rm -rf a b out
