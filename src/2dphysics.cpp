#include "2dphysics.hpp"

Game::Game() {
    // Setup SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("[ERROR] SDL_Init: %s\n", SDL_GetError());
        cleanup(1);
    }

    is_sdl_init = true;

    if (SDL_Vulkan_LoadLibrary(nullptr) != 0) {
        printf("[ERROR] SDL_Vulkan_LoadLibrary: %s\n", SDL_GetError());
        cleanup(1);
    }

    // Create a window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+Vulkan example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr) {
        printf("[ERROR] SDL_CreateWindow: %s\n", SDL_GetError());
        cleanup(1);
    }

    // Setup vulkan
    VulkanBuilder vk_builder = VulkanBuilder(window, vk_context);
    vk_builder.setup();

    VkSurfaceKHR surface = vk_builder.create_vulkan_surface();
    vk_builder.create_frame_buffers(surface);
    is_vulkan_init = true;

    // Setup ImGui
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // multi-viewport does not work on wayland due to wayland devs
    // being wayland devs (as of October 2023) Could try and find
    // a way to make it work with xwayland (rootful?) in the future
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo init_info = {
        .Instance = vk_context.instance,
        .PhysicalDevice = vk_context.physical_device,
        .Device = vk_context.device,
        .QueueFamily = vk_context.queue_family,
        .Queue = vk_context.queue,
        .PipelineCache = vk_context.pipeline_cache,
        .DescriptorPool = vk_context.descriptor_pool,
        .Subpass = 0,
        .MinImageCount = vk_context.min_image_count,
        .ImageCount = vk_context.main_window_data.ImageCount,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .Allocator = vk_context.allocator,
        .CheckVkResultFn = vk_context.error_callback};

    ImGui_ImplVulkan_Init(&init_info, vk_context.main_window_data.RenderPass);

    // // there has to be a better way to pass by reference
    // this->painter = std::make_unique<Painter>(renderer, objects);
    // this->interface = std::make_unique<UserInferface>(*painter);
    // this->event_handler = std::make_unique<EventHandler>(*painter, *interface, objects);
    // this->scene_manager = std::make_unique<SceneManager>(objects);
}

void Game::game_loop() {
    SDL_Event event;
    while (is_running) {

        // get the next event in queue
        SDL_PollEvent(&event);

        // pass event to imgui
        ImGui_ImplSDL2_ProcessEvent(&event);

        // handle each event, send event to event handler if necessary
        switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;

        case SDL_KEYDOWN:
            // event_handler->keyboard_keydown(event.key);
            break;

        case SDL_KEYUP:
            // event_handler->keyboard_keyup(event.key);
            break;

        case SDL_MOUSEBUTTONDOWN:
            // event_handler->mouse_mousedown(event.button);
            break;
        }

        if (vk_context.swapchain_rebuild) {
            int width, height;
            SDL_GetWindowSize(window, &width, &height);
            if (width > 0 && height > 0)
                vk_context.rebuild_swapchain(width, height);
        }

        // interface->render();
        // painter->render();
        // scene_manager->tick();
    }
}

void Game::cleanup(int exit_code) {
    // imgui
    if (is_imgui_init) {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    // vulkan
    if (is_vulkan_init) {
        vk_context.cleanup();
    }

    // sdl
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(exit_code);
}

int main(int argc, char *argv[]) {
    Game game = Game();
    game.game_loop();
    game.cleanup(0);

    return 0;
}
