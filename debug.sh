#!/bin/bash

# TP2 - Test cache configurations (DEBUG VERSION)

echo "Testing cache configurations..."
echo ""

# Output file
RESULTS_FILE="results_tp2.txt"
echo "" > $RESULTS_FILE

# Compile with specific sizes
compile_with_sizes() {
  echo "  Cleaning..."
  make clean
  echo "  Compiling with L1=$1, L2=$2, L3=$3..."
  make CFLAGS="-Wall -Wextra -std=c11 -g -Iinclude -I.  -DL1_SIZE=$1 -DL2_SIZE=$2 -DL3_SIZE=$3"
}

# Test ONE configuration first
L1=8
L2=16
L3=32

echo "========================================="
echo "Testing L1=$L1, L2=$L2, L3=$L3"
echo "========================================="

compile_with_sizes $L1 $L2 $L3

if [ $? -ne 0 ]; then
  echo "ERROR: Compilation failed!"
  exit 1
fi

echo ""
echo "========================================="
echo "Running program..."
echo "========================================="

./bin/program

echo ""
echo "========================================="
echo "Program output finished"
echo "========================================="