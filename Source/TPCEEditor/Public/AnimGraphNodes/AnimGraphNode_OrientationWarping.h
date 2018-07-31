// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "AnimNodes/AnimNode_OrientationWarping.h"

#include "AnimGraphNode_OrientationWarping.generated.h"

/**
*
*/
UCLASS()
class TPCEEDITOR_API UAnimGraphNode_OrientationWarping : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Settings")
	FAnimMode_OrientationWarping Node;

	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FString GetNodeCategory() const override;

	UAnimGraphNode_OrientationWarping();
};
