// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNodes/AnimNode_OrientationWarping.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"

FAnimMode_OrientationWarping::FAnimMode_OrientationWarping()
{
}

void FAnimMode_OrientationWarping::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);
	BasePose.Initialize(Context);
	const FBoneContainer& BoneContainer = Context.AnimInstanceProxy->GetRequiredBones();
	IKFootRootBone.Initialize(BoneContainer);
	PelvisBone.Initialize(BoneContainer);
	for (FBoneRef & Bone : SpineBones)
	{
		Bone.Bone.Initialize(BoneContainer);
	}
}

void FAnimMode_OrientationWarping::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	BasePose.CacheBones(Context);
}

void FAnimMode_OrientationWarping::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);
	BasePose.Update(Context);
}

void FAnimMode_OrientationWarping::Evaluate_AnyThread(FPoseContext& Output)
{
	BasePose.Evaluate(Output);

	check(!FMath::IsNaN(LocomotionAngle) && FMath::IsFinite(LocomotionAngle));


	if (!FMath::IsNearlyZero(LocomotionAngle, KINDA_SMALL_NUMBER))
	{
		const FBoneContainer& BoneContainer = Output.AnimInstanceProxy->GetRequiredBones();

		// Get bone indexes and verify they exist.
		FCompactPoseBoneIndex IKFootRootBoneCompactPoseIndex = IKFootRootBone.GetCompactPoseIndex(BoneContainer);
		FCompactPoseBoneIndex PelvisBoneCompactPoseIndex = PelvisBone.GetCompactPoseIndex(BoneContainer);
		if (IKFootRootBoneCompactPoseIndex == INDEX_NONE || PelvisBoneCompactPoseIndex == INDEX_NONE)
			return;

		// Prepare convert Quat and BoneContainer.
		
		FComponentSpacePoseContext CSOutput(Output.AnimInstanceProxy);
		CSOutput.Pose.InitPose(Output.Pose);

		// Build our desired rotations for IK root bone and body.
		FRotator DeltaRotation(EForceInit::ForceInitToZero);
		FRotator BodyDeltaRotation(EForceInit::ForceInitToZero);

		const float BodyAlpha = FMath::Clamp((Settings.BodyOrientationAlpha - 1.0f), -1.f, 0.f);
		switch (Settings.YawRotationAxis)
		{
		case EAxis::X:
			DeltaRotation.Roll = LocomotionAngle;
			BodyDeltaRotation.Roll = LocomotionAngle * BodyAlpha;
			break;
		case EAxis::Y:
			DeltaRotation.Pitch = LocomotionAngle;
			BodyDeltaRotation.Pitch = LocomotionAngle * BodyAlpha;
			break;
		case EAxis::Z:
			DeltaRotation.Yaw = LocomotionAngle;
			BodyDeltaRotation.Yaw = LocomotionAngle * BodyAlpha;
			break;
		default:
			break;
		}

		const FQuat DeltaQuat(DeltaRotation);

		// Apply rotation to IK root bone.
		FTransform& IKRootBoneTransform = Output.Pose[IKFootRootBoneCompactPoseIndex];
		IKRootBoneTransform.ConcatenateRotation(DeltaQuat);
		IKRootBoneTransform.NormalizeRotation();

		// Pelvis must follow the IK root bone to prevent glitches from thigh and calf constraints
		const FTransform& PelvisBoneCSTransform = CSOutput.Pose.GetComponentSpaceTransform(PelvisBoneCompactPoseIndex);
		const FQuat PelvisQuat = PelvisBoneCSTransform.GetRotation();

		// Convert our rotation from Component Space to Mesh Space.
		// const FQuat MeshSpacePelvisDeltaQuat = PelvisQuat.Inverse() * DeltaQuat * PelvisQuat;
		FTransform& PelvisBoneTransform = Output.Pose[PelvisBoneCompactPoseIndex];
		// PelvisBoneTransform.ConcatenateRotation(MeshSpacePelvisDeltaQuat);
		PelvisBoneTransform.SetRotation(DeltaQuat * PelvisQuat);
		PelvisBoneTransform.NormalizeRotation();

		const int32 SpineBonesCount = SpineBones.Num();
		if (SpineBonesCount > 0)
		{
			const FRotator SpineDeltaRotation(BodyDeltaRotation.Pitch / SpineBonesCount, BodyDeltaRotation.Yaw / SpineBonesCount, BodyDeltaRotation.Roll / SpineBonesCount);
			const FQuat SpineDeltaQuat(SpineDeltaRotation);
			for (FBoneRef& SpineBoneRef : SpineBones)
			{
				if (SpineBoneRef.Bone.IsValidToEvaluate(BoneContainer))
				{
					const FCompactPoseBoneIndex SpineBoneIndex = SpineBoneRef.Bone.GetCompactPoseIndex(BoneContainer);
					const FTransform& SpineBoneTM = CSOutput.Pose.GetComponentSpaceTransform(SpineBoneIndex);
					const FQuat MeshSpaceSpineDeltaQuat = SpineBoneTM.GetRotation().Inverse() * SpineDeltaQuat * SpineBoneTM.GetRotation();

					FTransform& SpineBoneTransform = Output.Pose[SpineBoneIndex];
					SpineBoneTransform.ConcatenateRotation(MeshSpaceSpineDeltaQuat);
					SpineBoneTransform.NormalizeRotation();
				}

			}
		}
	}
}

void FAnimMode_OrientationWarping::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugData.AddDebugItem(DebugLine);

	BasePose.GatherDebugData(DebugData);
}
