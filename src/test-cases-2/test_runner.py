#!/usr/bin/env python3
import argparse
import os
import re
import glob
import shutil
import subprocess
import sys
import difflib
from collections import Counter
from pathlib import Path

# ----------------------------
# Config you may want to tweak
# ----------------------------

MYSHELL_PATH = "../mysh"     # from test-cases folder
TEST_GLOB = "T_*.txt"
TIMEOUT_SEC = 10
MT_RUNS = 20                 # how many times to run MT tests

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

def is_mt_test(test_file: Path) -> bool:
    return test_file.stem.startswith("T_MT")

def text_line_counter(s: str) -> Counter:
    # MT tests are nondeterministic in ordering, so compare line multiplicities.
    return Counter(normalize_text(s).splitlines())

def counter_mismatch_score(expected: Counter, actual: Counter) -> int:
    keys = set(expected.keys()) | set(actual.keys())
    return sum(abs(expected.get(k, 0) - actual.get(k, 0)) for k in keys)

def summarize_counter_diff(expected: Counter, actual: Counter, max_items=20) -> str:
    keys = sorted(set(expected.keys()) | set(actual.keys()))
    diffs = []
    for k in keys:
        e = expected.get(k, 0)
        a = actual.get(k, 0)
        if e != a:
            diffs.append((abs(e - a), k, e, a))
    diffs.sort(reverse=True)
    lines = []
    for _, line, e, a in diffs[:max_items]:
        lines.append(f"    expected {e}, got {a}: {line!r}")
    if len(diffs) > max_items:
        lines.append(f"    ... {len(diffs) - max_items} more differing lines ...")
    return "\n".join(lines)

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

def parse_args():
    parser = argparse.ArgumentParser(
        description="Run COMP 310 shell tests. MT tests are validated by unordered line counts."
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=TIMEOUT_SEC,
        help=f"Per-test timeout in seconds (default: {TIMEOUT_SEC})",
    )
    parser.add_argument(
        "--mt-runs",
        type=int,
        default=MT_RUNS,
        help=f"Number of repeated runs for MT tests (default: {MT_RUNS})",
    )
    return parser.parse_args()

# ----------------------------
# Main
# ----------------------------

def main():
    global TIMEOUT_SEC, MT_RUNS
    args = parse_args()
    TIMEOUT_SEC = args.timeout
    MT_RUNS = args.mt_runs
    if TIMEOUT_SEC <= 0:
        print("ERROR: --timeout must be > 0")
        return 2
    if MT_RUNS <= 0:
        print("ERROR: --mt-runs must be > 0")
        return 2

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
    print(f"Config: timeout={TIMEOUT_SEC}s, mt-runs={MT_RUNS}")
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

        mt_mode = is_mt_test(test_file)
        runs = MT_RUNS if mt_mode else 1
        total_deleted = 0
        had_stderr = False
        nonzero_rc = []
        matched_all_runs = True
        first_failure_reason = None
        first_failure_details = ""
        matched_expected_name = None

        # Pre-read expected files for faster repeated checks
        expected_payloads = []
        for exp in expected_files:
            exp_text = read_file_text(exp)
            expected_payloads.append((exp, normalize_text(exp_text), text_line_counter(exp_text)))

        for run_idx in range(1, runs + 1):
            before = snapshot_files(root) if CLEANUP_NEW_FILES else None
            rc, out, err, run_problem = run_test(mysh, test_file)
            after = snapshot_files(root) if CLEANUP_NEW_FILES else None
            deleted = cleanup_new_files(root, before, after) if CLEANUP_NEW_FILES else []
            total_deleted += len(deleted)

            if err.strip():
                had_stderr = True
            if rc not in (0, None):
                nonzero_rc.append((run_idx, rc))

            if run_problem is not None:
                matched_all_runs = False
                first_failure_reason = f"{run_problem} (run {run_idx}/{runs})"
                break

            matched = False
            best_expected_name = None
            best_diff = None
            best_counter_delta = None
            best_counter_detail = ""

            if mt_mode:
                out_counter = text_line_counter(out)
                for exp, _, exp_counter in expected_payloads:
                    if out_counter == exp_counter:
                        matched = True
                        best_expected_name = exp.name
                        break
                    delta = counter_mismatch_score(exp_counter, out_counter)
                    if best_counter_delta is None or delta < best_counter_delta:
                        best_counter_delta = delta
                        best_expected_name = exp.name
                        best_counter_detail = summarize_counter_diff(exp_counter, out_counter)
            else:
                norm_out = normalize_text(out)
                for exp, norm_exp, _ in expected_payloads:
                    if norm_out == norm_exp:
                        matched = True
                        best_expected_name = exp.name
                        break
                    d = unified_diff(norm_exp, norm_out, fromfile=exp.name, tofile=f"{test_file.stem}_actual.txt")
                    if best_diff is None or len(d) < len(best_diff):
                        best_diff = d
                        best_expected_name = exp.name

            if matched:
                matched_expected_name = best_expected_name
                continue

            matched_all_runs = False
            first_failure_reason = f"Mismatch on run {run_idx}/{runs} (closest: {best_expected_name})"
            if mt_mode:
                first_failure_details = best_counter_detail
            else:
                first_failure_details = best_diff or ""
            if err.strip():
                stderr_preview = "\n".join(f"    {line}" for line in err.splitlines()[:20])
                first_failure_details = (first_failure_details + "\n  stderr (first 20 lines):\n" + stderr_preview).strip()
            break

        if matched_all_runs:
            passed += 1
            extra_cleanup = f" (cleaned {total_deleted} files)" if total_deleted else ""
            if mt_mode:
                mode_note = f" [MT unordered line-count match x{runs}]"
            else:
                mode_note = ""
            if had_stderr:
                print(f"[PASS] {test_file.name} matched {matched_expected_name}{mode_note} but had stderr output (warning){extra_cleanup}")
            else:
                print(f"[PASS] {test_file.name} matched {matched_expected_name}{mode_note}{extra_cleanup}")
            if nonzero_rc:
                rc_summary = ", ".join(f"run {i}: rc={rc}" for i, rc in nonzero_rc[:10])
                if len(nonzero_rc) > 10:
                    rc_summary += f", ... ({len(nonzero_rc) - 10} more)"
                print(f"  note: non-zero return codes observed: {rc_summary}")
        else:
            print(f"[FAIL] {test_file.name} - {first_failure_reason}")
            if first_failure_details:
                if mt_mode:
                    print("  line-count differences vs closest expected:")
                else:
                    print("  diff vs closest expected:")
                print(first_failure_details)
            failed_tests.append((test_file.name, first_failure_reason))

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
