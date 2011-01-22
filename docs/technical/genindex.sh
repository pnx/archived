
#!/bin/sh

if [ $# -lt 1 ]; then
    exit 1
fi

(
    echo "Archived API Documentation"
    echo "--------------------------"

    for DOC in `ls *.txt`; do
        if [ "$DOC" = "$1" ]; then
            continue
        fi
        display=$(sed -e 1q "${DOC}")
        link=${DOC%.txt}.html
        echo "* link:${link}[${display}]"
    done

) > $1
