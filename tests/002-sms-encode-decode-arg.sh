#!/bin/sh
./sms -d "$(./sms 123 'hi' | tail -n1)" | tail -n1
echo
# hi
