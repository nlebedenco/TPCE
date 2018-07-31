// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ExtCharacter.h"
#include "Interfaces/PawnControlInterface.h"

#include "TopDownCharacter.generated.h"

class UArmComponent;
class APlayerController;

/**
 * Basic character implementation for top-down games where the generic action is to aim using a laser pointer.
 */
UCLASS(Abstract)
class TPCE_API ATopDownCharacter : public AExtCharacter, public IPawnControlInterface
{
	GENERATED_BODY()
	
public:

	ATopDownCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:

	UPROPERTY(EditDefaultsOnly, Category = Input, AdvancedDisplay)
	FName InteractInputName;

	UPROPERTY(EditDefaultsOnly, Category = Input, AdvancedDisplay)
	FName TauntInputName;

	/** Maximum distance the camera pivot can be when the character is moving. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0"))
    float MaxCameraDistanceNeutral;

	/** Maximum distance the camera pivot can be when the character is moving in an aggressive state (either aiming or firing). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0"))
    float MaxCameraDistanceAiming;

	/** Draws markers at the camera pivot target (in orange) and the circunference to which it's constrained. */
	UPROPERTY(EditDefaultsOnly, Category = Camera)
	bool bDrawDebugMarkers;

	/** Camera bracket that can keep a camera boom at a constant distance. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UArmComponent* CameraBracket;

protected:

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Handle player input to face a certain direction */
	virtual void PlayerInputLook(const FVector& Direction);

	/** Handle player input to interact with the scene */
	virtual void PlayerInputInteract();

	/** Handle player input to taunt other characters */
	virtual void PlayerInputTaunt();

public:

	/** If true, mouse is used to control look rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	bool bUseMouseToLook;

	virtual void Tick(float DeltaTime) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
	virtual bool HasActiveCameraComponent() const override;
	virtual bool HasActivePawnControlCameraComponent() const override;
	virtual void BecomeViewTarget(APlayerController* PC) override;
	virtual void EndViewTarget(APlayerController* PC) override;
	virtual void UnPossessed() override;
	virtual void OnStartGenericAction() override;

	//~Begin IPawnControl
	virtual void ProcessInput(const float DeltaTime, const bool bGamePaused);
	//~End IPawnControl

	/** Make the character interact with an object in the scene. */
	UFUNCTION(BlueprintCallable, Category = Character)
	virtual void Interact();

	/** Make the character taunt another character. */
	UFUNCTION(BlueprintCallable, Category = Character)
	virtual void Taunt();
};
