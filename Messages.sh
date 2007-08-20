#! /bin/sh
$EXTRACTRC `find . -name '*.rc'` > rc.cpp || exit 11
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp || exit 12
$XGETTEXT -ktr:1,1t -ktr:1,2c,2t -kQT_TRANSLATE_NOOP:1c,2,2t -kQT_TR_NOOP:1,1t -ktranslate:1c,2,2t -ktranslate:2,3c,3t rc.cpp `find . -name '*.cc'` -o $podir/step.pot
rm -f rc.cpp
