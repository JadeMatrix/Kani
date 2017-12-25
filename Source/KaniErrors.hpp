#pragma once
#ifndef KANIERRORS_H
#define KANIERRORS_H


#include <CoreServices/CoreServices.h>

#include <string>


void KaniHandleOSErrorDebugLine( OSStatus, std::string );

#define KaniHandleOSErrorDebug( E ) KaniHandleOSErrorDebugLine( E, std::string( " at " ) + __FILE__ + ":" + std::to_string( __LINE__ ) )


#endif
