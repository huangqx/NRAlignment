//---------------------------------------------------------------------------
#ifndef PTypes_H
#define PTypes_H
//---------------------------------------------------------------------------
//#include "GlobalDefinitions.h"
//---------------------------------------------------------------------------

/*--------------------------------------------------------------------------------*/
/* ptypes.h: elementary datatypes to enable portability between                   */
/*           multiple platforms                                                   */
/*--------------------------------------------------------------------------------*/


//---------------------------------------------------------------------------

typedef signed   char            int8;
typedef signed   short int       int16;
typedef signed   int             int32;
typedef signed   __int64         int64;

typedef unsigned char            card8;
typedef unsigned short int       card16;
typedef unsigned int             card32;
typedef unsigned __int64         card64;

typedef float                    float32;
typedef double                   float64;

// native platform int-types, maximum sized ints that can be used for addressing (indexing, pointers)
typedef unsigned int             max_platform_card;
typedef signed int               max_platform_int;


//---------------------------------------------------------------------------

// shortcuts
typedef max_platform_card mpcard;
typedef max_platform_int  mpint;


#endif

