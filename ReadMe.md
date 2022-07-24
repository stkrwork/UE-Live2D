# UE-Live2D
Unreal Engine Plugin for integrating Live2D Cubism Native SDK into Unreal Engine

## Supported features

| Feature | Supported |
|---------|-----------|
| Moc3 File Import | Yes |
| Model3 Json File Import | Yes |
| Physics3 Json File Import | Yes |
| Motion3 Json File Import | Yes |
| Expressions3 Json File Import | No |
| Moc3 Model Display | Yes |
| Motion System | Yes |
| Physics System | Yes |
| Expressions System | No |
| Custom Asset Editors | Yes |
| Utility functions for UMG Image to display model | Yes |


## Setup
1. Copy the plugin into your Project into the folder \<ProjectRoot\>/Plugins/UELive2D/
2. Download the Live2D Cubism Native SDK
3. Copy the Dll/Lib folder from the Live 2D Cubism Native SDK intp the folder Source/ThirdParty/Live2DLibrary/Core

## Importing assets
1. Export your model for Unity
2. Remove the .json file ending of all your created json files, otherwise Unreal Engine will get confused
3. Drag and drop the .model3 file into your content browser, and the system will generate all the assets needed

## Using the model with UMG Images
1. Have your model imported
2. In the Graph of the UMG Widget call the function SetBrushFromLive2DModelMotion with the UMG Image and the Motion that should be displayed.
    - It is best to save the model motion into a variable, especially for the follow up steps
3. In Construct don't forget to call StartMotion on your Model Motion.
3. In Destrcut don't forget to call StopMotion on your Model Motion.