#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$(mktemp -d "${TMPDIR:-/tmp}/main-monitor-tests.XXXXXX")"
trap 'rm -rf "${BUILD_DIR}"' EXIT

COMMON=(-std=c++11 -Wall -Wextra -Werror -O0 -g --coverage
  -I "${ROOT_DIR}/tests/fakes"
  -I "${ROOT_DIR}/SensorHandler")

g++ "${COMMON[@]}" \
  "${ROOT_DIR}/tests/fakes/Arduino.cpp" \
  "${ROOT_DIR}/SensorHandler/SensorHandler.cpp" \
  "${ROOT_DIR}/tests/test_sensor_handler.cpp" \
  -o "${BUILD_DIR}/test_sensor_handler"

g++ "${COMMON[@]}" \
  "${ROOT_DIR}/tests/fakes/Arduino.cpp" \
  "${ROOT_DIR}/tests/test_main_monitor.cpp" \
  -o "${BUILD_DIR}/test_main_monitor"

for test_binary in test_sensor_handler test_main_monitor; do
  "${BUILD_DIR}/${test_binary}"
  echo "PASS ${test_binary}"
done

coverage_line() {
  local object="$1"
  local source="$2"
  (cd "${BUILD_DIR}" && gcov -b -c "${object}" 2>&1) | awk -v source="${source}" '
    $0 ~ "^File .*" source { selected = 1; next }
    selected && /^Lines executed:/ { print source ": " substr($0, 16); exit }
  '
}

echo "Production line coverage:"
coverage_line "${BUILD_DIR}/test_sensor_handler-SensorHandler.gcno" "SensorHandler.cpp"
coverage_line "${BUILD_DIR}/test_main_monitor-test_main_monitor.gcno" "MainMonitor.ino"
