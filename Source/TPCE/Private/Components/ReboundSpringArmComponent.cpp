
#include "Components/ReboundSpringArmComponent.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Engine/World.h"
#include "Math/MathExtensions.h"

UReboundSpringArmComponent::UReboundSpringArmComponent()
{
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bStartAtFullLength = true;
	ProbeSize = 12.0f;
	ProbeChannel = ECC_Camera;
	bDoCollisionTest = true;
	bEnableReboundLag = false;
	ReboundLagSpeed = 5.0f;
	
}

void UReboundSpringArmComponent::OnRegister()
{
	Super::OnRegister();

	// Enforce reasonable limits to avoid potential div-by-zero
	ReboundLagSpeed = FMath::Max(ReboundLagSpeed, 0.f);
}

void UReboundSpringArmComponent::InitializeComponent()
{
	Super::InitializeComponent();

	PreviousTargetArmLength = bStartAtFullLength ? TargetArmLength : 0.0f;
}

FVector UReboundSpringArmComponent::CalcTargetArm(const FVector& Origin, const FRotator& Rotation, float DeltaTime)
{
	float MaxTargetArmLength = TargetArmLength;
	if (!FMath::IsNearlyZero(TargetArmLength))
	{
		if (bDoCollisionTest)
		{
			bIsCameraFixed = true;

			// Calculate socket offset in local space.
			const FVector LocalOffset = FRotationMatrix(Rotation).TransformVector(SocketOffset);
			// Calculate desired socket location. 
			const FVector DesiredLoc = Origin - (Rotation.Vector() * TargetArmLength) + LocalOffset;
			// Do a sweep to ensure we are not penetrating the world
			const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
			FHitResult Hit;
			if (GetWorld()->SweepSingleByChannel(Hit, Origin, DesiredLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams))
			{
				// Subtract the local offset to find the actual endpoint and compute the obtained arm length
				const FVector ArmEndpoint = Hit.Location - LocalOffset;
				const FVector DesiredTargetArm = Origin - ArmEndpoint;
				MaxTargetArmLength = FMath::Sign(TargetArmLength) * DesiredTargetArm.Size();
				if (FMath::Abs(MaxTargetArmLength) < FMath::Abs(PreviousTargetArmLength))
				{
					PreviousTargetArmLength =  MaxTargetArmLength;
					return DesiredTargetArm;
				}
			}
		}
	}

	PreviousTargetArmLength = bEnableReboundLag ? FMathEx::FSafeInterpTo(PreviousTargetArmLength, MaxTargetArmLength, DeltaTime, ReboundLagSpeed) : MaxTargetArmLength;
	return Rotation.Vector() * PreviousTargetArmLength;
}

FVector UReboundSpringArmComponent::GetUnfixedCameraPosition() const
{
	return UnfixedCameraPosition;
}

bool UReboundSpringArmComponent::IsCollisionFixApplied() const
{
	return bIsCameraFixed;
}
