// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Math/Bounds.h"

#include "ExtraTypes.generated.h"

/**
 * When you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list.
 */
#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
#define COLLISION_PICKUP		ECC_GameTraceChannel3
#define COLLISION_CAMERA_ACTOR 	ECC_GameTraceChannel4

extern TPCE_API const FName NAME_Spectator;
extern TPCE_API const FName NAME_Normal;
extern TPCE_API const FName NAME_Ragdoll;
extern TPCE_API const FName NAME_Crouched;
extern TPCE_API const FName NAME_Standing;
extern TPCE_API const FName NAME_Primary;
extern TPCE_API const FName NAME_Secondary;

// Epic Skeleton Bone Names

extern TPCE_API const FName NAME_Root;
extern TPCE_API const FName NAME_Pelvis;
extern TPCE_API const FName NAME_Spine_01;
extern TPCE_API const FName NAME_Spine_02;
extern TPCE_API const FName NAME_Spine_03;

extern TPCE_API const FName NAME_Clavicle_L;
extern TPCE_API const FName NAME_UpperArm_L;
extern TPCE_API const FName NAME_LowerArm_L;
extern TPCE_API const FName NAME_UpperArmTwist_L;
extern TPCE_API const FName NAME_LowerArmTwist_L;
extern TPCE_API const FName NAME_Hand_L;

extern TPCE_API const FName NAME_IndexFinger_01_L;
extern TPCE_API const FName NAME_IndexFinger_02_L;
extern TPCE_API const FName NAME_IndexFinger_03_L;
extern TPCE_API const FName NAME_MiddleFinger_01_L;
extern TPCE_API const FName NAME_MiddleFinger_02_L;
extern TPCE_API const FName NAME_MiddleFinger_03_L;
extern TPCE_API const FName NAME_PinkyFinger_01_L;
extern TPCE_API const FName NAME_PinkyFinger_02_L;
extern TPCE_API const FName NAME_PinkyFinger_03_L;
extern TPCE_API const FName NAME_RingFinger_01_L;
extern TPCE_API const FName NAME_RingFinger_02_L;
extern TPCE_API const FName NAME_RingFinger_03_L;
extern TPCE_API const FName NAME_Thumb_01_L;
extern TPCE_API const FName NAME_Thumb_02_L;
extern TPCE_API const FName NAME_Thumb_03_L;

extern TPCE_API const FName NAME_Clavicle_R;
extern TPCE_API const FName NAME_UpperArm_R;
extern TPCE_API const FName NAME_LowerArm_R;
extern TPCE_API const FName NAME_UpperArmTwist_R;
extern TPCE_API const FName NAME_LowerArmTwist_R;
extern TPCE_API const FName NAME_Hand_R;

extern TPCE_API const FName NAME_IndexFinger_01_R;
extern TPCE_API const FName NAME_IndexFinger_02_R;
extern TPCE_API const FName NAME_IndexFinger_03_R;
extern TPCE_API const FName NAME_MiddleFinger_01_R;
extern TPCE_API const FName NAME_MiddleFinger_02_R;
extern TPCE_API const FName NAME_MiddleFinger_03_R;
extern TPCE_API const FName NAME_PinkyFinger_01_R;
extern TPCE_API const FName NAME_PinkyFinger_02_R;
extern TPCE_API const FName NAME_PinkyFinger_03_R;
extern TPCE_API const FName NAME_RingFinger_01_R;
extern TPCE_API const FName NAME_RingFinger_02_R;
extern TPCE_API const FName NAME_RingFinger_03_R;
extern TPCE_API const FName NAME_Thumb_01_R;
extern TPCE_API const FName NAME_Thumb_02_R;
extern TPCE_API const FName NAME_Thumb_03_R;

extern TPCE_API const FName NAME_Neck_01;
extern TPCE_API const FName NAME_Head;

extern TPCE_API const FName NAME_Thigh_L;
extern TPCE_API const FName NAME_Calf_L;
extern TPCE_API const FName NAME_ThighTwist_L;
extern TPCE_API const FName NAME_CalfTwist_L;
extern TPCE_API const FName NAME_Foot_L;
extern TPCE_API const FName NAME_Ball_L;

extern TPCE_API const FName NAME_Thigh_R;
extern TPCE_API const FName NAME_Calf_R;
extern TPCE_API const FName NAME_ThighTwist_R;
extern TPCE_API const FName NAME_CalfTwist_R;
extern TPCE_API const FName NAME_Foot_R;
extern TPCE_API const FName NAME_Ball_R;

extern TPCE_API const FName NAME_IKFootRoot;
extern TPCE_API const FName NAME_IKFoot_L;
extern TPCE_API const FName NAME_IKFoot_R;

extern TPCE_API const FName NAME_IKHandRoot;
extern TPCE_API const FName NAME_IKHand_Gun;
extern TPCE_API const FName NAME_IKHand_L;
extern TPCE_API const FName NAME_IKHand_R;


 /** */
UENUM(BlueprintType)
enum class ETurnInPlaceState: uint8
{
	Done,
	InProgress,
	Suspended
};

/** */
UENUM(BlueprintType)
enum class ELongitudinalDirection: uint8
{
	Forward,
	Backward
};

/** */
UENUM(BlueprintType)
enum class ECardinalDirection: uint8
{
	North,
	South,
	East,
	West
};

UENUM(BlueprintType)
enum class ECharacterGait: uint8
{
	Walk,
	Run,
	Sprint
};


UENUM(BlueprintType)
enum class ECharacterRotationMode: uint8
{
	/** Do not perform any automatic actor rotation. */
	None						UMETA(DisplayName = "None"),
	/** Rotate actor towards its direction of movemment. */
	OrientToMovement			UMETA(DisplayName = "Orient to Movement"),
	/** Rotate actor in increments of 90 degrees after the angular distance to the control rotation goes beyond the limit. */
	OrientToController			UMETA(DisplayName = "Orient to Controller"),
};

/** Helper function for net serialization of FVector */
bool TPCE_API SerializeQuantizedVector(FArchive& Ar, FVector& Vector, EVectorQuantization QuantizationLevel);

/** Helper function for net serialization of FRotator */
void TPCE_API SerializeQuantizedRotator(FArchive& Ar, FRotator& Rotator, ERotatorQuantization QuantizationLevel);

/**
 * Replicated look rotation. 
 * Struct used for configurable replication precision.
 */
USTRUCT()
struct TPCE_API FRepLook
{
	GENERATED_BODY()

	FRepLook()
		: RotationQuantizationLevel(ERotatorQuantization::ByteComponents)
		, Rotation(ForceInitToZero)
	{
	}

private:

	/** Allows tuning the compression level for replicated rotation. You should only need to change this from the default if you see visual artifacts. */
	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay)
	ERotatorQuantization RotationQuantizationLevel;

public:

	UPROPERTY(Transient)
	FRotator Rotation;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		SerializeQuantizedRotator(Ar, Rotation, RotationQuantizationLevel);

		return true;
	}

	bool operator==(const FRepLook& Other) const
	{
		return Rotation == Other.Rotation;
	}

	bool operator!=(const FRepLook& Other) const
	{
		return !(*this == Other);
	}


};

template<>
struct TStructOpsTypeTraits<FRepLook>: public TStructOpsTypeTraitsBase2<FRepLook>
{
	enum
	{
		WithNetSerializer = true,
	};
};

/**
 * Replacement for FRepMovement that replicates acceleration normal, pivot turn state and turn in place target
 */
USTRUCT()
struct TPCE_API FRepExtMovement
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	uint8 bIsPivotTurning : 1;

	UPROPERTY(Transient)
	FVector Location;

	UPROPERTY(Transient)
	FRotator Rotation;

	UPROPERTY(Transient)
	FVector Velocity;

	UPROPERTY(Transient)
	FVector Acceleration;

	UPROPERTY(Transient)
	float TurnInPlaceTargetYaw;

	/** Allows tuning the compression level for the replicated velocity vector. You should only need to change this from the default if you see visual artifacts. */
	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay)
	EVectorQuantization VelocityQuantizationLevel;

	/** Allows tuning the compression level for the replicated location vector. You should only need to change this from the default if you see visual artifacts. */
	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay)
	EVectorQuantization LocationQuantizationLevel;

	/** Allows tuning the compression level for replicated rotation. You should only need to change this from the default if you see visual artifacts. */
	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay)
	ERotatorQuantization RotationQuantizationLevel;

	FRepExtMovement()
		: bIsPivotTurning(false)
		, Location(ForceInitToZero)
		, Rotation(ForceInitToZero)
		, Velocity(ForceInitToZero)
		, Acceleration(ForceInitToZero)
		, TurnInPlaceTargetYaw(0.f)
		, VelocityQuantizationLevel(EVectorQuantization::RoundWholeNumber)
		, LocationQuantizationLevel(EVectorQuantization::RoundWholeNumber)
		, RotationQuantizationLevel(ERotatorQuantization::ByteComponents)
		
	{}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// Pack bitfield with flags
		uint8 Flags = (bIsPivotTurning << 0);
		Ar.SerializeBits(&Flags, 1);
		bIsPivotTurning = (Flags & (1 << 0)) ? 1 : 0;

		bOutSuccess = true;

		bOutSuccess &= SerializeQuantizedVector(Ar, Location, LocationQuantizationLevel);
		SerializeQuantizedRotator(Ar, Rotation, RotationQuantizationLevel);
		bOutSuccess &= SerializeQuantizedVector(Ar, Velocity, VelocityQuantizationLevel);
		bOutSuccess &= SerializeFixedVector<1, 16>(Acceleration, Ar);
		
		Ar << TurnInPlaceTargetYaw;

		return true;
	}

	bool operator==(const FRepExtMovement& Other) const
	{
		return bIsPivotTurning == Other.bIsPivotTurning
			&& Location == Other.Location
			&& Rotation == Other.Rotation
			&& Velocity == Other.Velocity
			&& Acceleration == Other.Acceleration
			&& TurnInPlaceTargetYaw == Other.TurnInPlaceTargetYaw;
	}

	bool operator!=(const FRepExtMovement& Other) const
	{
		return !(*this == Other);
	}
};

template<>
struct TStructOpsTypeTraits<FRepExtMovement>: public TStructOpsTypeTraitsBase2<FRepExtMovement>
{
	enum
	{
		WithNetSerializer = true,
	};
};
