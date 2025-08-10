#include "pch.h" 
#include "node.hpp"
#include "entity.hpp"
 


static u32 s_nextID = 1;

Node3D::Node3D()
{
    ID = s_nextID++;
    local = MatrixIdentity();
}

Node3D::Node3D(Vector3 position)
{
    ID = s_nextID++;
    localPosition = position;
    local = GetLocalMatrix();
}

Node3D::Node3D(Vector3 position, Vector3 rotationEuler)
{
    ID = s_nextID++;
    localPosition = position;
    localRotation = QuaternionFromEuler(rotationEuler.x * DEG2RAD, rotationEuler.y * DEG2RAD, rotationEuler.z * DEG2RAD);
    local = GetLocalMatrix();
}

Node3D::~Node3D()
{
    for (auto* child : children)
    {
        child->parent = nullptr;
    }
    children.clear();
}

void Node3D::AddChild(Node3D* child)
{
    if (!child) return;
    if (child->parent == this) return;

    child->SetParent(this);
}

void Node3D::RemoveChild(Node3D* child)
{
    if (!child) return;
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
    child->parent = nullptr;
}

void Node3D::SetParent(Node3D* newParent)
{
    if (parent)
    {
        parent->RemoveChild(this);
    }
    parent = newParent;
    if (newParent)
    {
        newParent->children.push_back(this);
    }
    UpdateWorldTransform();
}

void Node3D::SetLocalMatrix(Matrix matrix)
{
    local = matrix;

    // // Decompor matriz em posição, rotação e escala
    // localPosition = { matrix.m12, matrix.m13, matrix.m14 };
    // localScale = {
    //     Vector3Length({ matrix.m0, matrix.m1, matrix.m2 }),
    //     Vector3Length({ matrix.m4, matrix.m5, matrix.m6 }),
    //     Vector3Length({ matrix.m8, matrix.m9, matrix.m10 })
    // };

    // Matrix rot = matrix;
    // rot.m12 = rot.m13 = rot.m14 = 0;
    // rot = MatrixMultiply(rot, MatrixScale(1.0f / localScale.x, 1.0f / localScale.y, 1.0f / localScale.z));
    // localRotation = QuaternionFromMatrix(rot);

    UpdateWorldTransform();
}

void Node3D::SetLocalPosition(Vector3 position)
{
    localPosition = position;
    UpdateWorldTransform();
}

void Node3D::SetLocalRotation(Quaternion rotation)
{
    localRotation = rotation;
    UpdateWorldTransform();
}

void Node3D::SetLocalRotationEuler(Vector3 eulerAngles)
{
    localRotation = QuaternionFromEuler(eulerAngles.x * DEG2RAD, eulerAngles.y * DEG2RAD, eulerAngles.z * DEG2RAD);
    UpdateWorldTransform();
}

void Node3D::SetLocalScale(Vector3 scale)
{
    localScale = scale;
    UpdateWorldTransform();
}

void Node3D::Translate(Vector3 translation)
{
    localPosition = Vector3Add(localPosition, translation);
    UpdateWorldTransform();
}

void Node3D::Rotate(Vector3 eulerRotation)
{
    Quaternion delta = QuaternionFromEuler(eulerRotation.x * DEG2RAD, eulerRotation.y * DEG2RAD, eulerRotation.z * DEG2RAD);
    localRotation = QuaternionMultiply(localRotation, delta);
    UpdateLocalTransform();
    UpdateWorldTransform();
}

void Node3D::Scale(Vector3 scale)
{
    localScale.x *= scale.x;
    localScale.y *= scale.y;
    localScale.z *= scale.z;
    UpdateWorldTransform();
}

void Node3D::SetWorldPosition(Vector3 position)
{
    if (parent)
    {
        Matrix invParent = parent->GetInverseWorldMatrix();
        localPosition = Vector3Transform(position, invParent);
    }
    else
    {
        localPosition = position;
    }
    UpdateWorldTransform();
}

void Node3D::SetWorldRotation(Quaternion rotation)
{
    if (parent)
    {
        Quaternion invParent = QuaternionInvert(parent->worldRotation);
        localRotation = QuaternionMultiply(invParent, rotation);
    }
    else
    {
        localRotation = rotation;
    }
    UpdateWorldTransform();
}

Vector3 Node3D::GetForward() const
{
    return Vector3Transform({ 0, 0, -1 }, GetWorldMatrix());
}

Vector3 Node3D::GetRight() const
{
    return Vector3Transform({ 1, 0, 0 }, GetWorldMatrix());
}

Vector3 Node3D::GetUp() const
{
    return Vector3Transform({ 0, 1, 0 }, GetWorldMatrix());
}

Matrix Node3D::GetLocalMatrix() const { return local; }

Matrix Node3D::GetWorldMatrix() const
{
    if (parent)
    {
        return MatrixMultiply(GetLocalMatrix(), parent->GetWorldMatrix());
    }
    return GetLocalMatrix();
}

Matrix Node3D::GetInverseWorldMatrix() const
{
    return MatrixInvert(GetWorldMatrix());
}

void Node3D::Update(float dt)
{
    // Este Update é chamado por cena ou manualmente
    UpdateWorldTransform();
    for (auto* child : children)
    {
        child->Update(dt);
    }
}

void Node3D::UpdateLocalTransform() 
{
    local= MatrixMultiply(
        MatrixMultiply(MatrixScale(localScale.x, localScale.y, localScale.z), 
        QuaternionToMatrix(localRotation)),
        MatrixTranslate(localPosition.x, localPosition.y, localPosition.z));

}

void Node3D::UpdateWorldTransform()
{
    Matrix world = GetWorldMatrix();

    worldPosition = { world.m12, world.m13, world.m14 };

    worldScale = {
        Vector3Length({ world.m0, world.m1, world.m2 }),
        Vector3Length({ world.m4, world.m5, world.m6 }),
        Vector3Length({ world.m8, world.m9, world.m10 })
    };

    Matrix rot = world;
    rot.m12 = rot.m13 = rot.m14 = 0;
    rot = MatrixMultiply(rot, MatrixScale(1.0f / worldScale.x, 1.0f / worldScale.y, 1.0f / worldScale.z));
    worldRotation = QuaternionFromMatrix(rot);

    UpdateChildrenWorldTransform();
}

void Node3D::UpdateChildrenWorldTransform()
{
    for (auto* child : children)
    {
        child->UpdateWorldTransform();
    }
}

