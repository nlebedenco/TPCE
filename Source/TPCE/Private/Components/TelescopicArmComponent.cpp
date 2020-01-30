
#include "Components/TelescopicArmComponent.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Engine/World.h"
#include "Math/MathExtensions.h"

UTelescopicArmComponent::UTelescopicArmComponent()
{
	bStartAtFullLength = true;
	bEnableResizeLag = false;
	ResizeLagSpeed = 10.0f;
	PreviousTargetArmLength = 0.0f;
}

void UTelescopicArmComponent::OnRegister()
{
	Super::OnRegister();

	// Enforce reasonable limits to avoid potential div-by-zero
	ResizeLagSpeed = FMath::Max(ResizeLagSpeed, 0.f);
}

void UTelescopicArmComponent::InitializeComponent()
{
	Super::InitializeComponent();
	PreviousTargetArmLength = bStartAtFullLength ? TargetArmLength : 0.0f;
}

FVector UTelescopicArmComponent::CalcTargetArm(const FVector& Origin, const FRotator& Rotation, float DeltaTime)
{
	PreviousTargetArmLength = bEnableResizeLag ? FMathEx::FSafeInterpTo(PreviousTargetArmLength, TargetArmLength, DeltaTime, ResizeLagSpeed) : TargetArmLength;
	return Rotation.Vector() * PreviousTargetArmLength;
}
