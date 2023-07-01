#include "Input.h"

#include "Util/Logger.h"

namespace Cosmos
{
	Input* Input::s_Input = nullptr;

	std::shared_ptr<Input> Input::Create()
	{
		return std::make_shared<Input>();
	}

	// constructor
	Input::Input()
	{
		LOG_ASSERT(s_Input == nullptr, "The Input has already been created");
		s_Input = this;
	}

	// destructor
	Input::~Input()
	{
		s_Input = nullptr;
	}

	void Input::Update(f32 timestep)
	{
		std::memcpy(&m_LastKeyboardInfo, &m_CurrentKeyboardInfo, sizeof(KeyboardInfo));
		std::memcpy(&m_LastMouseInfo, &m_CurrentMouseInfo, sizeof(MouseInfo));
	}

	bool Input::IsKeyDown(Keycode key)
	{
		return m_CurrentKeyboardInfo.Keys[key] == true;
	}

	bool Input::IsKeyUp(Keycode key)
	{
		return m_CurrentKeyboardInfo.Keys[key] == false;
	}

	bool Input::IsButtonDown(Buttoncode button)
	{
		return m_CurrentMouseInfo.Buttons[button] == true;
	}

	bool Input::IsButtonUp(Buttoncode button)
	{
		return m_CurrentMouseInfo.Buttons[button] == false;
	}

	std::pair<i32, i32> Input::GetCurrentMousePosition()
	{
		return std::pair<i32, i32>(m_CurrentMouseInfo.MousePosition.first, m_CurrentMouseInfo.MousePosition.second);
	}

	bool Input::WasKeyDown(Keycode key)
	{
		return m_LastKeyboardInfo.Keys[key] == true;
	}

	bool Input::WasKeyUp(Keycode key)
	{
		return m_LastKeyboardInfo.Keys[key] == false;
	}

	bool Input::WasButtonDown(Buttoncode button)
	{
		return m_LastMouseInfo.Buttons[button] == true;
	}

	bool Input::WasButtonUp(Buttoncode button)
	{
		return m_LastMouseInfo.Buttons[button] == false;
	}

	std::pair<i32, i32> Input::GetLastMousePosition()
	{
		return std::pair<i32, i32>(m_LastMouseInfo.MousePosition.first, m_LastMouseInfo.MousePosition.second);
	}

	void Input::HandleKey(Keycode key, bool pressed)
	{
		if (m_CurrentKeyboardInfo.Keys[key] != pressed)
		{
			m_CurrentKeyboardInfo.Keys[key] = pressed;

			if (pressed)
			{
				EventData e;
				e.data.u32[0] = key;
				EVENT_FIRE(EventType::KEY_PRESSED, 0, e);
			}

			else
			{
				EventData e;
				e.data.u32[0] = key;
				EVENT_FIRE(EventType::KEY_RELEASED, 0, e);
			}
		}
	}

	void Input::HandleButton(Buttoncode button, bool pressed)
	{
		if (m_CurrentMouseInfo.Buttons[button] != pressed)
		{
			m_CurrentMouseInfo.Buttons[button] = pressed;

			if (pressed)
			{
				EventData e;
				e.data.u16[0] = button;
				EVENT_FIRE(EventType::BUTTON_PRESSED, 0, e);
			}

			else
			{
				EventData e;
				e.data.u16[0] = button;
				EVENT_FIRE(EventType::BUTTON_RELEASED, 0, e);
			}
		}
	}

	void Input::HandleMove(i32 x, i32 y)
	{
		if (m_CurrentMouseInfo.MousePosition.first != x || m_CurrentMouseInfo.MousePosition.second != y)
		{
			m_CurrentMouseInfo.MousePosition.first = x;
			m_CurrentMouseInfo.MousePosition.second = y;

			EventData e;
			e.data.i32[0] = x;
			e.data.i32[1] = y;
			EVENT_FIRE(EventType::MOUSE_MOVED, 0, e);
		}
	}

	void Input::HandleWheel(i32 z)
	{
		EventData e;
		e.data.i32[0] = z;
		EVENT_FIRE(EventType::MOUSE_WHEELED, 0, e);
	}
}