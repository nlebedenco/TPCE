// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/KismetMathLibrary.h"
#include "Math/Bounds.h"
#include "ExtraTypes.h"

#include "KismetMathLibraryExtensions.generated.h"


UCLASS(meta = (BlueprintThreadSafe))
class TPCE_API UKismetMathLibraryEx : public UKismetMathLibrary
{
	GENERATED_UCLASS_BODY()

public:

	//
	// Intersection
	//

	/**
	 * Computes the intersection point between a line and a plane.
	 * @param		T - The t of the intersection between the line and the plane
	 * @param		Intersection - The point of intersection between the line and the plane
	 * @return		True if the intersection test was successful.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Intersection")
	static bool RayPlaneIntersection(const FVector& RayStart, const FVector& RayDir, const FPlane& APlane, float& T, FVector& Intersection);

	//
	// Bounds functions
	//

	/** Scales Bounds A by B */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds * float", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|Bounds")
	static FBounds Multiply_BoundsFloat(FBounds  A, float B);

	/** Scales Bounds A by B */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds * int", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|Bounds")
	static FBounds Multiply_BoundsInt(FBounds A, int32 B);

	/** Element-wise Bounds multiplication (Result = {A.x*B.x, A.y*B.y, A.z*B.z}) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds * bounds", CompactNodeTitle = "*", Keywords = "* multiply", CommutativeAssociativeBinaryOperator = "true"), Category = "Math|Bounds")
	static FBounds Multiply_BoundsBounds(FBounds A, const FBounds&  B);

	/** Bounds divide by a float */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds / float", CompactNodeTitle = "/", Keywords = "/ divide division"), Category = "Math|Bounds")
	static FBounds Divide_BoundsFloat(FBounds A, float B = 1.f);

	/** Bounds divide by an integer */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds / int", CompactNodeTitle = "/", Keywords = "/ divide division"), Category = "Math|Bounds")
	static FBounds Divide_BoundsInt(FBounds A, int32 B = 1);

	/** Element-wise Bounds division (Result = {A.x/B.x, A.y/B.y, A.z/B.z}) */
	UFUNCTION(BlueprintPure, meta = (AutoCreateRefTerm = "B", DisplayName = "bounds / bounds", CompactNodeTitle = "/", Keywords = "/ divide division"), Category = "Math|Bounds")
	static FBounds Divide_BoundsBounds(FBounds A, const FBounds& B);

	/** Bounds addition */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds + bounds", CompactNodeTitle = "+", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"), Category = "Math|Bounds")
	static FBounds Add_BoundsBounds(FBounds A, const FBounds& B);

	/** Adds a float to each component of bounds */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds + float", CompactNodeTitle = "+", Keywords = "+ add plus"), Category = "Math|Bounds")
	static FBounds Add_BoundsFloat(FBounds A, float B);

	/** Adds an integer to each component of bounds */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds + int", CompactNodeTitle = "+", Keywords = "+ add plus"), Category = "Math|Bounds")
	static FBounds Add_BoundsInt(FBounds A, int32 B);

	/** Bounds subtraction */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds - bounds", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "Math|Bounds")
	static FBounds Subtract_BoundsBounds(FBounds A, const FBounds& B);

	/** Subtracts a float from each component of bounds */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds - float", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "Math|Bounds")
	static FBounds Subtract_BoundsFloat(FBounds A, float B);

	/** Subtracts an integer from each component of bounds */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "bounds - int", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "Math|Bounds")
	static FBounds Subtract_BoundsInt(FBounds A, int32 B);

	/** Returns true if bounds A is equal to bounds B (A == B) within a specified error tolerance */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (bounds)", CompactNodeTitle = "==", Keywords = "== equal"), Category = "Math|Bounds")
	static bool EqualEqual_BoundsBounds(const FBounds&  A, const FBounds& B, float ErrorTolerance = 1.e-4f);

	/** Returns true if bounds A is not equal to bounds B (A != B) within a specified error tolerance */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Not Equal (bounds)", CompactNodeTitle = "!=", Keywords = "!= not equal"), Category = "Math|Bounds")
	static bool NotEqual_BoundsBounds(const FBounds&  A, const FBounds& B, float ErrorTolerance = 1.e-4f);

	/** Returns true if bounds is valid(Min <= Max). */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IsReversed"), Category = "Math|Bounds")
	static bool BoundsIsReversed(const FBounds& A);

	/** Returns length of bounds (Max - Min). */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Length"), Category = "Math|Bounds")
	static float BoundsLength(const FBounds& A);

	/** Returns true if value is within bounds (Min <= Value <= Max). */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Contains"), Category = "Math|Bounds")
	static bool BoundsContains(const FBounds& A, const float Value);

	/** Expands bounds to both sides by the specified amount. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Expand"), Category = "Math|Bounds")
	static void BoundsExpand(FBounds& A, float Amount);

	/** Expands bounds if necessary to include the specified value. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Includes"), Category = "Math|Bounds")
	static void BoundsInclude(FBounds& A, float Value);

	/** Returns a value from Min to Max interpolated by Alpha. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Interpolate"), Category = "Math|Bounds")
	static float BoundsInterpolate(const FBounds& A, float Alpha);

	/** Returns a value from Min to Max interpolated by Alpha. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "CalculateCardinalDirection"), Category = "Math|Orientation")
	static ECardinalDirection CalculateCardinalDirection(float Angle, ECardinalDirection CurrentCardinalDirection, const float NorthSegmentHalfWidth, float Buffer);
};