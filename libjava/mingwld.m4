AC_DEFUN([CHECK_FOR_BROKEN_MINGW_LD],
[
AC_MSG_CHECKING(whether 'ld' is at least 2.13)
LD_PROG=`$CC --print-prog-name=ld`
LD_VERSION=`$LD_PROG --version`
LD_VERSION_MAJOR=`echo "$LD_VERSION" | head -1 | cut -d '.' -f 1 | cut -d ' ' -f 4`
LD_VERSION_MINOR=`echo "$LD_VERSION" | head -1 | cut -d '.' -f 2`
if expr "$LD_VERSION_MAJOR" \> 2 > /dev/null; then
  LD_OK="ok"
else
  if expr "$LD_VERSION_MAJOR" = 2 && expr "$LD_VERSION_MINOR" \>= 13 > /dev/null; then
    LD_OK="ok"
  fi
fi
if test "x$LD_OK" != x; then
  AC_MSG_RESULT([yes; major=$LD_VERSION_MAJOR, minor=$LD_VERSION_MINOR])
else
  AC_MSG_RESULT([no; major=$LD_VERSION_MAJOR, minor=$LD_VERSION_MINOR])
  AC_MSG_WARN([ld <2.13 detected; enabling JV_LINKER_CANNOT_8BYTE_ALIGN_STATICS hack...])
  AC_DEFINE(JV_LINKER_CANNOT_8BYTE_ALIGN_STATICS, 1,
            [Indicate that linker is not able to 8-byte align static data])
fi])
