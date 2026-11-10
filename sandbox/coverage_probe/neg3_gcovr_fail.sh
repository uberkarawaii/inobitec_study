#!/usr/bin/env bash
# Negative 3: gcovr failure.
# Expected: non-zero rc, no cobertura report.
set -u

REPO="${REPO:-$HOME/test/inobitec_study}"
BUILD="$REPO/build/coverage"
REPORT_DIR="$BUILD/probe/neg_gcovr"

CMAKE="$(command -v cmake)" || { printf 'FAIL: cmake not found\n' >&2; exit 1; }
CTEST="$(command -v ctest)" || { printf 'FAIL: ctest not found\n' >&2; exit 1; }

rm -rf "$REPORT_DIR"
mkdir -p "$REPORT_DIR"

"$CMAKE" \
  -DCOVERAGE_BACKEND=gcovr \
  -DCTEST_EXE="$CTEST" \
  -DBUILD_DIR="$BUILD" \
  -DSOURCE_DIR="$REPO" \
  -DREPORT_DIR="$REPORT_DIR" \
  -DCOVERAGE_UNION_EXE="$BUILD/tools/coverage_union/coverage_union" \
  -DEXPECTED_SOURCES="$BUILD/coverage_expected_sources.txt" \
  -DGCOVR_EXE="/no/such/gcovr" \
  -P "$REPO/cmake/coverage.cmake"

rc=$?
printf 'gcovr-failure rc=%s\n' "$rc"

status=0
if [ "$rc" -eq 0 ]; then
  printf 'FAIL: expected non-zero rc\n' >&2
  status=1
else
  printf 'OK: expected non-zero rc\n'
fi

if [ -f "$REPORT_DIR/coverage.cobertura.xml" ]; then
  printf 'FAIL: cobertura report should not exist\n' >&2
  status=1
else
  printf 'OK: cobertura report not created\n'
fi

exit "$status"
