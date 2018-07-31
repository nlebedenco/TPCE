// Fill copyright
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/KismetGuidLibrary.h"
#include "Kismet/KismetInputLibrary.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetNodeHelperLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetStringTableLibrary.h"
#include "Kismet/KismetTextLibrary.h"
#include "KismetMathLibraryExtensions.h"

static struct TPCE_API Kismet: public UKismetSystemLibrary
{
	typedef ::UKismetArrayLibrary Array;
	typedef ::UKismetGuidLibrary Guid;
	typedef ::UKismetInputLibrary Input;
	typedef ::UKismetInternationalizationLibrary i18n;
	typedef ::UKismetMaterialLibrary Material;
	typedef ::UKismetMathLibraryEx Math;
	typedef ::UKismetNodeHelperLibrary Util;
	typedef ::UKismetRenderingLibrary Rendering;
	typedef ::UKismetStringLibrary String;
	typedef ::UKismetStringTableLibrary StringTable;
	typedef ::UKismetTextLibrary Text;
};


/**
 *
 */
template <typename EnumType>
FORCEINLINE FString GetEnumeratorUserFriendlyName(const TCHAR* EnumClass, const EnumType EnumValue)
{
	if (const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, EnumClass, true))
		return EnumPtr->GetDisplayNameTextByValue((uint8)EnumValue).ToString();

	return FName().ToString();
}

/**
 *
 */
#define GetEnumeratorDisplayName(EnumClass, EnumValue)  GetEnumeratorUserFriendlyName(TEXT(#EnumClass), (EnumValue))
