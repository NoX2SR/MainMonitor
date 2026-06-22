#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build/firmware}"
ARDUINO_HARDWARE="${ARDUINO_HARDWARE:-/usr/share/arduino/hardware}"
ARDUINO_TOOLS="${ARDUINO_TOOLS:-/usr/share/arduino/hardware/tools/avr}"
FQBN="${FQBN:-arduino:avr:uno}"

for command in arduino-builder avr-g++ avr-gcc; do
  command -v "${command}" >/dev/null 2>&1 || {
    echo "Missing ${command}. Install arduino-builder, arduino-core-avr, gcc-avr, and avr-libc." >&2
    exit 1
  }
done

mkdir -p "${BUILD_DIR}"
arduino-builder \
  -compile \
  -hardware "${ARDUINO_HARDWARE}" \
  -tools "${ARDUINO_TOOLS}" \
  -fqbn "${FQBN}" \
  -warnings all \
  -build-path "${BUILD_DIR}" \
  -libraries "${ROOT_DIR}" \
  "${ROOT_DIR}/MainMonitor.ino"

echo "MainMonitor firmware is in ${BUILD_DIR}."
