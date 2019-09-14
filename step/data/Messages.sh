#! /bin/sh

EXTRACTXML="./extractxml"

$EXTRACTXML --extract \
        --context='%(filename)s:%(tag)s' --tag-regex='^(?:title|body|p|h[1-6])$' --recursive --strip \
        ./objectinfo/*.html --output=objinfo.cpp
$EXTRACTXML --extract \
        --context='%(tag)s' --tag=name --tag=text --unquote \
        --parse-unquoted='--context=HTML:%(tag)s --tag-regex=^(?:title|body|p|h[1-6])$ --recursive --strip' \
        ./examples/*.step ./tutorials/*.step --output=examples.cpp

# Temporary commented-out
#$XGETTEXT objinfo.cpp -o $podir/step_objinfo_files.pot
#$XGETTEXT examples.cpp -o $podir/step_example_files.pot

rm -f objinfo.cpp
rm -f examples.cpp

