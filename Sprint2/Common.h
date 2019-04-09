#pragma once

// General utility macros

#if defined( ARRAY_COUNT )
#undef ARRAY_COUNT
#endif
#define ARRAY_COUNT( x ) ( sizeof( ( x ) ) / sizeof( ( x )[ 0 ] ) )

#if defined( BIT )
#undef BIT
#endif
#define BIT( x ) ( 1 << ( x ) )