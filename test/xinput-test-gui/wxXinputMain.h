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

#ifndef WXXINPUTMAIN_H
#define WXXINPUTMAIN_H

//(*Headers(wxXinputDialog)
#include <wx/gauge.h>
#include <wx/dialog.h>
#include <wx/timer.h>
#include <wx/statbmp.h>
#include <wx/listbox.h>
//*)

class wxXinputDialog: public wxDialog
{
    public:

        wxXinputDialog(wxWindow* parent,wxWindowID id = -1);
        virtual ~wxXinputDialog();

    private:

        int _pad_mask;
        int _pad_selected;

        //(*Handlers(wxXinputDialog)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnPollingTimerTrigger(wxTimerEvent& event);
        void OnGamepadListBoxSelect(wxCommandEvent& event);
        //*)

        //(*Identifiers(wxXinputDialog)
        static const long ID_BACKGROUND;
        static const long ID_STATICBITMAPA;
        static const long ID_STATICBITMAPX;
        static const long ID_STATICBITMAPB;
        static const long ID_STATICBITMAPY;
        static const long ID_STATICBITMAPSELECT;
        static const long ID_STATICBITMAPSTART;
        static const long ID_STATICBITMAPLB;
        static const long ID_STATICBITMAPRB;
        static const long ID_STATICBITMAPGUIDE;
        static const long ID_GAUGERSX;
        static const long ID_GAUGELSX;
        static const long ID_GAUGERSY;
        static const long ID_GAUGELSY;
        static const long ID_GAUGELT;
        static const long ID_GAUGERT;
        static const long ID_STATICBITMAPPADUR;
        static const long ID_STATICBITMAPPADDR;
        static const long ID_STATICBITMAPPADDL;
        static const long ID_STATICBITMAPPADUL;
        static const long ID_STATICBITMAPPADU;
        static const long ID_STATICBITMAPPADR;
        static const long ID_STATICBITMAPPADD;
        static const long ID_STATICBITMAPPADL;
        static const long ID_GAMEPADLISTBOX;
        static const long ID_POLLINGTIMER;
        //*)

        //(*Declarations(wxXinputDialog)
        wxStaticBitmap* padDown;
        wxStaticBitmap* redButton;
        wxListBox* GamepadListBox;
        wxStaticBitmap* guideButton;
        wxStaticBitmap* leftButton;
        wxStaticBitmap* padUp;
        wxStaticBitmap* padRight;
        wxStaticBitmap* selectButton;
        wxGauge* rightTriggerGauge;
        wxGauge* rightStickXGauge;
        wxStaticBitmap* padLeft;
        wxStaticBitmap* padUpRight;
        wxGauge* leftStickXGauge;
        wxGauge* leftTriggerGauge;
        wxStaticBitmap* padUpLeft;
        wxStaticBitmap* gamepadBackground;
        wxStaticBitmap* padDownLeft;
        wxStaticBitmap* yellowButton;
        wxGauge* rightStickYGauge;
        wxStaticBitmap* startButton;
        wxStaticBitmap* greenButton;
        wxTimer pollingTimer;
        wxStaticBitmap* padDownRight;
        wxStaticBitmap* rightButton;
        wxGauge* leftStickYGauge;
        wxStaticBitmap* blueButton;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // WXXINPUTMAIN_H
