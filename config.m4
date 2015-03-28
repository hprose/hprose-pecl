dnl $Id$
dnl config.m4 for extension hprose
PHP_ARG_ENABLE(hprose, hprose support,
[  --enable-hprose         Enable hprose support], [enable_hprose="yes"])

dnl Check whether the extension is enabled at all
if test "$PHP_HPROSE" != "no"; then
  dnl Finally, tell the build system about the extension and what files are needed
  HPROSE_SOURCES="src/hprose_tags.c \
    src/hprose_bytes_io.c \
    src/hprose_class_manager.c \
    src/hprose_writer.c \
    src/hprose_raw_reader.c \
    src/hprose_reader.c \
    src/hprose_formatter.c \
    src/hprose_result_mode.c \
    src/hprose_filter.c \
    src/hprose_client.c \
    src/hprose_service.c"
  PHP_NEW_EXTENSION(hprose, hprose.c $HPROSE_SOURCES, $ext_shared)
  PHP_ADD_INCLUDE([$ext_srcdir/include])
  PHP_SUBST(HPROSE_SHARED_LIBADD)
fi
