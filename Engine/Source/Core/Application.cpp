#include "Application.h"

#include "Util/Logger.h"

namespace Cosmos
{
    Application* Application::s_Application = nullptr;

    Application::Application()
    {
        LOG_ASSERT(s_Application == nullptr, "There is an Application already");
        s_Application = this;

        m_Specification.Platform = Platform::Create("Cosmos", 1366, 768, 100, 100);
        m_Specification.Input = Input::Create();
        m_Specification.EventSystem = EventSystem::Create();
        m_Specification.Clock = Clock::Create();
        
        LOG_TRACE("The time is now %f", GetTime());

        RegisterEvents();
    }

    Application::~Application()
    {
        UnregisterEvents();
    }

    void Application::Run()
    {
        m_Specification.IsRunning = true;

        // setup frame system
        m_Specification.Clock->Start();
        m_Specification.Clock->Update();
        m_Specification.LastTime = m_Specification.Clock->GetElapsedTime();
        f64 runningTime = 0;
        f64 frameCount = 0;
        f64 targetFrameSeconds = 1.0 / 60;

        OnInitialize(); // calls runtime initialze after engine systems were initialized

        while (m_Specification.IsRunning)
        {
            if (!m_Specification.Platform->ProcessMessages()) // quit message was not triggered by platform
            {
                m_Specification.IsRunning = false;
            }

            // update clock
            m_Specification.Clock->Update();
            f64 currentTime = m_Specification.Clock->GetElapsedTime();
            f64 timeStep = currentTime - m_Specification.LastTime;
            f64 frameStartTime = GetTime();

            // update runtime
            OnUpdate((f32)timeStep);
            OnRender((f32)timeStep);

            // draws frame (does not have renderer yet)

            // update clock
            f64 frameEndTime = GetTime();
            f64 frameElapsedTime = frameEndTime - frameStartTime;
            runningTime += frameElapsedTime;
            f64 remainingSeconds = targetFrameSeconds - frameElapsedTime;

            if (remainingSeconds > 0)
            {
                u64 remainingMs = (u64)(remainingSeconds * 1000);
                bool limitFrames = false;
                if (remainingMs > 0 && limitFrames)
                {
                    m_Specification.Platform->Sleep(remainingMs - 1);
                }
            }

            // update inputs
            m_Specification.Input->Update((f32)timeStep);

            // update last time
            m_Specification.LastTime = currentTime;
        }

        m_Specification.IsRunning = false;

        OnTerminate(); // calls runtime terminates before shutting down the engine systems
    }

    void Application::RegisterEvents()
    {
        EVENT_REGISTER(EventType::PLATFORM_QUIT, 0, OnPlatformEvent);
        EVENT_REGISTER(EventType::KEY_PRESSED, 0, OnKeyEvent);
        EVENT_REGISTER(EventType::KEY_RELEASED, 0, OnKeyEvent);
        EVENT_REGISTER(EventType::BUTTON_PRESSED, 0, OnMouseEvent);
        EVENT_REGISTER(EventType::BUTTON_RELEASED, 0, OnMouseEvent);
    }

    void Application::UnregisterEvents()
    {
        EVENT_UNREGISTER(EventType::PLATFORM_QUIT, 0, OnPlatformEvent);
        EVENT_UNREGISTER(EventType::KEY_PRESSED, 0, OnKeyEvent);
        EVENT_UNREGISTER(EventType::KEY_RELEASED, 0, OnKeyEvent);
        EVENT_UNREGISTER(EventType::BUTTON_PRESSED, 0, OnMouseEvent);
        EVENT_UNREGISTER(EventType::BUTTON_RELEASED, 0, OnMouseEvent);
    }

    bool OnPlatformEvent(EventType type, void* sender, void* listener, EventData edata)
    {
        switch (type)
        {
            case EventType::PLATFORM_QUIT:
            {
                Application::Get().GetSpecification().IsRunning = false;
                return true;
            }
        }
        return false;
    }

    bool OnKeyEvent(EventType type, void* sender, void* listener, EventData edata)
    {
        switch (type)
        {
            case EventType::KEY_PRESSED:
            {
                if (edata.data.u16[0] == Input::Keycode::KEYCODE_ESCAPE)
                {
                    Application::Get().GetSpecification().IsRunning = false;
                }
            }
        }
        return false;
    }

    bool OnMouseEvent(EventType type, void* sender, void* listener, EventData edata)
    {
        switch (type)
        {
            case EventType::BUTTON_PRESSED:
            {
                if (edata.data.u16[0] == Input::Buttoncode::BUTTON_MIDDLE)
                {
                    auto& pos = Input::Get().GetCurrentMousePosition();
                    LOG_INFO("Current mouse position: %d-%d", pos.first, pos.second);
                }
            }
        }
        return false;
    }
}