#include "StdAfx.h"

namespace DuiLib
{
	CTooltip::CTooltip()
		: m_ToolTip()
		, m_hWndTooltip(NULL)
	{ }

	CTooltip::~CTooltip() 
	{
		if (m_hWndTooltip != NULL && ::IsWindow(m_hWndTooltip))
		{
			::DestroyWindow(m_hWndTooltip);
		}
	}

	void CTooltip::Open(HINSTANCE hInstance, HWND hWndPaint, LPCTSTR szText, RECT const& rc)
	{
		::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
		m_ToolTip.cbSize   = sizeof(TOOLINFO);
		m_ToolTip.uFlags   = TTF_IDISHWND;
		m_ToolTip.hwnd     = hWndPaint;
		m_ToolTip.uId      = (UINT_PTR)hWndPaint;
		m_ToolTip.hinst    = hInstance;
		m_ToolTip.lpszText = const_cast<LPTSTR>((LPCTSTR)szText);
		m_ToolTip.rect     = rc;

		if (m_hWndTooltip == NULL || !::IsWindow(m_hWndTooltip))
		{
			m_hWndTooltip = ::CreateWindowEx(0, TOOLTIPS_CLASS, NULL
				, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP
				, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
				, CW_USEDEFAULT, hWndPaint, NULL, hInstance, NULL);

			//ASSERT(m_hWndTooltip != NULL);

			::SendMessage(m_hWndTooltip, TTM_ADDTOOL, 0, (LPARAM)&m_ToolTip);
			::SendMessage(m_hWndTooltip, TTM_SETMAXTIPWIDTH, 0, 150);
		}

		::SendMessage(m_hWndTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&m_ToolTip);
		::SendMessage(m_hWndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ToolTip);
	}

	void CTooltip::Close()
	{
		if (m_hWndTooltip != NULL) 
		{
			::SendMessage(m_hWndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &m_ToolTip);
		}
	}

	DWORD CTooltip::GetDelayTime()
	{
		if (m_hWndTooltip == NULL) 
		{
			return 400UL;
		}

		return (DWORD) ::SendMessage(m_hWndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
	}
}
