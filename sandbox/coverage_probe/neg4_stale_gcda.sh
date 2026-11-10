#!/usr/bin/env bash
# Negative 4: stale .gcda must be removed before tests.
# Expected: rc=0, stale file removed, summary created.
set -u

REPO="${REPO:-$HOME/test/inobitec_study}"
BUILD="$REPO/build/coverage"
REPORT_DIR="$BUILD/probe/neg_stale"
STALE="$BUILD/probe/stale.gcda"

CMAKE="$(command -v cmake)" || { printf 'FAIL: cmake not found\n' >&2; exit 1; }
CTEST="$(command -v ctest)" || { printf 'FAIL: ctest not found\n' >&2; exit 1; }
GCOVR="$(command -v gcovr)" || { printf 'FAIL: gcovr not found\n' >&2; exit 1; }

rm -rf "$REPORT_DIR"
mkdir -p "$REPORT_DIR" "$(dirname "$STALE")"
: > "$STALE"
if [ ! -e "$STALE" ]; then
  printf 'FAIL: could not create %s\n' "$STALE" >&2
  exit 1
fi
printf 'created stale: %s\n' "$STALE"

"$CMAKE" \
  -DCOVERAGE_BACKEND=gcovr \
  -DCTEST_EXE="$CTEST" \
  -DBUILD_DIR="$BUILD" \
  -DSOURCE_DIR="$REPO" \
  -DREPORT_DIR="$REPORT_DIR" \
  -DCOVERAGE_UNION_EXE="$BUILD/tools/coverage_union/coverage_union" \
  -DEXPECTED_SOURCES="$BUILD/coverage_expected_sources.txt" \
  -DGCOVR_EXE="$GCOVR" \
  -P "$REPO/cmake/coverage.cmake"

rc=$?
printf 'stale-gcda rc=%s\n' "$rc"

status=0
if [ "$rc" -ne 0 ]; then
  printf 'FAIL: expected rc=0, got %s\n' "$rc" >&2
  status=1
fi

if [ -e "$STALE" ]; then
  printf 'FAIL: stale .gcda still exists\n' >&2
  status=1
else
  printf 'OK: stale .gcda removed\n'
fi

if [ -f "$REPORT_DIR/coverage_summary.txt" ]; then
  if grep -q 'files: 14/14 expected' "$REPORT_DIR/coverage_summary.txt"; then
    printf 'OK: summary reports 14/14 expected\n'
  else
    printf 'FAIL: summary does not report 14/14 expected\n' >&2
    status=1
  fi
else
  printf 'FAIL: summary not created\n' >&2
  status=1
fi

exit "$status"
