#ifndef CoreTime_h
#define CoreTime_h
#include "../../Pure.h"

static inline uint64_t seconds_to_milliseconds ( uint64_t seconds );
static inline uint64_t secs_to_millis          ( uint64_t secs );

static inline uint64_t milliseconds_to_seconds ( uint64_t milliseconds );
static inline uint64_t millis_to_secs          ( uint64_t millis );

static inline uint64_t secs_to_millis( uint64_t secs )
{
	return seconds_to_milliseconds( secs );
}

static inline uint64_t seconds_to_milliseconds( uint64_t seconds )
{
	return seconds * 1000;
}

static inline uint64_t millis_to_secs( uint64_t millis )
{
	return milliseconds_to_seconds( millis );
}

static inline uint64_t milliseconds_to_seconds( uint64_t milliseconds )
{
	return milliseconds / 1000;
}

#endif // Time_h