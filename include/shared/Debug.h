/**
 * Debug.h
 * @brief Any Necessary Debug Functions
 *
 * @author Ryan Massie (rmassie)
 * @date 3/27/26
 */
#ifndef DEBUG_LIBRARY_H
#define DEBUG_LIBRARY_H

#include <stdio.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) do {} while (0)
#endif

#endif // DEBUG_LIBRARY_H
