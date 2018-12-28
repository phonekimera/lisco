## Process this file with automake to produce Makefile.in -*-Makefile-*-

aclocaldir = @aclocaldir@
aclocal_DATA = isc-posix.m4

# Generate this list with
# find . -type f -name '*.m4' -printf '%f\n'|sort |fmt |tr '\012' @ \
#   |sed 's/@$/%/;s/@/ \\@/g' |tr @% '\012\012'
EXTRA_DIST = README \
c-bs-a.m4 getline.m4 isc-posix.m4 ssize_t.m4
