dnl $Id$
dnl config.m4 for extension hprose
PHP_ARG_ENABLE(hprose, hprose support,
[  --enabled-hprose       Enable hprose support], [enabled_hprose="yes"])

dnl Check whether the extension is enabled at all
if test "$PHP_HPROSE" != "no"; then
  dnl Finally, tell the build system about the extension and what files are needed
  HPROSE_SOURCES="src/hprose_tags.c \
    src/hprose_bytes_io.c \
    src/hprose_class_manager.c"
  PHP_NEW_EXTENSION(hprose, php_hprose.c $HPROSE_SOURCES, $ext_shared)
  PHP_SUBST(HPROSE_SHARED_LIBADD)
fi
