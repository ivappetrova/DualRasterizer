#include "Camera.h"
#include "Timer.h"
#include "MathHelpers.h"
#include <iostream>

using namespace dae;

Camera::Camera(const Vector3& origin, float fovAngle)
	: m_Origin(origin)
	, m_FovAngle(fovAngle)
{
}

void Camera::Initialize(const Vector3& origin, float fovAngle, float nearPlane, float farPlane)
{
	m_Origin = origin;
	m_FovAngle = fovAngle;
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;

	m_Forward = Vector3::UnitZ;
	m_Up = Vector3::UnitY;
	m_Right = Vector3::UnitX;

	m_TotalPitch = 0.0f;
	m_TotalYaw = 0.0f;
}

void Camera::Update(const Timer* pTimer)
{
	const float DELTA_TIME = pTimer->GetElapsed();

	Move(DELTA_TIME);
	Rotate();

	CalculateViewMatrix();
}

void Camera::CalculateViewMatrix()
{
	m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
	m_Up = Vector3::Cross(m_Forward, m_Right).Normalized();

	m_ViewMatrix = Matrix{
		m_Right,
		m_Up,
		m_Forward,
		m_Origin
	};
	m_ViewMatrix = m_ViewMatrix.Inverse();
}

void Camera::CalculateProjectionMatrix(float aspectRatio)
{
	m_AspectRatio = aspectRatio;
	const float FOV_IN_RAD = m_FovAngle * TO_RADIANS;
	m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(FOV_IN_RAD, aspectRatio, m_NearPlane, m_FarPlane);
}

void Camera::Move(float deltaTime)
{
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
	const float moveSpeed = 30.0f;

	// Move Forward/Backward
	if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
		m_Origin += m_Forward * moveSpeed * deltaTime;
	if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
		m_Origin -= m_Forward * moveSpeed * deltaTime;

	// Move Left/Right
	if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
		m_Origin -= m_Right * moveSpeed * deltaTime;
	if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
		m_Origin += m_Right * moveSpeed * deltaTime;

	// Move Up/Down
	if (pKeyboardState[SDL_SCANCODE_E])
		m_Origin += Vector3::UnitY * moveSpeed * deltaTime;
	if (pKeyboardState[SDL_SCANCODE_Q])
		m_Origin -= Vector3::UnitY * moveSpeed * deltaTime;

}

void Camera::Rotate()
{
	const uint32_t MOUSE_STATE = SDL_GetRelativeMouseState(&m_MouseX, &m_MouseY);
	const float MOUSE_SENSITIVITY = 0.005f;

	if (MOUSE_STATE & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		m_TotalYaw += m_MouseX * MOUSE_SENSITIVITY;
		m_TotalPitch -= m_MouseY * MOUSE_SENSITIVITY;

		Matrix rotation = Matrix::CreateRotation(m_TotalPitch, m_TotalYaw, 0.0f);
		m_Forward = rotation.TransformVector(Vector3::UnitZ);
		m_Forward.Normalize();
	}
}