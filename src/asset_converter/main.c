#include "xcr.h"
#include "../common/log.h"
#include "../common/path.h"
#include "../common/filesystem.h"

static void file_proc(const char* file_path)
{
    const char* extension = wb_path_get_extension(file_path);
    if (extension == NULL || extension[0] != 'x' || extension[1] != 'c' || extension[2] != 'r')
    {
        return;
    }

	wb_xcr* xcr = wb_xcr_load(file_path);
	if (xcr == NULL)
	{
		return;
	}

	wb_log_info("XCR loaded: %s", file_path);
	for (u32 i = 0; i < xcr->resource_count; i++)
	{
		switch(xcr->resources[i].type)
		{
			case WB_XCR_RESOURCE_BMP:

				break;
		}
	}

	wb_xcr_unload(xcr);
}

int main(int argc, char *argv[])
{
	const char* wbc3_path = wb_filesystem_folder_dialog("Warlords Battlecry 3 folder", "C:\\Games\\Warlords Battlecry 3");
	if (wbc3_path == NULL)
	{
		wb_log_info("User canceled folder selection.");
		return 0;
	}
	else
	{
		wb_log_info("Warlords Battlecry 3 path: %s", wbc3_path);
	}

	wb_xcr_init();
	wb_filesystem_directory_iterate(wbc3_path, file_proc);
	wb_xcr_quit();

    return 0;
}