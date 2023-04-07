#!/bin/sh

RESMGR_NAME="metronome"
RESMGR_HELP="$RESMGR_NAME-help"
RESMGR_PATH="/dev/local"

BIN_NAME="metronome_resmgr"
BIN_PATH="/tmp"

cd "$BIN_PATH" || (echo "Error: Failed to cd into $BIN_PATH" && exit 1)

[ ! -f "$BIN_NAME" ] && echo "Error: $BIN_NAME does not exist in $BIN_PATH" && exit 1

# Test A
printf "\nExpected: Usage Message\nActual:\n"
./$BIN_NAME

# Test B
printf "\nExpected: 1 measure per second\nActual:\n"
./$BIN_NAME 120 2 4 &
sleep 5

# Test C
printf "\nExpected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000\nActual:\n"
cat "$RESMGR_PATH/$RESMGR_NAME"

# Test D
printf "\nExpected: Information regarding the resource manager's API\nActual:\n"
cat "$RESMGR_PATH/$RESMGR_HELP"

# Test E
printf "\nExpected: Metronome changes to 100bpm 2/4 time\nActual:\n"
echo "set 100 2 4" > "$RESMGR_PATH/$RESMGR_NAME"
sleep 5

# Test F
printf "\nExpected: [metronome: 100 beats/min, time signature 2/4, secs-per-interval: 0.30, nanoSecs: 300000000\nActual:\n"
cat "$RESMGR_PATH/$RESMGR_NAME"

# Test G
printf "\nExpected: Metronome changes to 200bpm 5/4 time\nActual:\n"
echo "set 200 5 4" > "$RESMGR_PATH/$RESMGR_NAME"
sleep 5

# Test H
printf "\nExpected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000\nActual:\n"
cat "$RESMGR_PATH/$RESMGR_NAME"

# Test I
printf "\nExpected: Metronome stops running. Resource manager is still running as a process\nActual:\n"
echo "stop" > "$RESMGR_PATH/$RESMGR_NAME"
pidin | grep "$RESMGR_NAME"

# Test J
printf "\nExpected: Metronome starts running again at 200bpm 5/4 time\nActual:\n"
echo "start" > "$RESMGR_PATH/$RESMGR_NAME"
pidin | grep "$RESMGR_NAME"
sleep 5

# Test K
printf "\nExpected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000\nActual:\n"
cat "$RESMGR_PATH/$RESMGR_NAME"

# Test L
printf "\nExpected: Metronome stops running. Resource manager is still running as a process\nActual:\n"
echo "stop" > "$RESMGR_PATH/$RESMGR_NAME"
pidin | grep "$RESMGR_NAME"

# Test M
printf "\nExpected: Metronome stops running. Resource manager is still running as a process\nActual:\n"
echo "stop" > "$RESMGR_PATH/$RESMGR_NAME"
pidin | grep "$RESMGR_NAME"

# Test N
printf "\nExpected: Metronome starts running again at 200bpm 5/4 time\nActual:\n"
echo "start" > "$RESMGR_PATH/$RESMGR_NAME"
pidin | grep "$RESMGR_NAME"
sleep 5

# Test O
printf "\nExpected: Metronome is still running again at 200bpm 5/4 time\nActual:\n"
echo "start" > "$RESMGR_PATH/$RESMGR_NAME"
pidin | grep "$RESMGR_NAME"
sleep 5

# Test P
printf "\nExpected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000\nActual:\n"
cat "$RESMGR_PATH/$RESMGR_NAME"

# Test Q
printf "\nExpected: Metronome continues on next beat\nActual:\n"
echo "pause 3" > "$RESMGR_PATH/$RESMGR_NAME"
sleep 5

# Test R
printf "\nExpected: Properly formatted error message, and metronome continues to run\nActual:\n"
echo "pause 10" > "$RESMGR_PATH/$RESMGR_NAME"

# Test S
printf "\nExpected: Properly formatted error message, and metronome continues to run\nActual:\n"
echo "bogus" > "$RESMGR_PATH/$RESMGR_NAME"

# Test T
printf "\nExpected: 1 measure per second\nActual:\n"
echo "set 120 2 4" > "$RESMGR_PATH/$RESMGR_NAME"
sleep 5

# Test U
printf "\nExpected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000\nActual:\n"
cat "$RESMGR_PATH/$RESMGR_NAME"

# Test V
printf "\nExpected: Information regarding the resource manager's API\nActual:\n"
cat "$RESMGR_PATH/$RESMGR_HELP"

# Test W
printf "\nExpected: Properly formatted error message, and metronome continues to run\nActual:\n"
echo "Writes-Not_Allowed" > "$RESMGR_PATH/$RESMGR_HELP"

# Test X
printf "\nExpected: Metronome gracefully terminates\nActual:\n"
echo "quit" > "$RESMGR_PATH/$RESMGR_NAME"
