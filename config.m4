dnl $Id$
dnl config.m4 for extension hprose
PHP_ARG_ENABLE(hprose, whether to enable hprose support,
[  --enable-hprose         Enable hprose support])

dnl Check whether the extension is enabled at all
if test "$PHP_HPROSE" != "no"; then
  dnl Finally, tell the build system about the extension and what files are needed
  PHP_NEW_EXTENSION(hprose, php_hprose.c hprose_common.c hprose_tags.c hprose_bytes_io.c hprose_class_manager.c hprose_writer.c hprose_raw_reader.c hprose_reader.c hprose_formatter.c, $ext_shared)
  PHP_SUBST(HPROSE_SHARED_LIBADD)
fi
