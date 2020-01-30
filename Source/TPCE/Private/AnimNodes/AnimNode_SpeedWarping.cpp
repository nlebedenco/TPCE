// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNodes/AnimNode_SpeedWarping.h"
#include "GameFramework/WorldSettings.h"
#include "Animation/AnimInstanceProxy.h"
#include "Engine/Engine.h"
#include "SceneManagement.h"
#include "AnimationRuntime.h"

TAutoConsoleVariable<int32> CVarAnimSpeedWarpingEnable(TEXT("a.AnimNode.SpeedWarping.Enable"), 1, TEXT("Toggle SpeedWarping node."));

DECLARE_CYCLE_STAT(TEXT("SpeedWarping Eval"), STAT_SpeedWarping_Eval, STATGROUP_Anim);

FAnimNode_SpeedWarping::FAnimNode_SpeedWarping()
	: Space(EBoneControlSpace::BCS_BoneSpace)
	, Direction(FVector::RightVector)
	, PelvisInterpSpeed(10.f)
	, PelvisAdjustmentAlpha(1.0f)
	, bClampIKUsingFKLeg(true)
{
}

void FAnimNode_SpeedWarping::UpdateInternal(const FAnimationUpdateContext& Context)
{
	Super::UpdateInternal(Context);

	DeltaTime = Context.GetDeltaTime() * TimeDilation;
}


void FAnimNode_SpeedWarping::GatherDebugData(FNodeDebugData & DebugData)
{
	ComponentPose.GatherDebugData(DebugData);
}

void FAnimNode_SpeedWarping::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_SpeedWarping_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	// Get bone indexes and verify they exist.
	FCompactPoseBoneIndex IKFootRootBoneCompactPoseIndex = IKFootRootBone.GetCompactPoseIndex(BoneContainer);
	FCompactPoseBoneIndex PelvisBoneCompactPoseIndex = PelvisBone.GetCompactPoseIndex(BoneContainer);
	// ::IsValidToEvaluate should have taken care for this to never happen
	check(IKFootRootBoneCompactPoseIndex != INDEX_NONE && PelvisBoneCompactPoseIndex != INDEX_NONE);

	const FTransform& ComponentTransform = Output.AnimInstanceProxy->GetSkelMeshCompLocalToWorld();
	const FTransform& IKFootRootCSTransform = Output.Pose.GetComponentSpaceTransform(IKFootRootBoneCompactPoseIndex);
	const FTransform& PelvisBoneCSTransform = Output.Pose.GetComponentSpaceTransform(PelvisBoneCompactPoseIndex);

	// Caclulate Pelvis adjustment
	const FVector OriginalPelvisLocation = PelvisBoneCSTransform.GetLocation();
	const float PelvisOffsetAlpha = FMath::Clamp<float>(FMath::GetRangePct(FVector2D(1.0f, 2.0f), SpeedScaling), 0.f, 1.f);

	CurrentPelvisOffset = FMath::VInterpConstantTo(CurrentPelvisOffset, FMath::Lerp(FVector::ZeroVector, PelvisOffset, PelvisOffsetAlpha * FMath::Clamp(PelvisAdjustmentAlpha, 0.f, 1.f)), DeltaTime, PelvisInterpSpeed);

	const FVector AdjustedPelvisLocation = OriginalPelvisLocation + CurrentPelvisOffset;

#if WITH_EDITOR
	CachedOriginalPelvisLocation = OriginalPelvisLocation;
	CachedAdjustedPelvisLocation = AdjustedPelvisLocation;
#endif

	const FVector PelvisDelta = AdjustedPelvisLocation - OriginalPelvisLocation;

	// Set pelvis actual location
	FTransform ActualPelvisCSTransform = PelvisBoneCSTransform;
	ActualPelvisCSTransform.SetLocation(AdjustedPelvisLocation);

	// Output new transform for current bone. Must insert before the feet.
	OutBoneTransforms.Add(FBoneTransform(PelvisBoneCompactPoseIndex, ActualPelvisCSTransform));

	const int32 FeetDefinitionsNum = FeetDefinitions.Num();
#if WITH_EDITOR
	// Array to collect calculated locations for each foot.
	CachedIKFootInfo.Empty(FeetDefinitionsNum);
#endif

	// Calculate location for each foot
	for (int32 i = 0; i < FeetDefinitionsNum; ++i)
	{
		const FSpeedWarpingFootDefinition& FootDefinition = FeetDefinitions[i];

		const FCompactPoseBoneIndex IKFootBoneCompactPoseIndex = FootDefinition.IKFootBone.GetCompactPoseIndex(BoneContainer);
		const FCompactPoseBoneIndex FKFootBoneCompactPoseIndex = FootDefinition.FKFootBone.GetCompactPoseIndex(BoneContainer);

		// Get Component Space transforms
		const FTransform& FKFootBoneCSTransform = Output.Pose.GetComponentSpaceTransform(FKFootBoneCompactPoseIndex);
		const FVector FKFootLocation = FKFootBoneCSTransform.GetLocation();

		// Find Limb location and length with correction after pelvis adjustment.
		FVector LimbLocation = FKFootLocation;
		FCompactPoseBoneIndex CurrentBoneIndex = FKFootBoneCompactPoseIndex;
		for (int32 j = 0; j < FootDefinition.NumBonesInLimb; ++j)
		{
			CurrentBoneIndex = BoneContainer.GetParentBoneIndex(CurrentBoneIndex);
			// if any leg is invalid, cancel the whole process.
			if (CurrentBoneIndex == INDEX_NONE)
				return;

			const FTransform& BoneCSTransform = Output.Pose.GetComponentSpaceTransform(CurrentBoneIndex);
			const FVector BoneLocation = BoneCSTransform.GetLocation();
			LimbLocation = BoneLocation;
		}

		// Convert SpeedWarpingAxis to IKRootBone space.
		FTransform SpeedWarpTransform = IKFootRootCSTransform;
		FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, SpeedWarpTransform, IKFootRootBoneCompactPoseIndex, Space);
		const FQuat SpeedWarpingRotation = FQuat::FindBetweenVectors(SpeedWarpTransform.InverseTransformVector(Direction), FVector::ForwardVector);

		FTransform IKFootBoneCSTransform = Output.Pose.GetComponentSpaceTransform(IKFootBoneCompactPoseIndex);
		const FVector OriginalLocation = IKFootBoneCSTransform.GetLocation();

		// Convert coordinates to IKFootRoot space, orient to speed warp axis, apply scale and convert back to component space
		const FVector RelativeOriginalIKFootLocation = SpeedWarpingRotation.RotateVector(IKFootRootCSTransform.InverseTransformPosition(OriginalLocation));
		const FVector RelativeLimbLocation = SpeedWarpingRotation.RotateVector(IKFootRootCSTransform.InverseTransformPosition(LimbLocation));
		const FVector RelativeNewIKFootLocation((RelativeOriginalIKFootLocation.X - RelativeLimbLocation.X) * SpeedScaling + RelativeLimbLocation.X, RelativeOriginalIKFootLocation.Y, RelativeOriginalIKFootLocation.Z);

		
		const FVector BaseLocation = LimbLocation + PelvisDelta;
		const FVector CurrentLocation = OriginalLocation + PelvisDelta;
		const FVector DesiredLocation = IKFootRootCSTransform.TransformPosition(SpeedWarpingRotation.UnrotateVector(RelativeNewIKFootLocation));

		// Clamp IK Foot by FK leg. Allows for compression but no extension.
		const FVector ActualLocation = bClampIKUsingFKLeg ? BaseLocation + (DesiredLocation - BaseLocation).GetClampedToMaxSize((CurrentLocation - BaseLocation).Size()) : DesiredLocation;

		// Set IK foot's actual position.
		IKFootBoneCSTransform.SetTranslation(ActualLocation);
		OutBoneTransforms.Add(FBoneTransform(IKFootBoneCompactPoseIndex, IKFootBoneCSTransform));

#if WITH_EDITOR
		// Add foot info to the list
		CachedIKFootInfo.Add(FIKFootInfo(OriginalLocation, DesiredLocation, ActualLocation));
#endif		
	}
}

bool FAnimNode_SpeedWarping::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	const int32 FeetDefinitionsNum = FeetDefinitions.Num();
	bool bIsValid = (CVarAnimSpeedWarpingEnable.GetValueOnAnyThread() != 0)
		&& FeetDefinitionsNum > 0
		&& IKFootRootBone.IsValidToEvaluate(RequiredBones)
		&& PelvisBone.IsValidToEvaluate(RequiredBones);

#ifdef ENABLE_ANIM_DEBUG
	for (int32 i = 0; bIsValid && i < FeetDefinitionsNum; ++i)
	{
		auto& Item = FeetDefinitions[i];
		bIsValid = Item.IKFootBone.IsValidToEvaluate(RequiredBones) &&  Item.FKFootBone.Initialize(RequiredBones);
	}
#endif

	bIsValid &= !Direction.IsZero();

	return bIsValid;
}

void FAnimNode_SpeedWarping::InitializeBoneReferences(const FBoneContainer & RequiredBones)
{
	IKFootRootBone.Initialize(RequiredBones);
	PelvisBone.Initialize(RequiredBones);
	for (auto& Each : FeetDefinitions)
	{
		Each.IKFootBone.Initialize(RequiredBones);
		Each.FKFootBone.Initialize(RequiredBones);
	}
}

void FAnimNode_SpeedWarping::PreUpdate(const UAnimInstance* InAnimInstance)
{
	World = InAnimInstance->GetWorld();
	check(World->GetWorldSettings());
	TimeDilation = World->GetWorldSettings()->GetEffectiveTimeDilation();
}


#if WITH_EDITOR
// can't use World Draw functions because this is called from Render of viewport, AFTER ticking component, 
// which means LineBatcher already has ticked, so it won't render anymore
// to use World Draw functions, we have to call this from tick of actor
void FAnimNode_SpeedWarping::ConditionalDebugDraw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* MeshComp) const
{
	DrawWireSphereAutoSides(PDI, CachedAdjustedPelvisLocation, FLinearColor::Blue, 10.f, SDPG_Foreground);
	DrawWireSphereAutoSides(PDI, CachedOriginalPelvisLocation, FLinearColor::Red, 10.f, SDPG_Foreground);

	for (auto& Each: CachedIKFootInfo)
	{
		PDI->DrawLine(CachedAdjustedPelvisLocation, Each.DesiredLocation, FLinearColor::Green, SDPG_Foreground);
		DrawWireSphereAutoSides(PDI, Each.DesiredLocation, FLinearColor::Green, 10.f, SDPG_Foreground);

		PDI->DrawLine(CachedAdjustedPelvisLocation, Each.ActualLocation, FLinearColor::Blue, SDPG_Foreground);
		DrawWireSphereAutoSides(PDI, Each.ActualLocation, FLinearColor::Blue, 10.f, SDPG_Foreground);

		PDI->DrawLine(CachedOriginalPelvisLocation, Each.OriginalLocation, FLinearColor::Red, SDPG_Foreground);
		DrawWireSphereAutoSides(PDI, Each.OriginalLocation, FLinearColor::Red, 10.f, SDPG_Foreground);
	}
}
#endif // WITH_EDITOR