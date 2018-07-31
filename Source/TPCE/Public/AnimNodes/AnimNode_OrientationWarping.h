// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimNodeBase.h"
#include "BoneContainer.h"
#include "AnimNode_OrientationWarping.generated.h"

USTRUCT(BlueprintType)
struct FAxisSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "FAxisSettings")
	TEnumAsByte<EAxis::Type> YawRotationAxis;

	UPROPERTY(EditAnywhere, Category = "FAxisSettings")
	float BodyOrientationAlpha;

	FAxisSettings() :
		YawRotationAxis(EAxis::Z),
		BodyOrientationAlpha(0.2f) 
	{}
};

USTRUCT()
struct FBoneRef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Settings")
	FBoneReference Bone;
};

USTRUCT(BlueprintType)
struct TPCE_API FAnimMode_OrientationWarping : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FPoseLink BasePose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (PinShownByDefault))
	float LocomotionAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FAxisSettings Settings;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FBoneReference PelvisBone;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<FBoneRef> SpineBones;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FBoneReference IKFootRootBone;


public:
	FAnimMode_OrientationWarping();

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext & Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface
};
