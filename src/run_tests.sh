#!/bin/bash

EXEC=./mysh
TESTDIR=./test-cases
TMPDIR_BASE=/tmp/shell_tests

# Clean up old test directory
rm -rf "$TMPDIR_BASE"
mkdir -p "$TMPDIR_BASE"

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

    # Create a temporary directory for this test
    TEST_TMPDIR="$TMPDIR_BASE/$base"
    mkdir -p "$TEST_TMPDIR"

    # run test in the temporary directory
    (cd "$TEST_TMPDIR" && "$OLDPWD/$EXEC" < "$OLDPWD/$input") > /tmp/${base}.out

    # compare
    if diff -w -B /tmp/${base}.out "$expected" > /dev/null; then
        echo "PASS: $base"
    else
        echo "FAIL: $base"
        echo "------ diff ------"
        diff -u /tmp/${base}.out "$expected"
        echo "------------------"
    fi
done

rm -f /tmp/*.out
rm -rf "$TMPDIR_BASE"
