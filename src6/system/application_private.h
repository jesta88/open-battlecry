#pragma once

ENGINE_API void app_run(const app_info* application_desc);

ENGINE_API void app_open_window(void);
ENGINE_API void app_close_window(void);
ENGINE_API bool app_handle_events(void);