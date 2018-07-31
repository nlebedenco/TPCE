// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/ActorWidget.h"
#include "ExtraTypes.h"

#include "ExtCharacterDebugWidgetBase.generated.h"

class AExtCharacter;
class UTextBlock;

/**
 * 
 */
UCLASS(abstract)
class TPCE_API UExtCharacterDebugWidgetBase : public UActorWidget
{
	GENERATED_BODY()

public:

	UExtCharacterDebugWidgetBase(const FObjectInitializer& ObjectInitializer);

private:

	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	AExtCharacter* ExtCharacterOwner;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess= "true"))
	UTextBlock* RotationModeText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UTextBlock* StanceText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UTextBlock* GaitText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UTextBlock* AttitudeText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UTextBlock* MovementStateText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UTextBlock* GroundSpeedText;

	void SetGroundSpeed(float Value);

protected:

	static const FText TEXT_Standing;
	static const FText TEXT_Crouched;
	static const FText TEXT_Primary;
	static const FText TEXT_Secondary;
	static const FText TEXT_Ragdoll;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    virtual void OnWidgetComponentChanged(UActorWidgetComponent* OldWidgetComponent) override;

	UFUNCTION()
	virtual void HandleRagdollChanged(AExtCharacter* Sender);

	UFUNCTION()
	virtual void HandleMovementModeChanged(ACharacter* Sender, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);

	UFUNCTION()
	virtual void HandleCrouchChanged(AExtCharacter* Sender);

	UFUNCTION()
	virtual void HandleGaitChanged(AExtCharacter* Sender);

	UFUNCTION()
	virtual void HandleGenericActionChanged(AExtCharacter* Sender);

	UFUNCTION()
	virtual void HandleRotationModeChanged(AExtCharacter* Sender);

	UFUNCTION()
	virtual void HandleGroundSpeedChanged(AExtCharacter* Sender, float Value);

	virtual void ClearAllText();

public:

	static FString FormatMovementMode(EMovementMode MovementMode, uint8 CustomMovementMode);

	FORCEINLINE AExtCharacter* GetExtCharacterOwner() const { return ExtCharacterOwner;  }

	FORCEINLINE UTextBlock* GetRotationModeText() const { return RotationModeText; }

	FORCEINLINE UTextBlock* GetStanceText() const { return StanceText; }

	FORCEINLINE UTextBlock* GetGaitText() const { return GaitText; }

	FORCEINLINE UTextBlock* GetAttitudeText() const { return AttitudeText; }

	FORCEINLINE UTextBlock* GetMovementStateText() const { return MovementStateText; }

	FORCEINLINE UTextBlock* GetGroundSpeedText() const { return GroundSpeedText; }
};
