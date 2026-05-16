#!/usr/bin/env bash
# Rebuild csound.node with cmake-js and the environment / CMake cache variables
# that CMakeLists.txt expects (avoids: Csound not found, missing CsoundAC paths,
# and "NODE_ADDON_API_INCLUDE ... is required" when not using a global node-addon-api).
#
# Usage:
#   ./rebuild.bash [CMake -D flags...] [other cmake-js options...]
#
# CMake-style -DVAR=value is rewritten to cmake-js --CDVAR=value. Do not pass raw
# -D... to cmake-js: there, -D means "debug build" and breaks option parsing.
#
# Optional env (override auto-detection):
#   CSOUND_ROOT          — install prefix for your Csound 7 (bin/, lib/, include/ or Frameworks)
#   CSOUND_AC_ROOT       — csound-ac repo root (must contain CsoundAC/CsoundProducer.hpp)
#   CSOUNDAC_LIBRARY     — explicit path to libCsoundAC (.dylib / .so / .dll / .lib)
#   NW_RUNTIME           — e.g. nw (default: node for stock Node headers)
#   NW_RUNTIME_VERSION   — NW.js SDK version when NW_RUNTIME=nw

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${ROOT}"

# --- Split "$@": CMake -D / --define -> cmake-js --CD... ; rest -> cmake-js passthrough ---
USER_CMAKE_DEFS=()
PASSTHROUGH=()
while (($# > 0)); do
  case "$1" in
    -D)
      if [[ -z "${2:-}" ]]; then
        echo "rebuild.bash: missing argument after -D" >&2
        exit 1
      fi
      if [[ "$2" =~ ^([A-Za-z_][A-Za-z0-9_]*)=(.*)$ ]]; then
        USER_CMAKE_DEFS+=(--CD"${BASH_REMATCH[1]}=${BASH_REMATCH[2]}")
        if [[ "${BASH_REMATCH[1]}" == CSOUND_AC_ROOT ]]; then
          export CSOUND_AC_ROOT="${BASH_REMATCH[2]}"
        fi
      else
        echo "rebuild.bash: expected NAME=value after -D, got: $2" >&2
        exit 1
      fi
      shift 2
      ;;
    -D[A-Za-z_]*=*)
      _pair="${1#-D}"
      if [[ "${_pair}" =~ ^([A-Za-z_][A-Za-z0-9_]*)=(.*)$ ]]; then
        USER_CMAKE_DEFS+=(--CD"${BASH_REMATCH[1]}=${BASH_REMATCH[2]}")
        if [[ "${BASH_REMATCH[1]}" == CSOUND_AC_ROOT ]]; then
          export CSOUND_AC_ROOT="${BASH_REMATCH[2]}"
        fi
      else
        PASSTHROUGH+=("$1")
      fi
      shift
      ;;
    --define=*)
      _pair="${1#--define=}"
      if [[ "${_pair}" =~ ^([A-Za-z_][A-Za-z0-9_]*)=(.*)$ ]]; then
        USER_CMAKE_DEFS+=(--CD"${BASH_REMATCH[1]}=${BASH_REMATCH[2]}")
        if [[ "${BASH_REMATCH[1]}" == CSOUND_AC_ROOT ]]; then
          export CSOUND_AC_ROOT="${BASH_REMATCH[2]}"
        fi
      else
        PASSTHROUGH+=("$1")
      fi
      shift
      ;;
    *)
      PASSTHROUGH+=("$1")
      shift
      ;;
  esac
done
if ((${#PASSTHROUGH[@]} > 0)); then
  set -- "${PASSTHROUGH[@]}"
else
  set --
fi

# --- node-addon-api include (required by CMake when CMAKE_JS_INC is unset) ---
if [[ -f "${ROOT}/native/package.json" ]]; then
  if [[ ! -d "${ROOT}/native/node_modules/node-addon-api" ]]; then
    (cd "${ROOT}/native" && npm install --no-audit --no-fund)
  fi
  # Use include_dir, not deprecated include (the latter is the string "<path>" with literal quote chars).
  export NODE_ADDON_API_INCLUDE="$(
    cd "${ROOT}/native" && node -e "const p=require('path'); const n=require('node-addon-api'); process.stdout.write(p.resolve(process.cwd(), n.include_dir));"
  )"
else
  if command -v node >/dev/null 2>&1; then
    _napi="$(
      node -e "try{const p=require('path');const n=require('node-addon-api');process.stdout.write(p.resolve(process.cwd(),n.include_dir));}catch(e){}" 2>/dev/null || true
    )"
    if [[ -n "${_napi}" ]]; then
      export NODE_ADDON_API_INCLUDE="${_napi}"
    fi
  fi
fi

if [[ -z "${NODE_ADDON_API_INCLUDE:-}" ]]; then
  echo "Could not resolve node-addon-api include path." >&2
  echo "Add native/package.json + run (cd native && npm install), or: npm install -g node-addon-api" >&2
  exit 1
fi

# --- csound-ac tree (CMake does not read CSOUND_AC_ROOT from the environment) ---
if [[ -z "${CSOUND_AC_ROOT:-}" ]]; then
  for c in "${ROOT}/../csound-ac" "${HOME}/csound-ac" "${HOME}/src/csound-ac"; do
    if [[ -f "${c}/CsoundAC/CsoundProducer.hpp" ]]; then
      CSOUND_AC_ROOT="$(cd "${c}" && pwd)"
      export CSOUND_AC_ROOT
      break
    fi
  done
fi

if [[ -z "${CSOUND_AC_ROOT:-}" ]]; then
  echo "Set CSOUND_AC_ROOT to your csound-ac repository root (directory containing CsoundAC/)." >&2
  echo "Example:  ./rebuild.bash -DCSOUND_AC_ROOT=\$HOME/csound-ac" >&2
  exit 1
fi

# --- optional explicit libCsoundAC (otherwise CMake searches under CSOUND_AC_ROOT) ---
CMAKE_EXTRAS=()
CMAKE_EXTRAS+=(--CDCSOUND_AC_ROOT="${CSOUND_AC_ROOT}")

if [[ -n "${CSOUNDAC_LIBRARY:-}" ]]; then
  CMAKE_EXTRAS+=(--CDCSOUNDAC_LIBRARY="${CSOUNDAC_LIBRARY}")
else
  # Best-effort: pick a built libCsoundAC under the csound-ac tree
  _lib=""
  case "$(uname -s)" in
    Darwin)
      # Each find must not fail the subshell under "set -e" when a directory is missing.
      _lib="$(
        {
          find "${CSOUND_AC_ROOT}/build-macos" -maxdepth 1 -name 'libCsoundAC*.dylib' -type f 2>/dev/null || true
          find "${CSOUND_AC_ROOT}" \( -path '*/build*/CsoundAC/libCsoundAC*.dylib' -o -path '*/build-macos/CsoundAC/libCsoundAC*.dylib' \) -type f 2>/dev/null || true
          find "${CSOUND_AC_ROOT}/dist/csound-ac/lib" -maxdepth 1 -name 'libCsoundAC*.dylib' -type f 2>/dev/null || true
        } | head -1
      )"
      ;;
    Linux)
      _lib="$(
        {
          find "${CSOUND_AC_ROOT}/build-linux" -maxdepth 1 -name 'libCsoundAC.so*' -type f 2>/dev/null || true
          find "${CSOUND_AC_ROOT}" \( -path '*/build*/CsoundAC/libCsoundAC.so*' -o -path '*/build-linux/CsoundAC/libCsoundAC.so*' \) -type f 2>/dev/null || true
          find "${CSOUND_AC_ROOT}/dist/csound-ac/lib" -maxdepth 1 -name 'libCsoundAC.so*' -type f 2>/dev/null || true
        } | head -1
      )"
      ;;
    MINGW* | MSYS* | CYGWIN*)
      _lib="$(find "${CSOUND_AC_ROOT}" -path '*/build*/CsoundAC/*CsoundAC*.dll' -type f 2>/dev/null | head -1 || true)"
      ;;
  esac
  if [[ -n "${_lib}" ]]; then
    CMAKE_EXTRAS+=(--CDCSOUNDAC_LIBRARY="${_lib}")
    echo "Using libCsoundAC: ${_lib}"
  fi
fi

# --- Csound 7 prefix: CMake reads CSOUND_ROOT from the environment; also pass cache hint for cmake-js ---
if [[ -n "${CSOUND_ROOT:-}" ]]; then
  export CSOUND_ROOT
  CMAKE_EXTRAS+=(--CDCSOUND_ROOT_HINT="${CSOUND_ROOT}")
  echo "Using CSOUND_ROOT=${CSOUND_ROOT}"
fi

# User -D / --define last so they override defaults above
if ((${#USER_CMAKE_DEFS[@]} > 0)); then
  CMAKE_EXTRAS+=("${USER_CMAKE_DEFS[@]}")
fi

echo "NODE_ADDON_API_INCLUDE=${NODE_ADDON_API_INCLUDE}"
echo "CSOUND_AC_ROOT=${CSOUND_AC_ROOT}"
echo "Running cmake-js rebuild..." >&2

# Avoid "${ARRAY[@]}" on empty arrays with set -u (bash can treat [@] as unbound).
if command -v cmake-js >/dev/null 2>&1; then
  exec cmake-js rebuild \
    ${NW_RUNTIME:+--runtime "${NW_RUNTIME}"} \
    ${NW_RUNTIME_VERSION:+--runtime-version "${NW_RUNTIME_VERSION}"} \
    ${CMAKE_EXTRAS[@]+"${CMAKE_EXTRAS[@]}"} \
    "$@"
else
  exec npx --yes cmake-js rebuild \
    ${NW_RUNTIME:+--runtime "${NW_RUNTIME}"} \
    ${NW_RUNTIME_VERSION:+--runtime-version "${NW_RUNTIME_VERSION}"} \
    ${CMAKE_EXTRAS[@]+"${CMAKE_EXTRAS[@]}"} \
    "$@"
fi
