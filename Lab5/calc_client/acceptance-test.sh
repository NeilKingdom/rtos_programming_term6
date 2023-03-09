#!/bin/sh

###
### Acceptance Test Script for calc_client and calc_server
###
### Usage
###   ./acceptance_test.sh [calc_serverPID]
###   where [calc_serverPID] is the processID of calc_server (./calc_server &)
###
### Installation
###   1. Copy this shell script file into the same directory on Neutrino as the calc_client and calc_server exe binaries.
###   2. Use sed (stream editor) to remove the carriage-return (^M) end-of-line symbol: sed 's/\r$//g' ./acceptance_test.sh.txt > acceptance_test.sh
###   3. Change file permissions of the script file to be runnable: chmod +x acceptance_test.sh
###
### Author
###   Gerald.Hurdle@AlgonquinCollege.com
###

## validate correct number of command-line arguments; exit if incorrect number
if [[ $# -ne 1 ]]; then
  echo "Usage: ./acceptance_test.sh [calc_serverPID]"
  exit 1
fi

echo "\n"
echo "Unit Test ID 1: ./calc_client"
echo "Expected result: usage message of calc_client"
./calc_client

echo "\n"
echo "Unit Test ID 2: ./calc_client 3 2 + 3"
echo "Expected result: error, as calc_client can't connect attached to processID 3 (proc/boot/pipe)"
./calc_client 3 2 + 3

echo "\n"
echo "Unit Test ID 3: ./calc_client $1 2 + 3"
echo "Expected result: 5.00 (normal case +)"
./calc_client $1 2 + 3

echo "\n"
echo "Unit Test ID 4: ./calc_client $1 2 - 3"
echo "Expected result: -1.00 (normal case -)"
./calc_client $1 2 - 3

echo "\n"
echo "Unit Test ID 5: ./calc_client $1 2 x 3"
echo "Expected result: 6.00 (normal case x)"
./calc_client $1 2 x 3

echo "\n"
echo "Unit Test ID 6: ./calc_client $1 22 / 7"
echo "Expected result: 3.14 (normal case /; approx. of PI)"
./calc_client $1 22 / 7

echo "\n"
echo "Unit Test ID 7: ./calc_client $1 2 / 0"
echo "Expected result: SRVR_UNDEFINED (handle divide by 0)"
./calc_client $1 2 / 0

echo "\n"
echo "Unit Test ID 8: ./calc_client $1 2 ? 3"
echo "Expected result: SRVR_INVALID_OPERATOR (handle unsupported operator)"
./calc_client $1 2 ? 3

echo "\n"
echo "Unit Test ID 9c: ./calc_client $1 10000000000 + 1234567890"
echo "Expected result: SRVR_OVERFLOW (handle overflow) on Client-side"
echo "Known-Issue: If client does not check for overflow,"
echo "             then left-operator will overflow as"
echo "                  cli-argument exceeds INT_MAX."
./calc_client $1 10000000000 + 1234567890

echo "\n"
echo "Unit Test ID 9s: ./calc_client $1 1000000000 + 1234567890"
echo "Expected result: SRVR_OVERFLOW (handle overflow) on Server-side"
./calc_client $1 1000000000 + 1234567890

## end of unit tests
exit 0
