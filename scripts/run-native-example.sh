#!/bin/bash

# Script arguments in order:
#
# <preset-name>     One of CMakePresets.txt, see `cmake --list-presets`
#

script_args="$@"

username=${USER}

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

. $rootdir/jaulib/scripts/setup-machine-arch.sh "-quiet"

tripleid="$os_name-$archabi"

if [ ! -z "$1" ] ; then
    preset_name=$1
    shift 1
else
    echo "ERROR: No preset passed as 1st argument, use one of:"
    cmake --list-presets
    exit 1
fi

dist_dir="$rootdir/dist/${preset_name}-${tripleid}"
build_dir="$rootdir/build/${preset_name}"
echo dist_dir $dist_dir
echo build_dir $build_dir

if [ ! -e $dist_dir/bin/$bname ] ; then
    echo "Not existing $dist_dir/bin/$bname"
    exit 1
fi

if [ ! -e $dist_dir/lib/libgamp.so ] ; then
    echo "Not existing $dist_dir/lib/libgamp.so"
    exit 1
fi

if [ "$1" = "-log" ] ; then
    logbasename=$2
    shift 2
else
    logbasename=${bname}-${preset_name}-${tripleid}
fi

mkdir -p $rootdir/doc/test
logfile=$rootdir/doc/test/$logbasename.0.log
rm -f $logfile

valgrindlogfile=$logbasename-valgrind.log
rm -f $valgrindlogfile

callgrindoutfile=$logbasename-callgrind.out
rm -f $callgrindoutfile

# echo 'core_%e.%p' | sudo tee /proc/sys/kernel/core_pattern
# echo 'core_%e.%p' > /proc/sys/kernel/core_pattern
ulimit -c unlimited

# run as root 'dpkg-reconfigure locales' enable 'en_US.UTF-8'
# perhaps run as root 'update-locale LC_MEASUREMENT=en_US.UTF-8 LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8'
export LC_MEASUREMENT=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

# export EXE_WRAPPER="valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes --malloc-fill=0xff --free-fill=0xfe --error-limit=no --default-suppressions=yes --suppressions=$sdir/valgrind.supp --gen-suppressions=all -s --log-file=$valgrindlogfile"
# export EXE_WRAPPER="valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite --track-origins=yes --malloc-fill=0xff --free-fill=0xfe --error-limit=no --default-suppressions=yes --suppressions=$sdir/valgrind.supp --gen-suppressions=all -s --log-file=$valgrindlogfile"
# export EXE_WRAPPER="valgrind --tool=helgrind --track-lockorders=yes  --ignore-thread-creation=yes --default-suppressions=yes --suppressions=$sdir/valgrind.supp --gen-suppressions=all -s --log-file=$valgrindlogfile"
# export EXE_WRAPPER="valgrind --tool=drd --segment-merging=no --ignore-thread-creation=yes --trace-barrier=no --trace-cond=no --trace-fork-join=no --trace-mutex=no --trace-rwlock=no --trace-semaphore=no --default-suppressions=yes --suppressions=$sdir/valgrind.supp --gen-suppressions=all -s --log-file=$valgrindlogfile"
# export EXE_WRAPPER="valgrind --tool=callgrind --instr-atstart=yes --collect-atstart=yes --collect-systime=yes --combine-dumps=yes --separate-threads=no --callgrind-out-file=$callgrindoutfile --log-file=$valgrindlogfile"

runit() {
    echo "script invocation: $0 ${script_args}"
    echo bname $bname
    echo username $username
    echo ${bname} commandline "$@"
    echo EXE_WRAPPER $EXE_WRAPPER
    echo logbasename $logbasename
    echo logfile $logfile
    echo valgrindlogfile $valgrindlogfile
    echo callgrindoutfile $callgrindoutfile
    echo gamp_debug $gamp_debug
    echo gamp_verbose $gamp_verbose

    echo LD_LIBRARY_PATH=$dist_dir/lib $EXE_WRAPPER $dist_dir/bin/${bname} "$@"
    LD_LIBRARY_PATH=$dist_dir/lib $EXE_WRAPPER $dist_dir/bin/${bname} "$@"
}

runit "$@" 2>&1 | tee $logfile

