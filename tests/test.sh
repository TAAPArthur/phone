#!/bin/sh -e
export CONTACTS_FILE=tests/sample_contacts.json
failTest() {
    echo "############################"
    . $1
    echo "############################"
    exit 3
}
for script in tests/0*.sh; do
    printf "%s..." $script
    exec 3<<EOF
$(grep '# ' $script | cut -d' ' -f2-)
EOF
    i=0
    /bin/sh $script 1>/dev/null 2>&1
    /bin/sh $script 2>/dev/null 3>/dev/null | {
        while read -r output; do
            i=$((i+1))
            read -r expected <&3 || expected=""
            if [ "$output" != "$expected" ]; then
                echo "failed on line $i"
                echo "'$output'" vs "'$expected'"
                failTest $script
            fi
        done
        if [ -z "$expected" ]; then
            echo "failed on line $i"
            failTest $script
        fi
        #read -r expected <&3 && exit 1
        echo "passed"
    }
    exec 3<&-
done
