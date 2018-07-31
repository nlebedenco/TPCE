# Third Person Character Extensions

Unreal Engine 4 plug-in that adds an extended character class with an integrated 
locomotion system and components for third person based games.


## About

The TPCE plug-in implements a module that provides an extended Character class 
derived from ACharacter and extra components to speed up development of third person
character based games. 


## Supported Platforms

This plug-in should work in all platforms.


## Dependencies

No external dependencies.

## Usage

You can use this plug-in as a project plug-in, or an Engine plug-in.

If you use it as a project plug-in, clone this repository into your project's
*/Plugins* directory and compile your game in Visual Studio. A C++ code project
is required for this to work.

If you use it as an Engine plug-in, clone this repository into the
*/Engine/Plugins/Gameplay* directory and compile your game. Engine source code is 
required for this so make sure to have it selected in Epic's Launcher Library 
options.

After compiling the plug-in, you have to **enable it** in Unreal Editor's
plug-in browser.


## Support

**Note: This plugin is not supported by Epic Games.**

Please [file an issue here](https://github.com/nlebedenco/TCPE/issues)


## References

* [Introduction to UE4 Plugins](https://wiki.unrealengine.com/An_Introduction_to_UE4_Plugins)


# CHANGELOG

## Version 1.0

#### ExtCharacter

- Multiple Gaits: March(Walk), Run, Sprint
- Multiple Stances: Normal, Aim (Left Foot Forward), Crouched
- Multiple Actions: March, Sprint, Crouch, Jump, Aim
- Can distinguish between Jump and unintentional Fall or Launch.
- Can be configured to land with zero sliding or conserve movement.
- Compression of the legs on landing blends naturally with movement.
- Enhanced Rotation Modes: None, OrientToMovement, OrientToController.
- Can be configured to either use Acceleration or Velocity as direction of movement.
- Rotation can be interpolated (spring-like smooth damp) or use a constant rate (default).
- Adaptive Rotation system can adjust rotation rate/speed according to the character's speed.
- Configurable input acceleration scale. Can be used used to impose a resistance to turn independent of friction.
- Supports Pivot Turning (turn by accelerating in the opposite direction of velocity)
- Ragdoll implementation that keeps the capsule attached to the character and does not interfere with movement replication.
- Supports two get-up animations to recover from ragdoll (face down and back down).
- Idle transitions between stances to avoid weird blend poses.
- Start transitions extracted from the loop animations, no need for specific animations.
- Stop transitions adapted to foot orientation and position.
- Turn In Place using distance matching.
- Leaning on ground and in air proportional to the character's speed and orientation.
- Support for secondary motions such as breathing.
- Breathing rate dynamically adjusted according to the character's speed.
- Aim Offset adapted to the Rotation Mode.
- Aim Offset can be overriden for a target (LookAtActor).

- Network friendly:
    - Extends the movement replication already provided by the engine avoiding race conditions.
    - Replicated actions include march, aim and sprint. 
    - Jump state replicated with movement mode to simulated proxies. No replication race conditions or inconsistent states.
    - Replicates RotationMode so animations can also be consistent in all peers.
    - Replicates Character Acceleration, LookRotation, LookAtActor (for aim offset overriding), TurnInPlace Target rotation and Pivot Turn state.
    - Animations rely solely on replicated information, no RPCs

- Can be configured to use a single shared camera component attached to the player controller.
- Can be controlled by AI controllers all the same (bUseAccelerationForPaths must be true).

- Animation Graph built in layers to allow for easy introduction of intermediary animations and montages.
- Only use in-place animations. No root motion or ad-hoc solutions to extract movement from animations.
- Example anim notify for footsteps.
- Movement animations synchronized using sync markers to reduce foot sliding.
- Editor Tool (Animation Modifier) to automatically create foot sync markers along any axis.
- Speed warping as used in Epic's Paragon: scales foot IK target positions potentially producing shorter or larger steps to reduce foot sliding.
- Adaptive play rates and speed warping scale calculated according to the character's speed in relation to the intended movement speed. 
- Foot placement correction with IK for uneven terrain, ramps and staircases (slopes should not exceed 30deg for natural good poses).


#### Custom Anim Nodes

**Speed Warping Node**: scale foot IK targets according to the character's speed potentially producing shorter or larger steps.

**Orientation Warping Node**: rotates the IK Foot Root bone in a certain direction while adjusting the torso to compensate.

**Foot Placement Node**: adjusts Foot IK bones within limits according to terrain.


#### Custom Components

**Actor Widget **: specialized UserWidget that has a reference to the WidgetComponent that owns it. A reference to the owining actor can be obtained through the owning WidgetComponent then.

**Actor Widget Component**: specialized WidgetComponent that can auto assign itself to a Player and become visible/invisble when its player joins/leaves the game.
    
**Custom Arm Components (camera boom)**: 
    - *Fixed Length*: basic fixed length arm
    - *Telescopic*: can smoothly interpolate to a new lenth
    - *Rebound Spring*: can be immediately compressed against scene objects and smoothly decompressed when there's no contact anymore. 
    
**PushToTargetComponent**: custom movement component that updates the position of another component simulating an unrealistic force that 
pushes the updated component towards a target. This 'force' becomes stronger with distance and ignores both inertia and gravity. 
It supports sliding over surfaces and can simulate friction. Normally the root component of the owning actor is moved. Does not affect 
components that are simulating physics.

**StateMachineComponent**: a simple actor component that stores and replicates a single variable representing its state. It can handle state changes that happen on the spot and after the 
fact due to network relevancy and late replication (see unreal manual's chapter about [replication and network relevancy parts 1-4](https://docs.unrealengine.com/en-us/Resources/ContentExamples/Networking/2_1]).

    
#### Editor Extensions

- Custom Content Browser Extension to create distance curves from anim sequences.
- Custom Editor Details extensions for Character and CharacterMovementComponent to have categories always displayed in a consistent order.


