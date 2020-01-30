// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNodes/AnimNode_FootPlacement.h"
#include "GameFramework/WorldSettings.h"
#include "Animation/AnimInstanceProxy.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/CollisionProfile.h"
#include "SceneManagement.h"
#include "AnimationRuntime.h"

#if ENABLE_ANIM_DEBUG
	#include "Async/Async.h"

	TAutoConsoleVariable<int32> CVarAnimNodeFootPlacementDebug(TEXT("a.AnimNode.FootPlacement.Debug"), 0, TEXT("Turn on debug for FAnimNode_FootPlacement"));
#endif

TAutoConsoleVariable<int32> CVarAnimFootPlacementEnable(TEXT("a.AnimNode.FootPlacement.Enable"), 1, TEXT("Toggle FootPlacement node."));

DECLARE_CYCLE_STAT(TEXT("FootPlacement Eval"), STAT_FootPlacement_Eval, STATGROUP_Anim);

FAnimNode_FootPlacement::FAnimNode_FootPlacement():
	TraceLengthAboveFoot(50.f),
	TraceLengthBelowFoot(75.f),
	MinZOffset(-30.f),
	MaxZOffset(20.f),
	MinAngle(-30.f),
	MaxAngle(30.f),
	PelvisAdjustmentAlpha(1.0f),
	CollisionProfileName(UCollisionProfile::Pawn_ProfileName)
{
}

void FAnimNode_FootPlacement::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	Super::Initialize_AnyThread(Context);
}

void FAnimNode_FootPlacement::UpdateInternal(const FAnimationUpdateContext& Context)
{
	Super::UpdateInternal(Context);
	DeltaTime = Context.GetDeltaTime() * TimeDilation;
}

void FAnimNode_FootPlacement::GatherDebugData(FNodeDebugData & DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugData.AddDebugItem(DebugLine);
	ComponentPose.GatherDebugData(DebugData);
}

void FAnimNode_FootPlacement::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_FootPlacement_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	// Get bone index and verify it exists.
	FCompactPoseBoneIndex PelvisBoneCompactPoseIndex = PelvisBone.GetCompactPoseIndex(BoneContainer);

	// ::IsValidToEvaluate should have taken care for this to never happen
	check(PelvisBoneCompactPoseIndex != INDEX_NONE);

	const USkeletalMeshComponent* SkelComp = Output.AnimInstanceProxy->GetSkelMeshComponent();
	const FTransform& ComponentTransform = Output.AnimInstanceProxy->GetSkelMeshCompLocalToWorld();
	
	const FCompactPoseBoneIndex RootBoneIndex = BoneContainer.GetCompactPoseIndexFromSkeletonIndex(0);
	check(RootBoneIndex != INDEX_NONE);

	const FTransform& RootCSTransform = Output.Pose.GetComponentSpaceTransform(RootBoneIndex);
	const FTransform RootTransform = RootCSTransform * ComponentTransform;
	const FVector BaseLocation = RootTransform.GetLocation();

	// Min of offset of all feet used to calculate the pelvis offset
	float MinFootOffsetZ = INFINITY;
	//Calculate each foot IK real position.
	for (auto& Each : FootBones)
	{
		const FCompactPoseBoneIndex IKFootBoneCompactPoseIndex = Each.IKFootBone.GetCompactPoseIndex(BoneContainer);
		check(IKFootBoneCompactPoseIndex != INDEX_NONE);

		// Get component space transform
		FTransform IKBoneCSTransform = Output.Pose.GetComponentSpaceTransform(IKFootBoneCompactPoseIndex);
		// Make a world space transform
		FTransform IKBoneWSTransform = IKBoneCSTransform * ComponentTransform;
		// Calculate world space offsets
		FFootPlacementOffset& OutFootOffset = Each.Offset;
		CalculateFootPlacement(SkelComp, BaseLocation, IKBoneWSTransform.GetLocation(), OutFootOffset);

		// Save min foot offset for pelvis adjustment
		if (OutFootOffset.Z < MinFootOffsetZ)
			MinFootOffsetZ = OutFootOffset.Z;

		// Apply offsets in world space
		IKBoneWSTransform.AddToTranslation(FVector(0.0f, 0.0f, OutFootOffset.Z));
		IKBoneWSTransform.SetRotation(FQuat(FRotator(OutFootOffset.Pitch, 0.0f, OutFootOffset.Roll)) * IKBoneWSTransform.GetRotation());

		const FVector IKBoneLocation = IKBoneWSTransform.GetLocation();
		const FQuat IKBoneRotation = IKBoneWSTransform.GetRotation();

		// Convert back to component space and apply to the foot
		IKBoneCSTransform.SetLocation(ComponentTransform.InverseTransformPosition(IKBoneLocation));
		IKBoneCSTransform.SetRotation(ComponentTransform.InverseTransformRotation(IKBoneRotation));

		OutBoneTransforms.Add(FBoneTransform(IKFootBoneCompactPoseIndex, IKBoneCSTransform));

#if ENABLE_ANIM_DEBUG && ENABLE_DRAW_DEBUG		
		const bool bShowDebug = (CVarAnimNodeFootPlacementDebug.GetValueOnAnyThread() != 0);
		if (bShowDebug)
		{
			const UWorld* World = SkelComp->GetWorld();
			AsyncTask(ENamedThreads::GameThread, [World, IKBoneLocation, IKBoneRotation]()
			{
				DrawDebugCoordinateSystem(World, IKBoneLocation, IKBoneRotation.Rotator(), 25.0f);
			});
		}
#endif
	}
	
	// Set new pelvis transform.
	PelvisZOffset = FMath::FInterpTo(PelvisZOffset, MinFootOffsetZ * FMath::Clamp(PelvisAdjustmentAlpha, 0.f, 1.f), DeltaTime, 20.f);
	FTransform PelvisBoneCSTransform = Output.Pose.GetComponentSpaceTransform(PelvisBoneCompactPoseIndex);	
	PelvisBoneCSTransform.AddToTranslation(ComponentTransform.InverseTransformVector(FVector(0.0f, 0.0f, PelvisZOffset)));
	OutBoneTransforms.Insert(FBoneTransform(PelvisBoneCompactPoseIndex, PelvisBoneCSTransform), 0);
}

bool FAnimNode_FootPlacement::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	const int32 FootBonesNum = FootBones.Num();
	bool bIsValid = (CVarAnimFootPlacementEnable.GetValueOnAnyThread() != 0)
		&& FootBonesNum > 0
		&& PelvisBone.IsValidToEvaluate(RequiredBones);

#ifdef ENABLE_ANIM_DEBUG
	for (int32 i = 0; bIsValid && i < FootBonesNum; ++i)
	{
		auto& Item = FootBones[i];
		bIsValid = Item.IKFootBone.IsValidToEvaluate(RequiredBones);
	}
#endif
	return bIsValid;
}

void FAnimNode_FootPlacement::InitializeBoneReferences(const FBoneContainer & RequiredBones)
{
	PelvisBone.Initialize(RequiredBones);
	for (auto& Each : FootBones)
		Each.IKFootBone.Initialize(RequiredBones);
}

void FAnimNode_FootPlacement::PreUpdate(const UAnimInstance * InAnimInstance)
{
	const UWorld* World = InAnimInstance->GetWorld();
	check(World->GetWorldSettings());
	TimeDilation = World->GetWorldSettings()->GetEffectiveTimeDilation();
}

void FAnimNode_FootPlacement::CalculateFootPlacement(const USkeletalMeshComponent* SkelComp, const FVector& BaseLocation, const FVector& FootLocation, FFootPlacementOffset& OutFootOffset)
{
	static const float AngleTolerance = 1e-3f;

	const FVector Start(FootLocation.X, FootLocation.Y, BaseLocation.Z + TraceLengthAboveFoot);
	const FVector End(FootLocation.X, FootLocation.Y, BaseLocation.Z - TraceLengthBelowFoot);

	const UWorld* World = SkelComp->GetWorld();
	const AActor* IgnoredActor = SkelComp->GetOwner();

	static const FName TraceName(TEXT("Foot IK Trace"));
	const FCollisionQueryParams Params(TraceName, true, IgnoredActor);

	FHitResult Hit(Start, End);
	if (World && World->LineTraceSingleByProfile(Hit, Start, End, CollisionProfileName, Params))
	{
		OutFootOffset.Roll = FMath::FInterpTo(OutFootOffset.Roll, FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(Hit.Normal.Y, Hit.Normal.Z)), MinAngle, MaxAngle), DeltaTime, 15.0f);
		OutFootOffset.Pitch = FMath::FInterpTo(OutFootOffset.Pitch, FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(Hit.Normal.X, Hit.Normal.Z)) * -1.0f, MinAngle, MaxAngle), DeltaTime, 15.0f);

		// These are all in world space, not relative to the feet.
		// The interpolation serves to smooth out the leg correction and avoid snaps on sudden level differences
		// such as in staircases but it can cause perceivable penetration in the ground so we don't use it for ramps.
		// TODO: find a way to handle staircases that does not require interpolation of the offset.
		const float DeltaZ = Hit.Location.Z - BaseLocation.Z;
		OutFootOffset.Z = (FMath::IsNearlyZero(OutFootOffset.Pitch, AngleTolerance) && FMath::IsNearlyZero(OutFootOffset.Roll, AngleTolerance)) ? FMath::FInterpTo(OutFootOffset.Z, FMath::Clamp(DeltaZ, MinZOffset, MaxZOffset), DeltaTime, 15.0f) : DeltaZ;
	}
	else
	{
		OutFootOffset.Z = 0.0f;
		OutFootOffset.Roll = 0.0f;
		OutFootOffset.Pitch = 0.0f;
	}

#if ENABLE_ANIM_DEBUG && ENABLE_DRAW_DEBUG
	const bool bShowDebug = (CVarAnimNodeFootPlacementDebug.GetValueOnAnyThread() != 0);
	if (bShowDebug)
	{
		AsyncTask(ENamedThreads::GameThread, [World, Hit]()
		{
			DrawDebugLine(World, Hit.TraceStart, Hit.TraceEnd, FColor::Red, false, -1.0f, SDPG_Foreground);
			if (Hit.bBlockingHit)
				DrawDebugCircle(World, FTransform(Hit.Normal.Rotation(), Hit.Location, FVector::OneVector).ToMatrixNoScale(), 5.0f, 16, FColor::Red, false, -1.0f, SDPG_Foreground);
		});
	}
#endif
}
