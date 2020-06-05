#! /bin/sh

EXTRACTXML="./extractxml"

rm -f rc.cpp
$EXTRACTXML --extract \
        --context='%(filename)s:%(tag)s' --tag-regex='^(?:title|body|p|h[1-6])$' --recursive --strip \
        ./objectinfo/*.html --output=rc.cpp
$XGETTEXT rc.cpp -o $podir/step_objinfo_files.pot

rm -f rc.cpp
$EXTRACTXML --extract \
        --context='%(tag)s' --tag=name --tag=text --unquote \
        --parse-unquoted='--context=HTML:%(tag)s --tag-regex=^(?:title|body|p|h[1-6])$ --recursive --strip' \
        ./examples/*.step ./tutorials/*.step --output=rc.cpp
$XGETTEXT rc.cpp -o $podir/step_example_files.pot

rm -f rc.cpp
