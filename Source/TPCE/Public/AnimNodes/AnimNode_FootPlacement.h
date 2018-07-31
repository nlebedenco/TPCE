// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BoneContainer.h"
#include "BonePose.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"

#include "AnimNode_FootPlacement.generated.h"

class UWorld;
class USkeletalMeshComponent;

USTRUCT(BlueprintType)
struct FFootPlacementOffset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Roll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch;

	FFootPlacementOffset():
		Z(0.0f),
		Roll(0.0f),
		Pitch(0.0f)
	{

	}

	FFootPlacementOffset(const float InZ, const float InRoll, const float InPitch):
		Z(InZ),
		Roll(InRoll),
		Pitch(InPitch)
	{}
};

USTRUCT(BlueprintType)
struct FFootPlacementBone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Skeleton)
	FBoneReference IKFootBone;

	UPROPERTY()
	FFootPlacementOffset Offset;
};

USTRUCT(BlueprintInternalUseOnly) 
struct TPCE_API FAnimNode_FootPlacement: public FAnimNode_SkeletalControlBase
{
	GENERATED_BODY()

public:
	FAnimNode_FootPlacement();

public:

	UPROPERTY(EditAnywhere, Category = Settings)
	FBoneReference PelvisBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TArray<FFootPlacementBone> FootBones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float TraceLengthAboveFoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float TraceLengthBelowFoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float MinZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float MaxZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float MinAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float MaxAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault, ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float PelvisAdjustmentAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	FName CollisionProfileName;

private:

	float DeltaTime;
	float TimeDilation;
	float ElapsedTime;

	float PelvisZOffset; 

public:	

	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	virtual bool HasPreUpdate() const override { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;

	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;

private:

	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;

	void CalculateFootPlacement(const USkeletalMeshComponent* SkelComp, const FVector& BaseLocation, const FVector& FootLocation, FFootPlacementOffset& OutFootOffset);
};
