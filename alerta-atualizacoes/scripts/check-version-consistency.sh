#!/bin/sh
set -eu

script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_directory=$(dirname -- "$script_directory")
cd "$project_directory"

debian_version=$(dpkg-parsechangelog -S Version)
cmake_version=$(sed -nE \
    's/^[[:space:]]*project\(system-upgrade VERSION ([^ ]+) LANGUAGES CXX\)[[:space:]]*$/\1/p' \
    CMakeLists.txt)
appstream_version=$(sed -nE \
    's/.*<release version="([^"]+)".*/\1/p' \
    data/metainfo/io.github.aratie_os.system_upgrade.metainfo.xml | sed -n '1p')

if [ -z "$cmake_version" ] || [ -z "$appstream_version" ]; then
    echo "Could not read every declared project version" >&2
    exit 1
fi

if [ "$debian_version" != "$cmake_version" ] || \
   [ "$debian_version" != "$appstream_version" ]; then
    echo "Version mismatch:" >&2
    echo "  Debian:   $debian_version" >&2
    echo "  CMake:    $cmake_version" >&2
    echo "  AppStream: $appstream_version" >&2
    exit 1
fi

echo "Version consistency OK: $debian_version"

if [ "$#" -eq 0 ]; then
    exit 0
fi

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 [vX.Y.Z]" >&2
    exit 2
fi

release_tag=$1
if ! printf '%s\n' "$release_tag" | grep -Eq '^v[0-9]+\.[0-9]+\.[0-9]+$'; then
    echo "Invalid release tag format: $release_tag" >&2
    exit 1
fi

tag_version=${release_tag#v}
if [ "$tag_version" != "$debian_version" ]; then
    echo "Release tag $release_tag does not match Debian version $debian_version" >&2
    exit 1
fi

echo "Release tag matches package version: $release_tag"
