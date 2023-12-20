#include "epch.h"
#include "FileDialog.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <shobjidl.h> 
#endif

namespace Cosmos
{
#if defined(PLATFORM_WINDOWS)
	std::string LPCWSTR_ToString(void* lpcwstr)
	{
		uint32_t strLen = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpcwstr, -1, nullptr, 0, nullptr, nullptr);
		std::string str(strLen, 0);
		WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)lpcwstr, -1, &str[0], strLen, nullptr, nullptr);
		return str;
	}
#endif

	void OpenFileDialog::Show()
	{
#if defined(PLATFORM_WINDOWS)
		HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		if (SUCCEEDED(hr))
		{
			IFileOpenDialog* dialog = nullptr;
			hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog));

			if (SUCCEEDED(hr))
			{
				hr = dialog->Show(nullptr);

				if (SUCCEEDED(hr))
				{
					IShellItem* pItem;
					hr = dialog->GetResult(&pItem);

					if (SUCCEEDED(hr))
					{
						PWSTR str = nullptr;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &str);
						
						if (SUCCEEDED(hr))
						{
							mFilePath = LPCWSTR_ToString(str);
							CoTaskMemFree(str);
						}
						
						hr = pItem->GetDisplayName(SIGDN_NORMALDISPLAY, &str);
						
						if (SUCCEEDED(hr))
						{
							mFileName = LPCWSTR_ToString(str);
							CoTaskMemFree(str);
						}

						pItem->Release();
					}
				}
				dialog->Release();
			}
			CoUninitialize();
		}
#endif
	}

	void SaveFileDialog::Show()
	{
#if defined(PLATFORM_WINDOWS)
		HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		if (SUCCEEDED(hr))
		{
			IFileSaveDialog* dialog = nullptr;
			hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&dialog));

			if (SUCCEEDED(hr))
			{
				COMDLG_FILTERSPEC ext = { L"Cosmos Project", L"Cosmos Project "};
				dialog->SetFileTypes(1, &ext);
				dialog->SetDefaultExtension(L"cosmos");
				hr = dialog->Show(nullptr);

				if (SUCCEEDED(hr))
				{
					IShellItem* pItem;
					hr = dialog->GetResult(&pItem);

					if (SUCCEEDED(hr))
					{
						PWSTR str = nullptr;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &str);

						if (SUCCEEDED(hr))
						{
							mFilePath = LPCWSTR_ToString(str);
							CoTaskMemFree(str);
						}

						hr = pItem->GetDisplayName(SIGDN_NORMALDISPLAY, &str);

						if (SUCCEEDED(hr))
						{
							mFileName = LPCWSTR_ToString(str);
							CoTaskMemFree(str);
						}

						pItem->Release();
					}
				}
				dialog->Release();
			}
			CoUninitialize();
		}
#endif
	}
}