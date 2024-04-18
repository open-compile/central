//
// Created by lugt on 24-4-17.
//

#include "arithm_util.h"

/**
 * Calculate the right most bit
 * @param n
 * @return
 */
INT32 Right_most_bit(UINT32 n) {
  if (n == 0)
    return -1; // No bits are set

  // Calculate the position of the rightmost set bit
  int position = 1; // Position starts at 1, not 0
  unsigned int mask = n & (-n);

  // Shift the mask until we find the set bit
  while ((mask & 1) == 0) {
    mask >>= 1;
    position++;
  }

  return position;
}