#! /bin/sh
$EXTRACTRC `find . -name '*.rc'` >> rc.cpp || exit 11
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp || exit 12
$XGETTEXT `find step -name '*.cc'` rc.cpp -o $podir/step.pot
