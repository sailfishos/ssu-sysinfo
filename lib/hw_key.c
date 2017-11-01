/** @file hw_key.c
 *
 * ssu-sysinfo - HW keycode functions
 * <p>
 * Copyright (c) 2017 Jolla Ltd.
 * <p>
 * @author Simo Piiroinen <simo.piiroinen@jollamobile.com>
 *
 * ssu-sysinfo is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ssu-sysinfo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ssu-sysinfo; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "hw_key.h"

#include "xmalloc.h"

#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------------------- *
 * qt_key_names
 * ------------------------------------------------------------------------- */

static const struct
{
    hw_key_t    code;
    const char *name;
} qt_key_names[] =
{
    /* The entries must be in ascending code order to
     * facilitate binary search from hw_key_to_string().
     */
    { .code = 0x00000020, .name = "Key_Space" },
    { .code = 0x00000021, .name = "Key_Exclam" },
    { .code = 0x00000022, .name = "Key_QuoteDbl" },
    { .code = 0x00000023, .name = "Key_NumberSign" },
    { .code = 0x00000024, .name = "Key_Dollar" },
    { .code = 0x00000025, .name = "Key_Percent" },
    { .code = 0x00000026, .name = "Key_Ampersand" },
    { .code = 0x00000027, .name = "Key_Apostrophe" },
    { .code = 0x00000028, .name = "Key_ParenLeft" },
    { .code = 0x00000029, .name = "Key_ParenRight" },
    { .code = 0x0000002a, .name = "Key_Asterisk" },
    { .code = 0x0000002b, .name = "Key_Plus" },
    { .code = 0x0000002c, .name = "Key_Comma" },
    { .code = 0x0000002d, .name = "Key_Minus" },
    { .code = 0x0000002e, .name = "Key_Period" },
    { .code = 0x0000002f, .name = "Key_Slash" },
    { .code = 0x00000030, .name = "Key_0" },
    { .code = 0x00000031, .name = "Key_1" },
    { .code = 0x00000032, .name = "Key_2" },
    { .code = 0x00000033, .name = "Key_3" },
    { .code = 0x00000034, .name = "Key_4" },
    { .code = 0x00000035, .name = "Key_5" },
    { .code = 0x00000036, .name = "Key_6" },
    { .code = 0x00000037, .name = "Key_7" },
    { .code = 0x00000038, .name = "Key_8" },
    { .code = 0x00000039, .name = "Key_9" },
    { .code = 0x0000003a, .name = "Key_Colon" },
    { .code = 0x0000003b, .name = "Key_Semicolon" },
    { .code = 0x0000003c, .name = "Key_Less" },
    { .code = 0x0000003d, .name = "Key_Equal" },
    { .code = 0x0000003e, .name = "Key_Greater" },
    { .code = 0x0000003f, .name = "Key_Question" },
    { .code = 0x00000040, .name = "Key_At" },
    { .code = 0x00000041, .name = "Key_A" },
    { .code = 0x00000042, .name = "Key_B" },
    { .code = 0x00000043, .name = "Key_C" },
    { .code = 0x00000044, .name = "Key_D" },
    { .code = 0x00000045, .name = "Key_E" },
    { .code = 0x00000046, .name = "Key_F" },
    { .code = 0x00000047, .name = "Key_G" },
    { .code = 0x00000048, .name = "Key_H" },
    { .code = 0x00000049, .name = "Key_I" },
    { .code = 0x0000004a, .name = "Key_J" },
    { .code = 0x0000004b, .name = "Key_K" },
    { .code = 0x0000004c, .name = "Key_L" },
    { .code = 0x0000004d, .name = "Key_M" },
    { .code = 0x0000004e, .name = "Key_N" },
    { .code = 0x0000004f, .name = "Key_O" },
    { .code = 0x00000050, .name = "Key_P" },
    { .code = 0x00000051, .name = "Key_Q" },
    { .code = 0x00000052, .name = "Key_R" },
    { .code = 0x00000053, .name = "Key_S" },
    { .code = 0x00000054, .name = "Key_T" },
    { .code = 0x00000055, .name = "Key_U" },
    { .code = 0x00000056, .name = "Key_V" },
    { .code = 0x00000057, .name = "Key_W" },
    { .code = 0x00000058, .name = "Key_X" },
    { .code = 0x00000059, .name = "Key_Y" },
    { .code = 0x0000005a, .name = "Key_Z" },
    { .code = 0x0000005b, .name = "Key_BracketLeft" },
    { .code = 0x0000005c, .name = "Key_Backslash" },
    { .code = 0x0000005d, .name = "Key_BracketRight" },
    { .code = 0x0000005e, .name = "Key_AsciiCircum" },
    { .code = 0x0000005f, .name = "Key_Underscore" },
    { .code = 0x00000060, .name = "Key_QuoteLeft" },
    { .code = 0x0000007b, .name = "Key_BraceLeft" },
    { .code = 0x0000007c, .name = "Key_Bar" },
    { .code = 0x0000007d, .name = "Key_BraceRight" },
    { .code = 0x0000007e, .name = "Key_AsciiTilde" },
    { .code = 0x000000a0, .name = "Key_nobreakspace" },
    { .code = 0x000000a1, .name = "Key_exclamdown" },
    { .code = 0x000000a2, .name = "Key_cent" },
    { .code = 0x000000a3, .name = "Key_sterling" },
    { .code = 0x000000a4, .name = "Key_currency" },
    { .code = 0x000000a5, .name = "Key_yen" },
    { .code = 0x000000a6, .name = "Key_brokenbar" },
    { .code = 0x000000a7, .name = "Key_section" },
    { .code = 0x000000a8, .name = "Key_diaeresis" },
    { .code = 0x000000a9, .name = "Key_copyright" },
    { .code = 0x000000aa, .name = "Key_ordfeminine" },
    { .code = 0x000000ab, .name = "Key_guillemotleft" },
    { .code = 0x000000ac, .name = "Key_notsign" },
    { .code = 0x000000ad, .name = "Key_hyphen" },
    { .code = 0x000000ae, .name = "Key_registered" },
    { .code = 0x000000af, .name = "Key_macron" },
    { .code = 0x000000b0, .name = "Key_degree" },
    { .code = 0x000000b1, .name = "Key_plusminus" },
    { .code = 0x000000b2, .name = "Key_twosuperior" },
    { .code = 0x000000b3, .name = "Key_threesuperior" },
    { .code = 0x000000b4, .name = "Key_acute" },
    { .code = 0x000000b5, .name = "Key_mu" },
    { .code = 0x000000b6, .name = "Key_paragraph" },
    { .code = 0x000000b7, .name = "Key_periodcentered" },
    { .code = 0x000000b8, .name = "Key_cedilla" },
    { .code = 0x000000b9, .name = "Key_onesuperior" },
    { .code = 0x000000ba, .name = "Key_masculine" },
    { .code = 0x000000bb, .name = "Key_guillemotright" },
    { .code = 0x000000bc, .name = "Key_onequarter" },
    { .code = 0x000000bd, .name = "Key_onehalf" },
    { .code = 0x000000be, .name = "Key_threequarters" },
    { .code = 0x000000bf, .name = "Key_questiondown" },
    { .code = 0x000000c0, .name = "Key_Agrave" },
    { .code = 0x000000c1, .name = "Key_Aacute" },
    { .code = 0x000000c2, .name = "Key_Acircumflex" },
    { .code = 0x000000c3, .name = "Key_Atilde" },
    { .code = 0x000000c4, .name = "Key_Adiaeresis" },
    { .code = 0x000000c5, .name = "Key_Aring" },
    { .code = 0x000000c6, .name = "Key_AE" },
    { .code = 0x000000c7, .name = "Key_Ccedilla" },
    { .code = 0x000000c8, .name = "Key_Egrave" },
    { .code = 0x000000c9, .name = "Key_Eacute" },
    { .code = 0x000000ca, .name = "Key_Ecircumflex" },
    { .code = 0x000000cb, .name = "Key_Ediaeresis" },
    { .code = 0x000000cc, .name = "Key_Igrave" },
    { .code = 0x000000cd, .name = "Key_Iacute" },
    { .code = 0x000000ce, .name = "Key_Icircumflex" },
    { .code = 0x000000cf, .name = "Key_Idiaeresis" },
    { .code = 0x000000d0, .name = "Key_ETH" },
    { .code = 0x000000d1, .name = "Key_Ntilde" },
    { .code = 0x000000d2, .name = "Key_Ograve" },
    { .code = 0x000000d3, .name = "Key_Oacute" },
    { .code = 0x000000d4, .name = "Key_Ocircumflex" },
    { .code = 0x000000d5, .name = "Key_Otilde" },
    { .code = 0x000000d6, .name = "Key_Odiaeresis" },
    { .code = 0x000000d7, .name = "Key_multiply" },
    { .code = 0x000000d8, .name = "Key_Ooblique" },
    { .code = 0x000000d9, .name = "Key_Ugrave" },
    { .code = 0x000000da, .name = "Key_Uacute" },
    { .code = 0x000000db, .name = "Key_Ucircumflex" },
    { .code = 0x000000dc, .name = "Key_Udiaeresis" },
    { .code = 0x000000dd, .name = "Key_Yacute" },
    { .code = 0x000000de, .name = "Key_THORN" },
    { .code = 0x000000df, .name = "Key_ssharp" },
    { .code = 0x000000f7, .name = "Key_division" },
    { .code = 0x000000ff, .name = "Key_ydiaeresis" },
    { .code = 0x01000000, .name = "Key_Escape" },
    { .code = 0x01000001, .name = "Key_Tab" },
    { .code = 0x01000002, .name = "Key_Backtab" },
    { .code = 0x01000003, .name = "Key_Backspace" },
    { .code = 0x01000004, .name = "Key_Return" },
    { .code = 0x01000005, .name = "Key_Enter" },
    { .code = 0x01000006, .name = "Key_Insert" },
    { .code = 0x01000007, .name = "Key_Delete" },
    { .code = 0x01000008, .name = "Key_Pause" },
    { .code = 0x01000009, .name = "Key_Print" },
    { .code = 0x0100000a, .name = "Key_SysReq" },
    { .code = 0x0100000b, .name = "Key_Clear" },
    { .code = 0x01000010, .name = "Key_Home" },
    { .code = 0x01000011, .name = "Key_End" },
    { .code = 0x01000012, .name = "Key_Left" },
    { .code = 0x01000013, .name = "Key_Up" },
    { .code = 0x01000014, .name = "Key_Right" },
    { .code = 0x01000015, .name = "Key_Down" },
    { .code = 0x01000016, .name = "Key_PageUp" },
    { .code = 0x01000017, .name = "Key_PageDown" },
    { .code = 0x01000020, .name = "Key_Shift" },
    { .code = 0x01000021, .name = "Key_Control" },
    { .code = 0x01000022, .name = "Key_Meta" },
    { .code = 0x01000023, .name = "Key_Alt" },
    { .code = 0x01000024, .name = "Key_CapsLock" },
    { .code = 0x01000025, .name = "Key_NumLock" },
    { .code = 0x01000026, .name = "Key_ScrollLock" },
    { .code = 0x01000030, .name = "Key_F1" },
    { .code = 0x01000031, .name = "Key_F2" },
    { .code = 0x01000032, .name = "Key_F3" },
    { .code = 0x01000033, .name = "Key_F4" },
    { .code = 0x01000034, .name = "Key_F5" },
    { .code = 0x01000035, .name = "Key_F6" },
    { .code = 0x01000036, .name = "Key_F7" },
    { .code = 0x01000037, .name = "Key_F8" },
    { .code = 0x01000038, .name = "Key_F9" },
    { .code = 0x01000039, .name = "Key_F10" },
    { .code = 0x0100003a, .name = "Key_F11" },
    { .code = 0x0100003b, .name = "Key_F12" },
    { .code = 0x0100003c, .name = "Key_F13" },
    { .code = 0x0100003d, .name = "Key_F14" },
    { .code = 0x0100003e, .name = "Key_F15" },
    { .code = 0x0100003f, .name = "Key_F16" },
    { .code = 0x01000040, .name = "Key_F17" },
    { .code = 0x01000041, .name = "Key_F18" },
    { .code = 0x01000042, .name = "Key_F19" },
    { .code = 0x01000043, .name = "Key_F20" },
    { .code = 0x01000044, .name = "Key_F21" },
    { .code = 0x01000045, .name = "Key_F22" },
    { .code = 0x01000046, .name = "Key_F23" },
    { .code = 0x01000047, .name = "Key_F24" },
    { .code = 0x01000048, .name = "Key_F25" },
    { .code = 0x01000049, .name = "Key_F26" },
    { .code = 0x0100004a, .name = "Key_F27" },
    { .code = 0x0100004b, .name = "Key_F28" },
    { .code = 0x0100004c, .name = "Key_F29" },
    { .code = 0x0100004d, .name = "Key_F30" },
    { .code = 0x0100004e, .name = "Key_F31" },
    { .code = 0x0100004f, .name = "Key_F32" },
    { .code = 0x01000050, .name = "Key_F33" },
    { .code = 0x01000051, .name = "Key_F34" },
    { .code = 0x01000052, .name = "Key_F35" },
    { .code = 0x01000053, .name = "Key_Super_L" },
    { .code = 0x01000054, .name = "Key_Super_R" },
    { .code = 0x01000055, .name = "Key_Menu" },
    { .code = 0x01000056, .name = "Key_Hyper_L" },
    { .code = 0x01000057, .name = "Key_Hyper_R" },
    { .code = 0x01000058, .name = "Key_Help" },
    { .code = 0x01000059, .name = "Key_Direction_L" },
    { .code = 0x01000060, .name = "Key_Direction_R" },
    { .code = 0x01000061, .name = "Key_Back" },
    { .code = 0x01000062, .name = "Key_Forward" },
    { .code = 0x01000063, .name = "Key_Stop" },
    { .code = 0x01000064, .name = "Key_Refresh" },
    { .code = 0x01000070, .name = "Key_VolumeDown" },
    { .code = 0x01000071, .name = "Key_VolumeMute" },
    { .code = 0x01000072, .name = "Key_VolumeUp" },
    { .code = 0x01000073, .name = "Key_BassBoost" },
    { .code = 0x01000074, .name = "Key_BassUp" },
    { .code = 0x01000075, .name = "Key_BassDown" },
    { .code = 0x01000076, .name = "Key_TrebleUp" },
    { .code = 0x01000077, .name = "Key_TrebleDown" },
    { .code = 0x01000080, .name = "Key_MediaPlay" },
    { .code = 0x01000081, .name = "Key_MediaStop" },
    { .code = 0x01000082, .name = "Key_MediaPrevious" },
    { .code = 0x01000083, .name = "Key_MediaNext" },
    { .code = 0x01000084, .name = "Key_MediaRecord" },
    { .code = 0x01000085, .name = "Key_MediaPause" },
    { .code = 0x01000086, .name = "Key_MediaTogglePlayPause" },
    { .code = 0x01000090, .name = "Key_HomePage" },
    { .code = 0x01000091, .name = "Key_Favorites" },
    { .code = 0x01000092, .name = "Key_Search" },
    { .code = 0x01000093, .name = "Key_Standby" },
    { .code = 0x01000094, .name = "Key_OpenUrl" },
    { .code = 0x010000a0, .name = "Key_LaunchMail" },
    { .code = 0x010000a1, .name = "Key_LaunchMedia" },
    { .code = 0x010000a2, .name = "Key_Launch0" },
    { .code = 0x010000a3, .name = "Key_Launch1" },
    { .code = 0x010000a4, .name = "Key_Launch2" },
    { .code = 0x010000a5, .name = "Key_Launch3" },
    { .code = 0x010000a6, .name = "Key_Launch4" },
    { .code = 0x010000a7, .name = "Key_Launch5" },
    { .code = 0x010000a8, .name = "Key_Launch6" },
    { .code = 0x010000a9, .name = "Key_Launch7" },
    { .code = 0x010000aa, .name = "Key_Launch8" },
    { .code = 0x010000ab, .name = "Key_Launch9" },
    { .code = 0x010000ac, .name = "Key_LaunchA" },
    { .code = 0x010000ad, .name = "Key_LaunchB" },
    { .code = 0x010000ae, .name = "Key_LaunchC" },
    { .code = 0x010000af, .name = "Key_LaunchD" },
    { .code = 0x010000b0, .name = "Key_LaunchE" },
    { .code = 0x010000b1, .name = "Key_LaunchF" },
    { .code = 0x010000b2, .name = "Key_MonBrightnessUp" },
    { .code = 0x010000b3, .name = "Key_MonBrightnessDown" },
    { .code = 0x010000b4, .name = "Key_KeyboardLightOnOff" },
    { .code = 0x010000b5, .name = "Key_KeyboardBrightnessUp" },
    { .code = 0x010000b6, .name = "Key_KeyboardBrightnessDown" },
    { .code = 0x010000b7, .name = "Key_PowerOff" },
    { .code = 0x010000b8, .name = "Key_WakeUp" },
    { .code = 0x010000b9, .name = "Key_Eject" },
    { .code = 0x010000ba, .name = "Key_ScreenSaver" },
    { .code = 0x010000bb, .name = "Key_WWW" },
    { .code = 0x010000bc, .name = "Key_Memo" },
    { .code = 0x010000bd, .name = "Key_LightBulb" },
    { .code = 0x010000be, .name = "Key_Shop" },
    { .code = 0x010000bf, .name = "Key_History" },
    { .code = 0x010000c0, .name = "Key_AddFavorite" },
    { .code = 0x010000c1, .name = "Key_HotLinks" },
    { .code = 0x010000c2, .name = "Key_BrightnessAdjust" },
    { .code = 0x010000c3, .name = "Key_Finance" },
    { .code = 0x010000c4, .name = "Key_Community" },
    { .code = 0x010000c5, .name = "Key_AudioRewind" },
    { .code = 0x010000c6, .name = "Key_BackForward" },
    { .code = 0x010000c7, .name = "Key_ApplicationLeft" },
    { .code = 0x010000c8, .name = "Key_ApplicationRight" },
    { .code = 0x010000c9, .name = "Key_Book" },
    { .code = 0x010000ca, .name = "Key_CD" },
    { .code = 0x010000cb, .name = "Key_Calculator" },
    { .code = 0x010000cc, .name = "Key_ToDoList" },
    { .code = 0x010000cd, .name = "Key_ClearGrab" },
    { .code = 0x010000ce, .name = "Key_Close" },
    { .code = 0x010000cf, .name = "Key_Copy" },
    { .code = 0x010000d0, .name = "Key_Cut" },
    { .code = 0x010000d1, .name = "Key_Display" },
    { .code = 0x010000d2, .name = "Key_DOS" },
    { .code = 0x010000d3, .name = "Key_Documents" },
    { .code = 0x010000d4, .name = "Key_Excel" },
    { .code = 0x010000d5, .name = "Key_Explorer" },
    { .code = 0x010000d6, .name = "Key_Game" },
    { .code = 0x010000d7, .name = "Key_Go" },
    { .code = 0x010000d8, .name = "Key_iTouch" },
    { .code = 0x010000d9, .name = "Key_LogOff" },
    { .code = 0x010000da, .name = "Key_Market" },
    { .code = 0x010000db, .name = "Key_Meeting" },
    { .code = 0x010000dc, .name = "Key_MenuKB" },
    { .code = 0x010000dd, .name = "Key_MenuPB" },
    { .code = 0x010000de, .name = "Key_MySites" },
    { .code = 0x010000df, .name = "Key_News" },
    { .code = 0x010000e0, .name = "Key_OfficeHome" },
    { .code = 0x010000e1, .name = "Key_Option" },
    { .code = 0x010000e2, .name = "Key_Paste" },
    { .code = 0x010000e3, .name = "Key_Phone" },
    { .code = 0x010000e4, .name = "Key_Calendar" },
    { .code = 0x010000e5, .name = "Key_Reply" },
    { .code = 0x010000e6, .name = "Key_Reload" },
    { .code = 0x010000e7, .name = "Key_RotateWindows" },
    { .code = 0x010000e8, .name = "Key_RotationPB" },
    { .code = 0x010000e9, .name = "Key_RotationKB" },
    { .code = 0x010000ea, .name = "Key_Save" },
    { .code = 0x010000eb, .name = "Key_Send" },
    { .code = 0x010000ec, .name = "Key_Spell" },
    { .code = 0x010000ed, .name = "Key_SplitScreen" },
    { .code = 0x010000ee, .name = "Key_Support" },
    { .code = 0x010000ef, .name = "Key_TaskPane" },
    { .code = 0x010000f0, .name = "Key_Terminal" },
    { .code = 0x010000f1, .name = "Key_Tools" },
    { .code = 0x010000f2, .name = "Key_Travel" },
    { .code = 0x010000f3, .name = "Key_Video" },
    { .code = 0x010000f4, .name = "Key_Word" },
    { .code = 0x010000f5, .name = "Key_Xfer" },
    { .code = 0x010000f6, .name = "Key_ZoomIn" },
    { .code = 0x010000f7, .name = "Key_ZoomOut" },
    { .code = 0x010000f8, .name = "Key_Away" },
    { .code = 0x010000f9, .name = "Key_Messenger" },
    { .code = 0x010000fa, .name = "Key_WebCam" },
    { .code = 0x010000fb, .name = "Key_MailForward" },
    { .code = 0x010000fc, .name = "Key_Pictures" },
    { .code = 0x010000fd, .name = "Key_Music" },
    { .code = 0x010000fe, .name = "Key_Battery" },
    { .code = 0x010000ff, .name = "Key_Bluetooth" },
    { .code = 0x01000100, .name = "Key_WLAN" },
    { .code = 0x01000101, .name = "Key_UWB" },
    { .code = 0x01000102, .name = "Key_AudioForward" },
    { .code = 0x01000103, .name = "Key_AudioRepeat" },
    { .code = 0x01000104, .name = "Key_AudioRandomPlay" },
    { .code = 0x01000105, .name = "Key_Subtitle" },
    { .code = 0x01000106, .name = "Key_AudioCycleTrack" },
    { .code = 0x01000107, .name = "Key_Time" },
    { .code = 0x01000108, .name = "Key_Hibernate" },
    { .code = 0x01000109, .name = "Key_View" },
    { .code = 0x0100010a, .name = "Key_TopMenu" },
    { .code = 0x0100010b, .name = "Key_PowerDown" },
    { .code = 0x0100010c, .name = "Key_Suspend" },
    { .code = 0x0100010d, .name = "Key_ContrastAdjust" },
    { .code = 0x0100010e, .name = "Key_LaunchG" },
    { .code = 0x0100010f, .name = "Key_LaunchH" },
    { .code = 0x01000110, .name = "Key_TouchpadToggle" },
    { .code = 0x01000111, .name = "Key_TouchpadOn" },
    { .code = 0x01000112, .name = "Key_TouchpadOff" },
    { .code = 0x01000113, .name = "Key_MicMute" },
    { .code = 0x01001103, .name = "Key_AltGr" },
    { .code = 0x01001120, .name = "Key_Multi_key" },
    { .code = 0x01001121, .name = "Key_Kanji" },
    { .code = 0x01001122, .name = "Key_Muhenkan" },
    { .code = 0x01001123, .name = "Key_Henkan" },
    { .code = 0x01001124, .name = "Key_Romaji" },
    { .code = 0x01001125, .name = "Key_Hiragana" },
    { .code = 0x01001126, .name = "Key_Katakana" },
    { .code = 0x01001127, .name = "Key_Hiragana_Katakana" },
    { .code = 0x01001128, .name = "Key_Zenkaku" },
    { .code = 0x01001129, .name = "Key_Hankaku" },
    { .code = 0x0100112a, .name = "Key_Zenkaku_Hankaku" },
    { .code = 0x0100112b, .name = "Key_Touroku" },
    { .code = 0x0100112c, .name = "Key_Massyo" },
    { .code = 0x0100112d, .name = "Key_Kana_Lock" },
    { .code = 0x0100112e, .name = "Key_Kana_Shift" },
    { .code = 0x0100112f, .name = "Key_Eisu_Shift" },
    { .code = 0x01001130, .name = "Key_Eisu_toggle" },
    { .code = 0x01001131, .name = "Key_Hangul" },
    { .code = 0x01001132, .name = "Key_Hangul_Start" },
    { .code = 0x01001133, .name = "Key_Hangul_End" },
    { .code = 0x01001134, .name = "Key_Hangul_Hanja" },
    { .code = 0x01001135, .name = "Key_Hangul_Jamo" },
    { .code = 0x01001136, .name = "Key_Hangul_Romaja" },
    { .code = 0x01001137, .name = "Key_Codeinput" },
    { .code = 0x01001138, .name = "Key_Hangul_Jeonja" },
    { .code = 0x01001139, .name = "Key_Hangul_Banja" },
    { .code = 0x0100113a, .name = "Key_Hangul_PreHanja" },
    { .code = 0x0100113b, .name = "Key_Hangul_PostHanja" },
    { .code = 0x0100113c, .name = "Key_SingleCandidate" },
    { .code = 0x0100113d, .name = "Key_MultipleCandidate" },
    { .code = 0x0100113e, .name = "Key_PreviousCandidate" },
    { .code = 0x0100113f, .name = "Key_Hangul_Special" },
    { .code = 0x0100117e, .name = "Key_Mode_switch" },
    { .code = 0x01001250, .name = "Key_Dead_Grave" },
    { .code = 0x01001251, .name = "Key_Dead_Acute" },
    { .code = 0x01001252, .name = "Key_Dead_Circumflex" },
    { .code = 0x01001253, .name = "Key_Dead_Tilde" },
    { .code = 0x01001254, .name = "Key_Dead_Macron" },
    { .code = 0x01001255, .name = "Key_Dead_Breve" },
    { .code = 0x01001256, .name = "Key_Dead_Abovedot" },
    { .code = 0x01001257, .name = "Key_Dead_Diaeresis" },
    { .code = 0x01001258, .name = "Key_Dead_Abovering" },
    { .code = 0x01001259, .name = "Key_Dead_Doubleacute" },
    { .code = 0x0100125a, .name = "Key_Dead_Caron" },
    { .code = 0x0100125b, .name = "Key_Dead_Cedilla" },
    { .code = 0x0100125c, .name = "Key_Dead_Ogonek" },
    { .code = 0x0100125d, .name = "Key_Dead_Iota" },
    { .code = 0x0100125e, .name = "Key_Dead_Voiced_Sound" },
    { .code = 0x0100125f, .name = "Key_Dead_Semivoiced_Sound" },
    { .code = 0x01001260, .name = "Key_Dead_Belowdot" },
    { .code = 0x01001261, .name = "Key_Dead_Hook" },
    { .code = 0x01001262, .name = "Key_Dead_Horn" },
    { .code = 0x0100ffff, .name = "Key_MediaLast" },
    { .code = 0x01010000, .name = "Key_Select" },
    { .code = 0x01010001, .name = "Key_Yes" },
    { .code = 0x01010002, .name = "Key_No" },
    { .code = 0x01020001, .name = "Key_Cancel" },
    { .code = 0x01020002, .name = "Key_Printer" },
    { .code = 0x01020003, .name = "Key_Execute" },
    { .code = 0x01020004, .name = "Key_Sleep" },
    { .code = 0x01020005, .name = "Key_Play" },
    { .code = 0x01020006, .name = "Key_Zoom" },
    { .code = 0x01100000, .name = "Key_Context1" },
    { .code = 0x01100001, .name = "Key_Context2" },
    { .code = 0x01100002, .name = "Key_Context3" },
    { .code = 0x01100003, .name = "Key_Context4" },
    { .code = 0x01100004, .name = "Key_Call" },
    { .code = 0x01100005, .name = "Key_Hangup" },
    { .code = 0x01100006, .name = "Key_Flip" },
    { .code = 0x01100007, .name = "Key_ToggleCallHangup" },
    { .code = 0x01100008, .name = "Key_VoiceDial" },
    { .code = 0x01100009, .name = "Key_LastNumberRedial" },
    { .code = 0x01100020, .name = "Key_Camera" },
    { .code = 0x01100021, .name = "Key_CameraFocus" },
    { .code = 0x01ffffff, .name = "Key_unknown" },
};

/* ------------------------------------------------------------------------- *
 * hw_key_from_string
 * ------------------------------------------------------------------------- */

hw_key_t
hw_key_from_string(const char *name)
{
    hw_key_t code = 0;

    size_t n = sizeof qt_key_names / sizeof *qt_key_names;

    for( size_t i = 0; i < n; ++i ) {
        if( strcasecmp(qt_key_names[i].name, name) )
            continue;
        code = qt_key_names[i].code;
        break;
    }
    return code;
}

/* ------------------------------------------------------------------------- *
 * hw_key_to_string
 * ------------------------------------------------------------------------- */

const char *
hw_key_to_string(hw_key_t code)
{
    const char *name = 0;

    size_t l = 0;
    size_t h = sizeof qt_key_names / sizeof *qt_key_names;

    while( l < h ) {
        size_t i = (l + h) / 2;
        if( qt_key_names[i].code < code ) {
            l = i + 1; continue;
        }
        if( qt_key_names[i].code > code ) {
            h = i - 0; continue;
        }
        name = qt_key_names[i].name;
        break;
    }

    return name;
}

/* ------------------------------------------------------------------------- *
 * hw_key_names
 * ------------------------------------------------------------------------- */

const char **
hw_key_names(void)
{
    size_t       n = sizeof qt_key_names / sizeof *qt_key_names;
    const char **v = xcalloc(n + 1, sizeof *v);

    for( size_t i = 0; i < n; ++i )
        v[i] = qt_key_names[i].name;
    v[n] = 0;

    return v;
}

/* ------------------------------------------------------------------------- *
 * hw_key_is_valid
 * ------------------------------------------------------------------------- */

bool
hw_key_is_valid(hw_key_t code)
{
    return hw_key_to_string(code) != 0;
}

/* ------------------------------------------------------------------------- *
 * hw_key_sort_cb
 * ------------------------------------------------------------------------- */

static int
hw_key_sort_cb(const void *pa, const void *pb)
{
    hw_key_t a = *(const hw_key_t *)pa;
    hw_key_t b = *(const hw_key_t *)pb;
    return (a > b) - (a < b);
}

/* ------------------------------------------------------------------------- *
 * hw_key_parse
 * ------------------------------------------------------------------------- */

static bool
hw_key_parse(hw_key_t *code, const char **ppos)
{
    bool        ack = false;
    const char *pos = *ppos;
    char       *end = 0;

    *code = (hw_key_t)strtoul(pos, &end, 0);

    if( end > pos ) {
        ack = true;
        pos = end;
        if( *pos == ',' )
            ++pos;
    }
    else {
        pos = strchr(pos, 0);
    }

    *ppos = pos;

    return ack;
}

/* ------------------------------------------------------------------------- *
 * hw_key_parse_array
 * ------------------------------------------------------------------------- */

hw_key_t *
hw_key_parse_array(const char *text)
{
    size_t    used = 0;
    size_t    size = 32;
    hw_key_t *data = 0;

    if( !text )
        goto EXIT;

    data = xmalloc(size * sizeof *data);

    for( ;; ) {
        hw_key_t code = 0;
        if( !hw_key_parse(&code, &text) )
            break;

        if( used == size ) {
            size = size * 2;
            data = xrealloc(data, size * sizeof *data);
        }

        data[used++] = code;
    }

    size = used + 1;
    data = xrealloc(data, size * sizeof *data);
    data[used] = 0;

    if( used > 1 )
        qsort(data, used, sizeof *data, hw_key_sort_cb);

EXIT:
    return data;
}
