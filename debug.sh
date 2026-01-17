#!/bin/bash

# TP2 - Test cache configurations

echo "Testing cache configurations..."
echo ""

RESULTS_FILE="results_tp2.txt"
echo "" > $RESULTS_FILE

# Compile with specific sizes
compile_with_sizes() {
  echo "  Cleaning..."
  rm -rf build bin
  
  echo "  Compiling with L1=$1, L2=$2, L3=$3..."
  make CFLAGS="-Wall -Wextra -std=c11 -g -Iinclude -I.  -DL1_SIZE=$1 -DL2_SIZE=$2 -DL3_SIZE=$3" > /dev/null 2>&1
  
  return $? 
}

# Extract statistics (FIXED TIME EXTRACTION!)
extract_stats() {
  local output="$1"
  
  # Extract hit rates
  local l1_rate=$(echo "$output" | grep -A3 "L1 Cache Statistics" | grep "Hit Rate:" | grep -oP '\d+\.\d+' | head -1)
  local l2_rate=$(echo "$output" | grep -A3 "L2 Cache Statistics" | grep "Hit Rate:" | grep -oP '\d+\.\d+' | head -1)
  local l3_rate=$(echo "$output" | grep -A3 "L3 Cache Statistics" | grep "Hit Rate:" | grep -oP '\d+\.\d+' | head -1)
  
  # Extract totals
  local total_accesses=$(echo "$output" | grep "Total Memory Accesses:" | grep -oP '\d+' | head -1)
  local total_misses=$(echo "$output" | grep "Total Cache Misses:" | grep -oP '\d+' | head -1)
  
  # FIX: Extract ONLY the number from time line
  local total_time=$(echo "$output" | grep "Total Time" | grep -oP '\d+' | head -1)
  
  # Calculate RAM rate
  local ram_rate="0.00"
  if [ -n "$total_accesses" ] && [ -n "$total_misses" ] && [ "$total_accesses" -gt 0 ]; then
    ram_rate=$(awk "BEGIN {printf \"%.2f\", ($total_misses * 100.0) / $total_accesses}")
  fi
  
  local disk_rate="0.00"
  
  # Defaults
  l1_rate=${l1_rate:-0.00}
  l2_rate=${l2_rate:-0.00}
  l3_rate=${l3_rate:-0.00}
  total_time=${total_time:-0}
  
  echo "$l1_rate|$l2_rate|$l3_rate|$ram_rate|$disk_rate|$total_time"
}

configs=(
  "8 16 32"
  "32 64 128"
  "16 64 256"
  "8 32 128"
  "16 32 64"
)

echo "Cache 1  | Cache 2  | Cache 3  | Taxa C1 % | Taxa C2 % | Taxa C3 % | Taxa de RAM % | Taxa de disco %  | Tempo de execução (unidade)" | tee -a $RESULTS_FILE
echo "---------|----------|----------|-----------|-----------|-----------|---------------|------------------|-------------------------------" | tee -a $RESULTS_FILE

for config in "${configs[@]}"; do
  L1=$(echo $config | awk '{print $1}')
  L2=$(echo $config | awk '{print $2}')
  L3=$(echo $config | awk '{print $3}')
  
  echo ""
  echo "Testing L1=$L1, L2=$L2, L3=$L3..."
  
  compile_with_sizes $L1 $L2 $L3
  
  if [ $? -ne 0 ]; then
    echo "  ❌ Compilation FAILED!"
    continue
  fi
  
  output=$(./bin/exe 2>&1)
  stats=$(extract_stats "$output")
  
  IFS='|' read -r l1_rate l2_rate l3_rate ram_rate disk_rate time <<< "$stats"
  
  printf "%8s | %8s | %8s | %9s | %9s | %9s | %13s | %16s | %29s\n" \
    "$L1" "$L2" "$L3" "$l1_rate%" "$l2_rate%" "$l3_rate%" "$ram_rate%" "$disk_rate%" "$time" | tee -a $RESULTS_FILE
done

rm -rf build bin
make > /dev/null 2>&1

echo ""
echo "✓ Done!  Results saved to results_tp2.txt"
echo ""
cat $RESULTS_FILE