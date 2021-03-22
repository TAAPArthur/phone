#!/bin/sh
echo "hi" | ./sms 123 | tail -n1 | ./sms -d | tail -n1
# hi
