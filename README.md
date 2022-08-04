# GCJ (GNU Compiler for Java)

Note-- the compiler cannot even finish building, yet alone be used in the wild.

## Patches
I've sent patches upstream!
Here's their status:

- [ ] Re-add gcc/java/*
- [ ] Re-add libjava/*
- [ ] Re-add documentation for Java front-end and library.
- [ ] Re-add autotools glue for Java front-end and library.
- [ ] contrib: Re-add Java entries.
- [ ] Re-add tests for Java.
- [ ] Darwin: Re-add Java workaround constraint about putting small items into data/static data.
- [ ] Re-add MODIFY_JNI_METHOD_CALL.
- [ ] Re-add LIBGCJ_SONAME.
- [ ] Re-add JCR_SECTION_NAME, TARGET_USE_JCR_SECTION, \__LIBGCC_JCR_SECTION_NAME__, \_Jv_RegisterClasses, \_\_JCR_LIST__, \_\_JCR_END__.
- [ ] Re-add handling of Java file extensions (.java, .class, .zip, .jar).
- [ ] Re-add Java (to) comments.
- [ ] Re-add flag_evaluation_order, reorder_operands_p, and add reorder bool argument to tree_swap_operands_p.
- [ ] libcpp: Change deps_write and make_write to take class mkdeps instead of const cpp_reader and to learn about the the dependency phoniness via a bool argument.
- [ ] gcc: Re-add TYPE_METHODS.
- [ ] gcc: Add binfo value to tree_type_non_common and make TYPE_BINFO use it.
- [ ] Fix regression around friend declarations in local classes \[PR69410].
- [ ] Revert "Move void_list_node init to common code".
- [ ] Re-add Java handling to gcc and gcc/cp.
- [ ] Rename gcc/java/\*.c to \*.cc.
- [ ] java: Comment out -fassert documentation.
- [ ] java: Don't build by default.
- [ ] java: Add empty selftest hook.
- [ ] java: Replace PTR with 'void \*'.
- [ ] java: Remove 'FREE' macro.
- [ ] java: Replace source_location with location_t.
- [ ] java: Replace TYPE_MINVAL with TYPE_MIN_VALUE_RAW.
- [ ] java: Properly handle GET_MODE_SIZE as a poly_uint16.
- [ ] java: Add handle_nonnull_attribute function.
- [ ] java: Use HOST_WIDE_INT.
- [ ] java: Replace pfatal_with_name with fatal_error.
- [ ] java: Fix -Wformat-diag warnings.
- [ ] java: Replace flag_excess_precision_cmdline with
- [ ] java: Use checking forms of DECL_FUNCTION_CODE.
- [ ] java: Don't check the value of dump_switch_p.
- [ ] java: Build SWITCH_EXPR using build2 instead of build3.
- [ ] java: Use dump_flags_t.
- [ ] java: Check that type is array or integer before
- [ ] java: Fix Wstringop-* warnings.
- [ ] java: Replace struct deps with struct mkdeps and set
- [ ] java: Include memmodel.h in builtins.cc.
- [ ] java: Include opt-suggestions.h in jvspec.cc.
- [ ] java: Add FUNCTION_DECL_CHECK in METHOD_DUMMY.
- [ ] libjava: Remove unnecessary register keyword.
- [ ] libjava: Remove unnecessary parentheses.
- [ ] libjava: Use ucontext_t instead of struct ucontext.
- [ ] classpath: classpath: Rename documentation files from .texinfo to .texi.
- [ ] libjava: Move to autoconf 2.69, automake 1.15.
- [ ] libjava: Replace AC_LANG_PROGRAM with AC_LANG_SOURCE.
- [ ] libjava: Check for libz_convenience.la instead of libzgcj_convenience.la.
- [ ] libjava: Move from in-tree libltdl to external libltdl 2.4.7.
- [ ] classpath: Use modern autotools mkdir -p handling..
- [ ] classpath: Use info-in-builddir.
- [ ] classpath: Mark generate-locale-list.sh as executable.
- [ ] java, libjava: Upgrade from in-tree boehm-gc to external boehm-gc 8.3.0.

## GCJ

### What was GCJ?
GCJ was the GNU Compiler for Java, an open compiler for Java that could compile Java both into bytecode and **machine code**.
It also allowed for easy interoperability between Java and C++ with the [CNI (Compiled Native Interface)](https://en.wikipedia.org/wiki/GNU_Compiler_for_Java#Compiled_Native_Interface_(CNI)).

### Why was it removed?
GCJ was [removed from GCC](https://gcc.gnu.org/git/?p=gcc.git&a=commit;h=eae993948bae8b788c53772bcb9217c063716f93) several years ago, as development had stalled around 2015.
This was mainly because there was no real need for the GNU Classpath (an open reimplementation of Java), as the JDK was released under the GPL as the OpenJDK.
GCJ used Classpath as it's runtime library, and without any development on the runtime library, which had only ever had a patchy implementation that wasn't even fully complete with Java 1.2.

### Why bring it back?
Having alternative compilers is always good for a language, espescially open-source ones.
Although, there are several alternative bytecode Java compilers, so GCJ doesn't add much to that field.
However, GCJ was the most functional Java machine code compiler, and with an open Java implementation that removes one of the main barriers to GCJ development. (Creating an entire Java implementation from scratch).
Now, "all" that needs to be done is to import the OpenJDK over Classpath and provide native reimplementations of certain methods.

## Building

Note-- GCJ currently requires a bleeding-edge version of bdwgc built from source, once 8.3.0 is released and works its way into package managers you should be able to install `libgc-dev` (for apt) instead.

Fetch dependencies for apt-based systems:
```
sudo apt install build-essential libgmp3-dev libmpfr-dev libmpc-dev flex bison libltdl-dev autotools-dev autogen gcc-multilib g++-multilib
```

Build and install [bdwgc](https://github.com/ivmai/bdwgc#installation-and-portability), making sure to remove any versions you may have installed via a package manager to avoid file conflicts.

Clone the repository:
```
git clone https://github.com/Zopolis4/gcj
```

Create the build directory (GCC is [designed](https://gcc.gnu.org/install/configure.html) to be built out of tree):
```
mkdir gcjbuild
cd gcjbuild
```

Configure the build (`--disable-bootstrap` is faster and will proceed further into the build process, but the default, `--enable-bootstrap` is the intended method of compilation and GCJ needs to compile under it for upstreaming):
```
../gcj/configure --enable-languages=java
```

Make (`make -j $(nproc)` is faster but can sometimes introduce build issues due to runahead, and can conceal the root cause of an error, again due to runahead. Make sure to run regular `make` after the `make -j $(nproc)` build fails):
```
make -j $(nproc)
```

## Contributing

The current aim is to have gcj bootstrap and compile fully with all languages enabled (`--enable-languages=ada,c,c++,d,fortran,go,java,lto,objc,obj-c++`), and any and all help is welcome.
After that, the plan is to replace GNU Classpath with the OpenJDK, making gcj a fully featured compiler, and then to go about further modernizing it.

Please be aware this project is designed to be pushed upstream to GCC when we reach some milestones,
and this means we require copyright assignment or the Developer's Certificate of Origin sign-off.
Please see the [Contributing to GCC](https://gcc.gnu.org/contribute.html) guide or [Developer's Certificate of Origin (DCO) Sign-off](https://gcc.gnu.org/dco.html) guide.

## Will this be integrated into gcc?

That is the eventual aim, with the hope that it will be merged once it is at a functional state, then the OpenJDK work and similar will be conducted here then merged into upstream.
