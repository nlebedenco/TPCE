// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/TargetPoint.h"
// #include "EdGraph/EdGraphNodeUtils.h"
#include "BoneContainer.h"
#include "BonePose.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_SpeedWarping.generated.h"

class UWorld;
class USkeletalMeshComponent;

struct FIKFootInfo
{
	FVector OriginalLocation;
	FVector DesiredLocation;
	FVector ActualLocation;

	FIKFootInfo(const FVector& InOriginalLocation, const FVector& InDesiredLocation, const FVector& InActualLocation) :
		OriginalLocation(InOriginalLocation),
		DesiredLocation(InDesiredLocation),
		ActualLocation(InActualLocation)
	{
	}
};

USTRUCT(BlueprintType)
struct FSpeedWarpingFootDefinition 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Skeleton)
	FBoneReference IKFootBone;

	UPROPERTY(EditAnywhere, Category = Skeleton)
	FBoneReference FKFootBone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Skeleton)
	int32 NumBonesInLimb;

};

USTRUCT(BlueprintInternalUseOnly)
struct TPCE_API FAnimNode_SpeedWarping : public FAnimNode_SkeletalControlBase
{
	GENERATED_BODY() 

public:
	FAnimNode_SpeedWarping();

public:
	/** Name of bone to control. This is the main bone chain to modify from. **/
	UPROPERTY(EditAnywhere, Category = Settings)
	FBoneReference IKFootRootBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TArray<FSpeedWarpingFootDefinition> FeetDefinitions;

	UPROPERTY(EditAnywhere, Category = Settings)
	FBoneReference PelvisBone;

	/** Space to apply the speed warping effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	TEnumAsByte<EBoneControlSpace> Space;

	/** Direction in the specified space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	FVector Direction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault))
	float SpeedScaling;	

	/** Offset applied to the pelvis scaled by SpeedScaling in the range [1.0f, 2.0f]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHidenByDefault))
	FVector PelvisOffset;

	/** How fast the pelvis should get in place. 0 for immediate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float PelvisInterpSpeed;

	/** Amount of the pelvis adjustment to actually use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault, ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float PelvisAdjustmentAlpha;

	/** If true final IK bone location will not be constrained by the length of the leg. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	uint32 bClampIKUsingFKLeg : 1;

public:

	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	virtual bool HasPreUpdate() const override { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;

	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;

#if WITH_EDITOR
	void ConditionalDebugDraw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* MeshComp) const;
#endif // WITH_EDITOR

private:

	UWorld* World;

	float DeltaTime;
	float TimeDilation;

	FVector CurrentPelvisOffset;

#if WITH_EDITOR
	FVector CachedOriginalPelvisLocation;
	FVector CachedAdjustedPelvisLocation;
	TArray<FIKFootInfo> CachedIKFootInfo;
#endif // WITH_EDITOR


	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
};
