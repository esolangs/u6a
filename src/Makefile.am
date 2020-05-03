AUTOMAKE_OPTIONS = dejagnu

bin_PROGRAMS = u6ac u6a

u6ac_SOURCES = logging.c lexer.c parser.c codegen.c u6ac.c mnemonic.c dump.c
u6a_SOURCES  = logging.c vm_stack.c vm_pool.c runtime.c u6a.c

TEST_DIR = ${srcdir}/../tests
DEJAGNU_GLOBALS_BIN = U6A_BIN=${srcdir}/u6a U6AC_BIN=${srcdir}/u6ac U6A_RUN=${TEST_DIR}/u6a_run
DEJAGNU_GLOBALS_BASE64 = B64_ENCODE=${BASE64_ENCODE} B64_DECODE=${BASE64_DECODE}
RUNTESTDEFAULTFLAGS = --srcdir=${TEST_DIR} --tool default ${DEJAGNU_GLOBALS_BIN} ${DEJAGNU_GLOBALS_BASE64}
EXTRA_DEJAGNU_SITE_CONFIG = ../tests/init.exp
