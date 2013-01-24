#ifndef __GS_MACROS__
#define __GS_MACROS__


#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y)  ((X) > (Y) ? (X) : (Y))


#if defined(GS_VERSION)
	#define GS_ON() GS_On()
#else
	#define GS_ON()
#endif



#if defined(GS_VERSION)
        #define GS_OFF(a) GS_Off(a)
#else
        #define GS_OFF(a) 
#endif



#if defined(GS_VERSION)
        #define GS_FOPEN(res, a, b) res = GS_FOpen(a, b)
#else
	#define GS_FOPEN(res, a, b) \
{ \
	switch(#b[0]) \
	{ \
		case 'R': res = fopen(a, "r"); \
			break; \
		case 'W': res = fopen(a, "w"); \
			break; \
		case 'A': res = fopen(a, "a"); \
			break; \
		default: \
			res = NULL; \
			break; \
	} \
}
#endif



#if defined(GS_VERSION)
        #define GS_FCLOSE(a) GS_FClose(a)
#else
        #define GS_FCLOSE(a) fclose(a) 
#endif


#if defined(GS_VERSION)
        #define GS_SYSTEM(a) gs_result = GS_System(a)
#else
        #define GS_SYSTEM(a) system(a)
#endif

#endif  /* __GS_MACROS __ */

