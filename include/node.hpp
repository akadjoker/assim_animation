#pragma once
#include "Config.hpp"

class LoadMD3;

class Entity;


class Node3D 
{
public:
    std::string name { "Node3D" };
    int tag = 0;
    void* userData = nullptr;

    // Transformação local
    Vector3 localPosition = { 0.0f, 0.0f, 0.0f };
    Quaternion localRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    Vector3 localScale = { 1.0f, 1.0f, 1.0f };

    // Transformação mundial
    Vector3 worldPosition = { 0.0f, 0.0f, 0.0f };
    Quaternion worldRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    Vector3 worldScale = { 1.0f, 1.0f, 1.0f };

    // Hierarquia
    Node3D* parent = nullptr;

    Node3D();
    Node3D(Vector3 position);
    Node3D(Vector3 position, Vector3 rotationEuler);
    virtual ~Node3D();

    void AddChild(Node3D* child);
    void RemoveChild(Node3D* child);
    void SetParent(Node3D* newParent);

    void SetLocalMatrix(Matrix matrix);
    void SetLocalPosition(Vector3 position);
    void SetLocalRotation(Quaternion rotation);
    void SetLocalRotationEuler(Vector3 eulerAngles);
    void SetLocalScale(Vector3 scale);
    void Translate(Vector3 translation);
    void Rotate(Vector3 eulerRotation);
    void Scale(Vector3 scale);

    virtual void Render() {}
    virtual void Update(float dt);

    void SetWorldPosition(Vector3 position);
    void SetWorldRotation(Quaternion rotation);

    Vector3 GetWorldPosition() const { return worldPosition; }
    Quaternion GetWorldRotation() const { return worldRotation; }
    Vector3 GetWorldScale() const { return worldScale; }

    Vector3 GetForward() const;
    Vector3 GetRight() const;
    Vector3 GetUp() const;

    Matrix GetLocalMatrix() const;
    Matrix GetWorldMatrix() const;
    Matrix GetInverseWorldMatrix() const;

    u32 GetID() const { return ID; }

private:
friend class Scene;
friend class Animator;
friend class Entity;
Matrix local;
u32 ID = 0;
protected:
    std::vector<Node3D*> children;
    void UpdateLocalTransform();
    void UpdateWorldTransform();
    void UpdateChildrenWorldTransform();
};
