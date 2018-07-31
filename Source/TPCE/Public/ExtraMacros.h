// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HAL/Platform.h"
#include "Misc/AssertionMacros.h"

// Compile-time warnings and errors. Use these as "#pragma COMPILER_WARNING("XYZ")". GCC does not expand macro parameters to _Pragma, so we can't wrap the #pragma part.
#ifdef _MSC_VER
#define COMPILE_INFO(x) __pragma(message(__FILE__ "(" MSC_FORMAT_DIAGNOSTIC_HELPER(__LINE__) "): " x))
#define COMPILE_MESSAGE(x) COMPILE_INFO(x)
#define TODO(x) __pragma(message(__FILE__ "(" MSC_FORMAT_DIAGNOSTIC_HELPER(__LINE__) ") [TODO]: " x))
#else
#define COMPILE_INFO(x) GCC_DIAGNOSTIC_HELPER(x)
#define COMPILE_MESSAGE(x) COMPILE_INFO(x)
#define TODO(x) GCC_DIAGNOSTIC_HELPER("[TODO]:" x)
#endif

#if (UE_BUILD_TEST || UE_BUILD_SHIPPING)
#define FULL_OVERRIDE()
#else
#define FULL_OVERRIDE() COMPILE_INFO(__FUNCTION__ " marked as full override.")
#endif

 /*----------------------------------------------------------------------------
 Check, verify, etc macros
 ----------------------------------------------------------------------------*/

 /** Specialized check macro to express expectations on net role for actors */
#define checkActorRoleAtLeast(minRole) checkfSlow(Role >= minRole, TEXT("Expected net role of " #minRole " or above"));
#define checkActorRoleExactly(minRole) checkfSlow(Role == minRole, TEXT("Expected net role of " #minRole));

 /** Specialized check macro to express expectations on net role for components */
#define checkComponentRoleAtLeast(minRole) checkfSlow(!GetIsReplicated() || GetOwnerRole() >= minRole, TEXT("Expected replication to be disabled or net role to be " #minRole " or above"));
#define checkComponentRoleExactly(minRole) checkfSlow(!GetIsReplicated() || GetOwnerRole() == minRole, TEXT("Expected replication to be disabled or net role to be " #minRole));

 /** Specialized ensure macro to express expectations on net role for actors */
#define ensureActorRoleAtLeast(minRole) ensureMsg(Role >= minRole, TEXT("Expected net role of " #minRole " or above"));
#define ensureActorRoleExactly(minRole) ensureMsg(Role == minRole, TEXT("Expected net role of " #minRole));

 /** Specialized ensure macro to express expectations on net role for components */
#define ensureComponentRoleAtLeast(minRole) ensureMsg(!GetIsReplicated() || GetOwnerRole() >= minRole, TEXT("Expected replication to be disabled or net role to be " #minRole " or above"));
#define ensureComponentRoleExactly(minRole) ensureMsg(!GetIsReplicated() || GetOwnerRole() == minRole, TEXT("Expected replication to be disabled or net role to be " #minRole));
