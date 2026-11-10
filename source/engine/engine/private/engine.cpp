#include "engine/engine.h"

#include "audio_device.h"
#include "command_line.h"
#include "core/assert.h"
#include "core/log.h"
#include "core/paths.h"
#include "core/timer.h"
#include "engine/game.h"
#include "engine/screenshot_saver.h"
#include "engine/window.h"
#include "input_recorder.h"
#include "native_window.h"
#include "renderer_2d/screen_renderer.h"
#include "renderer_2d/sprite_pass.h"
#include "renderer_2d/sprite_pipeline.h"
#include "renderer_2d/world_renderer_2d.h"
#include "rhi/command_list.h"
#include "rhi/device.h"
#include "rhi/graphics_api.h"
#include "rhi/render_context.h"
#include "rhi/shader_library.h"
#include "rhi/texture_loader.h"
#include "screenshot.h"
#include "stats_display.h"

#include <SDL3/SDL_events.h>

#if defined(__EMSCRIPTEN__)
	#include <emscripten/emscripten.h>
#endif

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Every engine system, in the order they start, and the main
	/// loop's state from one frame to the next.
	///
	/// The order matters: C++ destroys members bottom to top, so everything
	/// declared after the device, and holding gpu resources, is gone before
	/// the device shuts down.
	struct engine_systems
	{
		engine_settings settings;

		native_window sdl_window;
		window main_window{ sdl_window };
		input_recorder recorded_input;
		input user_input{ recorded_input };
		audio_device sound_device;
		audio sound{ sound_device };

		std::unique_ptr<rhi::device> device;
		shader_library shaders;
		texture_loader loader;
		sprite_pipeline sprites;
		render_context context;
		sprite_pass world_drawing;
		world_renderer_2d world{ world_drawing };
		sprite_pass screen_drawing;
		screen_renderer screen{ screen_drawing };
		stats_display stats{ stats_overlay::no };
		screenshot capture;
		screenshot_saver screenshots{ capture };

		mat4 window_projection = mat4(1.0f);
		timer frame_timer;
		int frame = 0;
		bool exit_requested = false;
	};
}

namespace
{
	//--------------------------------------------------------------
	/// @brief A screenshot run steps time by exactly this much per frame.
	constexpr float k_capture_step = 1.0f / 60.0f;

	//--------------------------------------------------------------
	/// @brief Which window the graphics api needs.
	buz::window_graphics to_window_graphics(buz::rhi::graphics_api api)
	{
		switch (api)
		{
		case buz::rhi::graphics_api::d3d12: return buz::window_graphics::native;
		case buz::rhi::graphics_api::vulkan: return buz::window_graphics::vulkan;
		case buz::rhi::graphics_api::opengl: return buz::window_graphics::opengl;
		}
		return buz::window_graphics::native;
	}

	//--------------------------------------------------------------
	/// @brief The name of the graphics api, for messages.
	const char* get_api_name(buz::rhi::graphics_api api)
	{
		switch (api)
		{
		case buz::rhi::graphics_api::d3d12: return "DirectX 12";
		case buz::rhi::graphics_api::vulkan: return "Vulkan";
		case buz::rhi::graphics_api::opengl: return "OpenGL";
		}
		return "the graphics api";
	}

	//--------------------------------------------------------------
	/// @brief Returns true for a screenshot run: one that saves a frame and
	/// quits.
	///
	/// A screenshot run steps time by exactly 1/60 s and ignores the
	/// keyboard and mouse, so the captured frame depends on neither how fast
	/// the machine is nor a key pressed or where the cursor happens to be
	/// while the window has focus.
	bool is_capture_run(const buz::engine_systems& parts)
	{
		return !parts.settings.screenshot_path.empty();
	}

	//--------------------------------------------------------------
	/// @brief Returns true for the frame a screenshot run saves.
	bool is_capture_frame(const buz::engine_systems& parts)
	{
		return is_capture_run(parts) && parts.frame == parts.settings.screenshot_frame;
	}

	//--------------------------------------------------------------
	/// @brief Starts the input frame and hands every pending SDL event to
	/// the system it belongs to: keyboard and mouse to the input, closing
	/// the window to the exit flag.
	void handle_events(buz::engine_systems& parts)
	{
		// new_frame archives the previous frame's state, then the events
		// fill in what happened since
		parts.recorded_input.new_frame();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				parts.exit_requested = true;
			}
			else if (!is_capture_run(parts))
			{
				parts.recorded_input.process_event(event);
			}
		}
	}

	//--------------------------------------------------------------
	/// @brief Switches the stats overlay, the one debug view every game has.
	void handle_debug_keys(buz::engine_systems& parts)
	{
		if (parts.recorded_input.was_key_pressed(buz::key::f3))
		{
			parts.stats.toggle();
		}
	}

	//--------------------------------------------------------------
	/// @brief Draws the frame: the world, the screen on top, the stats,
	/// and saves it when a screenshot was asked for.
	void draw_frame(buz::engine_systems& parts, buz::engine& owner, buz::game& active_game, float measured_seconds)
	{
		buz::rhi::command_list& commands = parts.device->begin_frame(parts.settings.clear_color);

		// The 3D renderer draws into the world pass too, through the context
		parts.world_drawing.begin(commands, parts.sprites, parts.window_projection);
		parts.context.world_commands = &commands;
		active_game.render_world(owner, parts.world);
		parts.context.world_commands = nullptr;

		parts.screen_drawing.begin(commands, parts.sprites, parts.window_projection);
		active_game.render_screen(owner, parts.screen);

		const buz::rhi::frame_stats stats = parts.device->get_frame_stats();
		const float window_width = static_cast<float>(parts.sdl_window.get_width());
		parts.stats.draw(parts.screen, stats, measured_seconds, window_width);

		// A screenshot copies this frame's finished image before it goes
		// to the screen
		const bool save_screenshot = parts.capture.is_requested();
		if (save_screenshot)
		{
			parts.device->request_capture();
		}

		parts.device->end_frame();

		if (save_screenshot)
		{
			parts.capture.save(*parts.device);
		}
	}

	//--------------------------------------------------------------
	/// @brief One pass of the main loop: handle the window's events, update
	/// the game, draw a frame.
	void tick(buz::engine_systems& parts, buz::engine& owner, buz::game& active_game)
	{
		handle_events(parts);
		if (parts.exit_requested)
		{
			return;
		}

		const float measured_seconds = parts.frame_timer.elapsed_seconds();
		parts.frame_timer.reset();
		const float delta_seconds = is_capture_run(parts) ? k_capture_step : measured_seconds;

		handle_debug_keys(parts);

		active_game.update(owner, delta_seconds);
		parts.sound_device.update();

		if (is_capture_frame(parts))
		{
			parts.capture.request(buz::paths::get_output_path(parts.settings.screenshot_path));
		}

		draw_frame(parts, owner, active_game, measured_seconds);

		if (is_capture_frame(parts))
		{
			parts.exit_requested = true;
		}

		++parts.frame;
	}
}

namespace buz
{
	//--------------------------------------------------------------
	engine::engine(int argc, char** argv, const engine_settings& game_settings)
		: m_systems(std::make_unique<engine_systems>())
	{
		const engine_settings settings = read_command_line(argc, argv, game_settings);
		engine_systems& parts = *m_systems;
		parts.settings = settings;
		const rhi::graphics_api api = rhi::get_graphics_api();

		// The systems log why they failed; the fatal error says what that means
		const window_graphics graphics = to_window_graphics(api);
		if (!parts.sdl_window.initialize(settings.window_width, settings.window_height, settings.window_title, graphics))
		{
			fatal_error("Could not open the window. The console says why.");
		}

		rhi::device_desc device_desc;
		device_desc.window = parts.sdl_window.get_sdl_window();
		device_desc.width = settings.window_width;
		device_desc.height = settings.window_height;
		device_desc.gpu_preference = settings.gpu_preference;
		device_desc.use_vsync = settings.use_vsync;
		device_desc.cache_directory = paths::get_executable_directory() / "cache";

		parts.device = rhi::device::create(device_desc);
		if (parts.device == nullptr)
		{
			fatal_error("No graphics card could start {}. The console says why.", get_api_name(api));
		}

		const fs::path shader_directory = paths::get_content_directory() / "shaders";
		if (!parts.shaders.initialize(shader_directory, parts.device->get_shader_extension()))
		{
			fatal_error("The compiled shaders are missing from the content folder. Build the game again.");
		}

		parts.loader.initialize(*parts.device);
		parts.context = { parts.device.get(), &parts.shaders, &parts.loader };

		if (!parts.sprites.initialize(*parts.device, parts.shaders, parts.loader, settings.sampler_filter))
		{
			fatal_error("Could not create the sprite pipeline. The console says why.");
		}

		parts.sound_device.initialize();

		// The overlay's frame times differ every run, so a screenshot run
		// never shows it
		const bool show_stats = settings.show_stats && !is_capture_run(parts);
		parts.stats = stats_display(show_stats ? stats_overlay::yes : stats_overlay::no);
		parts.stats.set_font(parts.loader.load_from_file(paths::get_content_path("fonts/font_8x8.png")));

		log::info("Engine initialized ({}x{})", settings.window_width, settings.window_height);
	}

	//--------------------------------------------------------------
	// The systems shut down in reverse order, the device after everything
	// that holds gpu resources.
	engine::~engine()
	{
		log::info("Engine shutting down");
	}

	//--------------------------------------------------------------
	int engine::run(game& active_game)
	{
		engine_systems& parts = *m_systems;

		// Without a camera the world is drawn in window pixels, just like the
		// screen: (0, 0) is the top-left corner.
		const float width = static_cast<float>(parts.device->get_width());
		const float height = static_cast<float>(parts.device->get_height());
		parts.window_projection = make_orthographic_projection(width, height);
		parts.frame_timer.reset();

#if defined(__EMSCRIPTEN__)
		// A browser page must not loop forever; the browser calls a frame
		// function once per screen refresh instead. Handing the loop over
		// never returns, so the engine and the game in main() stay alive
		// while the page is open.
		struct frame_call
		{
			engine* owner;
			game* active_game;
		};
		frame_call call{ this, &active_game };

		const auto browser_frame = [](void* user_data) {
			const frame_call& current = *static_cast<frame_call*>(user_data);
			engine_systems& current_parts = *current.owner->m_systems;
			tick(current_parts, *current.owner, *current.active_game);
			if (current_parts.exit_requested)
			{
				emscripten_cancel_main_loop();
			}
		};
		emscripten_set_main_loop_arg(browser_frame, &call, 0, true);
#else
		// The main loop: handle window events, update the game, draw a
		// frame; repeat until the window is closed or the game quits.
		while (!parts.exit_requested)
		{
			tick(parts, *this, active_game);
		}
#endif

		return 0;
	}

	//--------------------------------------------------------------
	const input& engine::get_input() const
	{
		return m_systems->user_input;
	}

	//--------------------------------------------------------------
	audio& engine::get_audio()
	{
		return m_systems->sound;
	}

	//--------------------------------------------------------------
	const window& engine::get_window() const
	{
		return m_systems->main_window;
	}

	//--------------------------------------------------------------
	screenshot_saver& engine::get_screenshots()
	{
		return m_systems->screenshots;
	}

	//--------------------------------------------------------------
	void engine::quit()
	{
		m_systems->exit_requested = true;
	}

	//--------------------------------------------------------------
	const render_context& engine::get_render_context() const
	{
		return m_systems->context;
	}
}
