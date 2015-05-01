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

#include "DefaultServersSettings.h"

#include <uxtheme.h>
#include "Func/common.h"
#include "Func/Settings.h"
#include "LogWindow.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Gui/Controls/ServerSelectorControl.h"
#include "WizardDlg.h"

// CDefaultServersSettings
CDefaultServersSettings::CDefaultServersSettings(UploadEngineManager* uploadEngineManager)
{
    fileServerSelector_ = 0 ;
    imageServerSelector_ = 0;
    trayServerSelector_ = 0;
    contextMenuServerSelector_ = 0;
    urlShortenerServerSelector_ = 0;
    uploadEngineManager_ = uploadEngineManager;
}

CDefaultServersSettings::~CDefaultServersSettings()
{
    delete fileServerSelector_;
    delete imageServerSelector_;
    delete trayServerSelector_;
    delete contextMenuServerSelector_;
    delete urlShortenerServerSelector_;
}

LRESULT CDefaultServersSettings::OnServerListChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    imageServerSelector_->updateServerList();
    fileServerSelector_->updateServerList();
    trayServerSelector_->updateServerList();
    contextMenuServerSelector_->updateServerList();
    return 0;
}

LRESULT CDefaultServersSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TRC(IDC_REMEMBERIMAGESERVERSETTINGS, "���������� � ������� ��������� ������� ��� ��������");
    TRC(IDC_REMEMBERFILESERVERSETTINGS, "���������� � ������� ��������� ������� ��� ������ ����� ������");
    RECT serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_IMAGESERVERPLACEHOLDER);
    imageServerSelector_ = new CServerSelectorControl(uploadEngineManager_, true);
    if ( !imageServerSelector_->Create(m_hWnd, serverSelectorRect) ) {
        return 0;
    }
    imageServerSelector_->setTitle(TR("������ ��-��������� ��� �������� �����������"));
    imageServerSelector_->ShowWindow( SW_SHOW );
    imageServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
    imageServerSelector_->setServerProfile(Settings.imageServer);

    serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_FILESERVERPLACEHOLDER);

    fileServerSelector_ = new CServerSelectorControl(uploadEngineManager_);
    fileServerSelector_->setServersMask(CServerSelectorControl::smFileServers);
    fileServerSelector_->setShowImageProcessingParamsLink(false);
    fileServerSelector_->Create(m_hWnd, serverSelectorRect);
    fileServerSelector_->ShowWindow( SW_SHOW );
    fileServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
    fileServerSelector_->setServerProfile(Settings.fileServer);
    fileServerSelector_->setTitle(TR("������ ��-��������� ��� �������� ������ ����� ������"));

    serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_TRAYSERVERPLACEHOLDER);
    trayServerSelector_ = new CServerSelectorControl(uploadEngineManager_);
    //trayServerSelector_->setShowDefaultServerItem(true);
    trayServerSelector_->Create(m_hWnd, serverSelectorRect);
    trayServerSelector_->ShowWindow( SW_SHOW );
    trayServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);

    trayServerSelector_->setServerProfile(Settings.quickScreenshotServer);
    trayServerSelector_->setTitle(TR("������ ��� ������� �������� ����������"));

    serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_CONTEXTMENUSERVERPLACEHOLDER);


    contextMenuServerSelector_ = new CServerSelectorControl(uploadEngineManager_);
    //contextMenuServerSelector_->setShowDefaultServerItem(true);
    contextMenuServerSelector_->Create(m_hWnd, serverSelectorRect);
    contextMenuServerSelector_->ShowWindow( SW_SHOW );
    contextMenuServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);

    contextMenuServerSelector_->setServerProfile(Settings.contextMenuServer);
    contextMenuServerSelector_->setTitle(TR("������ ��� �������� �� ������������ ���� ����������"));


    serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_URLSHORTENERPLACEHOLDER);

    urlShortenerServerSelector_ = new CServerSelectorControl(uploadEngineManager_);
    urlShortenerServerSelector_->setServersMask(CServerSelectorControl::smUrlShorteners);
    urlShortenerServerSelector_->setShowImageProcessingParamsLink(false);
    urlShortenerServerSelector_->Create(m_hWnd, serverSelectorRect);
    urlShortenerServerSelector_->ShowWindow( SW_SHOW );
    urlShortenerServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
    urlShortenerServerSelector_->setServerProfile(Settings.urlShorteningServer);
    urlShortenerServerSelector_->setTitle(TR("������ ��� ���������� ������"));
    
    GuiTools::SetCheck(m_hWnd, IDC_REMEMBERIMAGESERVERSETTINGS, Settings.RememberImageServer);
    GuiTools::SetCheck(m_hWnd, IDC_REMEMBERFILESERVERSETTINGS, Settings.RememberFileServer);


    
    
    return 1;  // Let the system set the focus
}


    
bool CDefaultServersSettings::Apply()
{
    CServerSelectorControl* controls[] = { fileServerSelector_, imageServerSelector_, trayServerSelector_, contextMenuServerSelector_, urlShortenerServerSelector_ };
    for(int i = 0; i< ARRAY_SIZE(controls); i++ ) {
        if ( !controls[i]->serverProfile().serverName().empty() && !controls[i]->isAccountChosen() ) {
            CString message;
            message.Format(TR("�� �� ������� ������� ��� ������� \"%s\""), IuCoreUtils::Utf8ToWstring(controls[i]->serverProfile().serverName()).c_str());
            MessageBox(message, TR("������"));
            return 0;
        }
    }
    Settings.fileServer = fileServerSelector_->serverProfile();
    Settings.imageServer = imageServerSelector_->serverProfile();
    Settings.quickScreenshotServer = trayServerSelector_->serverProfile();
    Settings.contextMenuServer = contextMenuServerSelector_->serverProfile();
    Settings.urlShorteningServer = urlShortenerServerSelector_->serverProfile();
    Settings.RememberImageServer = GuiTools::GetCheck(m_hWnd, IDC_REMEMBERIMAGESERVERSETTINGS);
    Settings.RememberFileServer = GuiTools::GetCheck(m_hWnd, IDC_REMEMBERFILESERVERSETTINGS);
    pWizardDlg->setServersChanged(true);
    return true;
}

