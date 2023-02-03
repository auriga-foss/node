#!/bin/sh

# check if we have second arg as valid (non-empty) report for x86(?)
X86_TESTS_CNT=`cat $2 | wc -l`

TOTAL=`cat $1 | grep -v ",NAME" | wc -l`
PASS=`cat $1 | grep pass, | wc -l`
CRASH=`cat $1 | grep CRASH, | wc -l`
CRASH_SPAWN=`cat $1 | grep CRASH,+ | wc -l`
CRASH_WITH_TRACE=`cat $1 | grep CRASH,-,-,+ | wc -l`
CRASH_UNKNOWN=`cat $1 | grep CRASH,-,-,- | wc -l`
FAILED=`cat $1 | grep FAIL, | wc -l`
FAILED_SPAWN=`cat $1 | grep FAIL,+ | wc -l`
FAILED_SKIPPED=`cat $1 | grep FAIL,- | grep -v FAIL,-,- | wc -l`

# we need to remove local paths from report, so let's build a proper
# search string for sed without the 'kos' folder name
SUPRESS_STR=`dirname $PWD | sed -e 's#\/#\\\/#g'`
echo "suppress string:" $SUPRESS_STR
if [ x"$X86_TESTS_CNT" != x"0" ]; then

  LIST=`cat $1 | grep FAIL,-,-,- | awk -vFPAT='([^,]*)|("[^"]+")' -vOFS=, '{print $4}' | sed -e 's#${SUPRESS_STR}##g'`;

  echo "NAME,STATUS_KOS,STATUS_X86" > status.list

  for a in $LIST; do
    STATUS=`cat $2 | grep $a | awk -vFPAT='([^,]*)|("[^"]+")' -vOFS=, '{print $5}'`;
    echo $a,FAIL,$STATUS >> status.list;
  done;

  FAILED_X86=`cat status.list | grep FAIL,FAIL | wc -l`
  FAILED_UNKNOWN=`cat status.list | grep FAIL,pass | wc -l`

else
  FAILED_UNKNOWN=`cat $1 | grep FAIL,-,-,- | awk -vFPAT='([^,]*)|("[^"]+")' -vOFS=, '{print $4}' | sed -e 's#${SUPRESS_STR}##g' | tee failed_unknown.list | wc -l`;
fi

echo "Total         : $TOTAL"
echo "Passed        : $PASS"
echo "Crashed total : $CRASH"
echo " - by spawn   : $CRASH_SPAWN"
echo " - with trace : $CRASH_WITH_TRACE"
echo " - unknown    : $CRASH_UNKNOWN"
echo "Failed total  : $FAILED"
echo " - by spawn   : $FAILED_SPAWN"
echo " - by skipped : $FAILED_SKIPPED"

if [ x"$X86_TESTS_CNT" != x"0"  ]; then
  echo " - by x86     : $FAILED_X86"
fi

echo " - unknown    : $FAILED_UNKNOWN"
