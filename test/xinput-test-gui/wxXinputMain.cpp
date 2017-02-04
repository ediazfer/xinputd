/*
 * MIT License
 *
 * Unix XInput Gamepad interface implementation
 *
 * Copyright (c) 2016 Eric Diaz Fernandez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// gui generated using wxsmith

#include "wxXinputMain.h"
#include <wx/msgdlg.h>

#include <xinput.h>
#include <wx/stdpaths.h>
//(*InternalHeaders(wxXinputDialog)
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
//*)

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(wxXinputDialog)
const long wxXinputDialog::ID_BACKGROUND = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPA = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPX = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPB = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPY = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPSELECT = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPSTART = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPLB = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPRB = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPGUIDE = wxNewId();
const long wxXinputDialog::ID_GAUGERSX = wxNewId();
const long wxXinputDialog::ID_GAUGELSX = wxNewId();
const long wxXinputDialog::ID_GAUGERSY = wxNewId();
const long wxXinputDialog::ID_GAUGELSY = wxNewId();
const long wxXinputDialog::ID_GAUGELT = wxNewId();
const long wxXinputDialog::ID_GAUGERT = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPPADUR = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPPADDR = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPPADDL = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPPADUL = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPPADU = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPPADR = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPPADD = wxNewId();
const long wxXinputDialog::ID_STATICBITMAPPADL = wxNewId();
const long wxXinputDialog::ID_GAMEPADLISTBOX = wxNewId();
const long wxXinputDialog::ID_POLLINGTIMER = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxXinputDialog,wxDialog)
    //(*EventTable(wxXinputDialog)
    //*)
END_EVENT_TABLE()

wxXinputDialog::wxXinputDialog(wxWindow* parent,wxWindowID id)
{
    _pad_mask = 0;
    _pad_selected = -1;
    
    wxString cwd = wxGetCwd();
    wxString dir = wxStandardPaths::Get().GetInstallPrefix().Append("/share/xinput");
    wxSetWorkingDirectory(dir);
    
    //(*Initialize(wxXinputDialog)
    Create(parent, wxID_ANY, _("xinput GUI test"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(938,590));
    gamepadBackground = new wxStaticBitmap(this, ID_BACKGROUND, wxBitmap(wxImage(_T("xinputcontroller.png"))), wxPoint(0,0), wxDefaultSize, wxSIMPLE_BORDER, _T("ID_BACKGROUND"));
    greenButton = new wxStaticBitmap(this, ID_STATICBITMAPA, wxBitmap(wxImage(_T("xinputcontrollerbutton-a.png")).Rescale(wxSize(50,52).GetWidth(),wxSize(50,52).GetHeight())), wxPoint(741,289), wxSize(50,52), wxSIMPLE_BORDER, _T("ID_STATICBITMAPA"));
    blueButton = new wxStaticBitmap(this, ID_STATICBITMAPX, wxBitmap(wxImage(_T("xinputcontrollerbutton-x.png")).Rescale(wxSize(50,52).GetWidth(),wxSize(50,52).GetHeight())), wxPoint(690,240), wxSize(50,52), wxSIMPLE_BORDER, _T("ID_STATICBITMAPX"));
    redButton = new wxStaticBitmap(this, ID_STATICBITMAPB, wxBitmap(wxImage(_T("xinputcontrollerbutton-b.png")).Rescale(wxSize(50,52).GetWidth(),wxSize(50,52).GetHeight())), wxPoint(790,238), wxSize(50,52), wxSIMPLE_BORDER, _T("ID_STATICBITMAPB"));
    yellowButton = new wxStaticBitmap(this, ID_STATICBITMAPY, wxBitmap(wxImage(_T("xinputcontrollerbutton-y.png")).Rescale(wxSize(50,52).GetWidth(),wxSize(50,52).GetHeight())), wxPoint(738,187), wxSize(50,52), wxSIMPLE_BORDER, _T("ID_STATICBITMAPY"));
    selectButton = new wxStaticBitmap(this, ID_STATICBITMAPSELECT, wxBitmap(wxImage(_T("xinputcontrollerbutton-select.png")).Rescale(wxSize(64,41).GetWidth(),wxSize(64,41).GetHeight())), wxPoint(343,258), wxSize(64,41), wxSIMPLE_BORDER, _T("ID_STATICBITMAPSELECT"));
    startButton = new wxStaticBitmap(this, ID_STATICBITMAPSTART, wxBitmap(wxImage(_T("xinputcontrollerbutton-start.png")).Rescale(wxSize(64,41).GetWidth(),wxSize(64,41).GetHeight())), wxPoint(570,254), wxSize(64,41), wxSIMPLE_BORDER, _T("ID_STATICBITMAPSTART"));
    leftButton = new wxStaticBitmap(this, ID_STATICBITMAPLB, wxBitmap(wxImage(_T("xinputcontrollerbutton-lb.png")).Rescale(wxSize(151,35).GetWidth(),wxSize(151,35).GetHeight())), wxPoint(105,123), wxSize(151,35), wxSIMPLE_BORDER, _T("ID_STATICBITMAPLB"));
    rightButton = new wxStaticBitmap(this, ID_STATICBITMAPRB, wxBitmap(wxImage(_T("xinputcontrollerbutton-rb.png")).Rescale(wxSize(151,36).GetWidth(),wxSize(151,36).GetHeight())), wxPoint(685,121), wxSize(151,36), wxSIMPLE_BORDER, _T("ID_STATICBITMAPRB"));
    guideButton = new wxStaticBitmap(this, ID_STATICBITMAPGUIDE, wxBitmap(wxImage(_T("xinputcontrollerbutton-guide.png")).Rescale(wxSize(102,97).GetWidth(),wxSize(102,97).GetHeight())), wxPoint(436,192), wxSize(102,97), wxSIMPLE_BORDER, _T("ID_STATICBITMAPGUIDE"));
    rightStickXGauge = new wxGauge(this, ID_GAUGERSX, 100, wxPoint(496,496), wxSize(100,16), 0, wxDefaultValidator, _T("ID_GAUGERSX"));
    leftStickXGauge = new wxGauge(this, ID_GAUGELSX, 100, wxPoint(64,320), wxSize(100,16), 0, wxDefaultValidator, _T("ID_GAUGELSX"));
    rightStickYGauge = new wxGauge(this, ID_GAUGERSY, 100, wxPoint(536,392), wxSize(16,100), wxGA_VERTICAL, wxDefaultValidator, _T("ID_GAUGERSY"));
    leftStickYGauge = new wxGauge(this, ID_GAUGELSY, 100, wxPoint(104,216), wxSize(16,100), wxGA_VERTICAL, wxDefaultValidator, _T("ID_GAUGELSY"));
    leftTriggerGauge = new wxGauge(this, ID_GAUGELT, 100, wxPoint(194,40), wxSize(28,76), wxGA_VERTICAL, wxDefaultValidator, _T("ID_GAUGELT"));
    rightTriggerGauge = new wxGauge(this, ID_GAUGERT, 100, wxPoint(718,40), wxSize(28,76), wxGA_VERTICAL, wxDefaultValidator, _T("ID_GAUGERT"));
    padUpRight = new wxStaticBitmap(this, ID_STATICBITMAPPADUR, wxBitmap(wxImage(_T("xinputcontrollerpad-ur.png")).Rescale(wxSize(71,70).GetWidth(),wxSize(71,70).GetHeight())), wxPoint(370,350), wxSize(71,70), wxSIMPLE_BORDER, _T("ID_STATICBITMAPPADUR"));
    padDownRight = new wxStaticBitmap(this, ID_STATICBITMAPPADDR, wxBitmap(wxImage(_T("xinputcontrollerpad-dr.png")).Rescale(wxSize(72,70).GetWidth(),wxSize(72,70).GetHeight())), wxPoint(370,462), wxSize(72,70), wxSIMPLE_BORDER, _T("ID_STATICBITMAPPADDR"));
    padDownLeft = new wxStaticBitmap(this, ID_STATICBITMAPPADDL, wxBitmap(wxImage(_T("xinputcontrollerpad-dl.png")).Rescale(wxSize(68,72).GetWidth(),wxSize(68,72).GetHeight())), wxPoint(262,457), wxSize(68,72), wxSIMPLE_BORDER, _T("ID_STATICBITMAPPADDL"));
    padUpLeft = new wxStaticBitmap(this, ID_STATICBITMAPPADUL, wxBitmap(wxImage(_T("xinputcontrollerpad-ul.png")).Rescale(wxSize(67,72).GetWidth(),wxSize(67,72).GetHeight())), wxPoint(260,352), wxSize(67,72), wxSIMPLE_BORDER, _T("ID_STATICBITMAPPADUL"));
    padUp = new wxStaticBitmap(this, ID_STATICBITMAPPADU, wxBitmap(wxImage(_T("xinputcontrollerpad-u.png")).Rescale(wxSize(57,62).GetWidth(),wxSize(57,62).GetHeight())), wxPoint(322,348), wxSize(57,62), wxSIMPLE_BORDER, _T("ID_STATICBITMAPPADU"));
    padRight = new wxStaticBitmap(this, ID_STATICBITMAPPADR, wxBitmap(wxImage(_T("xinputcontrollerpad-r.png")).Rescale(wxSize(62,55).GetWidth(),wxSize(62,55).GetHeight())), wxPoint(382,414), wxSize(62,55), wxSIMPLE_BORDER, _T("ID_STATICBITMAPPADR"));
    padDown = new wxStaticBitmap(this, ID_STATICBITMAPPADD, wxBitmap(wxImage(_T("xinputcontrollerpad-d.png")).Rescale(wxSize(54,61).GetWidth(),wxSize(54,61).GetHeight())), wxPoint(324,470), wxSize(54,61), wxSIMPLE_BORDER, _T("ID_STATICBITMAPPADD"));
    padLeft = new wxStaticBitmap(this, ID_STATICBITMAPPADL, wxBitmap(wxImage(_T("xinputcontrollerpad-l.png")).Rescale(wxSize(61,54).GetWidth(),wxSize(61,54).GetHeight())), wxPoint(260,414), wxSize(61,54), wxSIMPLE_BORDER, _T("ID_STATICBITMAPPADL"));
    GamepadListBox = new wxListBox(this, ID_GAMEPADLISTBOX, wxPoint(392,24), wxSize(104,104), 0, 0, 0, wxDefaultValidator, _T("ID_GAMEPADLISTBOX"));
    pollingTimer.SetOwner(this, ID_POLLINGTIMER);
    Connect(ID_GAMEPADLISTBOX,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&wxXinputDialog::OnGamepadListBoxSelect);
    Connect(ID_POLLINGTIMER,wxEVT_TIMER,(wxObjectEventFunction)&wxXinputDialog::OnPollingTimerTrigger);
    //*)

    pollingTimer.Start(100, false);
    wxSetWorkingDirectory(cwd);
}

wxXinputDialog::~wxXinputDialog()
{
    //(*Destroy(wxXinputDialog)
    //*)
}

void wxXinputDialog::OnQuit(wxCommandEvent& event)
{
    Close();
}

void wxXinputDialog::OnAbout(wxCommandEvent& event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("xinput GUI test"));
}

void wxXinputDialog::OnPollingTimerTrigger(wxTimerEvent& event)
{
    XINPUT_CAPABILITIES caps;
    XINPUT_STATE_EX state;
    int pad_mask = 0;

    for(DWORD i = 0; i < XUSER_MAX_COUNT; ++i )
    {
        DWORD ret = XInputGetCapabilities(i, 0, &caps);
        switch(ret)
        {
            case ERROR_SUCCESS:
            {
                // device is on
                pad_mask |= 1 << i;
                break;
            }
            case ERROR_DEVICE_NOT_CONNECTED:
            {
                // disable device
                break;
            }
            default:
            {
                // oops ?
                break;
            }
        }
    }
    
    if(pad_mask == 0)
    {
        _pad_selected = -1;
    }

    if(_pad_mask != pad_mask)
    {
        GamepadListBox->Clear();
        for(DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
        {
            if(pad_mask & (1 << i))
            {
                GamepadListBox->Append(_("Gamepad"));
                if(_pad_selected < 0)
                {
                    GamepadListBox->Select(0);
                    _pad_selected = 0;
                }
            }
        }
        GamepadListBox->Update();
        _pad_mask = pad_mask;
    }
    
    // yup, it's ugly.  I'll make it better later.
    
    if((_pad_selected >= 0) && (_pad_mask & (1 << _pad_selected)))
    {
        if(XInputGetStateEx(_pad_selected, &state) == ERROR_SUCCESS)
        {
            int value;

            value = state.Gamepad.bLeftTrigger;
            value *= 100;
            value /= 255;
            leftTriggerGauge->SetValue(value);

            value = state.Gamepad.bRightTrigger;
            value *= 100;
            value /= 255;
            rightTriggerGauge->SetValue(value);

            value = state.Gamepad.sThumbLX;
            value += 32768;
            value *= 100;
            value /= 65535;
            leftStickXGauge->SetValue(value);

            value = state.Gamepad.sThumbLY;
            value += 32768;
            value *= 100;
            value /= 65535;
            leftStickYGauge->SetValue(value);

            value = state.Gamepad.sThumbRX;
            value += 32768;
            value *= 100;
            value /= 65535;
            rightStickXGauge->SetValue(value);

            value = state.Gamepad.sThumbRY;
            value += 32768;
            value *= 100;
            value /= 65535;
            rightStickYGauge->SetValue(value);

            if(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
            {
                yellowButton->Show();
            }
            else
            {
                yellowButton->Hide();
            }

            if(state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
            {
                blueButton->Show();
            }
            else
            {
                blueButton->Hide();
            }

            if(state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
            {
                redButton->Show();
            }
            else
            {
                redButton->Hide();
            }

            if(state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
            {
                greenButton->Show();
            }
            else
            {
                greenButton->Hide();
            }

            if(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
            {
                leftButton->Show();
            }
            else
            {
                leftButton->Hide();
            }

            if(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
            {
                rightButton->Show();
            }
            else
            {
                rightButton->Hide();
            }

            if(state.Gamepad.wButtons & XINPUT_GAMEPAD_START)
            {
                startButton->Show();
            }
            else
            {
                startButton->Hide();
            }

            if(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
            {
                selectButton->Show();
            }
            else
            {
                selectButton->Hide();
            }

            if(state.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE)
            {
                guideButton->Show();
            }
            else
            {
                guideButton->Hide();
            }

            padUpRight->Hide();
            padDownRight->Hide();
            padDownLeft->Hide();
            padUpLeft->Hide();
            padUp->Hide();
            padRight->Hide();
            padDown->Hide();
            padLeft->Hide();

            switch(state.Gamepad.wButtons & (XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_RIGHT))
            {
                case XINPUT_GAMEPAD_DPAD_UP:
                {
                    padUp->Show();
                    break;
                }
                case XINPUT_GAMEPAD_DPAD_DOWN:
                {
                    padDown->Show();
                    break;
                }
                case XINPUT_GAMEPAD_DPAD_LEFT:
                {
                    padLeft->Show();
                    break;
                }
                case XINPUT_GAMEPAD_DPAD_RIGHT:
                {
                    padRight->Show();
                    break;
                }
                case XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_LEFT:
                {
                    padUpLeft->Show();
                    break;
                }
                case XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_RIGHT:
                {
                    padUpRight->Show();
                    break;
                }
                case XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_LEFT:
                {
                    padDownLeft->Show();
                    break;
                }
                case XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_RIGHT:
                {
                    padDownRight->Show();
                    break;
                }
            }
        }
    }
}

void wxXinputDialog::OnGamepadListBoxSelect(wxCommandEvent& event)
{
    int selected = GamepadListBox->GetSelection();
    if(selected >= 0 && selected < XUSER_MAX_COUNT)
    {
        _pad_selected = selected;
    }
}
