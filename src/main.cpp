
#include "pch.h"
#include "Config.hpp"
#include <raylib.h>
#include "entity.hpp"
#include "assets.hpp"

 
 


int main()
{

    
    
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(1280, 720, "Assimp Animated Models By Luis Santos AKA DJOKER");
    SetTargetFPS(60);
 
   
  


    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 250.0f, 250.0f }; 
    camera.target = (Vector3){ 0.0f, 0.0f, 1.0f };     
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 90.0f;                               
    camera.projection = CAMERA_PERSPECTIVE;            

  
    
    AnimatedMesh mesh;
    mesh.Import("model/idle.fbx");
    mesh.ImportAnimation("model/Capoeira.fbx","capoeira");
    mesh.ImportAnimation("model/Breakdance 1990.fbx","breakdance");
    mesh.ImportAnimation("model/Mma Kick.fbx","kick");



    
    Entity* entity = new Entity(&mesh);
    




    //entity->Rotate({ 40.0f, 0.0f, 0.0f });
    entity->Scale({ 0.01f, 0.01f, 0.01f });
    //entity->GetAnimator()->Play("mixamo.com", Animation::LOOP, 0.25f);
    entity->GetAnimator()->Play("capoeira", Animation::LOOP, 0.25f);


    
    Shader shader = LOAD_SHADER("main", "shaders/skin.vs", "shaders/skin.fs");
    Vector3 lightDir = { 0.0f, -1.0f, -1.0f };   // Luz vinda de cima/diagonal
    Vector3 lightColor = { 1.0f, 1.0f, 1.0f };   // Luz branca
    Vector3 ambientColor = { 0.2f, 0.2f, 0.2f }; // Ambiente fraco


    Texture2D texture = LOAD_TEXTURE("model/Ch24_1001_Diffuse.png", "Ch24_1001_Diffuse");
    
    entity->AddMaterial(texture);
    entity->SetMaterial(-1, 0);







    while (!WindowShouldClose())
    {
        double dt =GetFrameTime();


        if (IsKeyPressed(KEY_ENTER))
        {
            if(IsCursorHidden()) ShowCursor();
            else HideCursor();
        }
      

        if (IsKeyPressed(KEY_C))
        {
            entity->GetAnimator()->Play("capoeira", Animation::LOOP, 0.25f);
        }

        if (IsKeyPressed(KEY_B))
        {
            entity->GetAnimator()->Play("breakdance", Animation::LOOP, 0.25f);
        }

        if (IsKeyPressed(KEY_K))
        {
            entity->GetAnimator()->Play("kick", Animation::LOOP, 0.25f);
        }
 
        if (IsKeyPressed(KEY_I))
        {
            entity->GetAnimator()->Play("mixamo.com", Animation::LOOP, 0.25f);
        }


       
        UpdateCamera(&camera, CAMERA_FREE);
 

        BeginDrawing();
         ClearBackground(BLACK);
      
         entity->Update((float)dt);
          

            BeginMode3D(camera);

            Matrix viewMatrix = GetCameraMatrix(camera);
            Matrix projectionMatrix = rlGetMatrixProjection();

            rlEnableShader(shader.id);

            int locIndex = GetShaderLocation(shader, "projection");
            rlSetUniformMatrix(locIndex, projectionMatrix);
         

            locIndex = GetShaderLocation(shader, "view");
            rlSetUniformMatrix(locIndex, viewMatrix);
     

            SetShaderValue(shader, GetShaderLocation(shader, "lightDir"), &lightDir, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, GetShaderLocation(shader, "lightColor"), &lightColor, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, GetShaderLocation(shader, "ambientColor"), &ambientColor, SHADER_UNIFORM_VEC3);






            entity->Render(shader);
     


            DrawGrid(20, 10.0f);  

            EndMode3D();


        DrawFPS(10, 10);
        EndDrawing();
    }
    UnloadTexture(texture);
    delete entity;
    UNLOAD_ASSETS();
    CloseWindow();
    return 0;
}



// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>
// #include "assimp/cimport.h"

// int main()
// {
//    Assimp::Importer importer;

//    int flags = aiProcess_Triangulate;
// 		flags |= aiProcess_JoinIdenticalVertices;
// 		flags |= aiProcess_GenSmoothNormals;
// 		flags |= aiProcess_GenUVCoords;
// 		flags |= aiProcess_SortByPType;
// 		flags |= aiProcess_TransformUVCoords;
// 		flags |= aiProcess_RemoveComponent;

//         const aiScene *scene = importer.ReadFile("model/zombie.b3d", flags);
  
//        //aiScene *scene = ( aiScene*) aiImportFile("model/zombie.b3d", flags);

//         if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
//         {
//              LogError( "Erro ao carregar modelo: ");
//             return false;
//         }

//        if (scene->HasAnimations())
//         {
    
            
          
                
				
//           const aiAnimation *anim = scene->mAnimations[0];
//           double Duration = anim->mDuration;
//           double TicksPerSecond = anim->mTicksPerSecond;
          
//           LogInfo("Load %d  %s %d animations with duration %f and %f ticks per second", anim->mName.C_Str(),scene->mNumAnimations, Duration, TicksPerSecond);

//                 for (unsigned int i = 0; i < anim->mNumChannels; i++)
//                 {
//                     aiNodeAnim *pNodeAnim = anim->mChannels[i];

//                     unsigned int mNumPositionKeys = pNodeAnim->mNumPositionKeys;
//                     unsigned int mNumRotationKeys = pNodeAnim->mNumRotationKeys;
//                     unsigned int mNumScalingKeys = pNodeAnim->mNumScalingKeys;

//                     std::string boneName = pNodeAnim->mNodeName.C_Str();
//                     LogInfo("Bone %s has %d position keys, %d rotation keys and %d scaling keys", boneName.c_str(), mNumPositionKeys, mNumRotationKeys, mNumScalingKeys);



//                     // for (unsigned int posIdx = 0; posIdx < pNodeAnim->mNumRotationKeys; posIdx++)
//                     // {
//                     //     aiVector3D pos = pNodeAnim->mPositionKeys[posIdx].mValue;
//                     //     float pTime = pNodeAnim->mPositionKeys[posIdx].mTime;
//                     // }
                    
//                     for (unsigned int posIdx = 0; posIdx < mNumRotationKeys; posIdx++)
//                     {

//                         aiQuatKey key = pNodeAnim->mRotationKeys[posIdx];   
//                         double rTime  = key.mTime;
//                         aiQuaternion rot = key.mValue;
//                         LogInfo("Rotation %f (%f %f %f %f)",rTime, rot.x, rot.y, rot.z, rot.w);
                  
//                         //aiQuaternion rot = pNodeAnim->mRotationKeys[posIdx].mValue;
//                        // float rTime = pNodeAnim->mRotationKeys[posIdx].mTime;
//                        // LogInfo("Rotation %f (%f %f %f %f)",rTime, rot.x, rot.y, rot.z, rot.w);
//                      }
 


                
//               }


//         }
          
// }