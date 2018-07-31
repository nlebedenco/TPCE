// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNodes/AnimNode_FootPlacement.h"
#include "AnimGraphNode_SkeletalControlBase.h" 

#include "AnimGraphNode_FootPlacement.generated.h"

/**
*
*/
UCLASS()
class TPCEEDITOR_API UAnimGraphNode_FootPlacement: public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Settings")
	FAnimNode_FootPlacement Node;

public:

	UAnimGraphNode_FootPlacement();

	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:

	virtual void CopyPinDefaultsToNodeData(UEdGraphPin* InPin) override;
	virtual const FAnimNode_SkeletalControlBase* GetNode() const { return &Node; }
};
