/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/
#include "UploadSettings.h"

#include "atlheaders.h"
#include "ServerFolderSelect.h"
#include "NewFolderDlg.h"
#include "ServerParamsDlg.h"
#include "Gui/GuiTools.h"
#include "ConvertPresetDlg.h"
#include "Func/MyEngineList.h"
#include "Func/Settings.h"
#include "Gui/Dialogs/SettingsDlg.h"
#include "Gui/GuiTools.h"
#include "Gui/IconBitmapUtils.h"
#include "Func/WinUtils.h"
#include "Func/IuCommonFunctions.h"
#include "AddFtpServerDialog.h"
#include "AddDirectoryServerDIalog.h"
#include <Gui/Controls/ServerSelectorControl.h>

CUploadSettings::CUploadSettings(CMyEngineList * EngineList, UploadEngineManager * uploadEngineManager) :�onvert_profiles_(Settings.ConvertProfiles)
{
	nImageIndex = nFileIndex = -1;
	m_EngineList = EngineList;
	m_ProfileChanged  = false;
	m_CatchChanges = false;
   	iconBitmapUtils_ = new IconBitmapUtils();
	useServerThumbnailsTooltip_ = 0;
	uploadEngineManager_ = uploadEngineManager;
}

CUploadSettings::~CUploadSettings()
{
	delete iconBitmapUtils_;
}

void CUploadSettings::TranslateUI()
{
	TRC(IDC_FORMATLABEL,"������:");
	TRC(IDC_QUALITYLABEL,"��������:");
	TRC(IDC_RESIZEBYWIDTH,"��������� ������:");
	//TRC(IDC_SAVEPROPORTIONS,"��������� ���������");
	//TRC(IDC_YOURLOGO,"�������� ������� ����");
	TRC(IDC_XLABEL,"�/��� ������:");
	TRC(IDC_PROFILELABEL,"�������:");
	TRC(IDC_YOURTEXT,"�������� ����� �� ��������");
	TRC(IDC_IMAGEPARAMETERS,"��������� �����������");
	TRC(IDC_LOGOOPTIONS,"�������������...");
	TRC(IDC_KEEPASIS,"������������ �����������");
	TRC(IDC_THUMBSETTINGS,"����������");
	TRC(IDC_CREATETHUMBNAILS,"��������� ��������� (������)");
	TRC(IDC_IMAGESERVERGROUPBOX,"C����� ��� �������� �����������");
	TRC(IDC_USESERVERTHUMBNAILS,"������������ ��������� ���������");
	TRC(IDC_WIDTHLABEL,"������ ���������:");
	TRC(IDC_ADDFILESIZE,"������� �� ���������");
	TRC(IDC_PRESSUPLOADBUTTON,"������� ������ \"���������\" ����� ������ ������� ��������");
	TRC(IDC_FILESERVERGROUPBOX, "������ ��� ��������� ����� ������");
	useServerThumbnailsTooltip_ = GuiTools::CreateToolTipForWindow(GetDlgItem(IDC_USESERVERTHUMBNAILS), TR("��� ��������, ��� ��������� ����� ����������� ������, � �� ����������.")); //  \r\n��� ���� ��, ��� ��� ����� ���������, �������� ������� �� ���������� �����.
}

LRESULT CUploadSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PageWnd = m_hWnd;
	sessionImageServer_ = WizardDlg->getSessionImageServer();
	sessionFileServer_ = WizardDlg->getSessionFileServer();
	imageServerLogin_ = sessionImageServer_.serverSettings().authData.Login;
	fileServerLogin_ = sessionFileServer_.serverSettings().authData.Login;
	//m_ThumbSizeEdit.SubclassWindow(GetDlgItem(IDC_QUALITYEDIT));
	TranslateUI();


	CBitmap hBitmap;
	HDC dc = ::GetDC(HWND_DESKTOP);
	// Get color depth (minimum requirement is 32-bits for alpha blended images).
	int iBitsPixel = GetDeviceCaps(dc,BITSPIXEL);
	/*if (iBitsPixel >= 32)
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_BITMAP5));
		m_PlaceSelectorImageList.Create(16,16,ILC_COLOR32,0,6);
		m_PlaceSelectorImageList.Add(hBitmap, (HBITMAP) NULL);
	}
	else*/
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_SERVERTOOLBARBMP2));
		m_PlaceSelectorImageList.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
		m_PlaceSelectorImageList.Add(hBitmap,RGB(255,0,255));
	}
	::ReleaseDC(HWND_DESKTOP,dc) ;
   HICON ico = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_DROPDOWN), IMAGE_ICON	, 16,16,0);
	SendDlgItemMessage( IDC_RESIZEPRESETSBUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(HICON)ico);
   m_ResizePresetIconButton.SubclassWindow(GetDlgItem(IDC_RESIZEPRESETSBUTTON));
	
   SendDlgItemMessage(IDC_SHORTENINGURLSERVERBUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(HICON)ico);
   m_ShorteningServerButton.SubclassWindow(GetDlgItem(IDC_SHORTENINGURLSERVERBUTTON));
   
   
   ico = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONEDIT), IMAGE_ICON	, 16,16,0);
   RECT profileRect;
    ::GetWindowRect(GetDlgItem(IDC_EDITPROFILE), &profileRect);
   ::MapWindowPoints(0, m_hWnd, (LPPOINT)&profileRect, 2);
   
   m_ProfileEditToolbar.Create(m_hWnd,profileRect,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD | TBSTYLE_LIST |TBSTYLE_FLAT| CCS_NORESIZE|/*CCS_BOTTOM |CCS_ADJUSTABLE|*/TBSTYLE_TOOLTIPS|CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
	m_ProfileEditToolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
   m_ProfileEditToolbar.SetButtonStructSize();
	m_ProfileEditToolbar.SetButtonSize(17,17);
		
	CImageList list;
	list.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
	list.AddIcon(ico);
	m_ProfileEditToolbar.SetImageList(list);
	m_ProfileEditToolbar.AddButton(IDC_EDITPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 0,TR("������������� �������"), 0);

   RECT Toolbar1Rect;
	::GetWindowRect(GetDlgItem(IDC_IMAGESERVERGROUPBOX), &Toolbar1Rect);
  
	::MapWindowPoints(0, m_hWnd, (LPPOINT)&Toolbar1Rect, 2);
	Toolbar1Rect.top += GuiTools::dlgY(9);
	Toolbar1Rect.bottom -= GuiTools::dlgY(3);
	Toolbar1Rect.left += GuiTools::dlgX(6);
	Toolbar1Rect.right -= GuiTools::dlgX(6);

	RECT Toolbar2Rect;
	::GetWindowRect(GetDlgItem(IDC_FILESERVERGROUPBOX), &Toolbar2Rect);
	::MapWindowPoints(0, m_hWnd, (LPPOINT)&Toolbar2Rect, 2);
	Toolbar2Rect.top += GuiTools::dlgY(9);
	Toolbar2Rect.bottom -= GuiTools::dlgY(3);
	Toolbar2Rect.left += GuiTools::dlgX(6);
	Toolbar2Rect.right -= GuiTools::dlgX(6);

	for(int i = 0; i<2; i++)
	{
		CToolBarCtrl& CurrentToolbar = (i == 0) ? Toolbar: FileServerSelectBar;
		CurrentToolbar.Create(m_hWnd,i?Toolbar2Rect:Toolbar1Rect,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD | TBSTYLE_LIST |TBSTYLE_FLAT| CCS_NORESIZE|/*CCS_BOTTOM |CCS_ADJUSTABLE|*/CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
		
		CurrentToolbar.SetButtonStructSize();
		CurrentToolbar.SetButtonSize(30,18);
		CurrentToolbar.SetImageList(m_PlaceSelectorImageList);
		
		CurrentToolbar.AddButton(IDC_SERVERBUTTON, TBSTYLE_DROPDOWN |BTNS_AUTOSIZE, TBSTATE_ENABLED, -1, TR("�������� ������..."), 0);
		CurrentToolbar.AddButton(IDC_TOOLBARSEPARATOR1, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, TR(""), 0);
		
		CurrentToolbar.AddButton(IDC_LOGINTOOLBUTTON + !i, /*TBSTYLE_BUTTON*/TBSTYLE_DROPDOWN |BTNS_AUTOSIZE, TBSTATE_ENABLED, 0, _T(""), 0);
		CurrentToolbar.AddButton(IDC_TOOLBARSEPARATOR2, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, TR(""), 0);
		
		CurrentToolbar.AddButton(IDC_SELECTFOLDER, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("�������� �����..."), 0);
		CurrentToolbar.AutoSize();
	}

	Toolbar.SetWindowLong(GWL_ID, IDC_IMAGETOOLBAR);
	FileServerSelectBar.SetWindowLong(GWL_ID, IDC_FILETOOLBAR);

	SendDlgItemMessage(IDC_THUMBWIDTHSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)1000, (short)40) );
	

	SendDlgItemMessage(IDC_QUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));
	SendDlgItemMessage(IDC_THUMBQUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));
	
	
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)TR("����"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("JPEG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("PNG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("GIF"));
	
	ShowParams();
	ShowParams(sessionImageServer_.getImageUploadParams().ImageProfileName);
	UpdateProfileList();
	UpdateAllPlaceSelectors();
	return 1;  // Let the system set the focus
}

LRESULT CUploadSettings::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

// It is called only on XP and older versions
LRESULT CUploadSettings::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MEASUREITEMSTRUCT* lpmis = reinterpret_cast<MEASUREITEMSTRUCT*>(lParam);
	if ( lpmis == NULL ) {
		return 0;
	}

	lpmis->itemWidth  = max(0, GetSystemMetrics(SM_CXSMICON) - GetSystemMetrics(SM_CXMENUCHECK) + 4);
	lpmis->itemHeight = GetSystemMetrics(SM_CYSMICON)+2;
	return TRUE;
}

// It is called only on XP and older versions
LRESULT CUploadSettings::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPCTSTR resource;
	DRAWITEMSTRUCT* lpdis = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
	if ((lpdis==NULL)||(lpdis->CtlType != ODT_MENU))
		return S_OK;		//not for a menu
	int i =0;
	int iconID=0;

	HICON hIcon = serverMenuIcons_[lpdis->itemID];

	if (hIcon == NULL)
		return 0;
	// fix from http://miranda.svn.sourceforge.net/viewvc/miranda/trunk/miranda/src/modules/clist/genmenu.cpp
	int	w = GetSystemMetrics(SM_CXSMICON);
	int h = GetSystemMetrics(SM_CYSMICON);
	int y = lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top - h) / 2;
	int x = 2;
	
	DrawIconEx(lpdis->hDC, x, y, hIcon, w, h, 0, NULL, DI_NORMAL);
	DeleteObject(hIcon);
	return TRUE;
}


LRESULT CUploadSettings::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CUploadSettings::OnBnClickedKeepasis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool checked = SendDlgItemMessage(IDC_KEEPASIS, BM_GETCHECK, 0, 0)!=FALSE;
	GuiTools::EnableNextN(GetDlgItem(IDC_KEEPASIS), 13, checked);
	m_ProfileEditToolbar.EnableWindow(checked);
	return 0;
}

void CUploadSettings::ShowParams(/*UPLOADPARAMS params*/)
{
	SendDlgItemMessage(IDC_KEEPASIS,BM_SETCHECK,sessionImageServer_.getImageUploadParams().ProcessImages);
	SendDlgItemMessage(IDC_THUMBFORMATLIST,CB_SETCURSEL, (int)sessionImageServer_.getImageUploadParams().getThumb().Format);
	SendDlgItemMessage(IDC_CREATETHUMBNAILS,BM_SETCHECK,sessionImageServer_.getImageUploadParams().CreateThumbs);
	SendDlgItemMessage(IDC_ADDFILESIZE,BM_SETCHECK,sessionImageServer_.getImageUploadParams().getThumb().AddImageSize);
	SendDlgItemMessage(IDC_USESERVERTHUMBNAILS,BM_SETCHECK,sessionImageServer_.getImageUploadParams().UseServerThumbs);

	bool shortenImages = sessionImageServer_.shortenLinks();
	bool shortenFiles = sessionFileServer_.shortenLinks();
	int shortenLinks = BST_INDETERMINATE;
	int checkboxStyle = BS_AUTO3STATE;
	HWND checkbox = GetDlgItem(IDC_SHORTENLINKSCHECKBOX);
	if (shortenImages == shortenFiles)
	{
		shortenLinks = shortenImages ? BST_CHECKED : BST_UNCHECKED;
		checkboxStyle = BS_AUTOCHECKBOX;
	} 
	::SetWindowLong(checkbox, GWL_STYLE, (::GetWindowLong(checkbox, GWL_STYLE) & ~(BS_AUTO3STATE | BS_AUTOCHECKBOX)) | checkboxStyle);
	SendDlgItemMessage(IDC_SHORTENLINKSCHECKBOX, BM_SETCHECK, shortenLinks);
	updateUrlShorteningCheckboxLabel();
}

LRESULT CUploadSettings::OnBnClickedCreatethumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	BOOL checked = SendDlgItemMessage(IDC_CREATETHUMBNAILS, BM_GETCHECK, 0, 0);
	
	::EnableWindow(GetDlgItem(IDC_THUMBWIDTH), checked);
	::EnableWindow(GetDlgItem(IDC_ADDFILESIZE), checked);
	::EnableWindow(GetDlgItem(IDC_WIDTHLABEL), checked);
	::EnableWindow(GetDlgItem(IDC_USETHUMBTEMPLATE), checked);
	::EnableWindow(GetDlgItem(IDC_USESERVERTHUMBNAILS), checked);
	//::EnableWindow(GetDlgItem(IDC_TEXTOVERTHUMB2), checked);
		
	if(!checked)
		::EnableWindow(GetDlgItem(IDC_DRAWFRAME), checked);
	else 
		OnBnClickedUseThumbTemplate(0, 0, 0, bHandled);
		
	::EnableWindow(GetDlgItem(IDC_PXLABEL), checked);
		
	return 0;
}

LRESULT CUploadSettings::OnBnClickedLogooptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: remove me! 
	return 0;
}

bool CUploadSettings::OnNext()
{	
	if(!sessionImageServer_.serverName().empty())
	{
		CUploadEngineData *ue = sessionImageServer_.uploadEngineData();
		if(ue->NeedAuthorization ==2 && sessionImageServer_.profileName().empty())
		{ 
			CString errorMsg;
			errorMsg.Format(TR("�������� ������ �� ������ '%s' ���������� ��� ������� ������� ������.\n����������, ����������������� �� ������ ������� � ������� ���� ��������������� ������ � ���������."), (LPCTSTR)Utf8ToWCstring(ue->Name));
			MessageBox(errorMsg, APPNAME, MB_ICONERROR);
			return false;
		}
	}
	if(!sessionFileServer_.serverName().empty())
	{
		CUploadEngineData *ue2 =sessionFileServer_.uploadEngineData();
		if(ue2->NeedAuthorization == 2 && sessionFileServer_.profileName().empty())
		{
			CString errorMsg;
			errorMsg.Format(TR("�� �� ������ ��������� ����������� �� ������� '%s'!"), (LPCTSTR)Utf8ToWstring(ue2->Name).c_str());
			MessageBox(errorMsg, APPNAME, MB_ICONWARNING);
			return false;
		}
	}
	
   if(sessionImageServer_.getImageUploadParamsRef().getThumbRef().ResizeMode != ThumbCreatingParams::trByHeight)
   {
     sessionImageServer_.getImageUploadParamsRef().getThumbRef().Size = GetDlgItemInt(IDC_THUMBWIDTH);
   }
   else
   {
        sessionImageServer_.getImageUploadParamsRef().getThumbRef().Size=  GetDlgItemInt(IDC_THUMBWIDTH);
   }

	sessionImageServer_.getImageUploadParamsRef().ProcessImages = SendDlgItemMessage(IDC_KEEPASIS, BM_GETCHECK, 0) == BST_CHECKED;
	sessionImageServer_.getImageUploadParamsRef().CreateThumbs = IS_CHECKED(IDC_CREATETHUMBNAILS);
	sessionImageServer_.getImageUploadParamsRef().UseServerThumbs = IS_CHECKED(IDC_USESERVERTHUMBNAILS);
	sessionImageServer_.getImageUploadParamsRef().getThumbRef().AddImageSize = IS_CHECKED(IDC_ADDFILESIZE);
	sessionImageServer_.getImageUploadParamsRef().getThumbRef().Size=  GetDlgItemInt(IDC_THUMBWIDTH);

	
	int shortenLinks = SendDlgItemMessage(IDC_SHORTENLINKSCHECKBOX, BM_GETCHECK);
	if (shortenLinks != BST_INDETERMINATE)
	{
		bool shorten = shortenLinks == BST_CHECKED;
		sessionImageServer_.setShortenLinks(shorten);
		sessionFileServer_.setShortenLinks(shorten);
	}

	WizardDlg->setSessionImageServer(sessionImageServer_);
	WizardDlg->setSessionFileServer(sessionFileServer_);
	if ( Settings.RememberImageServer ) {
		Settings.imageServer = sessionImageServer_;
	}
	if ( Settings.RememberFileServer ) {
		Settings.fileServer = sessionFileServer_;
	}
    SaveCurrentProfile();
	return true;
}

bool CUploadSettings::OnShow()
{
	BOOL temp;

	if ( WizardDlg->serversChanged() ) {
		sessionImageServer_ = WizardDlg->getSessionImageServer();
		//MessageBox(sessionImageServer_.serverName());
		sessionFileServer_ = WizardDlg->getSessionFileServer();
		imageServerLogin_ = sessionImageServer_.serverSettings().authData.Login;
		fileServerLogin_ = sessionFileServer_.serverSettings().authData.Login;
		WizardDlg->setServersChanged(false);
		ShowParams();
	}

   ShowParams(sessionImageServer_.getImageUploadParamsRef().ImageProfileName);
   UpdateProfileList();
   UpdateAllPlaceSelectors();
	OnBnClickedCreatethumbnails(0, 0, 0, temp);
	OnBnClickedKeepasis(0, 0, 0, temp);
	EnableNext();
	SetNextCaption(TR("&���������"));
	return true;
}

LRESULT CUploadSettings::OnBnClickedLogin(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;

	ServerProfile & serverProfile = ImageServer? sessionImageServer_ : sessionFileServer_;
	CLoginDlg dlg(serverProfile, uploadEngineManager_);


	ServerSettingsStruct & ss = ImageServer ? sessionImageServer_.serverSettings() : sessionFileServer_.serverSettings();
	std::string UserName = ss.authData.Login; 
	bool prevAuthEnabled = ss.authData.DoAuth;
	if( dlg.DoModal(m_hWnd) == IDOK)
	{
		if ( ImageServer ) {
			imageServerLogin_ = WCstringToUtf8(dlg.accountName());
		} else {
			fileServerLogin_ =  WCstringToUtf8(dlg.accountName());
		}
		serverProfile.setProfileName(WCstringToUtf8(dlg.accountName()));
		if(UserName != ss.authData.Login || ss.authData.DoAuth!=prevAuthEnabled)
		{
			serverProfile.setFolderId("");
			serverProfile.setFolderTitle("");
			serverProfile.setFolderUrl("");
			//iuPluginManager.UnloadPlugins();
			//m_EngineList->DestroyCachedEngine(WCstringToUtf8(serverProfile.serverName()), WCstringToUtf8(serverProfile.profileName()));
		}
			
		UpdateAllPlaceSelectors();
	}
	return 0;
}

LRESULT CUploadSettings::OnBnClickedUseThumbTemplate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL checked = SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_GETCHECK, 0, 0);

	if(checked && !WinUtils::FileExists(IuCommonFunctions::GetDataFolder() + _T("thumb.png")) && wID == IDC_USETHUMBTEMPLATE)
	{
		MessageBox(TR("���������� ������������ ������ ��� ���������. ���� ������� \"Data\\Thumb.png\" �� ������."), APPNAME, MB_ICONWARNING);
		SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_SETCHECK, false);
		return 0;
	}
	
	if(checked) SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_SETCHECK, false);

	checked = checked || SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_GETCHECK, 0, 0);
	::EnableWindow(GetDlgItem(IDC_DRAWFRAME), !checked);
	//::EnableWindow(GetDlgItem(IDC_TEXTOVERTHUMB2),!checked);
	
	return 0;
}
	
LRESULT CUploadSettings::OnBnClickedUseServerThumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL checked=SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_GETCHECK, 0, 0);
	
	if(checked) SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_SETCHECK, false);
	
	checked = checked || SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_GETCHECK, 0, 0);
	::EnableWindow(GetDlgItem(IDC_DRAWFRAME),!checked);
	return 0;
}
	
LRESULT CUploadSettings::OnBnClickedSelectFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
bool ImageServer = (hWndCtl == Toolbar.m_hWnd);	
//	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;
	
	ServerProfile& serverProfile = ImageServer ? sessionImageServer_ : sessionFileServer_;
	CUploadEngineData *ue = serverProfile.uploadEngineData();

	if(ue->SupportsFolders){
		CServerFolderSelect as(serverProfile, uploadEngineManager_);

		as.m_SelectedFolder.id= serverProfile.folderId();
		
		if(as.DoModal() == IDOK){
			ServerSettingsStruct& sss = serverProfile.serverSettings();
			if(!as.m_SelectedFolder.id.empty()){
				sss.defaultFolder = as.m_SelectedFolder;
				serverProfile.setFolderId(as.m_SelectedFolder.id);
				serverProfile.setFolderTitle(as.m_SelectedFolder.title);
				serverProfile.setFolderUrl(as.m_SelectedFolder.viewUrl);
			}
			else {
				sss.defaultFolder = CFolderItem();
				serverProfile.setFolderId("");
				serverProfile.setFolderTitle("");
				serverProfile.setFolderUrl("");

			}
		UpdateAllPlaceSelectors();
		}
	}
	return 0;
}
	
void CUploadSettings::UpdateToolbarIcons()
{
	HICON hImageIcon = NULL, hFileIcon = NULL;

	if(!sessionImageServer_.isNull())
		hImageIcon = m_EngineList->getIconForServer(sessionImageServer_.serverName());
		
	if(!sessionFileServer_.isNull())
		hFileIcon =m_EngineList->getIconForServer(sessionFileServer_.serverName());
	
	if(hImageIcon)
	{
		if(nImageIndex == -1)
		{
			nImageIndex = m_PlaceSelectorImageList.AddIcon( hImageIcon);
		}
		else nImageIndex= m_PlaceSelectorImageList.ReplaceIcon(nImageIndex, hImageIcon);
	} else nImageIndex =-1;

	if(hFileIcon)
	{
		if(nFileIndex == -1)
		{
			nFileIndex = m_PlaceSelectorImageList.AddIcon( hFileIcon);
		}
		else 
			nFileIndex = m_PlaceSelectorImageList.ReplaceIcon(nFileIndex, hFileIcon);
	} else nFileIndex = -1;

	Toolbar.ChangeBitmap(IDC_SERVERBUTTON, nImageIndex);
	FileServerSelectBar.ChangeBitmap(IDC_SERVERBUTTON, nFileIndex);
}

void CUploadSettings::UpdatePlaceSelector(bool ImageServer)
{
	TBBUTTONINFO bi;
	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;

//	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;
	ServerProfile& serverProfile = ImageServer ? sessionImageServer_ : sessionFileServer_;

	CUploadEngineData * uploadEngine = 0;
	
	if(serverProfile.isNull())
	{
		
		CurrentToolbar.HideButton(IDC_LOGINTOOLBUTTON + ImageServer ,true);
		CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR1, true);
		CurrentToolbar.HideButton(IDC_SELECTFOLDER, true);
		CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR2, true);
		return;
	}

	uploadEngine =  serverProfile.uploadEngineData();
	//MessageBox(serverProfile.serverName());
	CString serverTitle = (!serverProfile.isNull()) ? Utf8ToWCstring(serverProfile.serverName()): TR("�������� ������");

	ZeroMemory(&bi, sizeof(bi));
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_TEXT;
	bi.pszText = (LPWSTR)(LPCTSTR) serverTitle ;
	CurrentToolbar.SetButtonInfo(IDC_SERVERBUTTON, &bi);

	LoginInfo& li = serverProfile.serverSettings().authData;
	CString login = WinUtils::TrimString(Utf8ToWCstring(li.Login),23);
	
	CurrentToolbar.SetImageList(m_PlaceSelectorImageList);

	CurrentToolbar.HideButton(IDC_LOGINTOOLBUTTON + ImageServer,(bool)!uploadEngine->NeedAuthorization);
	CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR1,(bool)!uploadEngine->NeedAuthorization);
	
	bool ShowLoginButton = !login.IsEmpty() && li.DoAuth;
	if(!ShowLoginButton)
	{
		if(uploadEngine->NeedAuthorization == 2)
			login = TR("������ ������������...");
		else 
			login = TR("������� �� ������");

	}
	bi.pszText = (LPWSTR)(LPCTSTR)login;
	CurrentToolbar.SetButtonInfo(IDC_LOGINTOOLBUTTON+ImageServer, &bi);

	bool ShowFolderButton = uploadEngine->SupportsFolders && ShowLoginButton;

	CurrentToolbar.HideButton(IDC_SELECTFOLDER,!ShowFolderButton);
	CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR2,!ShowFolderButton);
		
	CString title = WinUtils::TrimString(Utf8ToWCstring(serverProfile.folderTitle()), 27);
	if(title.IsEmpty()) title = TR("����� �� �������");
	bi.pszText = (LPWSTR)(LPCTSTR)title;
	CurrentToolbar.SetButtonInfo(IDC_SELECTFOLDER, &bi);
	
}
void CUploadSettings::UpdateAllPlaceSelectors()
{
	UpdatePlaceSelector(false);
	UpdatePlaceSelector(true);
	UpdateToolbarIcons();	
}

LRESULT CUploadSettings::OnImageServerSelect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nServerIndex = wID - IDC_IMAGESERVER_FIRST_ID;
	selectServer(sessionImageServer_, nServerIndex);
	

	UpdateAllPlaceSelectors();
	return 0;
}

LRESULT CUploadSettings::OnFileServerSelect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nServerIndex = wID - IDC_FILESERVER_FIRST_ID;
	
	selectServer(sessionFileServer_, nServerIndex);
	UpdateAllPlaceSelectors();
	return 0;
}
	
LRESULT CUploadSettings::OnServerDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMTOOLBAR* pnmtb = (NMTOOLBAR *) pnmh;

	bool ImageServer = (idCtrl == IDC_IMAGETOOLBAR);
	ServerProfile & serverProfile = ImageServer ? sessionImageServer_ : sessionFileServer_;
	std::vector<HBITMAP> bitmaps;
	CUploadEngineData *uploadEngine = 0;
	if(!serverProfile.isNull())
	{
		uploadEngine = serverProfile.uploadEngineData();
	}

	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;
	
	CMenu sub;	
	MENUITEMINFO mi;
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_TYPE|MIIM_ID;
	mi.fType = MFT_STRING;
	sub.CreatePopupMenu();
	//sub.SetMenuInfo()
	bool isVistaOrLater = WinUtils::IsVista();
	if(pnmtb->iItem == IDC_SERVERBUTTON)
	{
		int menuItemCount=0;
		int FirstFileServerIndex = -1;
		int lastMenuBreakIndex = 0;
		bool nextItemBreaksLine = false;
		
		if(ImageServer)
		{
			for(int i=0; i<m_EngineList->count(); i++)
			{
				mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
				mi.fType = MFT_STRING;
				if(m_EngineList->byIndex(i)->Type != CUploadEngineData::TypeImageServer) continue;
				mi.wID = (ImageServer ? IDC_IMAGESERVER_FIRST_ID: IDC_FILESERVER_FIRST_ID  ) +i;
				CUploadEngineData* ued = m_EngineList->byIndex(i);
				CString name  = Utf8ToWCstring(ued->Name); 
				mi.dwTypeData  = (LPWSTR)(LPCTSTR) name;
				HICON hImageIcon = m_EngineList->getIconForServer(ued->Name);
				HBITMAP bm = 0;
				if (WinUtils::IsVista() ) {
					bm = iconBitmapUtils_->HIconToBitmapPARGB32(hImageIcon);
					bitmaps.push_back(bm);
				}

				mi.hbmpItem =  WinUtils::IsVista() ? bm: HBMMENU_CALLBACK;
				if (! WinUtils::IsVista() ) {
					serverMenuIcons_[mi.wID] = hImageIcon;
				}
				if ( mi.hbmpItem ) {
					mi.fMask |= MIIM_BITMAP;
				}
				if ( menuItemCount && (menuItemCount - lastMenuBreakIndex) % 34 == 0  ) {
					
					mi.fType |= MFT_MENUBARBREAK ;
					lastMenuBreakIndex = menuItemCount;
					
				}

				sub.InsertMenuItem(menuItemCount++, true, &mi);
			}

			ZeroMemory(&mi,sizeof(mi));
			mi.cbSize = sizeof(mi);
			mi.fMask = MIIM_TYPE|MIIM_ID;
			mi.wID = IDC_FILESERVER_LAST_ID + 1;
			mi.fType = MFT_SEPARATOR;
			
			if ( menuItemCount && (menuItemCount - lastMenuBreakIndex) >= 17   ) {
				nextItemBreaksLine = true;
			} else {
				sub.InsertMenuItem(menuItemCount++, true, &mi);
			}
			

			
		}

		mi.fType = MFT_STRING;
		for(int i=0; i<m_EngineList->count(); i++)
		{
			mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
			mi.fType = MFT_STRING;
			if(m_EngineList->byIndex(i)->Type != CUploadEngineData::TypeFileServer) continue;
			mi.wID = (ImageServer?IDC_IMAGESERVER_FIRST_ID: IDC_FILESERVER_FIRST_ID  ) +i;
			CUploadEngineData* ued = m_EngineList->byIndex(i);
			CString name  = Utf8ToWCstring(ued->Name); 
			mi.dwTypeData  = (LPWSTR)(LPCTSTR) name;
			HICON hImageIcon = m_EngineList->getIconForServer(ued->Name);
			if (! WinUtils::IsVista() ) {
				serverMenuIcons_[mi.wID] = hImageIcon;
			}
			HBITMAP bm = 0;
			if (WinUtils::IsVista() ) {
				bm = iconBitmapUtils_->HIconToBitmapPARGB32(hImageIcon);
				bitmaps.push_back(bm);
			}
		
			
			mi.hbmpItem =  WinUtils::IsVista() ? bm: HBMMENU_CALLBACK;
			if ( mi.hbmpItem ) {
				mi.fMask |= MIIM_BITMAP;
			}
			if ( menuItemCount && ((menuItemCount - lastMenuBreakIndex) % 30 == 0 || nextItemBreaksLine )  ) {

				mi.fType |= MFT_MENUBARBREAK ;
				lastMenuBreakIndex = menuItemCount;

			}
			nextItemBreaksLine = false;

			sub.InsertMenuItem(menuItemCount++, true, &mi);	
		}
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.wID = IDC_FILESERVER_LAST_ID + 1;
		mi.fType = MFT_SEPARATOR;
	
		sub.InsertMenuItem(menuItemCount++, true, &mi);
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
		mi.fType = MFT_STRING;
		mi.wID = ImageServer ? IDC_ADD_FTP_SERVER : IDC_ADD_FTP_SERVER_FROM_FILESERVER_LIST;
		
		mi.dwTypeData  = TR("�������� FTP ������...");
		mi.hbmpItem = 0;
		sub.InsertMenuItem(menuItemCount++, true, &mi);	

		mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
		mi.fType = MFT_STRING;
		mi.wID = ImageServer ? IDC_ADD_DIRECTORY_AS_SERVER : IDC_ADD_DIRECTORY_AS_SERVER_FROM_FILESERVER_LIST;

		mi.dwTypeData  = TR("�������� ��������� ����� ��� ������...");
		mi.hbmpItem = 0;
		sub.InsertMenuItem(menuItemCount++, true, &mi);	

		sub.SetMenuDefaultItem(ImageServer ? 
			(IDC_IMAGESERVER_FIRST_ID + _EngineList->GetUploadEngineIndex(Utf8ToWCstring(sessionImageServer_.serverName()))) :
			(IDC_FILESERVER_FIRST_ID+_EngineList->GetUploadEngineIndex(Utf8ToWCstring(sessionFileServer_.serverName()))),FALSE);
	}
	else
	{
		std::map <std::string, ServerSettingsStruct>& serverUsers = Settings.ServersSettings[serverProfile.serverName()];
	
		if((serverUsers.size()>1 || serverUsers.find("") == serverUsers.end()) )
		{	
			bool addedSeparator = false;
			CScriptUploadEngine *plug = uploadEngineManager_->getScriptUploadEngine(serverProfile);
			/*if(!plug) return TBDDRET_TREATPRESSED;*/

			int i =0;
			//ShowVar((int)serverUsers.size() );
			if ( serverUsers.size() && !serverProfile.profileName().empty() ) {
				mi.wID = IDC_LOGINTOOLBUTTON + (int)ImageServer;
 				mi.dwTypeData  = TR("�������� ������ ������� ������");
				sub.InsertMenuItem(i++, true, &mi);
			} else {
				addedSeparator = true;
			}

			menuOpenedUserNames_.clear();
			menuOpenedIsImageServer_ = ImageServer;

			if(plug && plug->supportsSettings()) {
   
				mi.wID = IDC_SERVERPARAMS + (int)ImageServer;
 				mi.dwTypeData  = TR("��������� �������...");
				sub.InsertMenuItem(i++, true, &mi);
			}
			int command = IDC_USERNAME_FIRST_ID;
			HICON userIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICONUSER));

			for( std::map <std::string, ServerSettingsStruct>::iterator it = serverUsers.begin(); it!= serverUsers.end(); ++it ) {
				//	CString login = Utf8ToWCstring(it->second.authData.Login);
				CString login = Utf8ToWCstring(it->first);
				if (!login.IsEmpty() )/*&& it->second.authData.DoAuth**/ {
					if ( !addedSeparator ) {
						ZeroMemory(&mi,sizeof(mi));
						mi.cbSize = sizeof(mi);
						mi.fMask = MIIM_TYPE|MIIM_ID;
						mi.wID = IDC_FILESERVER_LAST_ID + 1;
						mi.fType = MFT_SEPARATOR;
		
						sub.InsertMenuItem(i++, true, &mi);
						addedSeparator =  true;
					}
					ZeroMemory(&mi,sizeof(mi));
					mi.cbSize = sizeof(mi);
				
					mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
					mi.fType = MFT_STRING;
					mi.wID = command;

					mi.dwTypeData  = (LPWSTR)(LPCTSTR)login;
					HBITMAP bm = 0;
					if (WinUtils::IsVista() ) {
						bm = iconBitmapUtils_->HIconToBitmapPARGB32(userIcon);
						bitmaps.push_back(bm);
					}

					mi.hbmpItem =  WinUtils::IsVista() ? bm: HBMMENU_CALLBACK;
					if ( mi.hbmpItem ) {
						mi.fMask |= MIIM_BITMAP;
					}
					menuOpenedUserNames_.push_back(login);
					sub.InsertMenuItem(i++, true, &mi);
					command++;
				}
				
			}
			if ( uploadEngine->NeedAuthorization != CUploadEngineData::naObligatory ) {
				ZeroMemory(&mi,sizeof(mi));
				mi.cbSize = sizeof(mi);
				mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
				mi.fType = MFT_STRING;
				mi.wID = IDC_NO_ACCOUNT + !ImageServer;

				mi.dwTypeData  = (LPWSTR)(LPCTSTR)TR("<��� �����������>");
				sub.InsertMenuItem(i++, true, &mi);
			}


			
			ZeroMemory(&mi,sizeof(mi));
			mi.cbSize = sizeof(mi);
			mi.fMask = MIIM_TYPE|MIIM_ID;
			mi.wID = IDC_FILESERVER_LAST_ID + 1;
			mi.fType = MFT_SEPARATOR;


			sub.InsertMenuItem(i++, true, &mi);


			ZeroMemory(&mi,sizeof(mi));
			mi.cbSize = sizeof(mi);
			mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
			mi.fType = MFT_STRING;
			mi.wID = IDC_ADD_ACCOUNT + !ImageServer;

			mi.dwTypeData  = (LPWSTR)(LPCTSTR)TR("�������� ������� ������...");

			
			sub.InsertMenuItem(i++, true, &mi);




			sub.SetMenuDefaultItem(0,TRUE);
		}
		else
		{
			return TBDDRET_TREATPRESSED;
		}
	}
		
	RECT rc;
	::SendMessage(CurrentToolbar.m_hWnd,TB_GETRECT, pnmtb->iItem, (LPARAM)&rc);
	CurrentToolbar.ClientToScreen(&rc);
	TPMPARAMS excludeArea;
	ZeroMemory(&excludeArea, sizeof(excludeArea));
	excludeArea.cbSize = sizeof(excludeArea);
	excludeArea.rcExclude = rc;
	sub.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd, &excludeArea);
	bHandled = true;
	for ( int i = 0; i < bitmaps.size(); i++ ) {
		DeleteObject(bitmaps[i]);
	}
	return TBDDRET_DEFAULT;
}

LRESULT CUploadSettings::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwnd = (HWND) wParam;
	int xPos = LOWORD(lParam); 
	int yPos = HIWORD(lParam); 

	RECT rc;
	POINT pt = {xPos, yPos};

	if(hwnd == Toolbar.m_hWnd || hwnd == FileServerSelectBar.m_hWnd)
	{
		bool ImageServer = (hwnd == Toolbar.m_hWnd);
		CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;

		::SendMessage(CurrentToolbar.m_hWnd,TB_GETRECT, IDC_SERVERBUTTON, (LPARAM)&rc);
		CurrentToolbar.ClientToScreen(&rc);
		if(PtInRect(&rc, pt))
		{
			OnServerButtonContextMenu(pt, ImageServer);
			return 0;
		}
		if(!CurrentToolbar.IsButtonHidden(IDC_SELECTFOLDER))
		{
			::SendMessage(CurrentToolbar.m_hWnd,TB_GETRECT, IDC_SELECTFOLDER, (LPARAM)&rc);
			CurrentToolbar.ClientToScreen(&rc);
			if(PtInRect(&rc, pt))
			{
				OnFolderButtonContextMenu(pt, ImageServer);
				return 0;
			}
		}	
	}
	return 0;
}

void CUploadSettings::OnFolderButtonContextMenu(POINT pt, bool isImageServerToolbar)
{
	CMenu sub;	
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);	
	mi.fMask = MIIM_TYPE | MIIM_ID;
	mi.fType = MFT_STRING;

	sub.CreatePopupMenu();
	mi.wID = IDC_NEWFOLDER + (int)isImageServerToolbar;
	mi.dwTypeData  = TR("����� �����");
	sub.InsertMenuItem(0, true, &mi);

	mi.wID = IDC_OPENINBROWSER + (int)isImageServerToolbar;
	mi.dwTypeData  = TR("������� � ��������");
	sub.InsertMenuItem(1, true, &mi);
			
	sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd);
}

LRESULT CUploadSettings::OnNewFolder(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;
	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;
	ServerProfile & serverProfile = ImageServer ? sessionImageServer_ : sessionFileServer_;

	
	CUploadEngineData *ue = serverProfile.uploadEngineData();

	CScriptUploadEngine *m_pluginLoader = uploadEngineManager_->getScriptUploadEngine(serverProfile);
	if(!m_pluginLoader) return 0;

	CString title;
	std::vector<std::string> accessTypeList;

	m_pluginLoader->getAccessTypeList(accessTypeList);
	CFolderItem newFolder;
	if(serverProfile.folderId()	== IU_NEWFOLDERMARK)
		newFolder = serverProfile.serverSettings().newFolder;

	 CNewFolderDlg dlg(newFolder, true, accessTypeList);
	 if(dlg.DoModal(m_hWnd) == IDOK)
	 {
		 serverProfile.setFolderTitle(newFolder.title.c_str());
		 serverProfile.setFolderId(IU_NEWFOLDERMARK);
	     serverProfile.setFolderUrl("");

		
		serverProfile.serverSettings().newFolder = newFolder;
		UpdateAllPlaceSelectors();
	 }
	 return 0;
}
	
LRESULT CUploadSettings::OnOpenInBrowser(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;
	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;
//	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;
	ServerProfile & serverProfile = ImageServer? sessionImageServer_ : sessionFileServer_;
	CUploadEngineData *ue = serverProfile.uploadEngineData();


	CString str = Utf8ToWCstring(serverProfile.serverSettings().params["FolderUrl"]);
	if(!str.IsEmpty())
	{
		ShellExecute(0,_T("open"),str,_T(""),0,SW_SHOWNORMAL);
	}
	return 0;
}
	
void CUploadSettings::OnServerButtonContextMenu(POINT pt, bool isImageServerToolbar)
{
	ServerProfile & serverProfile = isImageServerToolbar? sessionImageServer_ : sessionFileServer_;
	if ( serverProfile.isNull() ) {
		return ;
	}
	CMenu sub;	
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);	
	mi.fMask = MIIM_TYPE | MIIM_ID;
	mi.fType = MFT_STRING;
	sub.CreatePopupMenu();
	mi.wID = IDC_SERVERPARAMS + (int)isImageServerToolbar;
	mi.dwTypeData  = TR("��������� �������");
	sub.InsertMenuItem(0, true, &mi);
	if(!serverProfile.uploadEngineData()->RegistrationUrl.empty())
	{
		mi.wID = IDC_OPENREGISTERURL + (int)isImageServerToolbar;
		mi.dwTypeData  = TR("������� �������� �����������");
		sub.InsertMenuItem(1, true, &mi);
	}
	sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd);
}

LRESULT CUploadSettings::OnServerParamsClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;
	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;

	ServerProfile& serverProfile = ImageServer ? sessionImageServer_ : sessionFileServer_;
	CUploadEngineData *ue = serverProfile.uploadEngineData();
	if(!ue->UsingPlugin) return false;

	CServerParamsDlg dlg(serverProfile, uploadEngineManager_);
	if ( dlg.DoModal() == IDOK) {
		serverProfile = dlg.serverProfile();
		UpdateAllPlaceSelectors();
	}
	return 0;
}

LRESULT CUploadSettings::OnOpenSignupPage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;
	ServerProfile & serverProfile = ImageServer? sessionImageServer_ : sessionFileServer_;

	CUploadEngineData *ue = serverProfile.uploadEngineData();
	if(ue && !ue->RegistrationUrl.empty())
		ShellExecute(0,_T("open"), Utf8ToWCstring(ue->RegistrationUrl), _T(""), 0, SW_SHOWNORMAL);
	return 0;
}

LRESULT CUploadSettings::OnResizePresetButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
   RECT rc;
   ::GetWindowRect(hWndCtl, &rc );
   POINT menuOrigin = {rc.left,rc.bottom};

	CMenu FolderMenu;
   int id =  IDC_RESIZEPRESETMENU_FIRST_ID;
	FolderMenu.CreatePopupMenu();
   FolderMenu.AppendMenu(MF_STRING, id++, TR("��� ���������"));
   FolderMenu.AppendMenu(MF_SEPARATOR, -1, _T(""));
   FolderMenu.AppendMenu(MF_STRING, id++, _T("800\u00D7600"));
	FolderMenu.AppendMenu(MF_STRING, id++, _T("1024\u00D7768"));
   FolderMenu.AppendMenu(MF_STRING, id++, _T("1600\u00D71200"));
   FolderMenu.AppendMenu(MF_SEPARATOR, -1,  _T(""));
     FolderMenu.AppendMenu(MF_STRING, id++, _T("25%"));
      FolderMenu.AppendMenu(MF_STRING, id++, _T("50%"));
FolderMenu.AppendMenu(MF_STRING, id++, _T("75%"));
	FolderMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, menuOrigin.x, menuOrigin.y, m_hWnd);
	
   return 0;
}

LRESULT CUploadSettings::OnShorteningUrlServerButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	CServerSelectorControl serverSelectorControl(uploadEngineManager_, false, false);
	serverSelectorControl.setServersMask(CServerSelectorControl::smUrlShorteners);
	serverSelectorControl.setShowImageProcessingParamsLink(false);
	serverSelectorControl.setTitle(TR("������ ��� ���������� ������"));
	serverSelectorControl.setServerProfile(Settings.urlShorteningServer);
	RECT clientRect;
	m_ShorteningServerButton.GetClientRect(&clientRect);
	m_ShorteningServerButton.ClientToScreen(&clientRect);
	POINT pt = { clientRect.left, clientRect.bottom };
	serverSelectorControl.OnChange.bind(this, &CUploadSettings::shorteningUrlServerChanged);
	serverSelectorControl.showPopup(m_hWnd, pt);
	Settings.urlShorteningServer = serverSelectorControl.serverProfile();
	updateUrlShorteningCheckboxLabel();
	return 0;
}

LRESULT CUploadSettings::OnResizePresetMenuItemClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  struct resize_preset
 {
    
      char *width;
      char * height;
   };
 
   resize_preset p[] =  {{"", ""},{"800", "600"}, {"1024","768"},
   {"1600","1200"},
    {"25%","25%"},
    {"50%","50%"},
    {"75%","75%"}};

   int presetIndex = wID - IDC_RESIZEPRESETMENU_FIRST_ID;
   int presetCount = sizeof(p)/sizeof(p[0]);
   if(presetIndex > presetCount -1) 
      return 0;
   ::SetDlgItemTextA(m_hWnd, IDC_IMAGEWIDTH,(p[presetIndex].width));
   ::SetDlgItemTextA(m_hWnd, IDC_IMAGEHEIGHT, (p[presetIndex].height));
   return 0;
}
LRESULT CUploadSettings::OnEditProfileClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
   SaveCurrentProfile();
   CSettingsDlg dlg(CSettingsDlg::spImages, uploadEngineManager_);
	dlg.DoModal(m_hWnd);
   CurrentProfileName = "";
   ShowParams(sessionImageServer_.getImageUploadParamsRef().ImageProfileName);
   UpdateProfileList();
   ShowParams(sessionImageServer_.getImageUploadParamsRef().ImageProfileName);
   return 0;
}

 void CUploadSettings::UpdateProfileList()
 {
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_RESETCONTENT);
    std::map<CString, ImageConvertingParams> ::const_iterator it;
    bool found = false;
    for(it = �onvert_profiles_.begin(); it!=�onvert_profiles_.end(); ++it)
    {
      GuiTools::AddComboBoxItem(m_hWnd, IDC_PROFILECOMBO, it->first);
      if(it->first == CurrentProfileName) found = true;
    }
    if(!found) GuiTools::AddComboBoxItem(m_hWnd, IDC_PROFILECOMBO, CurrentProfileName);
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_SELECTSTRING, -1,(LPARAM)(LPCTSTR) CurrentProfileName); 
 }

 void CUploadSettings::selectServer(ServerProfile& sp, int serverIndex)
 {
	 sp.setServerName(_EngineList->byIndex(serverIndex)->Name);
	 std::map <std::string, ServerSettingsStruct>& serverSettings = Settings.ServersSettings[sp.serverName()];
	 std::map <std::string, ServerSettingsStruct>::iterator firstAccount = serverSettings.begin();
	 if ( firstAccount != serverSettings.end() ) {
		 if ( firstAccount->first == "" ) {
			 ++firstAccount;
		 }
		 if ( firstAccount != serverSettings.end() ) {
			 sp.setProfileName(firstAccount->first);
		 }
	 } else {
		 sp.setProfileName("");
	 }
	 ServerSettingsStruct &ss =  sp.serverSettings();
	 sp.setFolderId(ss.defaultFolder.getId());
	 sp.setFolderTitle(ss.defaultFolder.getTitle());
	 sp.setFolderUrl(ss.defaultFolder.viewUrl);
 }

void CUploadSettings::updateUrlShorteningCheckboxLabel()
{
	CString text;
	CString serverName = Utf8ToWCstring(Settings.urlShorteningServer.serverName());
	text.Format(TR("��������� ������ � ������� %s"), static_cast<LPCTSTR>(serverName));
	SetDlgItemText(IDC_SHORTENLINKSCHECKBOX, text);
}

void CUploadSettings::shorteningUrlServerChanged(CServerSelectorControl* serverSelectorControl)
{
	Settings.urlShorteningServer = serverSelectorControl->serverProfile();
	updateUrlShorteningCheckboxLabel();
}

void CUploadSettings::ShowParams(const ImageConvertingParams& params)
 {
   m_ProfileChanged = false;
   m_CatchChanges = false;
   if(params.Quality)
		SetDlgItemInt(IDC_QUALITYEDIT,params.Quality);
	else
		SetDlgItemText(IDC_QUALITYEDIT,_T(""));
   
   SendDlgItemMessage(IDC_FORMATLIST,CB_SETCURSEL, params.Format);
   SendDlgItemMessage(IDC_YOURLOGO,BM_SETCHECK,  params.AddLogo);
   SendDlgItemMessage(IDC_YOURTEXT,BM_SETCHECK,  params.AddText);
   SetDlgItemText(IDC_IMAGEWIDTH,params.strNewWidth);
	SetDlgItemText(IDC_IMAGEHEIGHT,params.strNewHeight);
   m_ProfileChanged = false;
    m_CatchChanges = true;
 }

 void CUploadSettings::ShowParams(const CString profileName)
 {
    if(sessionImageServer_.getImageUploadParams().getThumb().ResizeMode!= ThumbCreatingParams::trByHeight)
   {
      TRC(IDC_WIDTHLABEL,"������:");
      SetDlgItemInt(IDC_THUMBWIDTH,sessionImageServer_.getImageUploadParams().getThumb().Size);
   }
   else
   {
      TRC(IDC_WIDTHLABEL,"������:");
      SetDlgItemInt(IDC_THUMBWIDTH,sessionImageServer_.getImageUploadParams().getThumb().Size);
   }
    if(CurrentProfileName == profileName) return;
   CurrentProfileName = profileName;
   CurrentProfileOriginalName = profileName; 
   ShowParams(�onvert_profiles_[profileName]);
   
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_SELECTSTRING, -1,(LPARAM)(LPCTSTR) profileName); 
 }

 bool CUploadSettings::SaveParams(ImageConvertingParams& params)
 {
   params.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	params.Format = SendDlgItemMessage(IDC_FORMATLIST, CB_GETCURSEL);

   params.strNewWidth = GuiTools::GetWindowText( GetDlgItem(IDC_IMAGEWIDTH));

  params.strNewHeight =  GuiTools::GetWindowText( GetDlgItem(IDC_IMAGEHEIGHT));
   return true;
 }

 void  CUploadSettings::ProfileChanged()
 {
if(!m_CatchChanges) return;
    if(!m_ProfileChanged)
    {
       CurrentProfileOriginalName = CurrentProfileName;
       CurrentProfileName.Replace(CString(_T(" "))+TR("(�������)"), _T(""));
      CurrentProfileName = CurrentProfileName + _T(" ")+ TR("(�������)");
     // ::SetWindowText(GetDlgItem(IDC_PROFILECOMBO), CurrentProfileName);
        m_ProfileChanged = true;
        UpdateProfileList();
    }
 }

LRESULT CUploadSettings::OnProfileEditedCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   ProfileChanged();
   return 0;
}

LRESULT CUploadSettings::OnUserNameMenuItemClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int userNameIndex = wID - IDC_USERNAME_FIRST_ID;
	CString userName = menuOpenedUserNames_[userNameIndex];
	bool ImageServer = menuOpenedIsImageServer_;
	ServerProfile & serverProfile = ImageServer? sessionImageServer_ : sessionFileServer_;
	serverProfile.setProfileName(WCstringToUtf8(userName));
	ServerSettingsStruct& sss = serverProfile.serverSettings();

	if ( ImageServer ) {
		imageServerLogin_ = WCstringToUtf8(userName);
	} else {
		fileServerLogin_ =  WCstringToUtf8(userName);
	}
	serverProfile.setFolderId(sss.defaultFolder.getId());
	serverProfile.setFolderTitle(sss.defaultFolder.getTitle());
	serverProfile.setFolderUrl(sss.defaultFolder.viewUrl);

	/*if(UserName != ss.authData.Login || ss.authData.DoAuth!=prevAuthEnabled)
	{
		serverProfile.setFolderId("");
		serverProfile.setFolderTitle("");
		serverProfile.setFolderUrl("");
		iuPluginManager.UnloadPlugins();
		m_EngineList->DestroyCachedEngine(m_EngineList->byIndex(nServerIndex)->Name);
	}*/

	UpdateAllPlaceSelectors();

//	MessageBox();
	return 0;
}

LRESULT CUploadSettings::OnAddAccountClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bool ImageServer = (wID % 2)!=0;

	ServerProfile & serverProfile = ImageServer? sessionImageServer_ : sessionFileServer_;
	ServerProfile serverProfileCopy = serverProfile;
	serverProfileCopy.setProfileName("");
	CLoginDlg dlg(serverProfileCopy, uploadEngineManager_, true);


	ServerSettingsStruct & ss = ImageServer ? sessionImageServer_.serverSettings() : sessionFileServer_.serverSettings();
	if( dlg.DoModal(m_hWnd) == IDOK)
	{
		
		if ( ImageServer ) {
			imageServerLogin_ = WCstringToUtf8(dlg.accountName());
		} else {
			fileServerLogin_ =  WCstringToUtf8(dlg.accountName());
		}
		serverProfileCopy.setProfileName(WCstringToUtf8(dlg.accountName()));
			serverProfileCopy.setFolderId("");
			serverProfileCopy.setFolderTitle("");
			serverProfileCopy.setFolderUrl("");
		
			serverProfile = serverProfileCopy;
		UpdateAllPlaceSelectors();
	}
	return 0;
}

LRESULT CUploadSettings::OnNoAccountClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bool ImageServer = (wID % 2)!=0;
	ServerProfile & serverProfile = ImageServer? sessionImageServer_ : sessionFileServer_;
	serverProfile.setProfileName("");
	serverProfile.setFolderId("");
	serverProfile.setFolderTitle("");
	serverProfile.setFolderUrl("");
	UpdateAllPlaceSelectors();
	return 0;
}

void CUploadSettings::SaveCurrentProfile()
{
     CString saveToProfile = CurrentProfileName;
   if(CurrentProfileOriginalName == _T("Default"))
saveToProfile = CurrentProfileOriginalName;

   if(!SaveParams(�onvert_profiles_[saveToProfile]))
      return;

   sessionImageServer_.getImageUploadParamsRef().ImageProfileName = saveToProfile;
}

bool  CUploadSettings::OnHide()
{
   SaveCurrentProfile();
   return true;
}
LRESULT CUploadSettings::OnProfileComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    CString profile = GuiTools::GetWindowText(GetDlgItem(IDC_PROFILECOMBO));

    ShowParams(profile);
    UpdateProfileList();
   return 0;
}


LRESULT CUploadSettings::OnAddFtpServer(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	CAddFtpServerDialog dlg(m_EngineList);
	if ( dlg.DoModal(m_hWnd) == IDOK ) {
			if ( wID == IDC_ADD_FTP_SERVER ) {
				sessionImageServer_.setServerName(WCstringToUtf8(dlg.createdServerName()));
				sessionImageServer_.setProfileName(WCstringToUtf8(dlg.createdServerLogin()));
			} else {
				sessionFileServer_.setServerName(WCstringToUtf8(dlg.createdServerName()));
				sessionFileServer_.setProfileName(WCstringToUtf8(dlg.createdServerLogin()));
			}
		
			UpdateAllPlaceSelectors();
	}
	return 0;
}

LRESULT CUploadSettings::OnAddDirectoryAsServer(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	CAddDirectoryServerDialog dlg(m_EngineList);
	if ( dlg.DoModal(m_hWnd) == IDOK ) {
			if ( wID == IDC_ADD_DIRECTORY_AS_SERVER ) {
				sessionImageServer_.setServerName(WCstringToUtf8(dlg.createdServerName()));
				sessionImageServer_.setProfileName("");
				sessionImageServer_.clearFolderInfo();
			} else {
				sessionFileServer_.setServerName(WCstringToUtf8(dlg.createdServerName()));
				sessionFileServer_.setProfileName("");
				sessionFileServer_.clearFolderInfo();
			}

			UpdateAllPlaceSelectors();

	}
	return 0;
}
