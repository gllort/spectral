# AX_FLAGS_SAVE
# -------------
AC_DEFUN([AX_FLAGS_SAVE],
[
   saved_LIBS="${LIBS}"
   saved_CC="${CC}"
   saved_CFLAGS="${CFLAGS}"
   saved_CXXFLAGS="${CXXFLAGS}"
   saved_CPPFLAGS="${CPPFLAGS}"
   saved_LDFLAGS="${LDFLAGS}"
])


# AX_FLAGS_RESTORE
# ----------------
AC_DEFUN([AX_FLAGS_RESTORE],
[
   LIBS="${saved_LIBS}"
   CC="${saved_CC}"
   CFLAGS="${saved_CFLAGS}"
   CXXFLAGS="${saved_CXXFLAGS}"
   CPPFLAGS="${saved_CPPFLAGS}"
   LDFLAGS="${saved_LDFLAGS}"
])


# AX_FIND_INSTALLATION
# --------------------
AC_DEFUN([AX_FIND_INSTALLATION],
[
	AC_REQUIRE([AX_SELECT_BINARY_TYPE])

	dnl Search for home directory
	AC_MSG_CHECKING([for $1 installation])
    for home_dir in [$2 "not found"]; do
        if test -d "$home_dir/$BITS" ; then
            home_dir="$home_dir/$BITS"
            break
        elif test -d "$home_dir" ; then
            break
        fi
    done
	AC_MSG_RESULT([$home_dir])
	$1_HOME="$home_dir"
	if test "$$1_HOME" = "not found" ; then
		$1_HOME=""
	else

		dnl Did the user passed a headers directory to check first?
		AC_ARG_WITH([$3-headers],
			AC_HELP_STRING(
				[--with-$3-headers@<:@=ARG@:>@],
				[Specify location of include files for package $3]
			),
			[ForcedHeaders="$withval"],
			[ForcedHeaders=""]
		)

		dnl Search for includes directory
		AC_MSG_CHECKING([for $1 includes directory])

		if test "${ForcedHeaders}" = "" ; then
			for incs_dir in [$$1_HOME/include$BITS $$1_HOME/include "not found"] ; do
				if test -d "$incs_dir" ; then
					break
				fi
			done
		else
			for incs_dir in [${ForcedHeaders} "not found"] ; do
				if test -d "$incs_dir" ; then
					break
				fi
			done
		fi

		AC_MSG_RESULT([$incs_dir])
		$1_INCLUDES="$incs_dir"
		if test "$$1_INCLUDES" = "not found" ; then
			AC_MSG_ERROR([Unable to find header directory for package $3. Check option --with-$3-headers.])
		else
			$1_CFLAGS="-I$$1_INCLUDES"
			$1_CXXFLAGS="-I$$1_INCLUDES"
			$1_CPPFLAGS="-I$$1_INCLUDES"
		fi

		dnl Did the user passed a headers directory to check first?
		AC_ARG_WITH([$3-libs],
			AC_HELP_STRING(
				[--with-$3-libs@<:@=ARG@:>@],
				[Specify location of library files for package $3]
			),
			[ForcedLibs="$withval"],
			[ForcedLibs=""]
		)

		dnl Search for libs directory
		AC_MSG_CHECKING([for $1 libraries directory])
		if test "${ForcedLibs}" = "" ; then
			for libs_dir in [$$1_HOME/lib$BITS $$1_HOME/lib "not found"] ; do
				if test -d "$libs_dir" ; then
					break
				fi
			done
		else
			for libs_dir in [${ForcedLibs} "not found"] ; do
				if test -d "$libs_dir" ; then
					break
				fi
			done
		fi

		AC_MSG_RESULT([$libs_dir])
		$1_LIBSDIR="$libs_dir"
		if test "$$1_LIBSDIR" = "not found" ; then
			AC_MSG_ERROR([Unable to find library directory for package $3. Check option --with-$3-libs.])
		else
       $1_LDFLAGS="-L$$1_LIBSDIR"
       if test -d "$$1_LIBSDIR/shared" ; then
          $1_SHAREDLIBSDIR="$$1_LIBSDIR/shared"
       else
          $1_SHAREDLIBSDIR=$$1_LIBSDIR
       fi
		fi
	fi

	dnl Everything went OK?
	if test "$$1_HOME" != "" -a "$$1_INCLUDES" != "" -a "$$1_LIBSDIR" != "" ; then
		$1_INSTALLED="yes"

		AC_SUBST($1_HOME)
		AC_SUBST($1_INCLUDES)

    AC_SUBST($1_CFLAGS)
    AC_SUBST($1_CXXFLAGS)
    AC_SUBST($1_CPPFLAGS)

    AC_SUBST($1_LDFLAGS)
    AC_SUBST($1_SHAREDLIBSDIR)
    AC_SUBST($1_LIBSDIR)

    dnl Update the default variables so the automatic checks will take into account the new directories
    CFLAGS="$CFLAGS $$1_CFLAGS"
    CXXFLAGS="$CXXFLAGS $$1_CXXFLAGS"
    CPPFLAGS="$CPPFLAGS $$1_CPPFLAGS"
    LDFLAGS="$LDFLAGS $$1_LDFLAGS"
	else	
		$1_INSTALLED="no"
	fi
])


# AX_CHECK_POINTER_SIZE
# ---------------------
AC_DEFUN([AX_CHECK_POINTER_SIZE],
[
      AC_TRY_RUN(
         [
            int main()
            {
               return sizeof(void *)*8;
            }
         ],
         [ POINTER_SIZE="0" ],
         [ POINTER_SIZE="$?"]
      )
])


# AX_SELECT_BINARY_TYPE
# ---------------------
# Check the binary type the user wants to build and verify whether it can be successfully built
AC_DEFUN([AX_SELECT_BINARY_TYPE],
[
	AC_ARG_WITH(binary-type,
		AC_HELP_STRING(
			[--with-binary-type@<:@=ARG@:>@],
			[choose the binary type between: 32, 64, default @<:@default=default@:>@]
		),
		[Selected_Binary_Type="$withval"],
		[Selected_Binary_Type="default"]
	)

	if test "$Selected_Binary_Type" != "default" -a "$Selected_Binary_Type" != "32" -a "$Selected_Binary_Type" != "64" ; then
		AC_MSG_ERROR([--with-binary-type: Invalid argument '$Selected_Binary_Type'. Valid options are: 32, 64, default.])
	fi

	C_compiler="$CC"
	CXX_compiler="$CXX"

	AC_LANG_SAVE([])
	m4_foreach([language], [[C], [C++]], [
		AC_LANG_PUSH(language)

		AC_CACHE_CHECK(
			[for $_AC_LANG_PREFIX[]_compiler compiler default binary type], 
			[[]_AC_LANG_PREFIX[]_ac_cv_compiler_default_binary_type],
			[
				AX_CHECK_POINTER_SIZE
				Default_Binary_Type="$POINTER_SIZE"
				[]_AC_LANG_PREFIX[]_ac_cv_compiler_default_binary_type="$Default_Binary_Type""-bit"
			]
		)

		if test "$Default_Binary_Type" != "32" -a "$Default_Binary_Type" != 64 ; then
			[]_AC_LANG_PREFIX[]_PRESENT="no"
                        msg="language compiler '$_AC_LANG_PREFIX[]_compiler' seems not to be installed in the system.
Please verify there is a working installation of the language compiler '$_AC_LANG_PREFIX[]_compiler'."
			if test "language" == "C" ; then
				AC_MSG_ERROR($msg)
			else 
                        	AC_MSG_WARN($msg)
			fi
		else
			[]_AC_LANG_PREFIX[]_PRESENT="yes"
		fi

		if test "$Selected_Binary_Type" = "default" ; then
			Selected_Binary_Type="$Default_Binary_Type"
		fi

		if test "$Selected_Binary_Type" != "$Default_Binary_Type" -a "$[]_AC_LANG_PREFIX[]_PRESENT" = "yes" ; then

			force_bit_flags="-m32 -q32 -32 -maix32 -m64 -q64 -64 -maix64 none"

			AC_MSG_CHECKING([for $_AC_LANG_PREFIX[]_compiler compiler flags to build a $Selected_Binary_Type-bit binary])
			for flag in [$force_bit_flags]; do
				old_[]_AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS"
				[]_AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS $flag"

				AX_CHECK_POINTER_SIZE()
				if test "$POINTER_SIZE" = "$Selected_Binary_Type" ; then
					AC_MSG_RESULT([$flag])
					break
				else
					[]_AC_LANG_PREFIX[]FLAGS="$old_[]_AC_LANG_PREFIX[]FLAGS"
					if test "$flag" = "none" ; then
						AC_MSG_RESULT([unknown])
						AC_MSG_NOTICE([${Selected_Binary_Type}-bit binaries not supported])
						AC_MSG_ERROR([Please use '--with-binary-type' to select an appropriate binary type.])

					fi
				fi
			done

		fi
		AC_LANG_POP(language)
	])
	AC_LANG_RESTORE([])
	BITS="$Selected_Binary_Type"
])


# AX_PROG_FFT
# -----------
AC_DEFUN([AX_PROG_FFT],
[
  AX_FLAGS_SAVE()

  dnl There are unresolved dependencies with fftw3
  AC_ARG_WITH(fft,
    AC_HELP_STRING(
      [--with-fft@<:@=DIR@:>@],
      [specify where to find FFT libraries and includes]
    ),
    [fft_paths="$withval"],
    [fft_paths="/gpfs/apps/NVIDIA/FFTW/3.3"] dnl List of possible default paths
  )
  dnl Search for FFT installation
  AX_FIND_INSTALLATION([FFT], [$fft_paths], [fft])

  if test "x${FFT_INSTALLED}" = "xyes" ; then
    FFT_LIBS="-lfftw3 -lm"

    AC_SUBST(FFT_LIBS)
    AC_DEFINE([HAVE_FFT], 1, [Define to 1 if FFTW is installed in the system])
  fi
  AM_CONDITIONAL(HAVE_FFT, test "x${FFT_INSTALLED}" = "xyes")

  AX_FLAGS_RESTORE()
])


# AX_CHECK_DEBUG
# --------------
AC_DEFUN([AX_CHECK_DEBUG],
[
  AC_ARG_ENABLE(debug,
    AC_HELP_STRING(
      [--enable-debug],
      [Enable debug (writes intermediate files)]
    ),
    [enable_debug="${enableval}"],
    [enable_debug="no"]
  )
  if test "x${enable_debug}" = "xyes" ; then
    CFLAGS="$CFLAGS -g -DDEBUG_MODE"
  fi
  AM_CONDITIONAL(DEBUG_MODE, test "x${enable_debug}" = "xyes")
])


# AX_PROG_EXTRAE
# --------------
AC_DEFUN([AX_PROG_EXTRAE],
[
  AC_ARG_WITH(extrae,
    AC_HELP_STRING(
      [--with-extrae@<:@=DIR@:>@],
      [specify where to find Extrae libraries and includes]
    ),
    [extrae_paths="$withval"],
    [extrae_paths="not_set"] dnl List of possible default paths
  )

  dnl Search for FFT installation
  AX_FIND_INSTALLATION([EXTRAE], [$extrae_paths], [extrae])
 
  if test "x${EXTRAE_INSTALLED}" = "xyes" ; then
    CFLAGS="$CFLAGS -DTRACE_MODE"
    EXTRAE_LIBS="-lseqtrace"
    AC_SUBST(EXTRAE_LIBS)
    AC_DEFINE([HAVE_EXTRAE], 1, [Define to 1 if Extrae is installed])
  fi
  AM_CONDITIONAL(HAVE_EXTRAE, test "x${EXTRAE_INSTALLED}" = "xyes")
])


# AX_OPENMP
# ---------
AC_DEFUN([AX_OPENMP],
[
  AC_PREREQ(2.59)

  AC_ARG_ENABLE(openmp,
    AC_HELP_STRING(
      [--enable-openmp],
      [Enable OpenMP parallelization]
    ),
    [enable_openmp="${enableval}"],
    [enable_openmp="no"]
  )
  if test "x${enable_openmp}" = "xyes" ; then

     AC_CACHE_CHECK([for OpenMP flag of _AC_LANG compiler],
      ax_cv_[]_AC_LANG_ABBREV[]_openmp,
      [save[]_AC_LANG_PREFIX[]FLAGS=$[]_AC_LANG_PREFIX[]FLAGS ax_cv_[]_AC_LANG_ABBREV[]_openmp=unknown

      # Flags to try:  -fopenmp (gcc), -openmp (icc), -mp (SGI &amp; PGI),
      #                -xopenmp (Sun), -omp (Tru64), -qsmp=omp (AIX), none
      ax_openmp_flags="-fopenmp -openmp -mp -xopenmp -omp -qsmp=omp none"
      if test "x$OPENMP_[]_AC_LANG_PREFIX[]FLAGS" != x; then
         ax_openmp_flags="$OPENMP_[]_AC_LANG_PREFIX[]FLAGS $ax_openmp_flags"
      fi
      for ax_openmp_flag in $ax_openmp_flags; do
         case $ax_openmp_flag in
            none) []_AC_LANG_PREFIX[]FLAGS=$save[]_AC_LANG_PREFIX[] ;;
            *) []_AC_LANG_PREFIX[]FLAGS="$save[]_AC_LANG_PREFIX[]FLAGS $ax_openmp_flag" ;;
         esac
         AC_TRY_LINK_FUNC(omp_set_num_threads,
               [ax_cv_[]_AC_LANG_ABBREV[]_openmp=$ax_openmp_flag; break])
      done
      []_AC_LANG_PREFIX[]FLAGS=$save[]_AC_LANG_PREFIX[]FLAGS])
      if test "x$ax_cv_[]_AC_LANG_ABBREV[]_openmp" = "xunknown"; then
         m4_default([$2],:)
      else
         if test "x$ax_cv_[]_AC_LANG_ABBREV[]_openmp" != "xnone"; then
            OPENMP_[]_AC_LANG_PREFIX[]FLAGS=$ax_cv_[]_AC_LANG_ABBREV[]_openmp
            AC_SUBST(OPENMP_[]_AC_LANG_PREFIX[]FLAGS)
         fi
         m4_default([$1], [AC_DEFINE(HAVE_OPENMP,1,[Define if OpenMP is enabled])])
      fi
  fi
  AM_CONDITIONAL(HAVE_OPENMP, test "x${enable_openmp}" = "xyes" -a "x$ax_cv_[]_AC_LANG_ABBREV[]_openmp" != "xunknown")
])

# AX_LIBTOOLS
# -----------
AC_DEFUN([AX_LIBTOOLS],
[

  HaveLibtools="yes"
  AC_MSG_CHECKING([for libtools])
  AC_ARG_WITH(libtools,
        AC_HELP_STRING(
                [--with-libtools@<:@=ARG@:>@],
                [Specify where libtools (libparaverconfig and libparavertraceparser) are installed]
        ),
        [LibToolsDir="$withval"],
        [LibToolsDir=""]
  )
  if test ! -d ${LibToolsDir} ; then
        AC_MSG_ERROR([Invalid directory specified in --with-libtools])
  fi
  if test ! -f ${LibToolsDir}/include/ParaverRecord.h -o ! -f ${LibToolsDir}/include/ParaverTrace.h ; then
        HaveLibtools="no"
        AC_MSG_WARN([Cannot find some header files of the libtools! Make sure that --with-libtools is pointing to the correct place.])
  else
        AC_MSG_RESULT([found in ${LibToolsDir}])
  fi

  AC_MSG_CHECKING([for libtools libraries placement])
  if test -f ${LibToolsDir}/lib/libparavertraceconfig.a -a \
          -f ${LibToolsDir}/lib/libparavertraceparser.a ; then
          LibToolsDir_Lib=${LibToolsDir}/lib
          AC_MSG_RESULT([found in ${LibToolsDir_Lib}])
  elif test -f ${LibToolsDir}/lib64/libparavertraceconfig.a -a \
            -f ${LibToolsDir}/lib64/libparavertraceparser.a; then
          LibToolsDir_Lib=${LibToolsDir}/lib64
          AC_MSG_RESULT([found in ${LibToolsDir_Lib}])
  else
          HaveLibtools="no"
          AC_MSG_WARN([Cannot find libparavertraceconfig.a! Make sure that --with-libtools is pointing to the correct place.])
  fi

  AC_MSG_CHECKING([whether libtools libraries have shared versions])
  if test -f ${LibToolsDir_Lib}/libparavertraceconfig.so -a \
          -f ${LibToolsDir_Lib}/libparavertraceparser.so ; then
          LibToolsDir_HasShared="yes"
  else
          LibToolsDir_HasShared="no"
  fi
  AC_MSG_RESULT([${LibToolsDir_HasShared}])

  AC_MSG_CHECKING([for prvparser-config])
  if test ! -f ${LibToolsDir}/bin/prvparser-config ; then
    HaveLibtools = "no"
  fi
  
  if test "${HaveLibtools}" = "yes" ; then
    LIBTOOLS_DIR=${LibToolsDir}
    LIBTOOLS_LIB_DIR=${LibToolsDir_Lib}
    LIBTOOLS_CFLAGS="-I$LIBTOOLS_DIR/include"
    LIBTOOLS_CXXFLAGS="-I$LIBTOOLS_DIR/include"
    LIBTOOLS_LDFLAGS="-L$LIBTOOLS_LIB_DIR -lparavertraceparser"
    PRVPARSER_CONFIG="${LibToolsDir}/bin/prvparser-config"
    AC_SUBST(LIBTOOLS_DIR)
    AC_SUBST(LIBTOOLS_LIB_DIR)
    AC_SUBST(LIBTOOLS_CFLAGS) 
    AC_SUBST(LIBTOOLS_CXXFLAGS)
    AC_SUBST(LIBTOOLS_LDFLAGS)
    AC_SUBST(PRVPARSER_CONFIG)
    AC_DEFINE(HAVE_LIBTOOLS, 1,[Define if libtools are available])
  fi

  AM_CONDITIONAL(LIBTOOLS_HAS_SHARED_LIBRARIES, test "${LibToolsDir_HasShared}" = "yes")
  AM_CONDITIONAL(HAVE_LIBTOOLS, test "${HaveLibtools}" = "yes")
])

