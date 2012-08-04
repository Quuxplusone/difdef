mkdir a b c
for d in {1..10}; do
    mkdir a/d$d b/d$d c/d$d
    for i in {1..100}; do
        if [[ $(($i % 3)) != 0 ]]; then
            echo "file $i" > a/d$d/f$i
        fi
        if [[ $(($i % 5)) != 0 ]]; then
            echo "file $i" > b/d$d/f$i
        fi
        if [[ $(($i % 7)) != 0 ]]; then
            echo "file $i" > c/d$d/f$i
        fi
    done
done

## We don't know what the output should look like, but it shouldn't
## run out of file handles.
function cleanup() {
    echo "difdef unexpectedly exited with an error!"
    rm -rf a b c out
}
trap cleanup ERR
ulimit -n 120
./difdef -r -DA -DB -DC a b c -o out
rm -rf a b c out

mkdir a b c
for d in {1..100}; do
    mkdir a/d$d b/d$d c/d$d
    for i in {1..10}; do
        mkdir a/d$d/d$i b/d$d/d$i  c/d$d/d$i
        if [[ $(($i % 3)) != 0 ]]; then
            echo "file $i" > a/d$d/d$i/f0
        fi
        if [[ $(($i % 5)) != 0 ]]; then
            echo "file $i" > b/d$d/d$i/f0
        fi
        if [[ $(($i % 7)) != 0 ]]; then
            echo "file $i" > c/d$d/d$i/f0
        fi
    done
done

ulimit -n 120
./difdef -r -DA -DB -DC a b c -o out
rm -rf a b c out