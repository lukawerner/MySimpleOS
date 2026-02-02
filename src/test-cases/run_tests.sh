#!/bin/bash
set -e

EXEC=../mysh
TESTDIR=.
OUTDIR=/tmp/shell_tests_out

EXPECTED_DIR="src/test-cases"

if [[ "$(pwd)" != *"$EXPECTED_DIR" ]]; then
  echo "Refusing to run outside $EXPECTED_DIR"
  exit 1
fi


rm -rf "$OUTDIR"
rm -rf ./test ./testdir ./toto
mkdir -p "$OUTDIR"



echo "Running tests..."
echo "------------------------------"

for input in "$TESTDIR"/*.txt; do
    # skip expected output files
    if [[ "$input" == *"_result.txt" ]]; then
        continue
    fi

    base=$(basename "$input" .txt)
    expected="$TESTDIR/${base}_result.txt"

    if [[ ! -f "$expected" ]]; then
        echo "SKIP: $base (no expected output)"
        continue
    fi

    out="$OUTDIR/${base}.out"

    # IMPORTANT: run from test-cases directory so source/run can find files like set.txt
    "$EXEC" < "$input" > "$out"

    if diff -w -B "$out" "$expected" > /dev/null; then
        echo "PASS: $base"
    else
        echo "FAIL: $base"
        echo "------ diff ------"
        diff -u "$out" "$expected"
        echo "------------------"
    fi
done

rm -rf "$OUTDIR"
rm -rf ./test ./testdir ./toto
