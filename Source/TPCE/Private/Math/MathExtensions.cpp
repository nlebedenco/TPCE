
#include "Math/MathExtensions.h"
#include "Math/UnrealMath.h"

FQuat FMathEx::QInterpTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed)
{
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if (DeltaTime == 0.f || Current == Target)
	{
		return Current;
	}

	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;

	const float InnerProd = Current.X*Target.X + Current.Y*Target.Y + Current.Z*Target.Z + Current.W*Target.W;
	const float Delta = 1.0f - (InnerProd * InnerProd);

	// If steps are too small, just return Target and assume we have reached our destination.
	if (FMath::IsNearlyZero(Delta))
	{
		return Target;
	}

	return FQuat::Slerp(Current, Target, Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f));
}

float FMathEx::FInterpAngleTo(const float Current, const float Target, float DeltaTime, float InterpSpeed)
{
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if (DeltaTime == 0.f || Current == Target)
	{
		return Current;
	}

	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	// Distance to reach
	const float Delta = FRotator::NormalizeAxis(Target - Current);

	// If step is too small, just return Target and assume we have reached our destination.
	if (FMath::IsNearlyZero(Delta, 1e-3f))
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const float DeltaMove = Delta * FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;
}

float FMathEx::FInterpConstantAngleTo(const float Current, const float Target, float DeltaTime, float InterpSpeed)
{
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if (DeltaTime == 0.f || Current == Target)
	{
		return Current;
	}

	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;

	const float DeltaMove = FRotator::NormalizeAxis(Target - Current);
	return Current + FMath::Clamp(DeltaMove, -DeltaInterpSpeed, DeltaInterpSpeed);
}


FVector FMathEx::VSafeInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed)
{
	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	// Distance to reach
	const FVector Delta = Target - Current;
	const float Distance = Delta.Size();
	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;

	// If distance is too small, just set the desired location
	if (Distance < 0.01f)
	{
		return Target;
	}

	// If distance is lower than 1 switch to constant interpolation
	if (Distance <= 1.0f)
	{
		if (Distance > DeltaInterpSpeed)
		{
			if (DeltaInterpSpeed > 0.f)
			{
				const FVector DeltaN = Delta / Distance;
				return Current + DeltaN * DeltaInterpSpeed;
			}
			else
			{
				return Current;
			}
		}

		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const FVector DeltaMove = Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;
}

FVector2D FMathEx::Vector2DSafeInterpTo(const FVector2D& Current, const FVector2D& Target, float DeltaTime, float InterpSpeed)
{
	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	// Distance to reach
	const FVector2D Delta = Target - Current;
	const float Distance = Delta.Size();
	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;

	// If distance is too small, just set the desired location
	if (Distance < 0.01f)
	{
		return Target;
	}

	// If distance is below 1 switch to constant interpolation
	if (Distance <= 1.0f)
	{
		if (Distance > DeltaInterpSpeed)
		{
			if (DeltaInterpSpeed > 0.f)
			{
				const FVector2D DeltaN = Delta / Distance;
				return Current + DeltaN * DeltaInterpSpeed;
			}
			else
			{
				return Current;
			}
		}

		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const FVector2D DeltaMove = Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;

}

FRotator FMathEx::RSafeInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed)
{
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if (DeltaTime == 0.f || Current == Target)
	{
		return Current;
	}

	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;
	const FRotator Delta = (Target - Current).GetNormalized();

	// If steps are too small, just return Target and assume we have reached our destination.
	if (Delta.IsNearlyZero())
	{
		return Target;
	}

	// If all axes are below 1 switch to constant interpolation
	if (Delta.IsNearlyZero(1.0f))
	{
		FRotator Result = Current;
		Result.Pitch += FMath::Clamp(Delta.Pitch, -DeltaInterpSpeed, DeltaInterpSpeed);
		Result.Yaw += FMath::Clamp(Delta.Yaw, -DeltaInterpSpeed, DeltaInterpSpeed);
		Result.Roll += FMath::Clamp(Delta.Roll, -DeltaInterpSpeed, DeltaInterpSpeed);
		return Result.GetNormalized();
	}

	// Delta Move, Clamp so we do not over shoot.
	const FRotator DeltaMove = Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f);
	return (Current + DeltaMove).GetNormalized();
}

float FMathEx::FSafeInterpTo(const float Current, const float Target, float DeltaTime, float InterpSpeed)
{
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if (DeltaTime == 0.f || Current == Target)
	{
		return Current;
	}

	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	// Distance to reach
	const float Delta = Target - Current;

	// If distance is too small, just set the desired location
	if (FMath::IsNearlyZero(Delta, KINDA_SMALL_NUMBER))
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;
	const float DeltaMove = (Delta < -1.0f || Delta > 1.0) ? Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f)
		                                                   : FMath::Clamp<float>(Delta, -DeltaInterpSpeed, DeltaInterpSpeed);

	return Current + DeltaMove;
}

float FMathEx::FSafeInterpAngleTo(const float Current, const float Target, float DeltaTime, float InterpSpeed)
{
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if (DeltaTime == 0.f || Current == Target)
	{
		return Current;
	}

	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	// Distance to reach
	const float Delta = FRotator::NormalizeAxis(Target - Current);

	// If step is too small, just return Target and assume we have reached our destination.
	if (FMath::IsNearlyZero(Delta, 1e-3f))
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;
	const float DeltaMove = (Delta < -1.0f || Delta > 1.0f) ? Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f)
										                    : FMath::Clamp(Delta, -DeltaInterpSpeed, DeltaInterpSpeed);

	return Current + DeltaMove;
}

FLinearColor FMathEx::CSafeInterpTo(const FLinearColor& Current, const FLinearColor& Target, float DeltaTime, float InterpSpeed)
{
	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	// Distance to reach
	const FLinearColor Delta = Target - Current;
	const float Distance = FLinearColor::Dist(Target, Current);
	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;

	// If distance is too small, just set the desired location
	if (Distance < KINDA_SMALL_NUMBER)
	{
		return Target;
	}

	// If distance is lower than 1 switch to constant interpolation
	if (Distance <= 1.0f)
	{
		if (Distance > DeltaInterpSpeed)
		{
			if (DeltaInterpSpeed > 0.f)
			{
				const FLinearColor DeltaN = Delta / Distance;
				return Current + DeltaN * DeltaInterpSpeed;
			}
			else
			{
				return Current;
			}
		}

		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const FLinearColor DeltaMove = Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;
}




float FMathEx::FSmoothInterpTo(const float Current, const float Target, float& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime)
{
	SmoothTime = FMath::Max(KINDA_SMALL_NUMBER, SmoothTime);
	const float Omega = 2.0f / SmoothTime;

	const float X = Omega * DeltaTime;
	const float Exp = 1.0f / (1.0f + X + 0.48f * X * X + 0.235f * X * X * X);

	// Clamp maximum speed
	const float MaxDelta = MaxSpeed * SmoothTime;
	float Delta = MaxDelta > 0.0f ? FMath::Clamp(Current - Target, -MaxDelta, MaxDelta) : Current - Target;

	const float Temp = (CurrentVelocity + Omega * Delta) * DeltaTime;

	CurrentVelocity = (CurrentVelocity - Omega * Temp) * Exp;
	float Result = (Current - Delta) + (Delta + Temp) * Exp;

	// Prevent overshooting
	if (((Target - Current) > 0.0f) == (Result > Target))
	{
		Result = Target;
		CurrentVelocity = 0.0f;
	}

	return Result;
}

float FMathEx::FSmoothInterpAngleTo(const float Current, const float Target, float& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime)
{
	return FSmoothInterpTo(Current, Current + FRotator::NormalizeAxis(Target - Current), CurrentVelocity, SmoothTime, MaxSpeed, DeltaTime);
}

FVector FMathEx::VSmoothInterpTo(const FVector& Current, const FVector& Target, FVector& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime)
{
	SmoothTime = FMath::Max(KINDA_SMALL_NUMBER, SmoothTime);
	const float Omega = 2.0f / SmoothTime;

	const float X = Omega * DeltaTime;
	const float Exp = 1.0f / (1.0f + X + 0.48f * X * X + 0.235f * X * X * X);
	const float MaxDeltaSize = MaxSpeed * SmoothTime;
	const FVector Delta = MaxDeltaSize > 0.0f ? (Current - Target).GetClampedToMaxSize(MaxDeltaSize) : Current - Target;

	const FVector Temp = (CurrentVelocity + Omega * Delta) * DeltaTime;
	CurrentVelocity = (CurrentVelocity - Omega * Temp) * Exp;
	FVector Result = (Current - Delta) + (Delta + Temp) * Exp;

	if (FVector::DotProduct(Target - Current, Result - Target) > 0.0f)
	{
		Result = Target;
		CurrentVelocity = FVector::ZeroVector;
	}

	return Result;
}

FRotator FMathEx::RSmoothInterpTo(const FRotator& Current, const FRotator& Target, FRotator& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime)
{
	return FRotator(
		FSmoothInterpAngleTo(Current.Pitch, Target.Pitch, CurrentVelocity.Pitch, SmoothTime, MaxSpeed, DeltaTime),
		FSmoothInterpAngleTo(Current.Yaw, Target.Yaw, CurrentVelocity.Yaw, SmoothTime, MaxSpeed, DeltaTime),
		FSmoothInterpAngleTo(Current.Roll, Target.Roll, CurrentVelocity.Roll, SmoothTime, MaxSpeed, DeltaTime)
	).GetNormalized();
}

FRotator FMathEx::RSmoothInterpTo(const FRotator& Current, const FRotator& Target, FRotator& CurrentVelocity, float SmoothTime, const FRotator& MaxSpeed, float DeltaTime)
{
	return FRotator(
		FSmoothInterpAngleTo(Current.Pitch, Target.Pitch, CurrentVelocity.Pitch, SmoothTime, MaxSpeed.Pitch, DeltaTime),
		FSmoothInterpAngleTo(Current.Yaw, Target.Yaw, CurrentVelocity.Yaw, SmoothTime, MaxSpeed.Yaw, DeltaTime),
		FSmoothInterpAngleTo(Current.Roll, Target.Roll, CurrentVelocity.Roll, SmoothTime, MaxSpeed.Roll, DeltaTime)
	).GetNormalized();
}


FORCEINLINE static bool CheckCardinalDirection(const float Angle, const bool bIsCurrentCardinalDirection, const float Min, const float Max, const float Buffer)
{
	return bIsCurrentCardinalDirection ? (Angle >= (Min - Buffer) && Angle <= (Max + Buffer))
		: (Angle >= (Min + Buffer) && Angle <= (Max - Buffer));
}

ECardinalDirection FMathEx::FindCardinalDirection(float Angle, const ECardinalDirection CurrentCardinalDirection, const float NorthSegmentHalfWidth, const float Buffer)
{
	if (CheckCardinalDirection(Angle, CurrentCardinalDirection == ECardinalDirection::North, -NorthSegmentHalfWidth, NorthSegmentHalfWidth, Buffer))
		return ECardinalDirection::North;

	if (CheckCardinalDirection(Angle, CurrentCardinalDirection == ECardinalDirection::East, NorthSegmentHalfWidth, 180.0f - NorthSegmentHalfWidth, Buffer))
		return ECardinalDirection::East;

	if (CheckCardinalDirection(Angle, CurrentCardinalDirection == ECardinalDirection::West, -180.0f + NorthSegmentHalfWidth, -NorthSegmentHalfWidth, Buffer))
		return ECardinalDirection::West;

	return ECardinalDirection::South;
}
