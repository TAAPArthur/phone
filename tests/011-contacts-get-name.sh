#!/bin/sh
./contacts.sh get-name 1234567890
# Test
./contacts.sh get-name "(123) 456-7890"
# Test
./contacts.sh get-name "+1 (123) 456-7890"
# Test
