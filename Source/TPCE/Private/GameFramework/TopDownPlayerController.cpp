// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/TopDownPlayerController.h"

#include "GameFramework/Pawn.h"
#include "Components/ArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/TopDownPushToTargetComponent.h"
#include "Components/InputComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Engine/CollisionProfile.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogTopDownPlayerController, Log, All);

ATopDownPlayerController::ATopDownPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShowMouseCursor = true;

	// Create camera mout 
	static const FName CameraMountName(TEXT("CameraMount"));
	CameraMount = CreateDefaultSubobject<USphereComponent>(CameraMountName);
	CameraMount->SetupAttachment(RootComponent);
	CameraMount->SetGenerateOverlapEvents(false);
	CameraMount->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	static const FName CameraMountCollisionProfileName(TEXT("CameraActor"));
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	const UCollisionProfile& CollisionProfiles = *UCollisionProfile::Get();
	const int32 ProfilesCount = CollisionProfiles.GetNumOfProfiles();
	bool bIsProfileValid = false;
	for (int32 i = 0; !bIsProfileValid && i < ProfilesCount; ++i)
		bIsProfileValid = CollisionProfiles.GetProfileByIndex(i)->Name == CameraMountCollisionProfileName;
	if (!bIsProfileValid)
		UE_LOG(LogTopDownPlayerController, Warning, TEXT("Collision profile not found: '%s'. Camera mount created with default collision settings."), *CameraMountCollisionProfileName.ToString());
#endif

	CameraMount->SetCollisionProfileName(CameraMountCollisionProfileName);
	CameraMount->SetCanEverAffectNavigation(false);
	CameraMount->SetAbsolute(true, true, true);
	CameraMount->SetIsReplicated(false);

	// Create camera boom
	static const FName CameraBoomName(TEXT("CameraBoom"));
	CameraBoom = CreateDefaultSubobject<UArmComponent>(CameraBoomName);
	CameraBoom->SetupAttachment(CameraMount);
	CameraBoom->SetWorldRotation(FRotator(-53.1301024f, -45.0f, 0.0f));
	CameraBoom->TargetArmLength = 2200.0f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bEnableTargetLag = true;
	CameraBoom->TargetLagSpeed = 10.0f;
	CameraBoom->bUseTargetLagSubstepping = true;
	CameraBoom->TargetLagMaxTimeStep = 1.f / 30.f;
	CameraBoom->SetIsReplicated(false);

	// Create camera
	static const FName CameraComponentName(TEXT("CameraComponent"));
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(CameraComponentName);
	CameraComponent->SetupAttachment(CameraBoom, UArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	CameraComponent->FieldOfView = 60.0f;
	CameraComponent->SetIsReplicated(false);

	// Create camera tractor
	static const FName CameraTractorName(TEXT("CameraTractor"));
	CameraTractor = CreateDefaultSubobject<UTopDownPushToTargetComponent>(CameraTractorName);
	CameraTractor->bEnableLag = true;
	CameraTractor->Speed = 1.0f;
	CameraTractor->bTeleportToTargetToStart = true;
	CameraTractor->bSlide = true;
	CameraTractor->Friction = 0.0f;
	CameraTractor->bForceSubStepping = true;
	CameraTractor->MaxSimulationTimeStep = 1.f / 30.f;
	CameraTractor->bAdjustTargetLagForViewTarget = true;
	CameraTractor->SetUpdatedComponent(CameraMount);
	CameraTractor->SetIsReplicated(false);
}

void ATopDownPlayerController::ForceAbsolute(USceneComponent* SceneComponent)
{
	check(SceneComponent);
	bool bAbsolute = SceneComponent->IsUsingAbsoluteLocation() && SceneComponent->IsUsingAbsoluteRotation() && SceneComponent->IsUsingAbsoluteScale();
	if (!bAbsolute)
	{
		UE_LOG(LogTopDownPlayerController, Warning, TEXT("'%s' coordinates need to be defined in world-space and were changed to absolute."), *SceneComponent->GetReadableName());
		SceneComponent->SetAbsolute(true, true, true);
	}
}

void ATopDownPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ForceAbsolute(CameraMount);

	// Some Physics related methods require the GEngine global to be created and cannot be on the constructor.
	CameraMount->SetMassOverrideInKg(NAME_None, 1.0f);
	CameraMount->SetLinearDamping(0.1f);
	CameraMount->SetEnableGravity(false);
	CameraMount->bIgnoreRadialImpulse = true;
	CameraMount->bIgnoreRadialImpulse = true;
	CameraMount->SetShouldUpdatePhysicsVolume(false);
}

void ATopDownPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CameraMount->SetWorldLocation(RootComponent->GetComponentLocation());
}

void ATopDownPlayerController::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	check(CameraComponent);
	CameraComponent->GetCameraView(DeltaTime, OutResult);
}

bool ATopDownPlayerController::HasActiveCameraComponent() const
{
	check(CameraComponent);
	return CameraComponent->IsActive();
}

bool ATopDownPlayerController::HasActivePawnControlCameraComponent() const
{
	check(CameraComponent);
	return (CameraComponent->IsActive() && CameraComponent->bUsePawnControlRotation);
}

void ATopDownPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

}

void ATopDownPlayerController::SetCameraTargetComponent(USceneComponent* NewTargetComponent, const FName& SocketName)
{
	if (CameraTractor)
		CameraTractor->SetTargetComponent(NewTargetComponent, SocketName);
}



