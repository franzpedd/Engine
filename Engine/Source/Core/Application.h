#pragma once

#include "Core/Defines.h"
#include "Event.h"
#include "Input.h"
#include "Scene.h"

#include "Platform/Platform.h"

#include "Renderer/Vulkan/VKRenderer.h"

#include "Util/Clock.h"

namespace Cosmos
{
    class COSMOS_API Application
    {
    public:

        struct Specification
        {
            std::shared_ptr<Platform> Platform;
            std::shared_ptr<Input> Input;
            std::shared_ptr<EventSystem> EventSystem;
            std::shared_ptr<Clock> Clock;
            std::shared_ptr<VKRenderer> Renderer;
            std::shared_ptr<Scene> Scene;

            f64 LastTime{};
            bool IsRunning{};
        };

    public:

        // constructor
        Application();
        
        // destructor
        virtual ~Application();

        // returns the application singleton
        static inline Application& Get() { return *s_Application; }

        // returns the application's members
        inline Specification& GetSpecification() { return m_Specification; }

    public:

        // initializes the main loop
        void Run();

    public:

        // should be called by the client after constructor by the client for loading initial resources
        virtual void OnInitialize() = 0;

        // updates the logic, should be defined on client side
        virtual void OnUpdate(f32 timestep) = 0;

        // updates the runtime render, should be defined on client side
        virtual void OnRender(f32 timestep) = 0;

        // should be called by the client after a quit event was listened by the application for freeing still used resources
        virtual void OnTerminate() = 0;

    private:

        // register the events application will listen to
        void RegisterEvents();

        // unregister the events application will listen to
        void UnregisterEvents();

    private:

        static Application* s_Application;

        Specification m_Specification;
    };

    // application events handler
    bool OnPlatformEvent(EventType type, void* sender, void* listener, EventData edata);
    bool OnKeyEvent(EventType type, void* sender, void* listener, EventData edata);
    bool OnMouseEvent(EventType type, void* sender, void* listener, EventData edata);
}