#define UNICODE
#include "napi.h"
#include <Windows.h>
#include <AtlBase.h>
#include <AtlCom.h>
#include <UIAutomation.h>
#include <stdlib.h>
#include <string>
#include <exception>

class WindowsURLAddon : public Napi::Addon<WindowsURLAddon>
{
public:
	WindowsURLAddon(Napi::Env env, Napi::Object exports)
	{
		try
		{
			if (FAILED(CoInitialize(NULL)))
			{
				throw std::exception();
			}

			DefineAddon(exports, {InstanceMethod("getBrowserUrl", &WindowsURLAddon::GetBrowserURL)});

			// initialize UIAutomation
			if FAILED (uia.CoCreateInstance(CLSID_CUIAutomation))
			{
				throw std::exception();
			}
		}
		catch (std::exception &e)
		{
			Napi::Error::New(env, "Failed to initialize native module").ThrowAsJavaScriptException();
		}
	}

	~WindowsURLAddon()
	{
		CoUninitialize();
	}

private:
	CComPtr<IUIAutomation> uia;

	Napi::Value getChromeUrl(Napi::Env env, HWND hwnd)
	{
		CComPtr<IUIAutomationElement> root;
		if (FAILED(uia->ElementFromHandle(hwnd, &root)) || !root)
			return env.Null();

		CComPtr<IUIAutomationCondition> condition;

		uia->CreatePropertyCondition(UIA_ControlTypePropertyId,
																 CComVariant(0xC354), &condition);

		CComPtr<IUIAutomationElement> edit;
		if (FAILED(root->FindFirst(TreeScope_Descendants, condition, &edit)) || !edit)
			return env.Null();

		CComVariant url;
		edit->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);
		if (url.bstrVal)
		{
			std::wstring str(url.bstrVal);
			return Napi::String::New(env, std::string(str.begin(), str.end()));
		}

		return env.Null();
	}

	Napi::Value getFirefoxUrl(Napi::Env env, HWND hwnd)
	{
		CComPtr<IUIAutomationElement> root, navigation, editbox;
		CComPtr<IUIAutomationCondition> c1, c2;

		// find root from hwnd handle
		if FAILED (uia->ElementFromHandle(hwnd, &root))
			env.Null();

		// find navigation bar as child of root
		uia->CreatePropertyCondition(UIA_AutomationIdPropertyId,
																 CComVariant(L"nav-bar"), &c1);
		if FAILED (root->FindFirst(TreeScope_Children, c1, &navigation))
			env.Null();

		// find editbox under as descendant of navigation
		uia->CreatePropertyCondition(UIA_AutomationIdPropertyId,
																 CComVariant(L"urlbar-input"), &c2);
		if FAILED (navigation->FindFirst(TreeScope_Descendants, c2, &editbox))
			env.Null();

		// get the string in editbox
		CComVariant url;
		if FAILED (editbox->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url))
			env.Null();
		if (url.bstrVal)
		{
			std::wstring str(url.bstrVal);
			return Napi::String::New(env, std::string(str.begin(), str.end()));
		}

		return env.Null();
	}

	Napi::Value GetBrowserURL(const Napi::CallbackInfo &info)
	{
		Napi::Env env = info.Env();
		Napi::Number windowHandle = info[0].ToNumber();
		std::string browserName = std::string(info[1].ToString());

		HWND hwnd = (HWND)windowHandle.Int64Value();
		if (browserName == "Firefox")
		{
			return getFirefoxUrl(env, hwnd);
		}
		if (browserName == "Google Chrome" || browserName == "Microsoft Edge")
		{
			return getChromeUrl(env, hwnd);
		}

		return env.Null();
	}
};

NODE_API_ADDON(WindowsURLAddon)
