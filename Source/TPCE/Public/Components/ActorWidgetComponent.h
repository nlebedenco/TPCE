// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"

#include "ActorWidgetComponent.generated.h"

class UActorWidget;

/**
 * A specialized widget component that supports actor widgets and auto assignment to player.
 */
UCLASS(Blueprintable, ClassGroup = "UserInterface", showCategories = (Activation), meta = (BlueprintSpawnableComponent))
class TPCE_API UActorWidgetComponent: public UWidgetComponent
{
	GENERATED_BODY()

public:

	UActorWidgetComponent();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UserInterface)
	TEnumAsByte<EAutoReceiveInput::Type> AutoAssignPlayer;

	virtual void OnHiddenInGameChanged() override;
	virtual void OnVisibilityChanged() override;

	UFUNCTION()
	virtual void OnPlayerAdded(int32 PlayerIndex);

	UFUNCTION()
	virtual void OnPlayerRemoved(int32 PlayerIndex);

	virtual void OnAutoAssignPlayerChanged();

public:

	UFUNCTION(BlueprintCallable, Category=UserInterface)
	void SetAutoAssignPlayer(EAutoReceiveInput::Type PlayerIndex);

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif

	virtual void OnRegister() override;

	virtual void InitWidget() override;
	virtual void SetWidget(UUserWidget* InWidget) override;
};
