#!/usr/bin/env python3
import os
import re
import glob
import shutil
import subprocess
import sys
import difflib
from pathlib import Path

# ----------------------------
# Config you may want to tweak
# ----------------------------

MYSHELL_PATH = "../mysh"     # from test-cases folder
TEST_GLOB = "T_*.txt"
TIMEOUT_SEC = 10

# If your grading ignores whitespace/capitalization, set these to True
NORMALIZE_WHITESPACE = False   # if True: collapse whitespace runs to single spaces, strip lines
IGNORE_CASE = False            # if True: compare lowercased output

# Cleanup:
# Some tests create files. Safest approach: snapshot directory before each test
# and delete any NEW files after. This avoids guessing names.
CLEANUP_NEW_FILES = True

# If you know some files should always be preserved, add patterns here
PRESERVE_PATTERNS = [
    "T_*.txt",
    "T_*_result*.txt",
    "test_runner.py",
    "*.py",
    "*.md",
]

# If there are known junk files created by your shell that you always want removed,
# you can add them here (optional).
EXTRA_DELETE_PATTERNS = [
    # "output.txt",
]

# ----------------------------
# Helpers
# ----------------------------

def normalize_text(s: str) -> str:
    if IGNORE_CASE:
        s = s.lower()
    if NORMALIZE_WHITESPACE:
        # Normalize per-line: strip, collapse internal whitespace
        out_lines = []
        for line in s.splitlines():
            line = re.sub(r"\s+", " ", line.strip())
            out_lines.append(line)
        return "\n".join(out_lines) + ("\n" if s.endswith("\n") else "")
    return s

def read_file_text(path: Path) -> str:
    return path.read_text(errors="replace")

def snapshot_files(root: Path):
    # Snapshot regular files only (relative paths)
    files = set()
    for p in root.rglob("*"):
        if p.is_file():
            files.add(p.relative_to(root))
    return files

def matches_any_pattern(relpath: str, patterns):
    for pat in patterns:
        if Path(relpath).match(pat):
            return True
    return False

def cleanup_new_files(root: Path, before_set, after_set):
    new_files = sorted(after_set - before_set)
    deleted = []
    for rel in new_files:
        rel_str = str(rel)
        if matches_any_pattern(rel_str, PRESERVE_PATTERNS):
            continue
        target = root / rel
        try:
            target.unlink()
            deleted.append(rel_str)
        except IsADirectoryError:
            # shouldn't happen because we snapshot files, not dirs
            pass
        except FileNotFoundError:
            pass
    # Also delete any known extras (optional patterns)
    for pat in EXTRA_DELETE_PATTERNS:
        for p in root.glob(pat):
            if p.is_file():
                try:
                    p.unlink()
                    deleted.append(str(p))
                except FileNotFoundError:
                    pass
    return deleted

def run_test(mysh: Path, test_file: Path):
    # Run: ../mysh < T_xxx.txt
    # We'll feed the test file as stdin to avoid shell redirection
    try:
        with test_file.open("rb") as f:
            proc = subprocess.run(
                [str(mysh)],
                stdin=f,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                timeout=TIMEOUT_SEC,
            )
        out = proc.stdout.decode(errors="replace")
        err = proc.stderr.decode(errors="replace")
        return proc.returncode, out, err, None
    except subprocess.TimeoutExpired:
        return None, "", "", f"TIMEOUT after {TIMEOUT_SEC}s"
    except FileNotFoundError:
        return None, "", "", f"Could not find executable: {mysh}"

def find_expected_files(test_file: Path):
    # Supports:
    # T_name_result.txt
    # T_name_result2.txt, T_name_result3.txt, ...
    # Also supports T_name_result1.txt if you have it
    stem = test_file.stem  # "T_name"
    patterns = [
        f"{stem}_result.txt",
        f"{stem}_result*.txt",
    ]
    expected = []
    for pat in patterns:
        expected.extend(test_file.parent.glob(pat))
    # Deduplicate & sort (ensure _result.txt first if present)
    expected = sorted(set(expected), key=lambda p: (p.name != f"{stem}_result.txt", p.name))
    # Filter out the test file itself if glob catches it (it won't, but safe)
    expected = [p for p in expected if p.is_file() and p != test_file]
    return expected

def unified_diff(a: str, b: str, fromfile: str, tofile: str, max_lines=200):
    a_lines = a.splitlines(keepends=True)
    b_lines = b.splitlines(keepends=True)
    diff = list(difflib.unified_diff(a_lines, b_lines, fromfile=fromfile, tofile=tofile))
    if len(diff) > max_lines:
        diff = diff[:max_lines] + [f"... diff truncated ({len(diff)-max_lines} more lines) ...\n"]
    return "".join(diff)

# ----------------------------
# Main
# ----------------------------

def main():
    root = Path(__file__).resolve().parent   # test-cases/
    mysh = (root / MYSHELL_PATH).resolve()

    tests = sorted(root.glob(TEST_GLOB))
    if not tests:
        print(f"No tests found matching {TEST_GLOB} in {root}")
        return 2

    if not mysh.exists():
        print(f"ERROR: {mysh} not found. Build first? (make mysh)")
        return 2

    total = 0
    passed = 0
    failed_tests = []

    print(f"Running {len(tests)} tests using {mysh}")
    print("-" * 60)

    for test_file in tests:
        # Skip result files that also match T_*.txt (unlikely but just in case)
        if "_result" in test_file.stem:
            continue

        total += 1
        expected_files = find_expected_files(test_file)
        if not expected_files:
            print(f"[SKIP] {test_file.name} (no expected result files found)")
            continue

        before = snapshot_files(root) if CLEANUP_NEW_FILES else None

        rc, out, err, run_problem = run_test(mysh, test_file)

        after = snapshot_files(root) if CLEANUP_NEW_FILES else None
        deleted = cleanup_new_files(root, before, after) if CLEANUP_NEW_FILES else []

        if run_problem is not None:
            print(f"[FAIL] {test_file.name} - {run_problem}")
            failed_tests.append((test_file.name, run_problem))
            continue

        # Compare output against ANY acceptable expected file
        norm_out = normalize_text(out)
        best_diff = None
        best_expected_name = None
        matched = False

        for exp in expected_files:
            exp_text = read_file_text(exp)
            norm_exp = normalize_text(exp_text)

            if norm_out == norm_exp:
                matched = True
                best_expected_name = exp.name
                break

            # Keep a diff to show if nothing matches
            d = unified_diff(norm_exp, norm_out, fromfile=exp.name, tofile=f"{test_file.stem}_actual.txt")
            if best_diff is None or len(d) < len(best_diff):
                best_diff = d
                best_expected_name = exp.name

        if matched:
            passed += 1
            extra = f" (cleaned {len(deleted)} files)" if deleted else ""
            # If stderr exists but output matched, still show a warning
            if err.strip():
                print(f"[PASS] {test_file.name} matched {best_expected_name} but had stderr output (warning){extra}")
            else:
                print(f"[PASS] {test_file.name} matched {best_expected_name}{extra}")
        else:
            print(f"[FAIL] {test_file.name} (no expected output matched)")
            if err.strip():
                print("  stderr (first 20 lines):")
                for line in err.splitlines()[:20]:
                    print(f"    {line}")
            if best_diff:
                print("  diff vs closest expected:", best_expected_name)
                print(best_diff)
            failed_tests.append((test_file.name, f"Mismatch (closest: {best_expected_name})"))

        # Nonzero exit code can still be acceptable if output matches, but you may want to see it
        if rc not in (0, None) and matched:
            print(f"  note: return code was {rc}")

    print("-" * 60)
    print(f"Summary: {passed}/{total} passed")

    if failed_tests:
        print("\nFailed tests:")
        for name, reason in failed_tests:
            print(f"  - {name}: {reason}")
        return 1

    return 0

if __name__ == "__main__":
    sys.exit(main())