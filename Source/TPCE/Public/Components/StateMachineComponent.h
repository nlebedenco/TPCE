// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/ActorComponent.h"

#include "StateMachineComponent.generated.h"

class FLifetimeProperty;

UENUM(BlueprintType)
enum class EStateChangeMode: uint8
{
	Local			UMETA(DisplayName = "Local Change"),
	Replicated		UMETA(DisplayName = "Replicated Change"),
	ReplicatedLate 	UMETA(DisplayName = "Replicated Late Change")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStateChangeHandler, class UStateMachineComponent* const, StateMachine, const uint8, PreviousState, const EStateChangeMode, StateChangeMode);


/**
 * StateMachineComponent is a simple actor component that stores and replicates a single variable representing its state.
 *
 * State changes are replicated to relevant clients and it's possible to distinguish when replication occurs right away
 * or later after the fact. 
 *
 * Be mindful of setting new states from within ReceiveStateChange or OnStateChange event handlers as this
 * will immediately re-trigger the same event handlers potentially leading to a stack overflow. It's a well-known best practice to
 * encapsulate state, so generally only the owning actor should be allowed to modify its state machine component directly.
 */
UCLASS(ClassGroup=(StateMachine), meta=(BlueprintSpawnableComponent))
class TPCE_API UStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UStateMachineComponent();

private:

	UPROPERTY(Transient, ReplicatedUsing = OnRep_State, VisibleAnywhere, BlueprintReadOnly, Category=State, meta = (AllowPrivateAccess="true"))
	uint8 State;

	/** if true, only replicates state to the owning client. */
	UPROPERTY(EditDefaultsOnly, Category=ComponentReplication, AdvancedDisplay)
	bool bOnlyReplicateToOwner;

	/** RELIABLE RPC to set state in server and all clients*/
	UFUNCTION(NetMulticast, Reliable)
	void MulticastReliableSetState(uint8 NewState);

	/** UNRELIABLE RPC to set state in server and all clients*/
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastUnreliableSetState(uint8 NewState);

	/** RELIABLE RPC to set state in server and owning client */
	UFUNCTION(Client, Reliable)
	void ClientReliableSetState(uint8 NewState);

	/** UNRELIABLE RPC to set state in server and owning client */
	UFUNCTION(Client, Unreliable)
	void ClientUnreliableSetState(uint8 NewState);

	/** Internally set state and call the state changed event */
	void SetStateInternal(uint8 NewState);

	UFUNCTION()
	virtual void OnRep_State(uint8 PreviousState);

protected:

	/** Multicast event triggered when state changes */
	UPROPERTY(BlueprintAssignable, Category = "Multicast Event", meta = (DisplayName="StateChange"))
	FStateChangeHandler StateChangeDelegate;

	UFUNCTION(BlueprintNativeEvent, Category = StateMachine, meta = (DisplayName = "StateChange"))
	void ReceiveStateChange(const uint8 PreviousState, const EStateChangeMode StateChangeMode);
	virtual void ReceiveStateChange_Implementation(const uint8 PreviousState, const EStateChangeMode StateChangeMode);

public:	

	/** if true, will issue reliable RPC calls to update relevant clients on state changes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ComponentReplication, AdvancedDisplay)
	bool bReliable;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Set the state machine to a new state. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=State)
	void SetState(const uint8 NewState);

	/** Current state of the state machine. */
	FORCEINLINE uint8 GetState() const { return State; }

	/** Stante change dynamic multicast delegate. */
	FORCEINLINE FStateChangeHandler& StateChange() { return StateChangeDelegate; }
};
