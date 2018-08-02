#! /usr/bin/env sh
##
## This script runs this tool with the tests files inside: /tests/files/
## envs:
##     - EFD_EXE
##     - EFD_HOME
##

ALGS_QX2="Q_dynprog Q_bmt Q_grdy Q_ibm Q_wpm Q_random Q_qubiter Q_wqubiter"
ALGS_QX3="Q_bmt Q_grdy Q_ibm Q_wpm Q_random"

ARCH_QX2="A_ibmqx2"
ARCH_QX3="A_ibmqx3"

FAIL_QX2="adder.qasm bigadder.qasm"
FAIL_QX3="bigadder.qasm"

BMT_CHILDREN=3
BMT_PARTIAL=40

EXECUTION_TIMES=1
OUTPUT="/dev/null"
FILES=`find tests/files/ -name "*.qasm"`

RET=0

cd $EFD_HOME

## IBMQX2
echo "-------= $ARCH_QX2 =-------"
for i in `seq 1 $EXECUTION_TIMES`; do
    for f in $FILES; do
        for alg in $ALGS_QX2; do
            filename=`basename $f`
            LOG="$i-$filename-$alg-qx2.log"

            ## Executing EFD
            $EFD_EXE -i $f -o $OUTPUT -alloc $alg -arch $ARCH_QX2 -stats -ord \
                --bsi-max-children $BMT_CHILDREN \
                --bsi-max-partial $BMT_PARTIAL > $LOG 2>&1
            ret=$?

            if echo "$FAIL_QX2" | grep -q "\<$filename\>"; then
                if [ $ret -eq 0 ]; then
                    echo "[ERROR]: benchmark \`$filename\` did NOT FAIL!"
                    cat $LOG
                    RET=1
                fi
            else
                if [ $ret -ne 0 ]; then
                    echo "[ERROR]: benchmark \`$filename\` did FAILED!"
                    cat $LOG
                    RET=1
                fi
            fi

            rm $LOG
        done
    done
done

## IBMQX3
echo "-------= $ARCH_QX3 =-------"
for i in `seq 1 $EXECUTION_TIMES`; do
    for f in $FILES; do
        for alg in $ALGS_QX3; do
            filename=`basename $f`
            LOG="$i-$filename-$alg-qx3.log"

            ## Executing EFD
            $EFD_EXE -i $f -o $OUTPUT -alloc $alg -arch $ARCH_QX3 -stats -ord \
                --bsi-max-children $BMT_CHILDREN \
                --bsi-max-partial $BMT_PARTIAL > $LOG 2>&1
            ret=$?

            if echo "$FAIL_QX3" | grep -q "\<$filename\>"; then
                if [ $ret -eq 0 ]; then
                    echo "[ERROR]: benchmark \`$filename\` did NOT FAIL!"
                    cat $LOG
                    RET=1
                fi
            else
                if [ $ret -ne 0 ]; then
                    echo "[ERROR]: benchmark \`$filename\` did FAILED!"
                    cat $LOG
                    RET=1
                fi
            fi

            rm $LOG
        done
    done
done

exit $RET
