#!/usr/bin/sh

cd examples

# Cannot execute scripts in windows
## Point to 'exe' file if on linux and compiling for windows
./pdc2-test1.sh > /dev/null; echo "pdc2-test1.sh $?"

cd ../test

# Why differing? (Unrecognized symbols)
## unicode-out.txt unicode-out-ref.txt differ: byte 43, line 1
./test-unicode_windows > /dev/null; echo "test-unicode_windows $?"

# Segmentation fault
## See https://github.com/zapping-vbi/zvbi/pull/42 for details
wine test-pdc.exe > /dev/null; echo "test-pdc $?"
wine test-packet-830.exe > /dev/null; echo "test-packet-830 $?"
wine test-dvb_mux.exe > /dev/null; echo "test-dvb_mux $?"

# Working!
./exoptest_windows > /dev/null; echo "exoptest_windows $?"
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
