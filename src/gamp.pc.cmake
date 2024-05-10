prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib@LIB_SUFFIX@
includedir=${prefix}/include/gamp

Name: gamp
Description: Graphics Audio Multimedia and Processing Library
Version: @gamp_VERSION_LONG@

Libs: -L${libdir} -lgamp
Cflags: -I${includedir}
