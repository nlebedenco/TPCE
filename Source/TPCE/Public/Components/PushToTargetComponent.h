// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"

#include "PushToTargetComponent.generated.h"

/**
 * Updates the position of another component simulating an unrealistic attraction force towards a target. 
 * This 'force' becomes stronger with distance and ignores both inertia and gravity. Supports sliding over
 * surfaces and can simulate friction.
 *
 * Normally the root component of the owning actor is moved. Does not affect components that are simulating physics.
 */
UCLASS(ClassGroup=Movement, meta=(BlueprintSpawnableComponent), hideCategories=(Velocity))
class TPCE_API UPushToTargetComponent: public UMovementComponent
{
	GENERATED_BODY()

public:	

	UPushToTargetComponent();

private:

	/** The current target we are homing towards. Can only be set at runtime (when the component is spawned or updating). */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = PushToTarget, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<USceneComponent> TargetComponent;

	/** The socket name of the current target we are homing towards. Can only be set at runtime (when the component is spawned or updating). */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = PushToTarget, meta = (AllowPrivateAccess = "true"))
	FName TargetSocketName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PushToTarget, meta = (AllowPrivateAccess = "true"))
	bool bIsBlocked;

	/**
	 * Draws markers at the target (in green) and the lagged position (in yellow).
	 * A line is drawn between the two locations, in green normally but in red if the distance to the lag target has been clamped (by TargetLagMaxDistance).
	 */
	UPROPERTY(EditDefaultsOnly, Category = PushToTarget, AdvancedDisplay)
	bool bDrawDebugMarkers;

	FVector PreviousTargetLocation;

protected:
	
	/** Minimum delta time considered when ticking. Delta times below this are not considered. This is a very small non-zero positive value to avoid potential divide-by-zero in simulation code. */
	static const float MIN_TICK_TIME;

	/** Return the target location considering the socket name if any */
	FVector GetTargetLocation() const;

	/** 
	 * Return the location of the UpdatedComponent in world space to be used for this frame. 
	 * Default implementation simply returns the UpdatedComponent's location unmodified, but derived classes can override
	 * this method to implement diferent rules depending on the state of the components involved, for example, producing 
	 * a boost or a drag effect.  
	 */
	virtual FVector AdjustCurrentLocationToTarget(const FVector& CurrentLocation, const FVector& TargetLocation) const;

	/** Interpolation method used for target lag */
	virtual FVector VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed);

	/** 
	 * Move the updated component accounting for initial penetration, blocking collisions and sliding surfaces. 
	 *
	 * @return true if the movement could be performed and the involved components remain valid, otherwise false.
	 */
	virtual bool MoveUpdatedComponent(const FVector& Delta, const FQuat& NewRotation);

public:	

	/**
	 * If true, the updated component lags behind the target to smooth its movement.
	 * @see Speed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PushToTarget)
	bool bEnableLag;

	/** 
	 * Controls how strong the attraction is, thus how quickly the updated component reaches the target. Low values are slower (more drag), high values are faster (less drag), while zero is instant (no drag). 
	 * If the updated componet is simulating physics 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PushToTarget, meta = (ClampMin = "0.0", ClampMax = "10000.0", UIMin = "0.0", UIMax = "10000.0"))
	float Speed;

	/** If true, the updated component will slide over surfaces when blocked. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PushToTarget)
	bool bSlide;

	/** A normalized coeficient of friction representing the ammount of movement lost due to friction when sliding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PushToTarget, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Friction;

	/** If true, the updated component is teleported immediately to the new homing target when assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PushToTarget)
	bool bTeleportToTargetToStart;

	/**
	 * If true, forces sub-stepping to break up movement into discrete smaller steps to improve accuracy of the trajectory.
	 * Objects that move in a straight line typically do *not* need to set this, as movement always uses continuous collision detection (sweeps).
	 * @see MaxSimulationTimeStep, MaxSimulationIterations
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PushToTargetSimulation, AdvancedDisplay)
	bool bForceSubStepping;

	/**
	 * Max time delta for each discrete simulation step.
	 * Lowering this value can address issues with fast-moving objects or complex collision scenarios, at the cost of performance.
	 *
	 * WARNING: if (MaxSimulationTimeStep * MaxSimulationIterations) is too low for the min framerate, the last simulation step may exceed MaxSimulationTimeStep to complete the simulation.
	 * @see MaxSimulationIterations, bForceSubStepping
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0166", ClampMax = "0.50", UIMin = "0.0166", UIMax = "0.50"), Category = PushToTargetSimulation, AdvancedDisplay)
	float MaxSimulationTimeStep;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void InitializeComponent() override;
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

	/**
	 * Return true if still in the world.  It will check things like the KillZ, outside world bounds, etc. and handle the situation.
	 */
	virtual bool IsStillInWorld();

	UFUNCTION(BlueprintCallable, Category = "Game|Components|PushToTarget")
	virtual void SetTargetComponent(USceneComponent* NewTargetComponent, const FName& SocketName = NAME_None);

	FORCEINLINE USceneComponent* GetTargetComponent() { return TargetComponent.Get(); }

	FORCEINLINE FName GetTargetSocketName() { return TargetSocketName; }

	FORCEINLINE USceneComponent* GetUpdatedComponent() { return UpdatedComponent; }

	FORCEINLINE UPrimitiveComponent* GetUpdatedPrimitive() { return UpdatedPrimitive; }

	FORCEINLINE bool IsBlocked() const { return bIsBlocked; }
};
