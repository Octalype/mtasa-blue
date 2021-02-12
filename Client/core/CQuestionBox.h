/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  PURPOSE:     Quick connect window
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include <vector>

typedef void (*pfnQuestionCallback)(void*, unsigned int);
typedef void (*pfnQuestionEditCallback)(void*, unsigned int, std::string);

class CGUIEdit;
class CGUIElement;
class CGUIWindow;
class CGUILabel;
class CGUIButton;

class CQuestionBox
{
public:
    CQuestionBox();
    ~CQuestionBox();

    void         Hide();
    void         Show();
    void         Reset();
    void         SetTitle(const SString& strTitle);
    void         SetMessage(const SString& strMsg);
    void         AppendMessage(const SString& strMsg);
    void         SetButton(unsigned int uiButton, const SString& strText);
    CGUIEdit*    SetEditbox(unsigned int uiEditbox, const SString& strText);
    void         SetCallback(pfnQuestionCallback callback, void* ptr = NULL);
    void         SetCallbackEdit(pfnQuestionEditCallback callback, void* ptr = NULL);
    void         SetOnLineHelpOption(const SString& strTroubleType);
    unsigned int PollButtons();
    bool         IsVisible();
    void         SetAutoCloseOnConnect(bool bEnable);
    void         OnConnect();

private:
    bool OnButtonClick(CGUIElement* pElement);

    CGUIWindow*              m_pWindow;
    CGUILabel*               m_pMessage;
    std::vector<CGUIButton*> m_ButtonList;
    std::vector<CGUIEdit*>   m_EditList;
    unsigned int             m_uiLastButton;
    unsigned int             m_uiActiveButtons;
    unsigned int             m_uiActiveEditboxes;
    pfnQuestionCallback      m_Callback;
    pfnQuestionEditCallback  m_CallbackEdit;
    void*                    m_CallbackParameter;
    SString                  m_strMsg;
    bool                     m_bAutoCloseOnConnect;
};
