#pragma once
#include "file.hpp"
#include "node.hpp"

#include <assimp/Importer.hpp> 
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/vector3.h>
#include <assimp/matrix3x3.h>
#include <assimp/matrix4x4.h>

const u8 MAX_BONES = 180;//for mixamo
const u8 MAX_BONE_INFLUENCE = 4;

class Entity;


struct SkinVertex
{
    Vector3 pos;
    Vector3 normal;
    Vector2 uv;
    int bones[MAX_BONE_INFLUENCE];
    float weights[MAX_BONE_INFLUENCE];
};

struct PosKeyFrame
{
    Vector3 pos;
    float frame;

    PosKeyFrame(const Vector3 &p, float f)
    {
        pos = p;
        frame = f;
    }
};

struct RotKeyFrame
{
    Quaternion rot;
    float frame;
    RotKeyFrame(const Quaternion &r, float f)
    {
        rot = r;
        frame = f;
    }
};



struct KeyFrame
{

    std::vector<PosKeyFrame> positionKeyFrames;
    std::vector<RotKeyFrame> rotationKeyFrames;

    KeyFrame() {}
    KeyFrame(KeyFrame *t);

    ~KeyFrame() {}

    int GetPositionIndex(float animationTime);
    int GetRotationIndex(float animationTime);
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    Quaternion AnimateRotation(float movetime);
    Vector3 AnimatePosition(float movetime);
    u32 numRotationKeys() const { return rotationKeyFrames.size(); }
    u32 numPositionKeys() const { return positionKeyFrames.size(); }

    void setPositionKey(int frame, const Vector3 &p)
    {
        positionKeyFrames[frame].pos = p;
    }

    void setRotationKey(int frame, const Quaternion &q)
    {
        rotationKeyFrames[frame].rot = q;
    }
    void AddPositionKeyFrame(float frame, const Vector3 &pos)
    {
        positionKeyFrames.push_back(PosKeyFrame(pos, frame));
    }

    void AddRotationKeyFrame(float frame, const Quaternion &rot)
    {

        rotationKeyFrames.push_back(RotKeyFrame(rot, frame));
    }
};


struct Frame
{
    std::string name;
    Vector3 position;
    Quaternion orientation;
    KeyFrame keys;
    Vector3 src_pos, dest_pos;
    Quaternion src_rot, dest_rot;
    bool pos{true};
    bool rot {true};
    bool IgnorePosition {false};
    bool IgnoreRotation {false};

    
};




struct  SkinSurface
{
    std::string name;
    std::vector<unsigned short> faces;
    std::vector<SkinVertex> vertices;
    std::vector<Vector3> skinnedVertices;
    int material;
    u32 vaoId{ 0 };
    u32 vboId { 0 };
    u32 iboId { 0 };
    u32 vertexCount { 0 };
    u32 numTriangles{ 0 }; 


    const Vector3 GetVertex(int id) { return vertices[id].pos; }
    void SetVertex(int id, const Vector3 &v) { vertices[id].pos = v; }
    void SetVertex(int id, float x, float y, float z) { vertices[id].pos={x,y,z}; }


     void AddVertex(const SkinVertex &vertex)
    {
        vertices.push_back(vertex);
        skinnedVertices.push_back(vertex.pos);
   
    }

    void AddVertex(const Vector3 &v)
    {
        SkinVertex vertex;
        vertex.pos = v;

        vertex.weights[0] = 0.0f;
        vertex.bones[0] = -1;

        vertex.weights[1] = 0.0f;
        vertex.bones[1] = -1;

        vertex.weights[2] = 1.0f;
        vertex.bones[2] = -1;

        vertex.weights[3] = 0.0f;
        vertex.bones[3] = -1;

        vertices.push_back(vertex);
   
    }

    void AddVertex(const Vector3 &v, const Vector2 &uv)
    {
        SkinVertex vertex;
        vertex.pos = v;
        vertex.uv = uv;

        vertex.weights[0] = 0.0f;
        vertex.bones[0] = -1;

        vertex.weights[1] = 0.0f;
        vertex.bones[1] = -1;

        vertex.weights[2] = 1.0f;
        vertex.bones[2] = -1;

        vertex.weights[3] = 0.0f;
        vertex.bones[3] = -1;

        vertices.push_back(vertex);

    }

     void AddVertex(const Vector3 &v, const Vector2 &uv, const Vector3 &normal)
    {
        SkinVertex vertex;
        vertex.pos = v;
        vertex.uv = uv;
        vertex.normal = normal;

        vertex.weights[0] = 0.0f;
        vertex.bones[0] = -1;

        vertex.weights[1] = 0.0f;
        vertex.bones[1] = -1;

        vertex.weights[2] = 1.0f;
        vertex.bones[2] = -1;

        vertex.weights[3] = 0.0f;
        vertex.bones[3] = -1;

        vertices.push_back(vertex);

    }

    void AddFace(int a, int b, int c)
    {
        faces.push_back(a);
        faces.push_back(b);
        faces.push_back(c);
    }

 

    void Init();
    void Release();
    void Render();
};


struct MeshJoint
{
    std::string name;
    Vector3 position;
    Quaternion orientation;
    Vector3 scale;
    std::string parentName;
    Matrix offset;
};

struct MeshAnimationFrames
{
    std::string name;
    double duration;
    double fps;
    std::vector<Frame*> frames;
    std::map<std::string, Frame*> framesMap;
    ~MeshAnimationFrames()
    {
        for (size_t i = 0; i < frames.size(); i++)
        {
            delete frames[i];
        }
        framesMap.clear();
        frames.clear();
    }
    Frame *AddFrame(const std::string& name);
    Frame *GetFrame(const std::string& name);
};

class AnimatedMesh
{

    std::vector<MeshJoint> joints;
    std::vector<SkinSurface> surfaces;
    std::vector<std::string> materials;

    std::map<std::string, MeshAnimationFrames*> animationsMap;
    std::vector<MeshAnimationFrames*> animations;
    friend class Entity;

    void ReadNodeHeirarchy(const aiNode *pNode);
    int GetJointIndex(const char *name);

public:
    AnimatedMesh();
    ~AnimatedMesh();
    bool Load(const char *szFileName);
    bool LoadAnimation(const char *szFileName);

    bool ImportAnimation(const char *fileName,
                                       const std::string &name);
    bool Import(const std::string &name);

    void SaveAnimation(const std::string &name, const std::string &fileName);
    void Save(const std::string &name);

    void Debug();

    MeshAnimationFrames * addAnimation(const std::string &name);

};





class  Animation
{
private:
    u64 n_frames;
    u64 state;
    u64 method;
    float currentTime;
    float duration;
    float fps;
    int mode;
    bool isEnd;
    std::vector<Frame *> frames;
    std::map<std::string, Frame *> framesMap;
    friend class Entity;

public:
    enum
    {
        LOOP = 0,
        PINGPONG = 1,
        ONESHOT = 2
    };
    enum
    {
        Stoped = 0,
        Looping = 1,
        Playing = 2
    };
    std::string name;

    Animation(const std::string &name);
    ~Animation();

    void Force();
 

    float GetDuration();
    float GetTime();
    float GetFPS();
    int GetMode();
    u64 GetState();

    std::string GetName() const { return name; }

    bool Play(u32 mode, float fps);
    bool Stop();
    bool IsEnded();

    Frame *AddFrame(const std::string& name);
    Frame *GetFrame(const std::string& name);
    Frame *GetFrame(int index);

    void Update(float elapsed);
};

class  Animator
{
public:
    Animator(Entity *parent);
    ~Animator();

    void Update(float elapsed);
 

 

    Animation *AddAnimation(const std::string &name);

    Animation *GetAnimation(const std::string &name);

    Animation *GetAnimation(int index);

    u32 numAnimations() const
    {
        return m_animations.size();
    }

    void SetIgnorePosition(const std::string &name, bool ignore);
    void SetIgnoreRotation(const std::string &name, bool ignore);

    bool Play(const std::string &name, int mode = Animation::LOOP, float blendTime = 0.25f);

    void Stop();
    bool IsEnded();
    bool IsPlaying();

    std::string GetCurrentAnimationName()
    {
        std::string s("");
        if (!currentAnimation)
            return s;
        return currentAnimation->name;
    }

    float GetCurrentFrame()
    {
        if (!currentAnimation)
            return 0.0f;
        return currentAnimation->GetTime();
    }

     bool PlayWithNext(const std::string& currentAnim, const std::string& nextAnim, 
                      int currentMode = Animation::ONESHOT, int nextMode = Animation::LOOP,
                      float blendTime = 0.3f);
    
 
    void SetDefaultAnimation(const std::string& defaultAnim, int mode = Animation::LOOP);
    
 
    void CheckForNextAnimation();

private:
    std::vector<Animation *> m_animations;
    std::map<std::string, Animation *> m_animations_map;
    Entity *entity;

    Animation *currentAnimation;

    std::string currentAnimationName;

    float blendFactor;
    float blendTime;
    bool blending;

     std::string nextAnimationName;     
    int nextAnimationMode;            
    float nextBlendTime;             
    bool hasNextAnimation;         

    void beginTrans();
    void updateTrans(float blend);
    void updateAnim(float elapsed);
};

class Entity: public Node3D
{
    public:
    std::vector<Node3D*>      joints;
    std::vector<SkinSurface>  surfaces;
    std::vector<Texture2D>    materials;
    std::vector<Matrix>       offset;
    Animator *animator;
    
    public:
         Entity(AnimatedMesh *mesh);
         ~Entity();
         void Render();
         void Render(Shader& sahder);
         void Render(Shader& sahder,int surface);
         void Update(float dt);

         void AddMaterial(Texture2D texture);

         void SetMaterial(int surface, u32 index);

         Animator *GetAnimator() { return animator; }
};
