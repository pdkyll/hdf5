#!rbin/sh 
# HDF Utilities Test script
# Usage: h5importtestutil.sh [machine-type]

# initialize errors variable
errors=0

TESTING() {
   SPACES="                                                               "
   echo "Testing $* $SPACES" | cut -c1-70 | tr -d '\012'
}

TOOLTEST()
{
err=0
./h5import $*
../h5dump/h5dump $5 >log2

cd tmp_testfiles
../../h5dump/h5dump $5 >log1
cd ..

cmp -s tmp_testfiles/log1 log2 || err=1
rm -f log2 tmp_testfiles/log1
if [ $err -eq 1 ]; then
errors="` expr $errors + 1 `";
  echo "*FAILED*"
else
  echo " PASSED"
fi
}

echo "" 
echo "=============================="
echo "H5IMPORT tests started"
echo "=============================="

if [ -f h5import -a -f h5importtest ]; then
#echo "** Testing h5import  ***"

rm -f  output.h5 log1 tx* b* *.dat

mkdir tmp_testfiles
cp $srcdir/testfiles/*.h5 tmp_testfiles/

./h5importtest

TESTING "ASCII I32 rank 3 - Output BE " ;
TOOLTEST txtin32 -c $srcdir/testfiles/textin32 -o test1.h5

TESTING "ASCII I16 rank 3 - Output LE - CHUNKED - extended" 
TOOLTEST txtin16 -c $srcdir/testfiles/textin16 -o test2.h5

TESTING "ASCII I8 - rank 3 - Output I16 LE-Chunked+Extended+Compressed " 
TOOLTEST txtin16 -c $srcdir/testfiles/textin8  -o test3.h5

TESTING "ASCII UI32 - rank 3 - Output BE" 
TOOLTEST $srcdir/testfiles/in1 -c $srcdir/testfiles/textuin32 -o test4.h5

TESTING "ASCII UI16 - rank 2 - Output LE+Chunked+Compressed " 
TOOLTEST $srcdir/testfiles/in1 -c $srcdir/testfiles/textuin16 -o test5.h5

TESTING "ASCII F32 - rank 3 - Output LE " 
TOOLTEST $srcdir/testfiles/fp1 -c $srcdir/testfiles/textfp32 -o test6.h5

TESTING "ASCII F64 - rank 3 - Output BE + CHUNKED+Extended+Compressed " 
TOOLTEST $srcdir/testfiles/fp2 -c $srcdir/testfiles/textfp64 -o test7.h5

TESTING "BINARY F64 - rank 3 - Output LE+CHUNKED+Extended+Compressed " 
TOOLTEST bfp64 -c $srcdir/testfiles/conbfp64 -o test8.h5

TESTING "BINARY I16 - rank 3 - Output order LE + CHUNKED + extended " 
TOOLTEST bin16 -c $srcdir/testfiles/conbin16 -o test9.h5

TESTING "BINARY I8 - rank 3 - Output I16LE + Chunked+Extended+Compressed " 
TOOLTEST bin8 -c $srcdir/testfiles/conbin8  -o test10.h5

TESTING "BINARY I32 - rank 3 - Output BE + CHUNKED " 
TOOLTEST bin32 -c $srcdir/testfiles/conbin32 -o test11.h5

TESTING "BINARY UI16 - rank 3 - Output byte BE + CHUNKED " 
TOOLTEST buin16 -c $srcdir/testfiles/conbuin16 -o test12.h5

TESTING "BINARY UI32 - rank 3 - Output LE + CHUNKED " 
TOOLTEST buin32 -c $srcdir/testfiles/conbuin32 -o test13.h5

rm -f  tx* b* *.dat
rm -f  test*.h5 
rm -rf tmp_testfiles
else
	echo "** h5import or h5importtest not available ***"
	errors="` expr $errors + 1 `";
fi

#
# Check errors result
if [ $errors -eq 0 ]; then
    echo "======================================"
    echo " H5IMPORT Utilities tests have passed."
    echo "======================================"    
else
    echo "*********************************************"
    echo " H5IMPORT Utilities tests encountered errors"
    echo "*********************************************"
fi
exit $errors
