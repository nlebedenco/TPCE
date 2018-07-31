// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNodes/AnimNode_SpeedWarping.h"
#include "AnimGraphNode_SkeletalControlBase.h"

#include "AnimGraphNode_SpeedWarping.generated.h"

/**
*
*/
UCLASS()
class TPCEEDITOR_API UAnimGraphNode_SpeedWarping : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

	/** Enable drawing of the debug information of the node */
	UPROPERTY(EditAnywhere, Category = Debug)
	bool bEnableDebugDraw;

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_SpeedWarping Node;

public:

	UAnimGraphNode_SpeedWarping();

	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:

	virtual void CopyPinDefaultsToNodeData(UEdGraphPin* InPin) override;
	virtual const FAnimNode_SkeletalControlBase* GetNode() const { return &Node; }
	virtual void Draw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelMeshComp) const override;
};
