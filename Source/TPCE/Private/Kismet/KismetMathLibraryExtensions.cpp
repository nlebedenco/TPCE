// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/KismetMathLibraryExtensions.h"
#include "Math/MathExtensions.h"

UKismetMathLibraryEx::UKismetMathLibraryEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UKismetMathLibraryEx::RayPlaneIntersection(const FVector& RayStart, const FVector& RayDir, const FPlane& APlane, float& T, FVector& Intersection)
{
	// Check ray is not parallel to plane
	if (FMath::IsNearlyZero((RayDir | APlane), SMALL_NUMBER))
	{
		T = 0.0f;
		Intersection = FVector::ZeroVector;
		return false;
	}

	T = ((APlane.W - (RayStart | APlane)) / (RayDir | APlane));

	// Calculate intersection point
	Intersection = RayStart + RayDir * T;

	return true;
}


FBounds UKismetMathLibraryEx::Multiply_BoundsFloat(FBounds A, float B)
{
	return A * B;
}

FBounds UKismetMathLibraryEx::Multiply_BoundsInt(FBounds A, int32 B)
{
	return A * B;
}

FBounds UKismetMathLibraryEx::Multiply_BoundsBounds(FBounds A, const FBounds&  B)
{
	return A * B;
}

FBounds UKismetMathLibraryEx::Divide_BoundsFloat(FBounds A, float B)
{
	return A / B;
}

FBounds UKismetMathLibraryEx::Divide_BoundsInt(FBounds A, int32 B)
{
	return A / B;
}

FBounds UKismetMathLibraryEx::Divide_BoundsBounds(FBounds A, const FBounds&  B)
{
	return A / B;
}

FBounds UKismetMathLibraryEx::Add_BoundsBounds(FBounds A, const FBounds&  B)
{
	return A + B;
}

FBounds UKismetMathLibraryEx::Add_BoundsFloat(FBounds A, float B)
{
	return A + B;
}

FBounds UKismetMathLibraryEx::Add_BoundsInt(FBounds A, int32 B)
{
	return A + B;
}

FBounds UKismetMathLibraryEx::Subtract_BoundsBounds(FBounds A, const FBounds&  B)
{
	return A - B;
}

FBounds UKismetMathLibraryEx::Subtract_BoundsFloat(FBounds A, float B)
{
	return A - B;
}

FBounds UKismetMathLibraryEx::Subtract_BoundsInt(FBounds A, int32 B)
{
	return A - B;
}

bool UKismetMathLibraryEx::EqualEqual_BoundsBounds(const FBounds&  A, const FBounds&  B, float ErrorTolerance)
{
	return A.Equals(B, ErrorTolerance);
}

bool UKismetMathLibraryEx::NotEqual_BoundsBounds(const FBounds&  A, const FBounds&  B, float ErrorTolerance)
{
	return !A.Equals(B, ErrorTolerance);
}

bool UKismetMathLibraryEx::BoundsIsReversed(const FBounds& A)
{
	return A.IsReversed();
}

float UKismetMathLibraryEx::BoundsLength(const FBounds& A)
{
	return A.Length();
}

bool UKismetMathLibraryEx::BoundsContains(const FBounds& A, const float Value)
{
	return A.Contains(Value);
}

void UKismetMathLibraryEx::BoundsExpand(FBounds& A, float Amount)
{
	A.Expand(Amount);
}

void UKismetMathLibraryEx::BoundsInclude(FBounds& A, float Value)
{
	A.Include(Value);
}

float UKismetMathLibraryEx::BoundsInterpolate(const FBounds& A, float Alpha)
{
	return A.Interpolate(Alpha);
}

ECardinalDirection UKismetMathLibraryEx::CalculateCardinalDirection(float Angle, ECardinalDirection CurrentCardinalDirection, const float NorthSegmentHalfWidth, float Buffer)
{
	return FMathEx::FindCardinalDirection(Angle, CurrentCardinalDirection, NorthSegmentHalfWidth, Buffer);
}
