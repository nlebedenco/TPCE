// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealEd.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorDelegates.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "ExtraEditorTypes.h"

class UAnimSequence;

DECLARE_LOG_CATEGORY_EXTERN(LogTPCEEditor, All, All)

/** Helper function to convert the input for GetActions to a list that can be used for delegates */
template <typename T>
static TArray<TWeakObjectPtr<T>> GetTypedWeakObjectPtrs(const TArray<FAssetData>& SelectedAssets)
{
	TArray<TWeakObjectPtr<T>> TypedObjects;

	const int32 Count = SelectedAssets.Num();
	for (int32 ObjIdx = 0; ObjIdx < Count; ++ObjIdx)
	{
		if (UObject* Asset = SelectedAssets[ObjIdx].GetAsset())
			TypedObjects.Add(CastChecked<T>(Asset));
	}

	return TypedObjects;
}

/** Helper to obtain the common parent class of a group of assets. */
UClass* FindCommonClass(const TArray<FAssetData>& SelectedAssets);

class FTPCEEditor : public IModuleInterface
{

private:

	TArray<FName> RegisteredClassNames; 
	TArray<FName> RegisteredPropertyTypes;
	TArray<FName> RegisteredComponentClassNames;
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
	
	FDelegateHandle ContentBrowserAssetExtenderDelegateHandle;

private:

	void UnregisterAssetTools();
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);
	void RegisterComponentVisualizer(const FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer);
	void RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate);
	void RegisterCustomPropertyTypeLayout(FName ClassName, FOnGetPropertyTypeCustomizationInstance DetailLayoutDelegate);

	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	void CreateContentBrowserAssetMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	void CreateDistanceCurveAssets(const TArray<TWeakObjectPtr<UAnimSequence>> AnimSequences, EDistanceCurveType DistanceCurveType);
	
	/** Creates a unique package and asset name taking the form InBasePackageName+InSuffix */
	void CreateUniqueAssetName(const FString& InBasePackageName, const FString& InSuffix, FString& OutPackageName, FString& OutAssetName) const;

protected:

	void RegisterPropertyEditors();
	void RegisterComponentVisualizers();
	void RegisterAssetTools(IAssetTools& AssetTools);
	void RegisterContentBrowserExtenders();

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};