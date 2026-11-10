#!/usr/bin/env bash
# Phase 0 probe for Linux coverage: gcov/gcovr behavior.
# Usage: bash run_probe.sh [repo-root]
# Default repo root: $HOME/test/inobitec_study

set -u

REPO_ROOT="${1:-$HOME/test/inobitec_study}"
PROBE_DIR="$REPO_ROOT/sandbox/coverage_probe_work"

fail() {
    printf 'FAIL: %s\n' "$*" >&2
    exit 1
}

[ -d "$REPO_ROOT" ] || fail "repo root not found: $REPO_ROOT"
command -v gcc >/dev/null 2>&1 || fail "gcc not found in PATH"
command -v gcov >/dev/null 2>&1 || fail "gcov not found in PATH"
command -v gcovr >/dev/null 2>&1 || fail "gcovr not found in PATH"

printf '=== tools ===\n'
gcc --version | head -n 1
gcov --version | head -n 2
gcovr --version | head -n 1
printf 'repo root : %s\n' "$REPO_ROOT"
printf 'probe dir : %s\n\n' "$PROBE_DIR"

rm -rf "$PROBE_DIR"
mkdir -p "$PROBE_DIR/common" "$PROBE_DIR/t1_probe" "$PROBE_DIR/tests/unit" "$PROBE_DIR/build"
cd "$PROBE_DIR" || fail "cannot cd to $PROBE_DIR"

printf '#pragma once\nint probe_abs(int x);\n' > common/probe_math.h
printf '#include "probe_math.h"\nint probe_abs(int x) {\n    if (x < 0)\n        return -x;\n    return x;\n}\n' > common/probe_math.c
cat > t1_probe/main.c <<'EOF'
#include <stdio.h>
#include "probe_math.h"
int main(void) {
    printf("%d\n", probe_abs(-3));
    printf("%d\n", probe_abs(4));
    return 0;
}
EOF
printf '#include "probe_math.h"\nint main(void) { return probe_abs(1) == 1 ? 0 : 1; }\n' > tests/unit/probe_unit.c

printf '=== build (--coverage -O0 -g) ===\n'
gcc --coverage -O0 -g -Icommon -c common/probe_math.c -o build/probe_math.o || fail "compile probe_math.c"
gcc --coverage -O0 -g -Icommon -c t1_probe/main.c -o build/t1_main.o || fail "compile main.c"
gcc --coverage -O0 -g -Icommon -c tests/unit/probe_unit.c -o build/probe_unit.o || fail "compile probe_unit.c"
gcc --coverage build/t1_main.o build/probe_math.o -o build/t1_probe || fail "link t1_probe"
gcc --coverage build/probe_unit.o build/probe_math.o -o build/probe_unit || fail "link probe_unit"
printf 'built\n\n'

printf '=== run ===\n'
./build/t1_probe || fail "run t1_probe"
./build/probe_unit || fail "run probe_unit"
printf 'ran\n\n'

printf '=== gcovr: relative filters ===\n'
gcovr --root "$PWD" --cobertura probe.cobertura.xml --cobertura-pretty \
    --filter '^common/probe_math\.c$' --filter '^t1_probe/main\.c$' \
    build
rc=$?
printf 'gcovr rc=%s\n' "$rc"

if [ -f probe.cobertura.xml ]; then
    printf '\n=== <class> lines ===\n'
    grep -n '<class ' probe.cobertura.xml || true
    printf '\n=== branch/condition attrs ===\n'
    grep -n 'condition-coverage\|branch=' probe.cobertura.xml | head -n 20 || true
    printf '\n=== first <line> entries ===\n'
    grep -n '<line ' probe.cobertura.xml | head -n 20 || true
    printf '\n=== tests/unit occurrences ===\n'
    grep -c 'tests/unit' probe.cobertura.xml || true
else
    printf 'probe.cobertura.xml was not created\n'
fi

if [ "$rc" -ne 0 ]; then
    printf '\n=== gcovr verbose (tail) ===\n'
    gcovr -v --root "$PWD" --cobertura probe_verbose.cobertura.xml \
        --filter '^common/probe_math\.c$' --filter '^t1_probe/main\.c$' \
        build > gcovr_verbose.log 2>&1 || true
    tail -n 60 gcovr_verbose.log || true
fi

if [ -f probe.cobertura.xml ] && ! grep -q '<class ' probe.cobertura.xml; then
    printf '\n=== gcovr: absolute filters ===\n'
    gcovr --root "$PWD" --cobertura probe_absfilter.cobertura.xml --cobertura-pretty \
        --filter "$PWD/common/probe_math\\.c$" --filter "$PWD/t1_probe/main\\.c$" \
        build
    printf 'gcovr abs-filter rc=%s\n' "$?"
    if [ -f probe_absfilter.cobertura.xml ]; then
        grep -n '<class ' probe_absfilter.cobertura.xml || true
    fi
fi

printf '\n=== CP1251 source ===\n'
printf '// проверка кодировки\nint probe_enc(void) { return 1; }\n' | iconv -f UTF-8 -t CP1251 > common/probe_enc.c
printf 'int probe_enc(void);\nint main(void) { return probe_enc() == 1 ? 0 : 1; }\n' > tests/unit/enc_main.c
gcc --coverage -O0 -g -c common/probe_enc.c -o build/probe_enc.o || fail "compile probe_enc.c"
gcc --coverage -O0 -g -c tests/unit/enc_main.c -o build/enc_main.o || fail "compile enc_main.c"
gcc --coverage build/probe_enc.o build/enc_main.o -o build/probe_enc || fail "link probe_enc"
./build/probe_enc || fail "run probe_enc"

printf '\n--- gcovr without --source-encoding ---\n'
gcovr --root "$PWD" --cobertura enc_default.xml --filter '^common/probe_enc\.c$' build
printf 'rc=%s\n' "$?"
printf '\n--- gcovr with --source-encoding cp1251 ---\n'
gcovr --root "$PWD" --source-encoding cp1251 --cobertura enc_cp1251.xml --filter '^common/probe_enc\.c$' build
printf 'rc=%s\n' "$?"

printf '\n--- enc_default.xml vs enc_cp1251.xml ---\n'
cmp -s enc_default.xml enc_cp1251.xml
printf 'encoding diff rc=%s\n' "$?"

printf '\n=== artifacts ===\n'
find "$PROBE_DIR" -maxdepth 2 -type f | sort

printf '\n=== what to send back ===\n'
printf '%s\n' \
    '- full output of this script' \
    '- <class filename=...> lines (path style)' \
    '- branch/condition lines, if any' \
    '- tests/unit occurrences (expected 0)' \
    '- CP1251 rc and error text for both runs'
