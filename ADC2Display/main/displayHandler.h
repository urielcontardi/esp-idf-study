/// \file		displayHandler.h
///
/// \brief	
///
/// \author		Uriel Abe Contardi (urielcontardi@hotmail.com)
/// \date		08-09-2024
///
/// \version	1.0
///
/// \note		Revisions:
/// 			08-09-2024 <urielcontardi@hotmail.com>
/// 			First revision.

#pragma once
#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                               INCLUDES                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "lvgl.h"

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                         TYPEDEFS AND STRUCTURES                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            EXPORTED FUNCTIONS                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void displayHandlerInit(void);
bool lvglLock(int timeout_ms);
void lvglUnlock(void);
void displayHandlerUpdateData(lv_coord_t value) ;

#endif // DISPLAY_HANDLER_H