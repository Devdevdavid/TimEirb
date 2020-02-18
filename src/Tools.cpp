#include <cstdio>
#include "Tools.h"

/**
 * @brief Get the index of the minimum value still positive
 */
int64_t get_min_pos_value(int64_t array[], int len)
{
	int64_t minValue = INT64_MAX;
	int minIndex = 0;

	for (int i = 0; i < len; ++i) {
		// Ignore negative value
		if (array[i] <= 0) {
			continue;
		}
		// Test min
		if (minValue >= array[i]) {
			minValue = array[i];
			minIndex = i;
		}
	}

	return minValue;
}