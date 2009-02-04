#! /bin/sh

EXTRACTXML="./extractxml"

EXTRACTXML_HTML="${EXTRACTXML} --tag-regex=. --recursive --strip"
EXTRACTXML_STEP="${EXTRACTXML} --tag=name --tag=text --unquote"

$EXTRACTXML_HTML --extract ./objinfo/*.html --output=objinfo.cpp
$EXTRACTXML_STEP --extract ./examples/*.step ./tutorials/*.step --output=examples.cpp

# Temporary commented-out
#$XGETTEXT objinfo.cpp -o $podir/step_objinfo_files.pot
#$XGETTEXT examples.cpp -o $podir/step_example_files.pot

rm -f objinfo.cpp
rm -f examples.cpp

