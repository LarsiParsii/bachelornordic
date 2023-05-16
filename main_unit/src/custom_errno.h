/**
 * @file
 * @brief Error codes custom to the MOB system
 */

#ifndef custom_errno_H
#define custom_errno_H

#include <errno.h>

#define ENODEVINIT 150		/**< No sensor devices initialized */
#define ESOMEDEVINIT 151	/**< Some sensor devices initialized */
#define ENODEVREAD 152		/**< No sensor devices read */
#define ESOMEDEVREAD 153	/**< Some sensor devices read */

#endif  // custom_errno_H