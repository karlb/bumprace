######## Process this file with autoconf to produce a configure script.
AC_INIT(README)

######## Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

######## Setup for automake
NAME="bumprace"
SDL_VERSION=1.1.5
AM_INIT_AUTOMAKE($NAME, 1.5.3)

######## Check for tools
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

######## Check for compiler environment
AC_C_CONST

######## Figure out which math library to use
case "$target" in
    *-*-mingw32*)
        MATHLIB=""
        ;;
    *-*-beos*)
        MATHLIB=""
        ;;
    *)
        MATHLIB="-lm"
	AC_CHECK_LIB(m, main, , AC_MSG_ERROR(Your system is missing libm (mathlib)!))
        ;;
esac
AC_SUBST(MATHLIB)

######## Check for SDL
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)

######## Check for other libs
AC_CHECK_LIB(jpeg, main, , AC_MSG_ERROR(Your system is missing libjpeg!))

#Commented out because of a bug in the SuSE 6.2
#AC_CHECK_LIB(png, main, , AC_MSG_ERROR(Your system is missing libpng!))

AC_CHECK_LIB(SDL_mixer, main, SOUND="-lSDL_mixer";CFLAGS="$CFLAGS -DSOUND")
AC_CHECK_LIB(SDL_image, main, , AC_MSG_ERROR(Your system is missing libSDL_image!))

######## Set compiler flags and libraries
CFLAGS="$CFLAGS $SDL_CFLAGS -DDATADIR=\\\"$datadir/bumprace\\\""
LIBS="$LIBS $SDL_LIBS -lSDL_image $SOUND -lz -lSDL"

######## Check for headers


######## Finally create all the generated files
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
data/gfx/Makefile
data/sound/Makefile
])
