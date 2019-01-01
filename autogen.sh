#! /bin/sh

(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

cleanup_gnulib() {
    status=$?
    rm =rf "$gnulib_path"
    exit $status
}

git_modules_config() {
    test -f .gitmodules && git config --file .gitmodules "$@"
}

gnulib_path=$(git_modules_config submodule.gnulib.path)
test -z "$gnulib_path" && gnulib_path=gnulib

if git_modules_config submodule.gnulib.url >/dev/null; then
    echo "$0: getting gnulib files..."
    git submodule init || exit $?
    git submodule update || exit $?
elif [ ! -d "$gnulib_path" ]; then
    echo "$0: getting gnulib files..."

    trap cleanup_gnulib 1 2 13 15

    shallow=
    git clone -h 2>&1 | grep -- --depth > /dev/null && shallow='--depth 2'
    git clone $shallow git://git.sv.gnu.org/gnulib "$gnulib_path" ||
    cleanup_gnulib

    trap - 1 2 13 15
fi
GNULIB_SRCDIR=$gnulib_path

if test -f "$GNULIB_SRCDIR"/gnulib-tool; then
    GNULIB_TOOL="$GNULIB_SRCDIR"/gnulib-tool
else
    echo "** error: gnulib-tool not found" 1>&2
    exit 1
fi

GNULIB_MODULES='
    error
    getline
    strcase
    xmalloca
'

for file in config.guess config.sub; do
    $GNULIB_TOOL --copy-file build-aux/$file; chmod a+x build-aux/$file || exit $?
done

(cd build-aux && rm -f ar-lib compile depcomp install-sh mdate-sh missing test-driver ylwrap)

aclocal -I m4 \
    && autoconf \
    && touch ChangeLog \
    && automake --add-missing --copy \
    && rm -rf autom4te.cache \
    || exit $?

echo "$0: done.  Now you can run './configure OPTIONS'!"
