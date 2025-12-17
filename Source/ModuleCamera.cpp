#include "Globals.h"

#include "ModuleCamera.h"
#include "Application.h"

#include "Mouse.h"
#include "Keyboard.h"

#define FAR_PLANE 2000.0f
#define NEAR_PLANE 0.1f

namespace
{
    constexpr float getRotationSpeed() { return 25.0f; }
    constexpr float getTranslationSpeed() { return 2.5f; }
}

bool ModuleCamera::init()
{
    position = Vector3(0.0f, 0.0f, 10.0f);
    rotation = Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), XMConvertToRadians(0.0f));

    Quaternion invRot;
    rotation.Inverse(invRot);

    view = Matrix::CreateFromQuaternion(invRot);
    view.Translation(-position);

    view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));

    return true;
}

void ModuleCamera::update()
{
    Mouse& mouse = Mouse::Get();
    const Mouse::State& mouseState = mouse.GetState();

    if (enabled)
    {
        Keyboard& keyboard = Keyboard::Get();

        const Keyboard::State& keyState = keyboard.GetState();

        float elapsedSec = app->getElapsedMilis() * 0.005f;
        if (keyState.LeftShift) elapsedSec = app->getElapsedMilis() * 0.01f;

        int mouseScrollWheel = mouseState.scrollWheelValue;

        Vector3 translate = Vector3::Zero;
        Vector2 rotate = Vector2::Zero;

        if (mouseState.rightButton)
        {
            rotate.x = float(dragPosX - mouseState.x) * 0.005f;
            rotate.y = float(dragPosY - mouseState.y) * 0.005f;

            if (keyState.W) translate.z -= 0.45f * elapsedSec;
            if (keyState.S) translate.z += 0.45f * elapsedSec;
            if (keyState.A) translate.x -= 0.45f * elapsedSec;
            if (keyState.D) translate.x += 0.45f * elapsedSec;
            if (keyState.Q) translate.y -= 0.45f * elapsedSec;
            if (keyState.E) translate.y += 0.45f * elapsedSec;
        }

        if (keyState.LeftAlt)
        {
            if (mouseState.leftButton)
            {
                translate.x = float(dragPosX - mouseState.x) * 0.005f;
                translate.y = float(dragPosY - mouseState.y) * 0.005f;
                rotate.x = float(dragPosX - mouseState.x) * 0.005f;
                rotate.y = float(dragPosY - mouseState.y) * 0.005f;
            }
        }

        if (mouseScrollWheel > prevWheel) translate.z -= 1.0f;
        if (mouseScrollWheel < prevWheel) translate.z += 1.0f;

        prevWheel = mouseScrollWheel;

        Vector3 localDir = Vector3::Transform(translate, rotation);
        params.worldSpacePosition += localDir * getTranslationSpeed();
        params.yaw += XMConvertToRadians(getRotationSpeed() * rotate.x);
        params.pitch += XMConvertToRadians(getRotationSpeed() * rotate.y);


        Quaternion rotation_yaw = Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), params.yaw);
        Quaternion rotation_pitch = Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), params.pitch);
        rotation = rotation_pitch * rotation_yaw;
        position = params.worldSpacePosition;

        Quaternion invRot;
        rotation.Inverse(invRot);

        view = Matrix::CreateFromQuaternion(invRot);
        view.Translation(Vector3::Transform(-position, invRot));

    }

    dragPosX = mouseState.x;
    dragPosY = mouseState.y;
}

Matrix ModuleCamera::getPerspectiveProjection(float aspect)
{
    return Matrix::CreatePerspectiveFieldOfView(XM_PIDIV4, aspect, NEAR_PLANE, FAR_PLANE);
}