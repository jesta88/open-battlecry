#include <stdio.h>
#include "xcr.h"
#include "../common/log.h"
#include "../third_party/tinyfiledialogs/tinyfiledialogs.h"

#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <Windows.h>
#include <Shlobj.h>

int main(int argc, char *argv[])
{
	char* wbc3_path = tinyfd_selectFolderDialog("Warlords Battlecry 3 folder", "C:\\Games\\Warlords Battlecry 3");
	if (wbc3_path == NULL)
	{
		wb_log_info("User canceled folder selection.");
		return 0;
	}

	//wb_xcr* xcr = wb_xcr_load("Barbarians.xcr");
	//wb_xcr_unload(xcr);

    return 0;
}