// Fill out your copyright notice in the Description page of Project Settings.

#include "ComponentVisualizers/ArmComponentVisualizer.h"
#include "SceneManagement.h"
#include "Components/ArmComponent.h"
#include "DrawDebugHelpers.h"

static const FColor	ArmColor(255,0,0);

void FArmComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (const UArmComponent* Arm = Cast<const UArmComponent>(Component))
	{
		const FVector TargetLocation = Arm->GetTargetLocation();
		const FTransform SocketTransform = Arm->GetSocketTransform(UArmComponent::SocketName, RTS_World);
		const FVector EndPointLocation = SocketTransform.GetTranslation();

		PDI->DrawLine(TargetLocation, EndPointLocation, ArmColor, SDPG_World );
	}
}
