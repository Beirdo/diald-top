/*
 * $Id$
 */

#ifndef __loglevels_h
#define __loglevels_h

#include <syslog.h>

/*
 * #define LOGOPTIONS   (LOG_PERROR | LOG_PID) 
 */
#define LOGOPTIONS	LOG_PID
#define LOGFACILITY	LOG_LOCAL5

#define LOGERR		(LOGFACILITY | LOG_ERR)
#define LOGNOTICE	(LOGFACILITY | LOG_NOTICE)
#define LOGINFO		(LOGFACILITY | LOG_INFO)
#define LOGDEBUG	(LOGFACILITY | LOG_DEBUG)

#ifndef LOG_PERROR
#define LOG_PERROR 0	/* For Solaris */
#endif

#endif
