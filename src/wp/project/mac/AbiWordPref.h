#include <MacHeaders.h>

#define ABIWORD_APP_NAME "AbiWord"


/*
	defined to 1 if we compile for Classic. Note that Universal Headers
	below 3.3 target for Classic
*/
#if (TARGET_API_MAC_CARBON < 0x0330) || TARGET_API_MAC_OS8
#define XP_MAC_TARGET_CLASSIC	1
#endif

/*
	defined to 1 if we compile for Classic
*/
#if TARGET_API_MAC_CARBON
#define XP_MAC_TARGET_CARBON 	1
#endif


// final definitions
#if !defined (XP_MAC_TARGET_CARBON)
#define XP_MAC_TARGET_CARBON 	0
#endif

#if !defined (XP_MAC_TARGET_CLASSIC)
#define XP_MAC_TARGET_CLASSIC 0
#endif

#if XP_MAC_TARGET_CARBON && XP_MAC_TARGET_CLASSIC
#error "Target Inconsistant: can't target both Class and Carbon"
#endif
