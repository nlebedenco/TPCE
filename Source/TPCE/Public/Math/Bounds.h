// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#include "Bounds.generated.h"

/**
 * Float bounds struct supported in blueprints. 
 */
USTRUCT(BlueprintType)
struct TPCE_API FBounds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowerBound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UpperBound;

public:

	FORCEINLINE FBounds();
	FORCEINLINE FBounds(float InMin, float InUpperBound);
	FORCEINLINE FBounds(const FBounds& Other);

	FORCEINLINE explicit FBounds(const FVector2D& Other);

public:

	FORCEINLINE explicit operator FVector2D() const
	{
		return FVector2D(LowerBound, UpperBound);
	}

	FORCEINLINE FBounds& operator+= (float X)
	{
		LowerBound += X;
		UpperBound += X;

		return *this;
	}

	FORCEINLINE FBounds& operator+= (int32 X)
	{
		LowerBound += X;
		UpperBound += X;

		return *this;
	}

	FORCEINLINE FBounds& operator+= (const FBounds& X)
	{
		LowerBound += X.LowerBound;
		UpperBound += X.UpperBound;

		return *this;
	}

	FORCEINLINE FBounds& operator-= (float X)
	{
		LowerBound -= X;
		UpperBound -= X;

		return *this;
	}

	FORCEINLINE FBounds& operator-= (int32 X)
	{
		LowerBound -= X;
		UpperBound -= X;

		return *this;
	}

	FORCEINLINE FBounds& operator-= (const FBounds& X)
	{
		LowerBound -= X.LowerBound;
		UpperBound -= X.UpperBound;

		return *this;
	}

	FORCEINLINE FBounds& operator* (float X)
	{
		LowerBound *= X;
		UpperBound *= X;

		return *this;
	}

	FORCEINLINE FBounds& operator*= (int32 X)
	{
		LowerBound *= X;
		UpperBound *= X;
		
		return *this;
	}

	FORCEINLINE FBounds& operator*= (const FBounds& X)
	{
		LowerBound *= X.LowerBound;
		UpperBound *= X.UpperBound;

		return *this;
	}

	FORCEINLINE FBounds& operator/= (float X)
	{
		LowerBound /= X;
		UpperBound /= X;

		return *this;
	}

	FORCEINLINE FBounds& operator/= (int32 X)
	{
		LowerBound /= X;
		UpperBound /= X;

		return *this;
	}

	FORCEINLINE FBounds& operator/= (const FBounds& X)
	{
		LowerBound /= X.LowerBound;
		UpperBound /= X.UpperBound;

		return *this;
	}

	/** Computes the length of these bounds. */
	FORCEINLINE float Length() const
	{
		return FMath::Abs(UpperBound - LowerBound);
	}

	/** Returns true if bounds are reversed (Min > UpperBound). */
	FORCEINLINE bool IsReversed() const
	{
		return (LowerBound > UpperBound);
	}

	/** Returns true if value is within bounds (Min <= value <= UpperBound). */
	FORCEINLINE bool Contains(const float Value) const
	{
		return IsReversed() ? (Value <= LowerBound && Value >= UpperBound)
			                : (Value >= LowerBound && Value <= UpperBound);
	}

	/** Returns the a reversed copy of these bounds. */
	FORCEINLINE FBounds GetReversed() const 
	{ 
		return FBounds(UpperBound, LowerBound);
	}

	/** Expands this bounds to both sides by the specified amount. */
	void Expand(float ExpandAmount);
    
	/** Expands this bounds if necessary to include the specified element. */
	void Include(float Value);

	/** Bounds interpolation */
	FORCEINLINE float Interpolate(float Alpha) const
	{
		return LowerBound + float(Alpha*Length());
	}

	FORCEINLINE bool Equals(const FBounds& Other, float Tolerance = 1e-6f) const
	{
		return FMath::Abs(LowerBound - Other.LowerBound) <= Tolerance && FMath::Abs(UpperBound - Other.UpperBound) <= Tolerance;
	}

	// NOTE: Friends defined inside class body are inline and are hidden from non-ADL lookup

	/** Serializes the bounds. */
	friend class FArchive& operator<<(class FArchive& Ar, FBounds& Bounds)
	{
		return Ar << Bounds.LowerBound << Bounds.UpperBound;
	}

	/** Gets the hash for the specified bounds. */
	friend uint32 GetTypeHash(const FBounds& Bounds)
	{
		return HashCombine(GetTypeHash(Bounds.LowerBound), GetTypeHash(Bounds.UpperBound));
	}

	// NOTE: Passing the first parameter of the operator by value helps optimize chained operatios such as a+b+c otherwise, both parameters may be const references

	template<typename T>
	friend FBounds operator+(FBounds lhs, const T& rhs)
	{
		lhs += rhs;
		return lhs;
	}

	template<typename T>
	friend FBounds operator-(FBounds lhs, const T& rhs)
	{
		lhs -= rhs;
		return lhs;
	}

	template<typename T>
	friend FBounds operator*(FBounds lhs, const T& rhs)
	{
		lhs *= rhs;
		return lhs;
	}

	template<typename T>
	friend FBounds operator/(FBounds lhs, const T& rhs)
	{
		lhs /= rhs;
		return lhs;
	}
};

