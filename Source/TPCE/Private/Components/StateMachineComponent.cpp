// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StateMachineComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/UserDefinedEnum.h"
#include "Net/UnrealNetwork.h"
#include "Logging/MessageLog.h"

#define LOCTEXT_NAMESPACE "StateMachine"

UStateMachineComponent::UStateMachineComponent()
{
	bAutoActivate = true;
	State = 0;

	SetIsReplicatedByDefault(true);
}

void UStateMachineComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UStateMachineComponent, State, bOnlyReplicateToOwner ? ELifetimeCondition::COND_OwnerOnly : ELifetimeCondition::COND_None, REPNOTIFY_OnChanged);
}

void UStateMachineComponent::SetStateInternal(uint8 NewState)
{
	if (IsActive() && State != NewState)
	{
		const uint8 PreviousState = State;
		const EStateChangeMode StateChangeMode = IsNetSimulating() ? EStateChangeMode::Replicated : EStateChangeMode::Local;
		State = NewState;
		ReceiveStateChange(PreviousState, StateChangeMode);
		StateChangeDelegate.Broadcast(this, PreviousState, StateChangeMode);
	}
}

void UStateMachineComponent::MulticastReliableSetState_Implementation(uint8 NewState)
{
	SetStateInternal(NewState);
}

void UStateMachineComponent::ClientReliableSetState_Implementation(uint8 NewState)
{
	SetStateInternal(NewState);
}

void UStateMachineComponent::MulticastUnreliableSetState_Implementation(uint8 NewState)
{
	SetStateInternal(NewState);
}

void UStateMachineComponent::ClientUnreliableSetState_Implementation(uint8 NewState)
{
	SetStateInternal(NewState);
}

void UStateMachineComponent::OnRep_State(uint8 PreviousState)
{
	ReceiveStateChange(PreviousState, EStateChangeMode::ReplicatedLate);
	StateChangeDelegate.Broadcast(this, PreviousState, EStateChangeMode::ReplicatedLate);
}

void UStateMachineComponent::ReceiveStateChange_Implementation(const uint8 PreviousState, const EStateChangeMode StateChangeMode)
{
}

void UStateMachineComponent::SetState(const uint8 NewState)
{
	if (IsNetSimulating())
	{
		FMessageLog("PIE").Warning(FText::Format(
			LOCTEXT("StateMachineSetStateAuthorityOnly", "SetState function should only be used by the network authority for {0}"),
			FText::FromName(GetOwner()->GetFName())
		));
		return;
	}

	if (bOnlyReplicateToOwner)
	{
		if (bReliable)
			ClientReliableSetState(NewState);
		else
			ClientUnreliableSetState(NewState);
	}
	else
	{
		if (bReliable)
			MulticastReliableSetState(NewState);
		else
			MulticastUnreliableSetState(NewState);
	}
}

#undef LOCTEXT_NAMESPACE