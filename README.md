# Animated Mesh Framework (Raylib + Assimp)

A minimalistic yet powerful **Animated Mesh System** built in **C++**, designed to load, manage, and render **skinned animated models** using **Assimp** for importing formats like `.fbx`, `.dae`, `.gltf`, etc., and **Raylib** for rendering.

Supports:
- Hardware Skinning (via Shaders)
- Software Skinning (CPU transformation)
- Multiple Animation Blending
- Bone Hierarchy Transformations
- Keyframe Animation System

---

## Features
- **AnimatedMesh Loader**: Load meshes, bones, weights, and animations from standard formats using Assimp.
- **SkinSurface Management**: Manage vertex buffers, indices, and bone influences.
- **KeyFrame Animation System**: Smooth interpolation of position and rotation keyframes.
- **Animator System**: Control playback, blending, and transition between animations.
- **Entity System**: Instantiate animated models as game objects with materials and transforms.
- **Software Skinning**: CPU-based vertex transformations.
- **Hardware Skinning**: GPU skinning via custom shaders.
- **Animation Modes**: Loop, Ping-Pong, OneShot, Blending.

---

## Dependencies
- [Raylib](https://www.raylib.com/) (Graphics & Input)
- [Assimp](https://github.com/assimp/assimp) (Asset Import Library)
- C++11 or higher

---



## Example Usage

```cpp
AnimatedMesh mesh;
mesh.Load("assets/character.fbx");

Entity *player = new Entity(&mesh);

// Load Textures/Materials
Texture2D texture = LoadTexture("assets/textures/character_diffuse.png");
player->AddMaterial(texture);
entity->SetMaterial(-1, 0);//-1 all sub mesh 0 texture layer

// Play Animation
player->GetAnimator()->Play("Run");

// In Game Loop
while (!WindowShouldClose())
{
    float deltaTime = GetFrameTime();

    player->Update(deltaTime);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    player->Render();

    EndMode3D();
    EndDrawing();
}
```

---

## Skinning Modes
### Hardware Skinning:
- Enable custom skinning shaders.
- Upload bone matrices as uniform arrays.
- Vertex shader will skin vertices in GPU.

### Software Skinning:
- Vertices are transformed on CPU per frame.
- Useful for testing or fallback in old hardware.

---

## Animation Blending Example:
```cpp
player->GetAnimator()->Play("Attack", Animation::ONESHOT);
player->GetAnimator()->PlayWithNext("Attack", "Idle", Animation::ONESHOT, Animation::LOOP, 0.5f);
```

---

## License
MIT License

---

## Author
Made by **Luis Santos AKA djoker**, for the pure joy of creating an animation system from scratch, without relying on heavy game engines.