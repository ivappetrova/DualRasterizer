#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include "Math.h"

namespace dae
{
	class Timer;

	class Camera final
	{
	public:
		Camera() = default;
		Camera(const Vector3& origin, float fovAngle);

		void Initialize(const Vector3& origin, float fovAngle, float nearPlane = 0.1f, float farPlane = 100.0f);
		void Update(const Timer* pTimer);

		Matrix GetViewMatrix() const { return m_ViewMatrix; }
		Matrix GetProjectionMatrix() const { return m_ProjectionMatrix; }
		Matrix GetViewProjectionMatrix() const { return m_ViewMatrix * m_ProjectionMatrix; }

		void CalculateViewMatrix();
		void CalculateProjectionMatrix(float aspectRatio);

		Vector3 GetPosition() const { return m_Origin; }
		Vector3 GetForward() const { return m_Forward; }

	private:
		void Move(float deltaTime);
		void Rotate();

		Vector3 m_Origin{ 0.0f, 0.0f, 0.0f };
		float m_FovAngle{ 90.0f };
		float m_NearPlane{ 0.1f };
		float m_FarPlane{ 100.0f };
		float m_AspectRatio{ 1.0f };

		Vector3 m_Forward{ Vector3::UnitZ };
		Vector3 m_Up{ Vector3::UnitY };
		Vector3 m_Right{ Vector3::UnitX };

		float m_TotalPitch{ 0.0f };
		float m_TotalYaw{ 0.0f };

		Matrix m_ViewMatrix{};
		Matrix m_ProjectionMatrix{};

		// Mouse input
		int m_MouseX{ 0 };
		int m_MouseY{ 0 };
	};
}