// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/ExtCharacterDebugWidgetBase.h"
#include "GameFramework/ExtCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ActorWidgetComponent.h"
#include "Components/TextBlock.h"
#include "Kismet/Kismet.h"

const FText UExtCharacterDebugWidgetBase::TEXT_Crouched = FText::FromName(NAME_Crouched);
const FText UExtCharacterDebugWidgetBase::TEXT_Standing = FText::FromName(NAME_Normal);
const FText UExtCharacterDebugWidgetBase::TEXT_Primary = FText::FromName(NAME_Primary);
const FText UExtCharacterDebugWidgetBase::TEXT_Secondary = FText::FromName(NAME_Secondary);
const FText UExtCharacterDebugWidgetBase::TEXT_Ragdoll = FText::FromName(NAME_Ragdoll);

FString UExtCharacterDebugWidgetBase::FormatMovementMode(EMovementMode MovementMode, uint8 CustomMovementMode)
{
	return MovementMode == MOVE_Custom
		? FString::Printf(TEXT("%s (%d)"), *GetEnumeratorDisplayName(EMovementMode, MovementMode), CustomMovementMode)
		: GetEnumeratorDisplayName(EMovementMode, MovementMode);
}

UExtCharacterDebugWidgetBase::UExtCharacterDebugWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UExtCharacterDebugWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ExtCharacterOwner)
		ClearAllText();
}

void UExtCharacterDebugWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ExtCharacterOwner)
		SetGroundSpeed(ExtCharacterOwner->GetVelocity().Size2D());		
}

void UExtCharacterDebugWidgetBase::OnWidgetComponentChanged(UActorWidgetComponent* OldWidgetComponent)
{
	if (ExtCharacterOwner)
	{
		ExtCharacterOwner->CrouchChangedDelegate.RemoveDynamic(this, &UExtCharacterDebugWidgetBase::HandleCrouchChanged);
		ExtCharacterOwner->GenericActionChangedDelegate.RemoveDynamic(this, &UExtCharacterDebugWidgetBase::HandleGenericActionChanged);
		ExtCharacterOwner->GaitChangedDelegate.RemoveDynamic(this, &UExtCharacterDebugWidgetBase::HandleGaitChanged);
		ExtCharacterOwner->RotationModeChangedDelegate.RemoveDynamic(this, &UExtCharacterDebugWidgetBase::HandleRotationModeChanged);
		ExtCharacterOwner->MovementModeChangedDelegate.RemoveDynamic(this, &UExtCharacterDebugWidgetBase::HandleMovementModeChanged);
		ExtCharacterOwner->RagdollChangedDelegate.RemoveDynamic(this, &UExtCharacterDebugWidgetBase::HandleRagdollChanged);

		ExtCharacterOwner = nullptr;
	}

	Super::OnWidgetComponentChanged(OldWidgetComponent);
	
	if (UActorWidgetComponent* NewWidgetComponent = GetActorWidgetComponent())
	{
		ExtCharacterOwner = Cast<AExtCharacter>(NewWidgetComponent->GetOwner());
		if (ExtCharacterOwner)
		{
			ExtCharacterOwner->CrouchChangedDelegate.AddDynamic(this, &UExtCharacterDebugWidgetBase::HandleCrouchChanged);
			ExtCharacterOwner->GenericActionChangedDelegate.AddDynamic(this, &UExtCharacterDebugWidgetBase::HandleGenericActionChanged);
			ExtCharacterOwner->GaitChangedDelegate.AddDynamic(this, &UExtCharacterDebugWidgetBase::HandleGaitChanged);
			ExtCharacterOwner->RotationModeChangedDelegate.AddDynamic(this, &UExtCharacterDebugWidgetBase::HandleRotationModeChanged);
			ExtCharacterOwner->MovementModeChangedDelegate.AddDynamic(this, &UExtCharacterDebugWidgetBase::HandleMovementModeChanged);
			ExtCharacterOwner->RagdollChangedDelegate.AddDynamic(this, &UExtCharacterDebugWidgetBase::HandleRagdollChanged);

			HandleCrouchChanged(ExtCharacterOwner);
			HandleGenericActionChanged(ExtCharacterOwner);
			HandleGaitChanged(ExtCharacterOwner);
			HandleRotationModeChanged(ExtCharacterOwner);
			HandleMovementModeChanged(ExtCharacterOwner, EMovementMode::MOVE_None, 0);
			HandleRagdollChanged(ExtCharacterOwner);
			HandleGroundSpeedChanged(ExtCharacterOwner, 0.f);
		}
		else
		{
			ClearAllText();
		}
	}
}

void UExtCharacterDebugWidgetBase::ClearAllText()
{
	if (MovementStateText)
		MovementStateText->SetText(FText::GetEmpty());

	if (StanceText)
		StanceText->SetText(FText::GetEmpty());

	if (GaitText)
		GaitText->SetText(FText::GetEmpty());

	if (AttitudeText)
		AttitudeText->SetText(FText::GetEmpty());

	if (RotationModeText)
		RotationModeText->SetText(FText::GetEmpty());

	if (GroundSpeedText)
		GroundSpeedText->SetText(FText::GetEmpty());
}

void UExtCharacterDebugWidgetBase::SetGroundSpeed(float Value)
{
	if (GroundSpeed != Value)
	{
		GroundSpeed = Value;
		HandleGroundSpeedChanged(ExtCharacterOwner, Value);
	}
}

void UExtCharacterDebugWidgetBase::HandleRagdollChanged(AExtCharacter* Sender)
{
	if (MovementStateText)
	{
		const bool bIsRagdoll = Sender->IsRagdoll();
		if (bIsRagdoll)
		{
			MovementStateText->SetText(TEXT_Ragdoll);
		}
		else
		{
			const EMovementMode MovementMode = Sender->GetCharacterMovement()->MovementMode;
			const uint8 CustomMovementMode = Sender->GetCharacterMovement()->CustomMovementMode;
			MovementStateText->SetText(FText::FromString(FormatMovementMode(MovementMode, CustomMovementMode)));
		}
	}
}

void UExtCharacterDebugWidgetBase::HandleMovementModeChanged(ACharacter* Sender, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	if (MovementStateText)
	{
		const bool bIsRagdoll = CastChecked<AExtCharacter>(Sender)->IsRagdoll();
		if (!bIsRagdoll)
		{
			const EMovementMode MovementMode = Sender->GetCharacterMovement()->MovementMode;
			const uint8 CustomMovementMode = Sender->GetCharacterMovement()->CustomMovementMode;
			MovementStateText->SetText(FText::FromString(FormatMovementMode(MovementMode, CustomMovementMode)));
		}
	}
}

void UExtCharacterDebugWidgetBase::HandleCrouchChanged(AExtCharacter* Sender)
{
	if (StanceText)
	{
		const bool bIsCrouched = Sender->bIsCrouched;
		StanceText->SetText(bIsCrouched ? TEXT_Crouched : TEXT_Standing);
	}
}

void UExtCharacterDebugWidgetBase::HandleGaitChanged(AExtCharacter* Sender)
{
	if (GaitText)
	{
		const ECharacterGait Gait = Sender->GetGait();
		GaitText->SetText(FText::FromString(GetEnumeratorDisplayName(ECharacterGait, Gait)));
	}
}

void UExtCharacterDebugWidgetBase::HandleGenericActionChanged(AExtCharacter* Sender)
{
	if (AttitudeText)
	{
		const bool bIsPerformingGenericAction = Sender->bIsPerformingGenericAction;
		AttitudeText->SetText(bIsPerformingGenericAction ? TEXT_Secondary : TEXT_Primary);
	}
}

void UExtCharacterDebugWidgetBase::HandleRotationModeChanged(AExtCharacter* Sender)
{
	if (RotationModeText)
	{
		const ECharacterRotationMode RotationMode = Sender->GetRotationMode();
		RotationModeText->SetText(FText::FromString(GetEnumeratorDisplayName(ECharacterRotationMode, RotationMode)));
	}
}

void UExtCharacterDebugWidgetBase::HandleGroundSpeedChanged(AExtCharacter* Sender, float Value)
{
	if (GroundSpeedText)
		GroundSpeedText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Value)));
}
