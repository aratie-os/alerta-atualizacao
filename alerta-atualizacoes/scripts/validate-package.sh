#!/bin/sh
set -eu

package=${1:?usage: validate-package.sh path/to/package.deb}
temporary_directory=$(mktemp -d)
trap 'rm -rf -- "$temporary_directory"' EXIT HUP INT TERM

dpkg-deb --info "$package"
dpkg-deb --contents "$package"

contents=$(dpkg-deb --contents "$package")
for required_path in \
    ./usr/bin/system-upgrade \
    ./etc/xdg/autostart/system-upgrade.desktop \
    ./usr/share/applications/system-upgrade.desktop \
    ./etc/sudoers.d/system-upgrade
do
    printf '%s\n' "$contents" | awk '{print $6}' | grep -Fx "$required_path" >/dev/null
done

if printf '%s\n' "$contents" | grep -F 'system-upgrade-exe' >/dev/null; then
    echo "legacy privileged helper was packaged" >&2
    exit 1
fi

printf '%s\n' "$contents" \
    | grep -E '^-r--r----- root/root +[0-9]+ .*\./etc/sudoers.d/system-upgrade$' >/dev/null

dependencies=$(dpkg-deb --field "$package" Depends)
if printf '%s\n' "$dependencies" \
    | grep -E '(^|, )(yad|snapd|plasma-discover-backend-snap|libsnapd)' >/dev/null; then
    echo "forbidden runtime dependency found: $dependencies" >&2
    exit 1
fi

dpkg-deb --extract "$package" "$temporary_directory/root"
visudo -cf "$temporary_directory/root/etc/sudoers.d/system-upgrade"
