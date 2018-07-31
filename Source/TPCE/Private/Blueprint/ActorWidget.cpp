// Fill out your copyright notice in the Description page of Project Settings.

#include "Blueprint/ActorWidget.h"

UActorWidget::UActorWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UActorWidget::OnWidgetComponentChanged(UActorWidgetComponent* OldWidgetComponent)
{

}

void UActorWidget::SetWidgetComponent(UActorWidgetComponent* NewWidgetComponent)
{
	if (ActorWidgetComponent != NewWidgetComponent)
	{
		auto OldWidgetComponent = ActorWidgetComponent;
		ActorWidgetComponent = NewWidgetComponent;
		OnWidgetComponentChanged(OldWidgetComponent);
		K2_OnWidgetComponentChanged(OldWidgetComponent);
	}
}

