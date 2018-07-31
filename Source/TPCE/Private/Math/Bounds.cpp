// Fill out your copyright notice in the Description page of Project Settings.

#include "Math/Bounds.h"

#include "Math/NumericLimits.h"
// #include "Math/UnrealMathUtility.h"

FBounds::FBounds():
	LowerBound(TNumericLimits<float>::Min()),
	UpperBound(TNumericLimits<float>::Max())
	{ }

FBounds::FBounds(float InLowerBound, float InUpperBound):
	LowerBound(InLowerBound),
	UpperBound(InUpperBound)
	{ }

FBounds::FBounds(const FBounds& Other)
{
	LowerBound = Other.LowerBound;
	UpperBound = Other.UpperBound;
}

FBounds::FBounds(const FVector2D& Other)
{
	LowerBound = Other.X;
	UpperBound = Other.Y;
}

void FBounds::Expand(float ExpandAmount)
{
	if (IsReversed())
	{
		LowerBound += ExpandAmount;
		UpperBound -= ExpandAmount;
	}
	else
	{
		LowerBound -= ExpandAmount;
		UpperBound += ExpandAmount;
	}
}

void FBounds::Include(float X)
{
	if (IsReversed())
	{
		if (X > LowerBound)
			LowerBound = X;

		if (X < UpperBound)
			UpperBound = X;
	}
	else
	{
		if (X < LowerBound)
			LowerBound = X;

		if (X > UpperBound)
			UpperBound = X;
	}
}