#include "ExtraTypes.h"

const FName NAME_Spectator(TEXT("Spectator"));
const FName NAME_Normal(TEXT("Normal"));
const FName NAME_Ragdoll(TEXT("Ragdoll"));
const FName NAME_Crouched(TEXT("Crouched"));
const FName NAME_Standing(TEXT("Standing"));
const FName NAME_Primary(TEXT("Primary"));
const FName NAME_Secondary(TEXT("Secondary"));

const FName NAME_Root(TEXT("root"));
const FName NAME_Pelvis(TEXT("pelvis"));
const FName NAME_Spine_01(TEXT("spine_01"));
const FName NAME_Spine_02(TEXT("spine_02"));
const FName NAME_Spine_03(TEXT("spine_03"));

const FName NAME_Clavicle_L(TEXT("clavicle_l"));
const FName NAME_UpperArm_L(TEXT("upperarm_l"));
const FName NAME_LowerArm_L(TEXT("lowerarm_l"));
const FName NAME_UpperArmTwist_L(TEXT("upperarm_twist_01_l"));
const FName NAME_LowerArmTwist_L(TEXT("lowerarm_twist_01_l"));
const FName NAME_Hand_L(TEXT("hand_l"));

const FName NAME_IndexFinger_01_L(TEXT("index_01_l"));
const FName NAME_IndexFinger_02_L(TEXT("index_02_l"));
const FName NAME_IndexFinger_03_L(TEXT("index_03_l"));
const FName NAME_MiddleFinger_01_L(TEXT("middle_01_l"));
const FName NAME_MiddleFinger_02_L(TEXT("middle_02_l"));
const FName NAME_MiddleFinger_03_L(TEXT("middle_03_l"));
const FName NAME_PinkyFinger_01_L(TEXT("pinky_01_l"));
const FName NAME_PinkyFinger_02_L(TEXT("pinky_02_l"));
const FName NAME_PinkyFinger_03_L(TEXT("pinky_03_l"));
const FName NAME_RingFinger_01_L(TEXT("ring_01_l"));
const FName NAME_RingFinger_02_L(TEXT("ring_02_l"));
const FName NAME_RingFinger_03_L(TEXT("ring_03_l"));
const FName NAME_Thumb_01_L(TEXT("thumb_01_l"));
const FName NAME_Thumb_02_L(TEXT("thumb_02_l"));
const FName NAME_Thumb_03_L(TEXT("thumb_03_l"));

const FName NAME_Clavicle_R(TEXT("clavicle_r"));
const FName NAME_UpperArm_R(TEXT("upperarm_r"));
const FName NAME_LowerArm_R(TEXT("lowerarm_r"));
const FName NAME_UpperArmTwist_R(TEXT("upperarm_twist_01_r"));
const FName NAME_LowerArmTwist_R(TEXT("lowerarm_twist_01_r"));
const FName NAME_Hand_R(TEXT("hand_r"));

const FName NAME_IndexFinger_01_R(TEXT("index_01_r"));
const FName NAME_IndexFinger_02_R(TEXT("index_02_r"));
const FName NAME_IndexFinger_03_R(TEXT("index_03_r"));
const FName NAME_MiddleFinger_01_R(TEXT("middle_01_r"));
const FName NAME_MiddleFinger_02_R(TEXT("middle_02_r"));
const FName NAME_MiddleFinger_03_R(TEXT("middle_03_r"));
const FName NAME_PinkyFinger_01_R(TEXT("pinky_01_r"));
const FName NAME_PinkyFinger_02_R(TEXT("pinky_02_r"));
const FName NAME_PinkyFinger_03_R(TEXT("pinky_03_r"));
const FName NAME_RingFinger_01_R(TEXT("ring_01_r"));
const FName NAME_RingFinger_02_R(TEXT("ring_02_r"));
const FName NAME_RingFinger_03_R(TEXT("ring_03_r"));
const FName NAME_Thumb_01_R(TEXT("thumb_01_r"));
const FName NAME_Thumb_02_R(TEXT("thumb_02_r"));
const FName NAME_Thumb_03_R(TEXT("thumb_03_r"));

const FName NAME_Neck_01(TEXT("neck_01"));
const FName NAME_Head(TEXT("head"));

const FName NAME_Thigh_L(TEXT("thigh_l"));
const FName NAME_ThighTwist_L(TEXT("thigh_twist_01_l"));
const FName NAME_Calf_L(TEXT("calf_l"));
const FName NAME_CalfTwist_L(TEXT("calf_twist_01_l"));
const FName NAME_Foot_L(TEXT("foot_l"));
const FName NAME_Ball_L(TEXT("ball_l"));

const FName NAME_Thigh_R(TEXT("thigh_r"));
const FName NAME_ThighTwist_R(TEXT("thigh_twist_01_r"));
const FName NAME_Calf_R(TEXT("calf_r"));
const FName NAME_CalfTwist_R(TEXT("calf_twist_01_r"));
const FName NAME_Foot_R(TEXT("foot_r"));
const FName NAME_Ball_R(TEXT("ball_r"));

const FName NAME_IKFootRoot(TEXT("ik_foot_root"));
const FName NAME_IKFoot_L(TEXT("ik_foot_l"));
const FName NAME_IKFoot_R(TEXT("ik_foot_r"));

const FName NAME_IKHandRoot(TEXT("ik_hand_root"));
const FName NAME_IKHand_Gun(TEXT("ik_hand_gun"));
const FName NAME_IKHand_L(TEXT("ik_hand_l"));
const FName NAME_IKHand_R(TEXT("ik_hand_r"));


bool SerializeQuantizedVector(FArchive& Ar, FVector& Vector, EVectorQuantization QuantizationLevel)
{
	// Since FRepMovement used to use FVector_NetQuantize100, we're allowing enough bits per component
	// regardless of the quantization level so that we can still support at least the same maximum magnitude
	// (2^30 / 100, or ~10 million).
	// This uses no inherent extra bandwidth since we're still using the same number of bits to store the
	// bits-per-component value. Of course, larger magnitudes will still use more bandwidth,
	// as has always been the case.
	switch (QuantizationLevel)
	{
	case EVectorQuantization::RoundTwoDecimals:
		return SerializePackedVector<100, 30>(Vector, Ar);
	case EVectorQuantization::RoundOneDecimal:
		return SerializePackedVector<10, 27>(Vector, Ar);
	default:
		return SerializePackedVector<1, 24>(Vector, Ar);
		
	}
}

void SerializeQuantizedRotator(FArchive& Ar, FRotator& Rotator, ERotatorQuantization QuantizationLevel)
{
	switch (QuantizationLevel)
	{
	case ERotatorQuantization::ByteComponents:
		Rotator.SerializeCompressed(Ar);
		break;
	case ERotatorQuantization::ShortComponents:
		Rotator.SerializeCompressedShort(Ar);
		break;
	}
}



