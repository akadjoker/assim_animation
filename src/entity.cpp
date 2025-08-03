#include "pch.h"
#include "glad.h"
#include "entity.hpp"


KeyFrame::KeyFrame(KeyFrame *t)
{
    if ((!t) || (t->numPositionKeys() == 0) || (t->numRotationKeys() == 0))
    {

        LogError("Null animation pointer or nor frames");
        return;
    }

    for (u32 i = 0; i < t->numPositionKeys(); i++)
    {
        positionKeyFrames.push_back(t->positionKeyFrames[i]);
    }

    for (u32 i = 0; i < t->numRotationKeys(); i++)
    {
        rotationKeyFrames.push_back(t->rotationKeyFrames[i]);
    }
}

int KeyFrame::GetPositionIndex(float animationTime)
{
    // SDL_Log("KeyFrame time %f  %ld",animationTime,positionKeyFrames.size());
    for (u32 index = 0; index < positionKeyFrames.size() - 1; ++index)
    {
        if (animationTime < positionKeyFrames[index + 1].frame)
            return index;
    }

    DEBUG_BREAK_IF(true);
    return 0;
}

int KeyFrame::GetRotationIndex(float animationTime)
{
    for (u32 index = 0; index < rotationKeyFrames.size() - 1; ++index)
    {
        if (animationTime < rotationKeyFrames[index + 1].frame)
            return index;
    }
 
    DEBUG_BREAK_IF(true);
    return 0;
}

float KeyFrame::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

Quaternion KeyFrame::AnimateRotation(float movetime)
{
    if (rotationKeyFrames.size() == 1)
    {
        return rotationKeyFrames[0].rot;
    }
    int currentIndex =  GetRotationIndex(movetime);
    int nextIndex = currentIndex + 1;

    float factor = GetScaleFactor(rotationKeyFrames[currentIndex].frame, rotationKeyFrames[nextIndex].frame, movetime);

    //    SDL_Log(" %f %f %f",rotationKeyFrames[currentIndex].frame, rotationKeyFrames[nextIndex].frame,movetime);

    return QuaternionSlerp(rotationKeyFrames[currentIndex].rot, rotationKeyFrames[nextIndex].rot, factor);
}
Vector3 KeyFrame::AnimatePosition(float movetime)
{
    if (positionKeyFrames.size() == 1)
    {
        return positionKeyFrames[0].pos;
    }
    
    int currentIndex = GetPositionIndex(movetime);
    int nextIndex = currentIndex + 1;

    float factor = GetScaleFactor(positionKeyFrames[currentIndex].frame, positionKeyFrames[nextIndex].frame, movetime);
    return Vector3Lerp(positionKeyFrames[currentIndex].pos, positionKeyFrames[nextIndex].pos, factor);
}

//-------------------------------------------------------------------------------
// Animation
//-------------------------------------------------------------------------------
Animation::Animation(const std::string &name)
{
    state = Stoped;
    method = 0;
    currentTime = 0.0f;
    fps = 30.0f;
    mode = LOOP;
    isEnd = false;
    this->name = name;
}

bool Animation::Play(u32 mode, float fps)
{
    if (state == Stoped)
    {
        state = Playing;
        this->mode = mode;
        this->fps = fps;
        currentTime = 0.0f;
        return true;
    }
    return false;
}

bool Animation::Stop()
{
    if (state == Playing)
    {
        state = Stoped;
        currentTime = 0.0f;
        isEnd = true;

        return true;
    }
    return false;
}

Frame *Animation::AddFrame(const std::string& name)
{
    Frame *frame = new Frame();
    frame->name = name;
    frames.push_back(frame);
    framesMap[name] = frame;
    return frame;
}

bool Animation::IsEnded()
{
    if (state == Stoped)
    {
        return true;
    }
    return isEnd;
}

Animation::~Animation()
{
    for (u32 i = 0; i < frames.size(); i++)
    {
        delete frames[i];
    }
    frames.clear();
}

Frame *Animation::GetFrame(const std::string& name)
{
    if (framesMap.find(name) == framesMap.end())
    {
        LogWarning("Frame not found: %s", name.c_str());
        return nullptr;
    }
    return framesMap[name];
}

Frame *Animation::GetFrame(int index)
{
    if (index < 0 || index >= (int)frames.size())
    {
        return NULL;
    }
    return frames[index];
}

float Animation::GetDuration()
{
    return duration;
}

float Animation::GetTime()
{
    return currentTime;
}

float Animation::GetFPS()
{
    return fps;
}

int Animation::GetMode()
{

    return mode;
}
u64 Animation::GetState()
{
    return state;
}

void Animation::Force()
{

    if (frames.size() == 0)
    {
        return;
    }

    for (u32 i = 0; i < frames.size(); i++)
    {
        Frame *b = frames[i];
        if (b->keys.numPositionKeys() > 0)
        {
            b->pos = true;
        }

        if (b->keys.numRotationKeys() > 0)
        {
            b->rot = true;
        }
    }
}

void Animation::Update(float elapsed)
{

 if (frames.size() == 0)
    {
        return;
    }
    if (state == Stoped)
    {
        return;
    }
    isEnd = false;

    currentTime += elapsed * fps;

    switch (mode)
    {

    case LOOP:
    {
        state = Playing;
        if (currentTime >= duration)
        {
            isEnd = true;
        }
        currentTime = fmod(currentTime, duration);
        if (currentTime < 0.0f)
        {
            currentTime += duration;
        }
    }
    break;
    case PINGPONG:
    {
        state = Playing;
        currentTime = fmod(currentTime, duration * 2);
        if (currentTime > (duration * 2))
        {
            isEnd = true;
        }
        if (currentTime < 0.0f)
        {
            currentTime += duration * 2;
        }

        if (currentTime > duration)
        {
            currentTime = duration - (currentTime - duration);
            fps = -fps;
        }
    }
    break;
    case ONESHOT:
    {
        if (currentTime >= duration)
        {
            LogWarning("Stop Animation : %s. Duration : %f. CurrentTime : %f", name.c_str(), duration, currentTime);

            Stop();
        }
    }
    }



    for (u32 i = 0; i < frames.size(); i++)
    {
        Frame *b = frames[i];

        if (b->keys.numPositionKeys() > 0)
        {
            b->position = b->keys.AnimatePosition(currentTime);
            b->pos = true;
        }

        if (b->keys.numRotationKeys() > 0)
        {
            b->orientation = b->keys.AnimateRotation(currentTime);
            b->rot = true;
        }
    }
}

//-------------------------------------------------------------------------------
// Animator
//-------------------------------------------------------------------------------

Animator::Animator(Entity *parent)
{
    currentAnimation = nullptr;

    blendFactor = 0.0f;
    blendTime = 0.0f;
    blending = false;
    currentAnimationName = "";
    entity = parent;
}

Animator::~Animator()
{
    for (u32 i = 0; i < m_animations.size(); i++)
    {
        delete m_animations[i];
    }
}
 
 
void Animator::Update(float elapsed)
{
    if (blending)
    {
        blendFactor += elapsed / blendTime;
    //    LogInfo("Blend Factor: %f", blendFactor);

        if (blendFactor >= 0.99f)
        {
            blendFactor = 1.0f;
            blending = false;
            updateAnim(elapsed);
            return;
        }

        updateTrans(blendFactor);
        return;
    }

    updateAnim(elapsed);

    CheckForNextAnimation();
}

Animation *Animator::GetAnimation(const std::string &name)
{
    if (m_animations_map.find(name) == m_animations_map.end())
    {
        return NULL;
    }
    return m_animations_map[name];
}

Animation *Animator::GetAnimation(int index)
{
    if (index < 0 || index >= (int)m_animations.size())
    {
        return NULL;
    }
    return m_animations[index];
}

Animation *Animator::AddAnimation(const std::string &name)
{
    Animation *a = new Animation(name);

    for (size_t i = 0; i < entity->joints.size(); ++i)
    {
        a->AddFrame(entity->joints[i]->name);
    }

    m_animations.push_back(a);
    m_animations_map[name] = a;
    return a;
}

void Animator::beginTrans()
{

    if (!currentAnimation)
        return;

    for (size_t i = 0; i < entity->joints.size(); i++)
    {

        Frame *frame = currentAnimation->GetFrame(i);
        if (frame)
        {
            if (frame->keys.positionKeyFrames.size() != 0 && !frame->IgnorePosition)
            {
                frame->src_pos = entity->joints[i]->localPosition;

                frame->dest_pos = frame->keys.positionKeyFrames[0].pos;
            }
            if (frame->keys.rotationKeyFrames.size() != 0 && !frame->IgnoreRotation)
            {
                frame->src_rot = entity->joints[i]->localRotation;
                frame->dest_rot = frame->keys.rotationKeyFrames[0].rot;
            }
        }
    }
}

void Animator::updateTrans(float blend)
{

    if (!currentAnimation)
        return;
    for (size_t i = 0; i < entity->joints.size(); i++)
    {
        Node3D *joint = entity->joints[i];

        Frame *frame = currentAnimation->GetFrame(i);
        if (frame)
        {
            if (frame->pos && !frame->IgnorePosition)
            {
                joint->localPosition =Vector3Lerp(frame->src_pos, frame->dest_pos, blend);
            }
             

            if (frame->rot && !frame->IgnoreRotation)
            {
                joint->localRotation=QuaternionSlerp(frame->src_rot, frame->dest_rot, blend);
            }
        }
          joint->UpdateLocalTransform();
    }
}

void Animator::updateAnim(float elapsed)
{
    if (!currentAnimation)
        return;
    currentAnimation->Update(elapsed);

    if (currentAnimation->IsEnded())
    {
     //   LogInfo("Animation %s is ended", currentAnimation->name.c_str());
        return;
    };

    for (size_t i = 0; i < entity->joints.size(); i++)
    {
        Frame *frame = currentAnimation->GetFrame(i);
        Node3D *joint = entity->joints[i];
        if (frame->name!=joint->name)
        {
            LogWarning("Frame name %s does not match joint name %s", frame->name.c_str(), joint->name.c_str());
            continue;
        }
        if (frame)
        {
            if (frame->pos && !frame->IgnorePosition)
            {
              //  Vector3 pos = Vector3Lerp(joint->localPosition, frame->position, elapsed * 0.5f);
                entity->joints[i]->localPosition =   frame->position;
            }
            if (frame->rot && !frame->IgnoreRotation)
            {
               // Quaternion rot = QuaternionSlerp(joint->localRotation, frame->orientation, elapsed * 0.5f);
                entity->joints[i]->localRotation =  frame->orientation;
            }
        }
        joint->UpdateLocalTransform();
    }
}

bool Animator::PlayWithNext(const std::string& currentAnim, const std::string& nextAnim, 
                           int currentMode, int nextMode, float blendTime)
{
    bool success = Play(currentAnim, currentMode, blendTime);
    
    if (success) 
    {
        nextAnimationName = nextAnim;
        nextAnimationMode = nextMode;
        nextBlendTime = blendTime;
        hasNextAnimation = true;
    }
    
    return success;
}

void Animator::SetDefaultAnimation(const std::string& defaultAnim, int mode)
{
    nextAnimationName = defaultAnim;
    nextAnimationMode = mode;
    nextBlendTime = 0.3f;
    hasNextAnimation = true;
}

void Animator::CheckForNextAnimation()
{
    if (currentAnimation && currentAnimation->IsEnded() && hasNextAnimation) 
    {
        LogInfo("Animation %s ended, switching to %s", 
                currentAnimationName.c_str(), nextAnimationName.c_str());
        
     
        Play(nextAnimationName, nextAnimationMode, nextBlendTime);
        
     
        hasNextAnimation = false;
    }
}


bool Animator::Play(const std::string &name, int mode, float blendTime)
{
    if (m_animations_map.find(name) == m_animations_map.end())
    {
        LogWarning("Animator: Animation %s not found", name.c_str());
        return false;
    }

    if (currentAnimation)
    {
        if (currentAnimationName == name && currentAnimation->GetState() == Animation::Playing)
        {

            return false;
        }
    }

    currentAnimationName = name;
    if (currentAnimation)
    {
        currentAnimation->Stop();
    }

    currentAnimation = m_animations_map[name];
    currentAnimation->Play(mode, currentAnimation->GetFPS());
    currentAnimation->Force();

    this->blendTime = blendTime;
    blendFactor = 0.0f;
    blending = true;

    beginTrans();

    return true;
}

void Animator::Stop()
{

    if (currentAnimation)
    {
        currentAnimation->Stop();
    }
}
bool Animator::IsEnded()
{

    if (currentAnimation)
    {
        return currentAnimation->IsEnded();
    }
    return true;
}
bool Animator::IsPlaying()
{

    if (currentAnimation)
    {
        return currentAnimation->GetState() == Animation::Playing;
    }
    return false;
}

void Animator::SetIgnorePosition(const std::string &name, bool ignore)
{

    for (size_t i = 0; i < m_animations.size(); i++)
    {
        Frame *frame = m_animations[i]->GetFrame(name);
        if (frame)
        {
            frame->IgnorePosition = ignore;
        }
    }
}
void Animator::SetIgnoreRotation(const std::string &name, bool ignore)
{

    for (size_t i = 0; i < m_animations.size(); i++)
    {
        Frame *frame = m_animations[i]->GetFrame(name);
        if (frame)
        {
            frame->IgnoreRotation = ignore;
        }
    }
}



void SkinSurface::Init()
{

    vertexCount  = vertices.size();
    numTriangles = faces.size() / 3;

	LogInfo("Vertex count: %d Triangles: %d", vertexCount, numTriangles);
	

glGenVertexArrays(1, &vaoId);
glGenBuffers(1, &vboId);
glGenBuffers(1, &iboId);

glBindVertexArray(vaoId);
glBindBuffer(GL_ARRAY_BUFFER, vboId);

glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SkinVertex), &vertices[0], GL_STATIC_DRAW);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), &faces[0], GL_STATIC_DRAW);

glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), (void *)0);

// uv
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), (void *)offsetof(SkinVertex, uv));


// normal

glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), (void *)offsetof(SkinVertex, normal));

// ids
glEnableVertexAttribArray(5);
glVertexAttribIPointer(5, 4, GL_INT, sizeof(SkinVertex), (void *)offsetof(SkinVertex, bones));

// weights
glEnableVertexAttribArray(6);
glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), (void *)offsetof(SkinVertex, weights));


glBindVertexArray(0);


 
 

}
void SkinSurface::Release()
{
    rlUnloadVertexBuffer(vboId);
    rlUnloadVertexBuffer(iboId);
    rlUnloadVertexArray(vaoId);
}
void SkinSurface::Render()
{
    if (vaoId == 0 || vertices.size() == 0 || faces.size() == 0) return;
	rlEnableVertexArray(vaoId);
	//rlDrawVertexArrayElements(0, faces.size() , 0);

    glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_SHORT, 0);

 //   LogInfo("Render %d faces", faces.size());
    rlDisableVertexArray();
}


AnimatedMesh::AnimatedMesh() {}

AnimatedMesh::~AnimatedMesh() 
{
    for (size_t i = 0; i < animations.size(); i++)
    {
        delete animations[i];
    }
     
    animations.clear();
    animationsMap.clear();
}


   int AnimatedMesh::GetJointIndex(const char *name)
    {
        for (int i = 0; i < (int)joints.size(); i++) 
        {
            if (strcmp(joints[i].name.c_str(), name) == 0)
            {
                return i;
            }
        }
        return -1;
    }


void SetVertexBoneData(SkinVertex &vertex, int boneID, float weight)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (vertex.bones[i] < 0)
        {
            vertex.weights[i] = weight;
            vertex.bones[i] = boneID;
            break;
        }
    }
}

  void AnimatedMesh::ReadNodeHeirarchy(const aiNode *pNode)
    {
        if (!pNode) return;
        std::string NodeName(pNode->mName.data);
        std::string ParentName = "";

        if (strcmp(NodeName.c_str(), "Camera") == 0)
            return;
        if (strcmp(NodeName.c_str(), "Light") == 0)
            return;

        if (pNode->mParent != NULL)
        {
            ParentName = std::string(pNode->mParent->mName.data);
        }

        MeshJoint node;
        node.name = NodeName;
        node.parentName = ParentName;

        LogInfo("Node %s Parent %s", NodeName.c_str(), ParentName.c_str());


        aiVector3D Position;
		aiQuaternion Rotation;
        aiVector3D Scaling;

        

	    pNode->mTransformation.Decompose(Scaling, Rotation, Position);


        node.position= {Position.x, Position.y, Position.z};
        node.orientation = {Rotation.x, Rotation.y, Rotation.z, Rotation.w};
        node.scale = {Scaling.x, Scaling.y, Scaling.z};


        joints.push_back(std::move(node));



        for (int i = 0; i < (int)pNode->mNumChildren; i++)
        {
            ReadNodeHeirarchy(pNode->mChildren[i]);
        }
    }


Matrix ConvertMatrix(const aiMatrix4x4 &from)
{
    Matrix to;

    to.m0  = from.a1; to.m4  = from.a2; to.m8  = from.a3; to.m12 = from.a4;
    to.m1  = from.b1; to.m5  = from.b2; to.m9  = from.b3; to.m13 = from.b4;
    to.m2  = from.c1; to.m6  = from.c2; to.m10 = from.c3; to.m14 = from.c4;
    to.m3  = from.d1; to.m7  = from.d2; to.m11 = from.d3; to.m15 = from.d4;

    return to;
}


bool AnimatedMesh::ImportAnimation(const char *fileName,
                                   const std::string &name)
{
    if (!FileExists(fileName))
    {
        LogError("File not found: %s", fileName);
        return false;
    }


    const aiScene *g_scene = NULL;
    Assimp::Importer importer;


    int flags = aiProcess_Triangulate;
    flags |= aiProcess_JoinIdenticalVertices;
    flags |= aiProcess_GenSmoothNormals;
    flags |= aiProcess_GenUVCoords;
    flags |= aiProcess_SortByPType;
    flags |= aiProcess_TransformUVCoords;
    flags |= aiProcess_RemoveComponent;

    g_scene = importer.ReadFile(fileName, flags);
    if (!g_scene)
    {
        LogError("Fail load load mesh : %s, %s", fileName,
                 importer.GetErrorString());
        return false;
    }



        if (g_scene->HasAnimations())
        {

            LogInfo("Load %d animations", g_scene->mNumAnimations);
            


                const aiAnimation *anim = g_scene->mAnimations[0];

                double Duration = anim->mDuration;
                double TicksPerSecond = anim->mTicksPerSecond;
                if (TicksPerSecond == 0)
                {
                    TicksPerSecond = 30.0f;
                }

                
   
                MeshAnimationFrames* animation = addAnimation(name);
                
                animation->duration = Duration;
                animation->fps = TicksPerSecond;
          


    


               

               LogInfo("Animation (%s) Duration %f FramesPerSecond %f  Frames por bone %d",animation->name.c_str(), Duration, TicksPerSecond,joints.size());

                for (u32 i = 0; i < anim->mNumChannels; i++)
                {
                    aiNodeAnim *pNodeAnim = anim->mChannels[i];
                    Frame* frame = animation->AddFrame(pNodeAnim->mNodeName.data);
                    frame->pos = (pNodeAnim->mNumPositionKeys > 0);
                    frame->rot = (pNodeAnim->mNumRotationKeys > 0);
                 
                    
                   
                    
                  //  LogInfo("Frame %s   %d %d %d",frame->name.c_str(),pNodeAnim->mNumPositionKeys,pNodeAnim->mNumRotationKeys, pNodeAnim->mNumScalingKeys);

                    for (u32 posIdx = 0; posIdx < pNodeAnim->mNumPositionKeys; posIdx++)
                    {
                        aiVector3D pos = pNodeAnim->mPositionKeys[posIdx].mValue;
                        float time = pNodeAnim->mPositionKeys[posIdx].mTime;
                        frame->keys.AddPositionKeyFrame(time,{ pos.x, pos.y, pos.z });
                    }

                    for (u32 rotIdx = 0; rotIdx < pNodeAnim->mNumRotationKeys; rotIdx++)
                    {
                        aiQuaternion rot = pNodeAnim->mRotationKeys[rotIdx].mValue;
                        float time = pNodeAnim->mRotationKeys[rotIdx].mTime;
                       // LogInfo("Rotation %f (%f %f %f %f)",time, rot.x, rot.y, rot.z, rot.w);
                        frame->keys.AddRotationKeyFrame(time, QuaternionNormalize({ rot.x, rot.y, rot.z, rot.w }));
                    }


       

                }
            
        }
        

        return true;
    }
    



bool AnimatedMesh::Import(const std::string &name)
{

    if (!FileExists(name.c_str()))
    {
        LogError("File not found: %s", name.c_str());
        return false;
    }


    const aiScene *g_scene = NULL;
    Assimp::Importer importer;


    int flags = aiProcess_Triangulate;
    flags |= aiProcess_JoinIdenticalVertices;
    flags |= aiProcess_GenSmoothNormals;
    flags |= aiProcess_GenUVCoords;
    flags |= aiProcess_SortByPType;
    flags |= aiProcess_TransformUVCoords;
    flags |= aiProcess_RemoveComponent;

    g_scene = importer.ReadFile(name.c_str(), flags);
    if (!g_scene)
    {
        LogError("Fail load load mesh : %s, %s", name.c_str(),
                 importer.GetErrorString());
        return false;
    }

    for (u32 i = 0; i < g_scene->mNumMaterials; i++)
    {
        aiMaterial *mat = g_scene->mMaterials[i];


        for (unsigned int i = 0;
             i < mat->GetTextureCount(aiTextureType_DIFFUSE); i++)
        {
            aiString str;
            mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
            // material->diffuse = str.C_Str();
            LogInfo("Material texture %s", str.C_Str());
        }
    }

    for (unsigned int i = 0; i < g_scene->mNumMeshes; i++)
    {
        LogInfo("Load mesh %s", g_scene->mMeshes[i]->mName.C_Str());
        aiMesh *mesh = g_scene->mMeshes[i];

        SkinSurface surface;
        surface.name = g_scene->mMeshes[i]->mName.C_Str();
        surface.material = mesh->mMaterialIndex;

        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            Vector3 pos = { 0, 0, 0 };
            pos.x = mesh->mVertices[j].x;
            pos.y = mesh->mVertices[j].y;
            pos.z = mesh->mVertices[j].z;
            Vector2 uv{ 0, 0 };
            if (mesh->HasTextureCoords(0))
            {
                uv.x = mesh->mTextureCoords[0][j].x;
                uv.y = 1 * -mesh->mTextureCoords[0][j].y;
            }

            Vector3 normal{ 0, 0, 0 };
            if (mesh->HasNormals())
            {
                normal.x = mesh->mNormals[j].x;
                normal.y = mesh->mNormals[j].y;
                normal.z = mesh->mNormals[j].z;
            }

            surface.AddVertex(pos, uv, normal);
        }

        for (u32 j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace *face = mesh->mFaces + j;
            int f0 = face->mIndices[0];
            int f1 = face->mIndices[1];
            int f2 = face->mIndices[2];
            surface.AddFace(f0, f1, f2);
        }
        surfaces.push_back(std::move(surface));
    }

    if (g_scene->mRootNode)
    {
        ReadNodeHeirarchy(g_scene->mRootNode);

        LogInfo("Joints: %d", joints.size());


        for (unsigned int i = 0; i < g_scene->mNumMeshes; i++)
        {
            aiMesh *iMesh = g_scene->mMeshes[i];
            SkinSurface *surface = &surfaces[i];
            for (u32 boneIndex = 0; boneIndex < iMesh->mNumBones; ++boneIndex)
            {
                aiBone *iBone = iMesh->mBones[boneIndex];

                int jointIndex = GetJointIndex(iBone->mName.data);

                MeshJoint *joint = &joints[jointIndex];

                if (strcmp(iBone->mName.data, joint->name.c_str()) != 0)
                {
                    LogError("Bone %s != Joint %s", iBone->mName.data,
                             joint->name.c_str());
                    return false;
                }

                joint->offset = ConvertMatrix(iBone->mOffsetMatrix);

                //   Logger::Instance().Info("Mesh %s - Bone %s  Joint
                //   %s",surface->name.c_str(),
                //   iBone->mName.data,joint->name.c_str());

                for (u32 weightIndex = 0; weightIndex < iBone->mNumWeights;
                     ++weightIndex)

                {
                    aiVertexWeight vw = iBone->mWeights[weightIndex];
                    int vertexID = vw.mVertexId;
                    float weight = vw.mWeight;

                    DEBUG_BREAK_IF(vertexID >= (int)surface->vertices.size());

                    SetVertexBoneData(surface->vertices[vertexID], jointIndex,
                                      weight);
                }
            }
        }

        if (g_scene->HasAnimations())
        {

            LogInfo("Load %d animations", g_scene->mNumAnimations);
            


                const aiAnimation *anim = g_scene->mAnimations[0];

                double Duration = anim->mDuration;
                double TicksPerSecond = anim->mTicksPerSecond;
                if (TicksPerSecond == 0)
                {
                    TicksPerSecond = 30.0f;
                }

                std::string name ="default";

                if(anim->mName.length>=3)
                {
                    name = anim->mName.C_Str();
                }
   
                MeshAnimationFrames* animation = addAnimation(name);
                
                animation->duration = Duration;
                animation->fps = TicksPerSecond;
          


    


               

               LogInfo("Animation (%s) Duration %f FramesPerSecond %f  Frames por bone %d",animation->name.c_str(), Duration, TicksPerSecond,joints.size());

                for (u32 i = 0; i < anim->mNumChannels; i++)
                {
                    aiNodeAnim *pNodeAnim = anim->mChannels[i];
                    Frame* frame = animation->AddFrame(pNodeAnim->mNodeName.data);
                    frame->pos = (pNodeAnim->mNumPositionKeys > 0);
                    frame->rot = (pNodeAnim->mNumRotationKeys > 0);
                 
                    
                   
                  //  
                  //  LogInfo("Frame %s   %d %d %d",frame->name.c_str(),pNodeAnim->mNumPositionKeys,pNodeAnim->mNumRotationKeys, pNodeAnim->mNumScalingKeys);

                    for (u32 posIdx = 0; posIdx < pNodeAnim->mNumPositionKeys; posIdx++)
                    {
                        aiVector3D pos = pNodeAnim->mPositionKeys[posIdx].mValue;
                        float time = pNodeAnim->mPositionKeys[posIdx].mTime;
                        frame->keys.AddPositionKeyFrame(time,{ pos.x, pos.y, pos.z });
                    }

                    for (u32 rotIdx = 0; rotIdx < pNodeAnim->mNumRotationKeys; rotIdx++)
                    {
                        aiQuaternion rot = pNodeAnim->mRotationKeys[rotIdx].mValue;
                        float time = pNodeAnim->mRotationKeys[rotIdx].mTime;
                       // LogInfo("Rotation %f (%f %f %f %f)",time, rot.x, rot.y, rot.z, rot.w);
                        frame->keys.AddRotationKeyFrame(time, QuaternionNormalize({ rot.x, rot.y, rot.z, rot.w }));
                    }


       

                }
            
        }
        }

        return true;
    }

    void AnimatedMesh::SaveAnimation(const std::string &name,
                                     const std::string &fileName)
    {
            LogWarning("Not implemented - TODO");
    }

    void AnimatedMesh::Save(const std::string &name) 
    {

        LogWarning("Not implemented - TODO");
        // FILE *file = fopen(name.c_str(), "wb");
        
        


        // fclose(file);
        
    }


    bool AnimatedMesh::Load(const char *szFileName)
    {

        BinaryFile file;
        if (!file.open(szFileName)) return false;

        file.readChar();
        file.readChar();
        file.readChar();
        file.readChar();

        u32 version = file.readInt();

        if (version != 2024)
        {
            LogError("Unsupported version %d", version);
            return false;
        }

        u32 countMaterial = file.readInt();

        for (u32 i = 0; i < countMaterial; i++)
        {
            int len = file.readInt();
            std::string matName;
            matName.resize(len);
            file.readBytes(&matName[0], len);

            len = file.readInt();
            std::string diffuse;
            diffuse.resize(len);
            file.readBytes(&diffuse[0], len);

            materials.push_back(diffuse);


            LogInfo("Material %s texture %s", matName.c_str(), diffuse.c_str());
        }

        u32 countSurfaces = file.readInt();
        for (u32 i = 0; i < countSurfaces; i++)
        {
            SkinSurface surf;

            int len = file.readInt();
            surf.name.resize(len);
            file.readBytes(&surf.name[0], len);

            surf.material = file.readInt();
            u32 countVertices = file.readInt();
            u32 countFaces = file.readInt();

            LogInfo("Surface (%s) Material: %d  Vertex %d Faces %d",
                    surf.name.c_str(), surf.material, countVertices,
                    countFaces);

            for (u32 j = 0; j < countVertices; j++)
            {
                SkinVertex vertex;

                vertex.pos.x = file.readFloat();
                vertex.pos.y = file.readFloat();
                vertex.pos.z = file.readFloat();

                vertex.normal.x = file.readFloat();
                vertex.normal.y = file.readFloat();
                vertex.normal.z = file.readFloat();

                vertex.uv.x = file.readFloat();
                vertex.uv.y = 1 * -file.readFloat();

                vertex.weights[0] = file.readFloat();
                vertex.bones[0] = file.readInt();

                vertex.weights[1] = file.readFloat();
                vertex.bones[1] = file.readInt();

                vertex.weights[2] = file.readFloat();
                vertex.bones[2] = file.readInt();

                vertex.weights[3] = file.readFloat();
                vertex.bones[3] = file.readInt();

                surf.AddVertex(vertex);
            }

            for (u32 j = 0; j < countFaces; j++)
            {
                int a = file.readInt();
                int b = file.readInt();
                int c = file.readInt();
                surf.AddFace(a, b, c);
            }

            surfaces.push_back(std::move(surf));
        }

        u32 countJoints = file.readInt();
        for (u32 i = 0; i < countJoints; i++)
        {
            int len = file.readInt();
            std::string name;
            name.resize(len);
            file.readBytes(&name[0], len);

            len = file.readInt();
            std::string parentName;
            parentName.resize(len);
            file.readBytes(&parentName[0], len);

            LogInfo("Joint %s parent %s", name.c_str(), parentName.c_str());

            MeshJoint node;
            node.name = name;
            node.parentName = parentName;
            node.position.x = file.readFloat();
            node.position.y = file.readFloat();
            node.position.z = file.readFloat();

            node.orientation.x = file.readFloat();
            node.orientation.y = file.readFloat();
            node.orientation.z = file.readFloat();
            node.orientation.w = file.readFloat();

            node.scale.x = file.readFloat();
            node.scale.y = file.readFloat();
            node.scale.z = file.readFloat();

            node.offset.m0 = file.readFloat();
            node.offset.m1 = file.readFloat();
            node.offset.m2 = file.readFloat();
            node.offset.m3 = file.readFloat();
            node.offset.m4 = file.readFloat();
            node.offset.m5 = file.readFloat();
            node.offset.m6 = file.readFloat();
            node.offset.m7 = file.readFloat();
            node.offset.m8 = file.readFloat();
            node.offset.m9 = file.readFloat();
            node.offset.m10 = file.readFloat();
            node.offset.m11 = file.readFloat();
            node.offset.m12 = file.readFloat();
            node.offset.m13 = file.readFloat();
            node.offset.m14 = file.readFloat();
            node.offset.m15 = file.readFloat();


            joints.push_back(std::move(node));
        }

        return true;
    }
 

bool AnimatedMesh::LoadAnimation(const char *szFileName)
{

    BinaryFile file;
    if (!file.open(szFileName)) return false;

    file.readChar();
    file.readChar();
    file.readChar();
    file.readChar();

    u32 version = file.readInt();

    if (version != 2024)
    {
        LogError("Unsupported version %d", version);
        return false;
    }
    
    int len = file.readInt();
    std::string name;
    name.resize(len);
    file.readBytes(&name[0], len);
    
    MeshAnimationFrames* animation = addAnimation(name);
    

    animation->duration = file.readDouble();
    animation->fps = file.readDouble();

    LogInfo("Loading animation (%s) %f %f", animation->name.c_str(), animation->fps, animation->duration);

    u32  countFrames = file.readInt();

    for (u32 i = 0; i < countFrames; i++)
    {
        int size = file.readInt();
        std::string name;
        name.resize(size);
        file.readBytes(&name[0], size);

        u32 positions = file.readInt();
        u32 rotations = file.readInt();

        Frame* frame = animation->AddFrame(name);
        
        for (u32 j = 0; j < positions; j++)
        {
            float time = file.readFloat();
            Vector3 pos;
            pos.x = file.readFloat();
            pos.y = file.readFloat();
            pos.z = file.readFloat();
            frame->keys.AddPositionKeyFrame(time, pos);
        }
        for (u32 j = 0; j < rotations; j++)
        {
            float time = file.readFloat();
            Quaternion rot;
            rot.x = file.readFloat();
            rot.y = file.readFloat();
            rot.z = file.readFloat();
            rot.w = file.readFloat();
            frame->keys.AddRotationKeyFrame(time, QuaternionNormalize(rot));
        }
        
    }

    

    return true;
}


static void DrawTria3D(Vector3 v1, Vector3 v2, Vector3 v3,Vector2 uv1,Vector2 uv2,Vector2 uv3, Color color)
{
    //rlCheckRenderBatchLimit(4);

   // rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlTexCoord2f(uv1.x, uv1.y);
        rlVertex3f(v1.x, v1.y, v1.z);

        rlTexCoord2f(uv2.x,uv2.y);
        rlVertex3f(v2.x, v2.y, v2.z);

        rlTexCoord2f(uv3.x,uv3.y);
        rlVertex3f(v3.x, v3.y, v3.z);


        rlTexCoord2f(uv1.x,uv1.y);
        rlVertex3f(v1.x, v1.y, v1.z);

  //  rlEnd();
}

void AnimatedMesh::Debug() 
{

    for (u32 i = 0; i < surfaces.size(); i++)
    {

        SkinSurface *surf = &surfaces[i];


 
       
      //  rlBegin(RL_QUADS);
        rlBegin(RL_LINES);
       
        for (u32 j = 0; j < surf->faces.size(); j+=3)
        {
            int i0 =  surf->faces[j + 0];
            int i1 =  surf->faces[j + 1];
            int i2 =  surf->faces[j + 2];

            SkinVertex v0 = surf->vertices[i0];
            SkinVertex v1 = surf->vertices[i1];
            SkinVertex v2 = surf->vertices[i2];
            DrawTria3D(v0.pos,v1.pos,v2.pos,v0.uv,v1.uv,v2.uv,Color{255,255,255,255});

            // Vector3 v0 = vertex[i0];
            // Vector3 v1 = vertex[i1];
            // Vector3 v2 = vertex[i2];
            // Vector2 uv0 = texCoords[i0];
            // Vector2 uv1 = texCoords[i1];
            // Vector2 uv2 = texCoords[i2];
            // DrawTria3D(v0,v1,v2,uv0,uv1,uv2,Color{255,255,255,255});
            

        }

        rlEnd();
 
        
     
    }

    // for (u32 i = 0; i < joints.size(); i++)
    // {
    //     DrawCube(joints[i].position, 0.1f, 0.1f, 0.1f, Color{255,0,0,255});
    // }

}

MeshAnimationFrames *AnimatedMesh::addAnimation(const std::string &name)
{
    MeshAnimationFrames *a = new MeshAnimationFrames();
    a->name = name;
    animations.push_back(a);
    animationsMap[name] = a;
    return a;
}


void Entity::Update(float dt)
{
    animator->Update(dt);
}

void Entity::SetMaterial(int surface, u32 index)
{
    if (surface==-1)
    {
        for (u32 i = 0; i < surfaces.size(); i++)
        {
            surfaces[i].material = index;
        }
        return;
    }
    if (surface<0 || surface > (int)surfaces.size())
    {
            LogWarning("Index out of range");
            return;
    }
    surfaces[surface].material = index;
}

void Entity::AddMaterial(Texture2D layer) 
{
    materials.push_back(layer);
}   


Entity::Entity(AnimatedMesh *mesh) 
{
    animator = new Animator(this);

    for (u32 i = 0; i < mesh->joints.size(); i++)
    {
        Node3D *b = new Node3D();
        b->name = mesh->joints[i].name;
        b->localPosition = mesh->joints[i].position;
        b->localRotation = mesh->joints[i].orientation;
        b->localScale = mesh->joints[i].scale;
        b->UpdateLocalTransform();
        joints.push_back(b);
        
        offset.push_back(mesh->joints[i].offset);
        for (u32 j = 0; j < joints.size(); j++)
        {
            if (joints[j]->name == mesh->joints[i].parentName)
            {
                joints[i]->parent = joints[j];
                break;
            }
        }
    }

    for (u32 i = 0; i < mesh->joints.size(); i++)
    {
       if (!joints[i]->parent)
       {
           joints[i]->parent = this;
       }
    }

    //materials = mesh->materials;

    for (u32 i = 0; i < mesh->surfaces.size(); i++)
    {
            SkinSurface *surf = &mesh->surfaces[i];
            surfaces.push_back(SkinSurface());
            SkinSurface *s = &surfaces.back();
            s->vertices = surf->vertices;
            s->faces = surf->faces;
            s->skinnedVertices = surf->skinnedVertices;
            s->material = surf->material;
            s->name = surf->name;
            s->Init();
        


    }

    for (u32 i = 0; i < mesh->animations.size(); i++)
    {
        const MeshAnimationFrames *anim = mesh->animations[i];
        Animation *animation = animator->AddAnimation(anim->name);
        animation->duration = anim->duration;
        animation->fps = anim->fps;

        LogInfo("Add Animation '%s'",anim->name.c_str());

        for (u32 j = 0; j < anim->frames.size(); j++)
        {
           Frame* frame = animation->GetFrame(anim->frames[j]->name);
               frame->keys= anim->frames[j]->keys;
          
        }
        
        
    }
    

}


Vector3 ApplySkinning(SkinVertex vertex, const std::vector<Node3D*>& joints, const std::vector<Matrix>& offset)
{
    Vector3 result = {0};

    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        int boneIndex = vertex.bones[i];
        float weight = vertex.weights[i];

        if (boneIndex == -1 || weight <= 0.0f)
            continue;

        Node3D* bone = joints[boneIndex];
        Matrix skinMatrix = MatrixMultiply(offset[boneIndex], bone->GetWorldMatrix());

        Vector3 transformed = Vector3Transform(vertex.pos, skinMatrix);
        result = Vector3Add(result, Vector3Scale(transformed, weight));
    }

    return result;
}

void Entity::Render(Shader& shader,int surface)
{
     const Matrix mat = GetWorldMatrix();
    int locIndex = GetShaderLocation(shader, "model");
    rlSetUniformMatrix(locIndex, mat);
    rlActiveTextureSlot(0);
    

    float finalBones[MAX_BONES][16];
    for (u32 i = 0; i < joints.size(); i++)
    {
        Node3D *b = joints[i];
        Matrix mat = MatrixMultiply(offset[i], b->GetWorldMatrix());
        finalBones[i][0] = mat.m0;
        finalBones[i][1] = mat.m1;
        finalBones[i][2] = mat.m2;
        finalBones[i][3] = mat.m3;
        finalBones[i][4] = mat.m4;
        finalBones[i][5] = mat.m5;
        finalBones[i][6] = mat.m6;
        finalBones[i][7] = mat.m7;
        finalBones[i][8] = mat.m8;
        finalBones[i][9] = mat.m9;
        finalBones[i][10] = mat.m10;
        finalBones[i][11] = mat.m11;
        finalBones[i][12] = mat.m12;
        finalBones[i][13] = mat.m13;
        finalBones[i][14] = mat.m14;
        finalBones[i][15] = mat.m15;
    }

    int jointsLoc = GetShaderLocation(shader, "Joints");
    glUniformMatrix4fv(jointsLoc, MAX_BONES, GL_FALSE, (float*)finalBones);


    // for (u32 i = 0; i < bones.size(); i++)
    // {
    
    //     Node3D *b = joints[i];
    //     bones[i] = MatrixMultiply( offset[i],b->GetWorldMatrix());
    //     int locIndex = GetShaderLocation(shader, TextFormat("Joints[%d]", i));
    //     rlSetUniformMatrix(locIndex, bones[i]);
       
    // }


        if (surface == -1) surface = 0;
        if (surface > (int)surfaces.size()) surface = 0;
        SkinSurface *surf = &surfaces[surface];
 
        if (surf->material != -1)
        {
            rlEnableTexture(materials[surf->material].id);
        }
        surf->Render();
    


    for (auto & child : children)
    {
        child->Render();
    }
}

void Entity::Render(Shader &shader) 
{
    const Matrix mat = GetWorldMatrix();
    int locIndex = GetShaderLocation(shader, "model");
    rlSetUniformMatrix(locIndex, mat);
    rlActiveTextureSlot(0);
    

    float finalBones[MAX_BONES][16];
    for (u32 i = 0; i < joints.size(); i++)
    {
        Node3D *b = joints[i];
        Matrix mat = MatrixMultiply(offset[i], b->GetWorldMatrix());
        finalBones[i][0] = mat.m0;
        finalBones[i][1] = mat.m1;
        finalBones[i][2] = mat.m2;
        finalBones[i][3] = mat.m3;
        finalBones[i][4] = mat.m4;
        finalBones[i][5] = mat.m5;
        finalBones[i][6] = mat.m6;
        finalBones[i][7] = mat.m7;
        finalBones[i][8] = mat.m8;
        finalBones[i][9] = mat.m9;
        finalBones[i][10] = mat.m10;
        finalBones[i][11] = mat.m11;
        finalBones[i][12] = mat.m12;
        finalBones[i][13] = mat.m13;
        finalBones[i][14] = mat.m14;
        finalBones[i][15] = mat.m15;

   
        
        // rlPushMatrix();
        // rlMultMatrixf(&finalBones[i][0]);
        // DrawCube({0,0,0}, 1.0f, 1.1f, 1.1f, Color{255,0,0,255});
        // rlPopMatrix();
   
    }

    int jointsLoc = GetShaderLocation(shader, "Joints");
    glUniformMatrix4fv(jointsLoc, MAX_BONES, GL_FALSE, (float*)finalBones);


    // for (u32 i = 0; i < bones.size(); i++)
    // {
    
    //     Node3D *b = joints[i];
    //     bones[i] = MatrixMultiply( offset[i],b->GetWorldMatrix());
    //     int locIndex = GetShaderLocation(shader, TextFormat("Joints[%d]", i));
    //     rlSetUniformMatrix(locIndex, bones[i]);
       
    // }


    for (u32 i = 0; i < surfaces.size(); i++)
    {
        SkinSurface *surf = &surfaces[i];
 
        if (surf->material != -1 && surf->material < (int)materials.size())
        {
            rlEnableTexture(materials[surf->material].id);
        }
        surf->Render();
    }


    for (auto & child : children)
    {
        child->Render();
    }

}



void Entity::Render()
{
    const Matrix mat = GetWorldMatrix();
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(mat));
    

     for (u32 i = 0; i < surfaces.size(); i++)
    {

        SkinSurface *surf = &surfaces[i];


      

        for (size_t i = 0; i < surf->vertices.size(); ++i)
        {
           Vector3 pos = surf->vertices[i].pos;
           Vector3 result = {0};
            
            if (surf->vertices[i].bones[0] != -1)
            {
                 
                float weight = surf->vertices[i].weights[0];
                int boneIndex = surf->vertices[i].bones[0];

                Node3D* bone = joints[boneIndex];
                Matrix skinMatrix = MatrixMultiply(offset[boneIndex], bone->GetWorldMatrix());

                Vector3 transformed = Vector3Transform(pos, skinMatrix);
                result = Vector3Add(result, Vector3Scale(transformed, weight));

                if (surf->vertices[i].bones[1] != -1)
                {
                    weight = surf->vertices[i].weights[1];
                    boneIndex = surf->vertices[i].bones[1];
                    bone = joints[boneIndex];
                    skinMatrix = MatrixMultiply(offset[boneIndex], bone->GetWorldMatrix());

                    transformed = Vector3Transform(pos, skinMatrix);
                    result = Vector3Add(result, Vector3Scale(transformed, weight));

                    if (surf->vertices[i].bones[2] != -1)
                    {
                        weight = surf->vertices[i].weights[2];
                        boneIndex = surf->vertices[i].bones[2];
                        bone = joints[boneIndex];
                        skinMatrix = MatrixMultiply(offset[boneIndex], bone->GetWorldMatrix());

                        transformed = Vector3Transform(pos, skinMatrix);
                        result = Vector3Add(result, Vector3Scale(transformed, weight));

                        if (surf->vertices[i].bones[3] != -1)
                        {
                            weight = surf->vertices[i].weights[3];
                            boneIndex = surf->vertices[i].bones[3];
                            bone = joints[boneIndex];
                            skinMatrix = MatrixMultiply(offset[boneIndex], bone->GetWorldMatrix());

                            transformed = Vector3Transform(pos, skinMatrix);
                            result = Vector3Add(result, Vector3Scale(transformed, weight));
                        }
                    }

                }

                surf->skinnedVertices[i] = result;

            }

        }
       
      //  rlBegin(RL_QUADS);
        rlBegin(RL_LINES);
       
        for (u32 j = 0; j < surf->faces.size(); j+=3)
        {
            int i0 =  surf->faces[j + 0];
            int i1 =  surf->faces[j + 1];
            int i2 =  surf->faces[j + 2];




            Vector3 p0 = surf->skinnedVertices[i0];
            Vector3 p1 = surf->skinnedVertices[i1];
            Vector3 p2 = surf->skinnedVertices[i2];

            // DrawTria3D(p0, p1, p2,
            //         surf->vertices[i0].uv,
            //         surf->vertices[i1].uv,
            //         surf->vertices[i2].uv,
            //         WHITE);

             SkinVertex v0 = surf->vertices[i0];
             SkinVertex v1 = surf->vertices[i1];
             SkinVertex v2 = surf->vertices[i2];

            // v0.pos = ApplySkinning(surf->vertices[i0], joints, offset);
            // v1.pos = ApplySkinning(surf->vertices[i1], joints, offset);
            // v2.pos = ApplySkinning(surf->vertices[i2], joints, offset);


            DrawTria3D(p0,p1,p2,v0.uv,v1.uv,v2.uv,Color{255,255,255,255});
           

        }

       rlEnd();
 
        
     
    }

    rlPopMatrix();
    
    // for (u32 i = 0; i < joints.size(); i++)
    // {
    //     const Matrix mat = joints[i]->GetWorldMatrix();
    //     rlPushMatrix();
    //     rlMultMatrixf(MatrixToFloat(mat));
    //     DrawCube({0,0,0}, 5.0f, 5.1f, 5.1f, Color{255,0,0,255});
    //     rlPopMatrix();
    // }

 

    for (auto & child : children)
    {
        child->Render();
    }

}

Entity::~Entity() 
{
    for (u32 i = 0; i < joints.size(); i++)
    {
        delete joints[i];
    }
    joints.clear();

    for (u32 i = 0;i<surfaces.size();i++) 
    {
       surfaces[i].Release();
    }
    surfaces.clear();
    delete animator;
    animator = nullptr;
}

Frame *MeshAnimationFrames::GetFrame(const std::string &name)
{
    if (framesMap.find(name) == framesMap.end())
    {
        LogWarning("Frame not found: %s", name.c_str());
        return nullptr;
    }
    return framesMap[name];
}

Frame *MeshAnimationFrames::AddFrame(const std::string& name)
{
    Frame *frame = new Frame();
    frame->name = name;
    frames.push_back(frame);
    framesMap[name] = frame;
    return frame;
}