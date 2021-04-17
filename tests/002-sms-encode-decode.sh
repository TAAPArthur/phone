#!/bin/sh
echo "hi" | ./sms 123 | tail -n1 | ./sms -d | tail -n1
echo "hi2" | ./sms 1234567890 | tail -n1 | ./sms -d | tail -n1
# hi
# hi2
