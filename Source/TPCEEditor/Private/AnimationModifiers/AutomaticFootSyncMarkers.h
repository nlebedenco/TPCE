// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "Animation/AnimTypes.h"

#include "AutomaticFootSyncMarkers.generated.h"

class UAnimSequence;

USTRUCT(BlueprintType)
struct FBoneModifier
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere)
	FName BoneName;

	/** An optional offset to add to the pelvis-foot distance to fine tune your markers and curves. */
	UPROPERTY(EditAnywhere)
	float Offset;

	FBoneModifier();

	FBoneModifier(FName BoneName);

	FBoneModifier(FName BoneName, float Offset);
};

/**
 * Animation Modifier to automatically generate foot sync markers. 
 * C++ implementation based on the work of Giuseppe Portelli <https://github.com/gportelli/FootSyncMarkers>
 */
UCLASS()
class UAutomaticFootSyncMarkers : public UAnimationModifier
{
	GENERATED_BODY()

public:

	static const FName NotifyTrackName;

	UAutomaticFootSyncMarkers();

protected:

	virtual void RemoveSyncTrack(UAnimSequence* AnimationSequence);

	virtual FVector GetRelativeBoneLocationAtTime(UAnimSequence* AnimationSequence, FName TargetBoneName, FName RelativeBoneName, float Time);

public:

	/** Only animations with a path containing this filter as a case insensitive substring will be affected. Empty value matches all. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FName PathFilter;
	
	/** The name of the pelvis bone in your character’s skeleton. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FName PelvisBoneName;

	/** The script computes foot distances from pelvis axis line in the movement direction. You have to set the direction along which your animation sequence is moving. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TEnumAsByte<EAxisOption::Type> MovementAxis;

	/** Custom forward vector of the pelvis bone in world coordinates. Should tipically be a a normalized vector. Only used when PelvisAxis is Custom. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FVector CustomMovementAxis;

	/** An array containing descriptors for each foot you want to analyze(the script works with any number of feet). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TArray<FBoneModifier> FootBones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, AdvancedDisplay)
	uint32 bMarkFootPlantOnly: 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, AdvancedDisplay)
	uint32 bCreateCurve : 1;

	virtual bool CanEditChange(const UProperty* InProperty) const override;

	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
	virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "SyncTrackName"))
	FName GetSyncTrackName() const;
};
