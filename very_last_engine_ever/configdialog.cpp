#include "stdafx.h"
#include "configdialog.h"
#include "config.h"
#include "init.h"

#include <string.h>

#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

using namespace config;

IDirect3D9 *config::direct3d = NULL;

UINT config::adapter = D3DADAPTER_DEFAULT;
D3DDISPLAYMODE config::mode = 
{
	0,           // UINT Width;
	0,          // UINT Height;
	D3DPRESENT_RATE_DEFAULT, // UINT RefreshRate;
	DEFAULT_FORMAT           // D3DFORMAT Format;
};
// D3DFORMAT config::format = DEFAULT_FORMAT;
D3DMULTISAMPLE_TYPE config::multisample = DEFAULT_MULTISAMPLE;
float config::aspect = 1.0; // float(DEFAULT_WIDTH) / DEFAULT_HEIGHT;
bool config::vsync = DEFAULT_VSYNC;
unsigned config::soundcard = DEFAULT_SOUNDCARD;


static void refreshModes(HWND hDlg)
{
	unsigned mode_count = direct3d->GetAdapterModeCount(adapter, mode.Format);
	unsigned best_mode = 0;
	unsigned best_mode_refresh_rate = 0;
	
	SendMessage(GetDlgItem(hDlg, IDC_RESOLUTION), (UINT)CB_RESETCONTENT, (WPARAM)0, 0);
	
	for (unsigned i = 0; i < mode_count; ++i)
	{
		D3DDISPLAYMODE mode;
		direct3d->EnumAdapterModes(adapter, config::mode.Format, i, &mode);

		char temp[256];
		sprintf_s(temp, 256, "%ux%u %uhz", mode.Width, mode.Height, mode.RefreshRate);
		SendMessage(GetDlgItem(hDlg, IDC_RESOLUTION), CB_ADDSTRING, 0, (LPARAM)temp);

		if ((config::mode.Width == mode.Width) && (config::mode.Height == mode.Height))
		{
			if (mode.RefreshRate == D3DPRESENT_RATE_DEFAULT)
			{
				if (best_mode_refresh_rate < mode.RefreshRate)
				{
					best_mode = i;
					best_mode_refresh_rate = mode.RefreshRate;
				}
			}
			else if (mode.RefreshRate == mode.RefreshRate)
			{
				best_mode = i;
			}
		}
	}

	SendMessage(GetDlgItem(hDlg, IDC_RESOLUTION), (UINT)CB_SETCURSEL, (WPARAM)best_mode, 0);
}

static bool is_multisample_type_ok(IDirect3D9 *direct3d, UINT Adapter, D3DFORMAT DepthBufferFormat, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, D3DMULTISAMPLE_TYPE multisample_type) {
	if (SUCCEEDED(direct3d->CheckDeviceMultiSampleType(Adapter, D3DDEVTYPE_HAL, BackBufferFormat, FALSE, multisample_type, NULL)) &&
	    SUCCEEDED(direct3d->CheckDeviceMultiSampleType(Adapter, D3DDEVTYPE_HAL, DepthBufferFormat, FALSE, multisample_type, NULL)))
	{
		return true;
	}
	return false;
}

static void refreshMultisampleTypes(HWND hDlg)
{
	SendMessage(GetDlgItem(hDlg, IDC_MULTISAMPLE), (UINT)CB_RESETCONTENT, (WPARAM)0, 0);

	static const D3DMULTISAMPLE_TYPE types[] = { D3DMULTISAMPLE_NONE, /* D3DMULTISAMPLE_NONMASKABLE, */ D3DMULTISAMPLE_2_SAMPLES, D3DMULTISAMPLE_3_SAMPLES, D3DMULTISAMPLE_4_SAMPLES, D3DMULTISAMPLE_5_SAMPLES, D3DMULTISAMPLE_6_SAMPLES, D3DMULTISAMPLE_7_SAMPLES, D3DMULTISAMPLE_8_SAMPLES, D3DMULTISAMPLE_9_SAMPLES, D3DMULTISAMPLE_10_SAMPLES, D3DMULTISAMPLE_11_SAMPLES, D3DMULTISAMPLE_12_SAMPLES, D3DMULTISAMPLE_13_SAMPLES, D3DMULTISAMPLE_14_SAMPLES, D3DMULTISAMPLE_15_SAMPLES, D3DMULTISAMPLE_16_SAMPLES };
//	static const char *type_strings[] = { "MULTISAMPLE NONE", /* "MULTISAMPLE NONMASKABLE", */ "MULTISAMPLE 2 SAMPLES", "MULTISAMPLE 3 SAMPLES", "MULTISAMPLE 4 SAMPLES", "MULTISAMPLE 5 SAMPLES", "MULTISAMPLE 6 SAMPLES", "MULTISAMPLE 7 SAMPLES", "MULTISAMPLE 8 SAMPLES", "MULTISAMPLE 9 SAMPLES", "MULTISAMPLE 10 SAMPLES", "MULTISAMPLE 11 SAMPLES", "MULTISAMPLE 12 SAMPLES", "MULTISAMPLE 13 SAMPLES", "MULTISAMPLE 14 SAMPLES", "MULTISAMPLE 15 SAMPLES", "MULTISAMPLE 16 SAMPLES" };
	static const char *type_strings[] = { "no multisample", /* "MULTISAMPLE NONMASKABLE", */ "2x multisample", "3x multisample", "4x multisample", "5x multisample", "6x multisample", "7x multisample", "8x multisample", "9x multisample", "10x multisample", "11x multisample", "12x multisample", "13x multisample", "14x multisample", "15x multisample", "16x multisample" };
	assert(ARRAY_SIZE(types) == ARRAY_SIZE(type_strings));

	unsigned best_hit = 0;
	unsigned item = 0;
	for (unsigned i = 0; i < ARRAY_SIZE(types); ++i)
	{
		if (true == is_multisample_type_ok(direct3d, adapter, mode.Format, mode.Format, init::get_best_depth_stencil_format(direct3d, adapter, mode.Format), types[i]))
		{
			SendMessage(GetDlgItem(hDlg, IDC_MULTISAMPLE), CB_ADDSTRING, 0, (LPARAM)type_strings[i]);
			SendMessage(GetDlgItem(hDlg, IDC_MULTISAMPLE), CB_SETITEMDATA, item, (UINT)types[i]);
			if (config::multisample == types[i]) best_hit = item;
			item++;
		}
	}

	// select previous selected mode (if found)
	SendMessage(GetDlgItem(hDlg, IDC_MULTISAMPLE), (UINT)CB_SETCURSEL, (WPARAM)best_hit, 0);
}

static void refreshFormats(HWND hDlg)
{
	unsigned item = 0;

	SendMessage(GetDlgItem(hDlg, IDC_FORMAT), (UINT)CB_RESETCONTENT, (WPARAM)0, 0);  

	static const D3DFORMAT formats[] = { D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_A2R10G10B10, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5 };
	static const char *format_strings[] = { "A8R8G8B8", "X8R8G8B8", "A2R10G10B10", "R5G6B5", "X1R5G5B5", "A1R5G5B5" };
	assert(ARRAY_SIZE(formats) == ARRAY_SIZE(format_strings));

	unsigned best_hit = 0;

	for (unsigned i = 0; i < (sizeof(formats) / sizeof(formats[0])); ++i)
	{
		D3DDISPLAYMODE mode;
		if (SUCCEEDED(direct3d->EnumAdapterModes(adapter, formats[i], 0, &mode)))
		{
			SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_ADDSTRING, 0, (LPARAM)format_strings[i]);
			SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_SETITEMDATA, item, formats[i]);

			if (config::mode.Format == formats[i]) best_hit = item;
			item++;
		}
	}

	SendMessage(GetDlgItem(hDlg, IDC_FORMAT), (UINT)CB_SETCURSEL, (WPARAM)0, 0);
	mode.Format = (D3DFORMAT)SendMessage(GetDlgItem(hDlg, IDC_FORMAT), (UINT)CB_GETITEMDATA, (WPARAM)best_hit, 0);
}

static void enableConfig(HWND hDlg, bool enable)
{
//	::ShowWindow(GetDlgItem(IDC_FORMAT), SW_HIDE);
	::EnableWindow(GetDlgItem(hDlg, IDC_DEVICE), enable);
	::EnableWindow(GetDlgItem(hDlg, IDC_FORMAT), enable);
	::EnableWindow(GetDlgItem(hDlg, IDC_RESOLUTION), enable);
	::EnableWindow(GetDlgItem(hDlg, IDC_MULTISAMPLE), enable);
	::EnableWindow(GetDlgItem(hDlg, IDC_VSYNC), enable);
	::EnableWindow(GetDlgItem(hDlg, IDC_SOUNDCARD), enable);
}



static LRESULT CALLBACK configDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR config::showDialog(HINSTANCE hInstance, IDirect3D9 *direct3d)
{
	assert(NULL != direct3d);
	config::direct3d = direct3d;
	return DialogBox(
		hInstance,
		MAKEINTRESOURCE(IDD_CONFIG),
		NULL,
		(DLGPROC)configDialogProc
	);
}

static LRESULT onInitDialog(HWND hDlg)
{
	direct3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	aspect = float(mode.Width) / mode.Height;

	// add adapters to list
	unsigned adapter_count = direct3d->GetAdapterCount();
	for (unsigned i = 0; i < adapter_count; ++i) {
		D3DADAPTER_IDENTIFIER9 identifier;
		memset(&identifier, 0, sizeof(D3DADAPTER_IDENTIFIER9));
		direct3d->GetAdapterIdentifier(i, 0, &identifier);
		static char temp[256];
		sprintf_s(temp, 256, "%s on %s", identifier.DeviceName, identifier.Description);
		SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_ADDSTRING, 0, (LPARAM)temp);
	}

	// select first adapter by default
	SendMessage(GetDlgItem(hDlg, IDC_DEVICE), (UINT)CB_SETCURSEL, (WPARAM)adapter, 0);

	refreshFormats(hDlg);
	refreshModes(hDlg);
	refreshMultisampleTypes(hDlg);

	// set vsync checkbutton to the default setting
	CheckDlgButton(hDlg, IDC_VSYNC, DEFAULT_VSYNC);

	int best_fit = 0;
	float best_ratio = FLT_MAX;

	static const struct {
		int w, h;
	} aspect_ratios[] = {
		{5, 4},
		{4, 3},
		{16, 10},
		{16, 9},
	};

	for (int i = 0; i < ARRAY_SIZE(aspect_ratios); ++i)
	{
		char temp[256];
		_snprintf(temp, 256, "%d:%d", aspect_ratios[i].w, aspect_ratios[i].h);
		SendMessage(GetDlgItem(hDlg, IDC_ASPECT), CB_ADDSTRING, 0, (LPARAM)temp);

		float curr_ratio = float(aspect_ratios[i].w) / aspect_ratios[i].h;
		if (fabs(curr_ratio - config::aspect) < fabs(best_ratio - config::aspect))
		{
			best_fit = i;
			best_ratio = curr_ratio;
		}
	}
	SendMessage(GetDlgItem(hDlg, IDC_ASPECT), (UINT)CB_SETCURSEL, (WPARAM)best_fit, 0);

	// select medium by default
//	CheckDlgButton(IDC_MEDIUM, true);
//	SendMessage(GetDlgItem(IDC_MEDIUM), (UINT)BN_CLICKED, (WPARAM)1, 0);
//	::PostMessage(GetDlgItem(IDC_MEDIUM), (UINT)BN_CLICKED, (WPARAM)0, 0);

#ifndef VJSYS
	// playback device
	for (unsigned i = 0; 0 != BASS_GetDeviceDescription(i); ++i)
	{
		SendMessage(GetDlgItem(hDlg, IDC_SOUNDCARD), CB_ADDSTRING, 0, (LPARAM)BASS_GetDeviceDescription(i));
	}
#else
	// record device
	for (unsigned i = 0; 0 != BASS_RecordGetDeviceDescription(i); ++i) {
		SendMessage(GetDlgItem(IDC_SOUNDCARD), CB_ADDSTRING, 0, (LPARAM)BASS_RecordGetDeviceDescription(i));
	}
#endif

	// select default soundcard
	SendMessage(GetDlgItem(hDlg, IDC_SOUNDCARD), (UINT)CB_SETCURSEL, (WPARAM)DEFAULT_SOUNDCARD, 0);

	return (LRESULT)TRUE;
}

static LRESULT onDeviceChange(HWND hDlg)
{
	adapter = (unsigned)SendMessage(GetDlgItem(hDlg, IDC_DEVICE), (UINT)CB_GETCURSEL, (WPARAM)0, 0);
	refreshFormats(hDlg);
	refreshModes(hDlg);
	refreshMultisampleTypes(hDlg);
	return (LRESULT)TRUE;
}

static LRESULT onFormatChange(HWND hDlg)
{
	mode.Format = (D3DFORMAT)SendMessage(GetDlgItem(hDlg, IDC_FORMAT), (UINT)CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hDlg, IDC_FORMAT), (UINT)CB_GETCURSEL, (WPARAM)0, 0), 0);
	refreshModes(hDlg);
	refreshMultisampleTypes(hDlg);
	return (LRESULT)TRUE;
}

static LRESULT onResolutionChange(HWND hDlg)
{
	direct3d->EnumAdapterModes(adapter, mode.Format, (UINT)SendMessage(GetDlgItem(hDlg, IDC_RESOLUTION), (UINT)CB_GETCURSEL, (WPARAM)0, 0), &mode);
	refreshMultisampleTypes(hDlg);
	return (LRESULT)TRUE;
}

static LRESULT onMultisampleChange(HWND hDlg)
{
	multisample = (D3DMULTISAMPLE_TYPE)SendMessage(GetDlgItem(hDlg, IDC_MULTISAMPLE), (UINT)CB_GETITEMDATA, (WPARAM)SendMessage(GetDlgItem(hDlg, IDC_MULTISAMPLE), (UINT)CB_GETCURSEL, (WPARAM)0, 0), 0);
		// throw FatalException("mordi");
	return 0;
}

static LRESULT onCloseCmd(HWND hDlg, WORD wID)
{
	if (IDOK == wID)
	{
		vsync = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_VSYNC));
		soundcard = (unsigned)SendMessage(GetDlgItem(hDlg, IDC_SOUNDCARD), (UINT)CB_GETCURSEL, (WPARAM)0, 0);

		char temp[256];
		int sel = (unsigned)SendMessage(GetDlgItem(hDlg, IDC_ASPECT), (UINT)CB_GETCURSEL, (WPARAM)0, 0);

		SendMessage(GetDlgItem(hDlg, IDC_ASPECT), (UINT)CB_GETLBTEXT, (WPARAM)sel, (LPARAM)temp);
		int w, h;
		sscanf(temp, "%d:%d", &w, &h);
		aspect = float(w) / h;
		sprintf(temp, "test %d:%d, %f\n", w, h, aspect);
	}
	EndDialog(hDlg, wID);
	return 0;
}

static LRESULT CALLBACK configDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: return onInitDialog(hDlg);
	
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			return onCloseCmd(hDlg, LOWORD(wParam));
			break;
		
		case IDC_DEVICE:
			if (CBN_SELCHANGE == HIWORD(wParam)) return onDeviceChange(hDlg);
			break;

		case IDC_FORMAT:
			if (CBN_SELCHANGE == HIWORD(wParam)) return onFormatChange(hDlg);
			break;

		case IDC_RESOLUTION:
			if (CBN_SELCHANGE == HIWORD(wParam)) return onResolutionChange(hDlg);
			break;

		case IDC_MULTISAMPLE:
			if (CBN_SELCHANGE == HIWORD(wParam)) return onMultisampleChange(hDlg);
			break;
		}
//	case WM_CLOSE: return onCloseCmd(hDlg, LOWORD(wParam));
	}
	
	return FALSE;
}