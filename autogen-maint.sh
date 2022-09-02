#! /bin/bash
# autogen.sh wrapper for the author/maintainer of this package.
# Everyone else should run autogen.sh instead.

case `whoami` in
  root)
    echo "Bad boy! Drop out of the root account and try again."
    exit 1
    ;;

  michael)
    # Build system is x86_64 but default host is x86.
    # host=${host-"i686-pc-linux-gnu"}
    # CONFIGOPTS=${CONFIGOPTS-"--host=$host"}

    # Regenerate all files. May require Perl, XML tools, Internet
    # access, dead cat, magic spells, ...
    AUTOGENOPTS=${AUTOGENOPTS-"--enable-maintainer-mode"}
    ;;

  *)
    echo "Who are you? Run autogen.sh instead."
    exit 1
    ;;
esac

# Default compiler.
#export CC=${CC-"gcc -V4.1.2"}
export CC=${CC-"gcc"}

if $CC -v 2>&1 | grep -q -e '^tcc version' ; then
  # If preprocessor output is required use GCC.
  AUTOGENOPTS="$AUTOGENOPTS CPP=cpp"

  # Optimizations.
  if ! echo "$CFLAGS" | grep -q -e '-O[s1-3]'; then
    CFLAGS="$CFLAGS -g -b -bt 10"
  fi

  # Warnings.
  CFLAGS="$CFLAGS -Wimplicit-function-declaration"
                                        # func use before declaration
  CFLAGS="$CFLAGS -Wunsupported"        # unsupported GCC features
  CFLAGS="$CFLAGS -Wwrite-strings"	# char *foo = "blah";

elif $CC -v 2>&1 | grep -q -e '^gcc version [3-9]\.' ; then
  # Optimizations.
  if echo "$CFLAGS" | grep -q -e '-O[s1-3]'; then
    CFLAGS="$CFLAGS -fomit-frame-pointer -pipe"
  else
    CFLAGS="$CFLAGS -O0 -g -pipe"
  fi

  # Warnings.
  CFLAGS="$CFLAGS -Wchar-subscripts"	# array subscript has char type
  CFLAGS="$CFLAGS -Wcomment"		# nested comments
  CFLAGS="$CFLAGS -Wformat"		# printf format args mismatch
  CFLAGS="$CFLAGS -Wformat-y2k"		# two-digit year strftime format
  CFLAGS="$CFLAGS -Wformat-nonliteral"	# printf format cannot be checked
  CFLAGS="$CFLAGS -Wformat-security"	# printf (var); where user may
                                        # supply var
  CFLAGS="$CFLAGS -Wnonnull"		# function __attribute__ says
                                        # argument must be non-NULL
  CFLAGS="$CFLAGS -Wimplicit-int"	# func decl without a return type
  CFLAGS="$CFLAGS -Wimplicit-function-declaration"
                                        # func use before declaration
  CFLAGS="$CFLAGS -Wmain"		# wrong main() return type or args
  CFLAGS="$CFLAGS -Wmissing-braces"	# int a[2][2] = { 0, 1, 2, 3 };
  CFLAGS="$CFLAGS -Wparentheses"	# if if else, or sth like x <= y <=z
  CFLAGS="$CFLAGS -Wsequence-point"	# a = a++;
  CFLAGS="$CFLAGS -Wreturn-type"	# void return in non-void function
  CFLAGS="$CFLAGS -Wswitch"		# missing case in enum switch
  CFLAGS="$CFLAGS -Wtrigraphs"		# suspicious ??x
  CFLAGS="$CFLAGS -Wunused-function"	# defined static but not used
  CFLAGS="$CFLAGS -Wunused-label"
  CFLAGS="$CFLAGS -Wunused-parameter"
  CFLAGS="$CFLAGS -Wunused-variable"	# declared but not used
  CFLAGS="$CFLAGS -Wunused-value"	# return x, 0;
  CFLAGS="$CFLAGS -Wunknown-pragmas"    # #pragma whatever
  CFLAGS="$CFLAGS -Wfloat-equal"	# x == 1.000
  CFLAGS="$CFLAGS -Wundef"		# #undef X, #if X == 3
  CFLAGS="$CFLAGS -Wendif-labels"	# #endif BLAH
  #CFLAGS="$CFLAGS -Wshadow"		# int foo; bar () { int foo; ... }
  CFLAGS="$CFLAGS -Wpointer-arith"	# void *p = &x; ++p;
  CFLAGS="$CFLAGS -Wbad-function-cast"
  CFLAGS="$CFLAGS -Wcast-qual"		# const int * -> int *
  CFLAGS="$CFLAGS -Wcast-align"		# char * -> int *
  CFLAGS="$CFLAGS -Wwrite-strings"	# char *foo = "blah";
  CFLAGS="$CFLAGS -Wsign-compare"	# int foo; unsigned bar; foo < bar
  #CFLAGS="$CFLAGS -Wconversion"	# proto causes implicit type conversion
  CFLAGS="$CFLAGS -Wmissing-prototypes"	# global func not declared in header
  CFLAGS="$CFLAGS -Wmissing-declarations"
                                        # global func not declared in header
  CFLAGS="$CFLAGS -Wpacked"		# __attribute__((packed)) may hurt
  #CFLAGS="$CFLAGS -Wpadded"		# struct fields were padded, different
                                        # order may save space
  CFLAGS="$CFLAGS -Wnested-externs"	# extern declaration in function;
                                        # use header files, stupid!
  CFLAGS="$CFLAGS -Winline"		# inline function cannot be inlined
  CFLAGS="$CFLAGS -Wall -W"		# other useful warnings
  
  if ! echo "$CFLAGS" | grep -q -e '-O0' ; then
    CFLAGS="$CFLAGS -Wuninitialized"	# int i; return i;
    CFLAGS="$CFLAGS -Wstrict-aliasing"	# code may break C rules used for
                                        # optimization
  fi

  if $CC --version | grep -q -e '(GCC) [4-9]\.' ; then
    CFLAGS="$CFLAGS -Winit-self"	# int i = i;
    CFLAGS="$CFLAGS -Wdeclaration-after-statement"
                                        # int i; i = 1; int j;
    CFLAGS="$CFLAGS -Wmissing-include-dirs"
    CFLAGS="$CFLAGS -Wmissing-field-initializers"
                                        # int a[2][2] = { 0, 1 };
    if ! (echo "$CFLAGS" | grep -q -e '-O0') ; then
      CFLAGS="$CFLAGS -Wstrict-aliasing=2"
    fi
  fi

fi # GCC >= 3.x

export CFLAGS
export CXXFLAGS=`echo $CFLAGS | sed \
 -e s/-Wbad-function-cast// \
 -e s/-Wdeclaration-after-statement// \
 -e s/-Wimplicit-function-declaration// \
 -e s/-Wimplicit-int// \
 -e s/-Wmain// \
 -e s/-Wmissing-declarations// \
 -e s/-Wmissing-prototypes// \
 -e s/-Wnested-externs// \
 -e s/-Wnonnull// \
`

# autopoint extracts the gettext files from an ad-hoc CVS repository.
export CVS_SIGN_COMMITS=off
export CVS_VERIFY_CHECKOUTS=off

./autogen.sh $@ \
  $AUTOGENOPTS \
  PATH="$PATH" \
  CC="$CC" \
  CFLAGS="$CFLAGS"
