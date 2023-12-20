#!/usr/bin/sh

cd examples

# Cannot execute scripts in windows
## Need to point to 'exe' file if on linux and compiling for windows
./pdc2-test1.sh > /dev/null; echo "pdc2-test1.sh $?"

cd ../test

# Cannot execute scripts in windows
## Need to point to 'exe' file if on linux and compiling for windows
./exoptest > /dev/null; echo "exoptest $?"
./test-unicode > /dev/null; echo "test-unicode $?"

# Missing dlls to import
## libgcc_s_seh-1.dll and libstdc++-6.dll
wine test-dvb_mux.exe > /dev/null; echo "test-dvb_mux $?"

# Wine error?
## Unhandled page fault on read access...
wine test-pdc.exe > /dev/null; echo "test-pdc $?"
wine test-packet-830.exe > /dev/null; echo "test-packet-830 $?"

# Working!
wine ctest.exe > /dev/null; echo "ctest $?"
wine cpptest.exe > /dev/null; echo "cpptest $?"
wine ctest-c89.exe > /dev/null; echo "ctest-c89 $?"
wine ctest-gnu89.exe > /dev/null; echo "ctest-gnu89 $?"
wine ctest-c94.exe > /dev/null; echo "ctest-c94 $?"
wine ctest-c99.exe > /dev/null; echo "ctest-c99 $?"
wine ctest-gnu99.exe > /dev/null; echo "ctest-gnu99 $?"
wine cpptest-cxx98.exe > /dev/null; echo "cpptest-cxx98 $?"
wine cpptest-gnuxx98.exe > /dev/null; echo "cpptest-gnuxx98 $?"
wine test-dvb_demux.exe > /dev/null; echo "test-dvb_demux $?"
wine test-hamm.exe > /dev/null; echo "test-hamm $?"
wine test-raw_decoder.exe > /dev/null; echo "test-raw_decoder $?"
wine test-vps.exe > /dev/null; echo "test-vps $?"
