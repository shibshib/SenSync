/*
 * hal.h
 *
 *  Created on: Feb 5, 2017
 *      Author: Ala
 */

#ifndef HAL_H_
#define HAL_H_

/*----------------------------------------------------------------------------
 * The following function names are deprecated.  These were updated to new
 * names to follow OneMCU naming convention.
 +---------------------------------------------------------------------------*/

#ifndef DEPRECATED
#define   initPorts       USBHAL_initPorts
#define   initClocks      USBHAL_initClocks
#endif

void initClocks(void);
void USBHAL_initPorts(void);

#endif /* HAL_H_ */
