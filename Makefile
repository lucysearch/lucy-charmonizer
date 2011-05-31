# GENERATED BY ../devel/bin/gen_charmonizer_makefiles.pl: do not hand-edit!!!
CC= cc
DEFS=
PROGNAME= charmonize
INCLUDES= -I. -Isrc
DEFINES= $(INCLUDES) $(DEFS)
CFLAGS= -g $(DEFINES)
LIBS=

TESTS= TestDirManip TestFuncMacro TestHeaders TestIntegers TestLargeFiles TestUnusedVars TestVariadicMacros

OBJS= charmonize.o src/Charmonizer/Core/Compiler.o src/Charmonizer/Core/ConfWriter.o src/Charmonizer/Core/Dir.o src/Charmonizer/Core/HeaderChecker.o src/Charmonizer/Core/OperatingSystem.o src/Charmonizer/Core/Stat.o src/Charmonizer/Core/Util.o src/Charmonizer/Probe.o src/Charmonizer/Probe/AtomicOps.o src/Charmonizer/Probe/DirManip.o src/Charmonizer/Probe/Floats.o src/Charmonizer/Probe/FuncMacro.o src/Charmonizer/Probe/Headers.o src/Charmonizer/Probe/Integers.o src/Charmonizer/Probe/LargeFiles.o src/Charmonizer/Probe/Memory.o src/Charmonizer/Probe/UnusedVars.o src/Charmonizer/Probe/VariadicMacros.o

TEST_OBJS= src/Charmonizer/Test.o src/Charmonizer/Test/TestDirManip.o src/Charmonizer/Test/TestFuncMacro.o src/Charmonizer/Test/TestHeaders.o src/Charmonizer/Test/TestIntegers.o src/Charmonizer/Test/TestLargeFiles.o src/Charmonizer/Test/TestUnusedVars.o src/Charmonizer/Test/TestVariadicMacros.o

HEADERS= src/Charmonizer/Core/Compiler.h src/Charmonizer/Core/ConfWriter.h src/Charmonizer/Core/Defines.h src/Charmonizer/Core/Dir.h src/Charmonizer/Core/HeaderChecker.h src/Charmonizer/Core/OperatingSystem.h src/Charmonizer/Core/Stat.h src/Charmonizer/Core/Util.h src/Charmonizer/Probe.h src/Charmonizer/Probe/AtomicOps.h src/Charmonizer/Probe/DirManip.h src/Charmonizer/Probe/Floats.h src/Charmonizer/Probe/FuncMacro.h src/Charmonizer/Probe/Headers.h src/Charmonizer/Probe/Integers.h src/Charmonizer/Probe/LargeFiles.h src/Charmonizer/Probe/Memory.h src/Charmonizer/Probe/UnusedVars.h src/Charmonizer/Probe/VariadicMacros.h src/Charmonizer/Test.h

.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $@

all: $(PROGNAME)

tests: $(TESTS)

$(PROGNAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGNAME) $(OBJS) $(LIBS)

$(OBJS) $(TEST_OBJS): $(HEADERS)

TestDirManip: src/Charmonizer/Test.o src/Charmonizer/Test/TestDirManip.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestDirManip.o $(LIBS)

TestFuncMacro: src/Charmonizer/Test.o src/Charmonizer/Test/TestFuncMacro.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestFuncMacro.o $(LIBS)

TestHeaders: src/Charmonizer/Test.o src/Charmonizer/Test/TestHeaders.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestHeaders.o $(LIBS)

TestIntegers: src/Charmonizer/Test.o src/Charmonizer/Test/TestIntegers.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestIntegers.o $(LIBS)

TestLargeFiles: src/Charmonizer/Test.o src/Charmonizer/Test/TestLargeFiles.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestLargeFiles.o $(LIBS)

TestUnusedVars: src/Charmonizer/Test.o src/Charmonizer/Test/TestUnusedVars.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestUnusedVars.o $(LIBS)

TestVariadicMacros: src/Charmonizer/Test.o src/Charmonizer/Test/TestVariadicMacros.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestVariadicMacros.o $(LIBS)


clean:
	rm -f $(OBJS) $(TEST_OBJS) $(PROGNAME) $(TESTS) core
