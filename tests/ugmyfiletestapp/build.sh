#!/bin/bash

tar xzf git.tgz
echo "Build"
    gbs build -A i586 -R https://download.tizen.org/snapshots/2.2-emul/common/latest/repos/tizen-main/ia32/packages -R https://download.tizen.org/snapshots/2.2-emul/common/latest/repos/tizen-base/ia32/packages --buildroot=~/GBS-ROOT --extra-packs=zypper,gdb,gdb-server --skip-conf-repos --include-all --keep-packs
    if [ $? == 1 ]; then
        grep "error:" ~/GBS-ROOT/local/repos/tizen/i586/logs/fail/org.tizen.ugmyfiletestapp-0.0.1-1/log
        rm -fR .git/
        exit 1;
    fi
rm -fR .git/
exit 0;
