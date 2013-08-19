/*
 * interval.cpp
 *
 *  Created on: Aug 6, 2013
 *      Author: jucs
 */

#include "interval.h"

bool interval::overlaps(interval *other) {
	if(start <= other->start)
		return other->start <= end;
	else
		return other->end >= start;
}

bool interval::starts_with(interval *other) {
	return other->start <= start && other->end <= end;
}

size_t interval::get_size() {
	return end - start + 1;
}

bool operator <=(size_t const &spot, interval &interval) {
	return (spot >= interval.start && spot <= interval.end);
}

bool operator <=(interval &a, interval &b) {
	return a.start >= b.start && a.end <= b.end;
}

bool operator >=(interval &a, interval &b) {
	return b <= a;
}

bool operator ==(interval &a, interval &b) {
	return a.start == b.start && a.end == b.end;
}