/*******************************************************************

(C) Copy Rights: 1998 Hiroyuki Kuwahara


DESIGNER:		Hiroyuki Kuwahara

FILE NAME:		type.h   

DESCRIPTION:	define all the type used 
*******************************************************************/
#if !defined(HAVE_TYPE)
#	define HAVE_TYPE
	

#	if defined(__cplusplus)
		extern "C" {
#	endif
	
/**************************************/

#	if !defined(AND)
#		define AND &&
#	endif

#	if !defined(OR)
#		define OR ||
#	endif


#	if !defined(BOOL)
#		define BOOL int
#	endif
			
#	if !defined(FALSE)
#		define FALSE 0
#	endif

#	if !defined(TRUE)
#		define TRUE (!FALSE)
#	endif


#	if !defined(BYTE)
#		define BYTE unsigned char
#	endif


#	if !defined(UINT16)
#		define UINT16 unsigned short
#	endif

#	if !defined(UINT32)
#		define UINT32 unsigned long
#	endif

#	if !defined(UINT)
#		define UINT unsigned int
#	endif


#	if !defined(INT16)
#		define INT16 short
#	endif

#	if !defined(INT32)
#		define INT32 long
#	endif

#	if !defined(INT)
#		define INT int
#	endif


#	if !defined(CADDR_T)
#		define CADDR_T char*
#	endif

#	if !defined(LPSTR)
#		define LPSTR char*
#	endif

#	if !defined(LPCSTR)
#		define LPCSTR const char*
#	endif

#	if !defined(ARG)
#		define ARG char*
#	endif
			

#	if !defined(UINT32_MAX)
#		define UINT32_MAX  (UINT32)0xFFFFFFFF
#	endif

#	if !defined(UINT32_MIN)
#		define UINT32_MIN (UINT32)0
#	endif

#	if !defined(UINT16_MAX)
#		define UINT16_MAX  (UINT16)0xFFFF
#	endif

#	if !defined(UINT16_MIN)
#		define UINT16_MIN (UINT16)0
#	endif

#	if !defined(INT32_MAX)
#		define INT32_MAX  (INT32)0x7FFFFFFF
#	endif

#	if !defined(INT32_MIN)
#		define INT32_MIN  (INT32)0x80000000
#	endif

#	if !defined(INT16_MAX)
#		define INT16_MAX  (INT16)0x7FFF
#	endif

#	if !defined(INT16_MIN)
#		define INT16_MIN  (INT16)0x8000
#	endif

/**************************************/



/**************************************/
#	if !defined(GET_MAX)
#		define GET_MAX(a,b) (((a)>(b))?(a):(b))
#	endif 

#	if !defined(GET_MIN)
#		define GET_MIN(a,b) (((a)<(b))?(a):(b))
#	endif 

#	if !defined(ROUND_DOUBLE)
#		define ROUND_DOUBLE(a) (((a)>0)?\
			(double)(INT32)((a)+0.5):(double)(INT32)((a)-0.5))
#	endif

#	if !defined(ROUND_INT32)
#		define ROUND_INT32(a) (((a)>0)?\
			(INT32)((a)+0.5):(INT32)((a)-0.5))
#	endif

#	if !defined(ROUND_INT16)
#		define ROUND_INT16(a) (((a)>0)?\
			(INT16)((a)+0.5):(INT16)((a)-0.5))
#	endif

#	if !defined(GET_MID_DOUBLE)
#		define GET_MID_DOUBLE(a,b) (((a)+(b))/2)		 
#	endif

#	if !defined(GET_MID_INT)
#		define GET_MID_INT(a,b) (((a)+(b))>>1)		 
#	endif

#	if !defined(LG_UINT32)
#		define LG_UINT32(x,a) { \
		a=0; \
		UINT32 xx = (UINT32)(x); \
		while(xx>>=1) a++; }
#	endif

#	if !defined(IS_POW2)
#		define IS_POW2(x) (((x)>0)&& !((x) & ((x)-1))?TRUE:FALSE)
#	endif


#	if !defined(BZERO)
#		define BZERO(b,s) memset((CADDR_T)(b),0,s) 
#	endif

#	if !defined(FILL_ZERO)
#		define FILL_ZERO(b) memset((CADDR_T)(b),0,sizeof(b)) 
#	endif

#	if !defined(BONE)
#		define BONE(b,s) memset((CADDR_T)(b),0XFF,s) 
#	endif

#	if !defined(FILL_ONE)
#		define FILL_ONE(b) memset((CADDR_T)(b),0XFF,sizeof(b)) 
#	endif

/**************************************/


/**************************************/
#	define MALLOC(size) ((size==0)?NULL:malloc(size))
#	define CALLOC(n,size) (((size==0)||(n==0))?NULL:calloc(n,size))
#	define REALLOC(ptr,size) (((size==0)&&(ptr==NULL))?NULL:realloc(ptr,size))

#	define FREE(ptr) \
	{\
		free(ptr);\
		ptr=NULL;\
	}
	 	
/**************************************/



/**************************************/
	typedef UINT32  RET_VAL;  
#	define SUCCESS			0X00000000
#	define WARNING			0X00000001
#	define FAILING			0X00000002 
#	define CHANGE			0X00000004 


#	define IS_FAILED(r)		((r)&FAILING)
#	define IS_OK(r)			(!((r)&FAILING))		

#	define HALF_OF(num) ((num)>>1)
#	define TWICE_OF(num) ((num)<<1) 	
/***************************************/
#define FILE_NAME_SIZE 256

#define IS_REAL_EQUAL(a,b) ( fabs((a)-(b)) < 2E-16 )



#if defined(WIN32)
#define FILE_SEPARATOR '\\'
#define NEW_LINE "\r\n"
#else
#define FILE_SEPARATOR '/'
#define NEW_LINE "\n"
#endif

#	if defined(__cplusplus)
		}
#	endif

#if defined(__cplusplus)
#  define BEGIN_C_NAMESPACE extern "C" {
#  define END_C_NAMESPACE   }
#else
#  define BEGIN_C_NAMESPACE
#  define END_C_NAMESPACE
#endif



#endif
