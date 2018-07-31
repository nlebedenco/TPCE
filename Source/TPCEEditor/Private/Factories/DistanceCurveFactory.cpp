#include "Factories/DistanceCurveFactory.h"
#include "Curves/CurveFloat.h"
#include "Animation/AnimSequence.h"

UDistanceCurveFactory::UDistanceCurveFactory(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UCurveFloat::StaticClass();
}

float UDistanceCurveFactory::GetValue(const float Time)
{
	FTransform Transform;
	AnimSequence->GetBoneTransform(Transform, 0, Time, true);

	switch (DistanceCurveType)
	{
	case EDistanceCurveType::Pith:
		return Transform.Rotator().Pitch;
	case EDistanceCurveType::Yaw:
		return Transform.Rotator().Yaw;
	case EDistanceCurveType::Roll:
		return Transform.Rotator().Roll;
	case EDistanceCurveType::X:
		return Transform.GetLocation().X;
	case EDistanceCurveType::Y:
		return Transform.GetLocation().Y;
	case EDistanceCurveType::Z:
		return Transform.GetLocation().Z;
	default:
		return 0.0f;
	}
}


UObject* UDistanceCurveFactory::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	UCurveFloat* NewCurve = NewObject<UCurveFloat>(InParent, Name, Flags);
	if (NewCurve && AnimSequence)
	{
		const float MaxValue = GetValue(AnimSequence->GetPlayLength());
		const int32 FrameCount = AnimSequence->GetNumberOfFrames();
		for (int32 i = 0; i < FrameCount; ++i)
		{
			const float Time = AnimSequence->GetTimeAtFrame(i);
			const float Value = MaxValue - GetValue(Time);
			NewCurve->FloatCurve.AddKey(Value, Time);
		}
	}

	return NewCurve;
}
