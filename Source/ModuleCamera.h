#pragma once

#include "Module.h"

class ModuleCamera : public Module
{
    struct Params
    {
        float yaw;
        float pitch;
        Vector3 worldSpacePosition;
    };

    Params params = { 0.0f, 0.0f, {0.0f, 2.0f, 10.0f} };
    Params tempParams = { 0.0f, 0.0f, {0.0f, 0.0f, 0.0f} };
    int dragPosX = 0;
    int dragPosY = 0;

    Quaternion rotation;
    Vector3 position;
    Matrix view;
    bool enabled = true;
    int prevWheel = 0;

public:

    bool init() override;
    void update() override;

    bool getEnabled() const { return enabled; }
    void setEnabled(bool isEnabled) { enabled = isEnabled; }

    float getYaw() const { return params.yaw; }
    float getPitch() const { return params.pitch; }
    const Vector3& getWorldSpacePosition() const { return params.worldSpacePosition; }

    void setYaw(float yaw) { params.yaw = yaw; }
    void setPitch(float pitch) { params.pitch = pitch; }
    void setWorldSpacePosition(const Vector3& wsp) { params.worldSpacePosition = wsp; }

    const Matrix& getView() const { return view; }
    const Quaternion& getRotation() const { return rotation; }
    const Vector3& getPosition() const { return position; }

    static Matrix getPerspectiveProjection(float aspect);
};
