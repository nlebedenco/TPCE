// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Math/UnrealMathUtility.h"
#include "Math/Bounds.h"
#include "ExtraTypes.h"

#define COS_0		(1.0f)
#define COS_15		(0.96592582629f)
#define COS_30		(0.86602540378f)
#define COS_45		(0.70710678118f)
#define COS_60		(0.5f)
#define COS_90		(0.0f)

struct FMathEx : public FMath
{
	using FMath::RInterpTo;

	/** Interpolate rotator from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	static TPCE_API FQuat QInterpTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed);

	/** Interpolate from Current to Target as rotation axes. Scaled by distance to Target, so it has a strong start speed and ease out. */
	static TPCE_API float FInterpAngleTo(const float Current, const float Target, float DeltaTime, float InterpSpeed);

	/** Interpolate from Current to Target as rotation axes. Scaled by distance to Target, so it has a strong start speed and ease out. */
	static TPCE_API float FInterpConstantAngleTo(const float Current, const float Target, float DeltaTime, float InterpSpeed);



	/**
	 * Interpolate vector from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API FVector VSafeInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate vector2D from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API FVector2D Vector2DSafeInterpTo(const FVector2D& Current, const FVector2D& Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate rotator from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API FRotator RSafeInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API float FSafeInterpTo(const float Current, const float Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate angle from Current to Target along the smallest arc with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API float FSafeInterpAngleTo(const float Current, const float Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate Linear Color from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API FLinearColor CSafeInterpTo(const FLinearColor& Current, const FLinearColor& Target, float DeltaTime, float InterpSpeed);



	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API float FSmoothInterpTo(float Current, float Target, float& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime);

	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API float FSmoothInterpAngleTo(float Current, float Target, float& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime);

	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API FVector VSmoothInterpTo(const FVector& Current, const FVector& Target, FVector& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime);

	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API FRotator RSmoothInterpTo(const FRotator& Current, const FRotator& Target, FRotator& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime);

	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API FRotator RSmoothInterpTo(const FRotator& Current, const FRotator& Target, FRotator& CurrentVelocity, float SmoothTime, const FRotator& MaxSpeed, float DeltaTime);

	/** Find the cardinal direction for an angle given the current cardinal direction, the half angle width of the north segment and a buffer for tolerance */
	static TPCE_API ECardinalDirection FindCardinalDirection(float Angle, const ECardinalDirection CurrentCardinalDirection, const float NorthSegmentHalfWidth = 60.f, const float Buffer = 5.0f);

};