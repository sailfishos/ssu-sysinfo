/** @file hw_feature.c
 *
 * ssu-sysinfo - HW feature functions
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

#include "hw_feature.h"

#include "xmalloc.h"

#include <string.h>

/** Key names used in CSD data files.
 *
 * @note: Used only for CSD data lookups - must not be exposed by
 *        the library API.
 */
static const char * const hw_feature_csd_key_lut[Feature_Count] =
{
    [Feature_Invalid]               = "Invalid",
    [Feature_Microphone1]           = "AudioMic1",
    [Feature_Microphone2]           = "AudioMic2",
    [Feature_BackCamera]            = "BackCamera",
    [Feature_BackCameraFlashlight]  = "BackCameraFlash",
    [Feature_DisplayBacklight]      = "Backlight",
    [Feature_Battery]               = "Battery",
    [Feature_Bluetooth]             = "Bluetooth",
    [Feature_CellularData]          = "CellularData",
    [Feature_CellularVoice]         = "CellularVoice",
    [Feature_CompassSensor]         = "ECompass",
    [Feature_FMRadioReceiver]       = "FmRadio",
    [Feature_FrontCamera]           = "FrontCamera",
    [Feature_FrontCameraFlashlight] = "FrontCameraFlash",
    [Feature_GPS]                   = "GPS",
    [Feature_CellInfo]              = "CellInfo",
    [Feature_AccelerationSensor]    = "GSensor",
    [Feature_GyroSensor]            = "Gyro",
    [Feature_CoverSensor]           = "Hall",
    [Feature_FingerprintSensor]     = "Fingerprint",
    [Feature_Headset]               = "Headset",
    [Feature_HardwareKeys]          = "Key",
    [Feature_Display]               = "LCD",
    [Feature_NotificationLED]       = "LED",
    [Feature_ButtonBacklight]       = "ButtonBacklight",
    [Feature_LightSensor]           = "LightSensor",
    [Feature_Loudspeaker]           = "Loudspeaker",
    [Feature_TheOtherHalf]          = "TOH",
    [Feature_ProximitySensor]       = "ProxSensor",
    [Feature_AudioPlayback]         = "Receiver",
    [Feature_MemoryCardSlot]        = "SDCard",
    [Feature_SIMCardSlot]           = "SIM",
    [Feature_StereoLoudspeaker]     = "StereoLoudspeaker",
    [Feature_TouchScreen]           = "Touch",
    [Feature_TouchScreenSelfTest]   = "TouchAuto",
    [Feature_USBCharging]           = "UsbCharging",
    [Feature_USBOTG]                = "UsbOtg",
    [Feature_Vibrator]              = "Vibrator",
    [Feature_WLAN]                  = "Wifi",
    [Feature_NFC]                   = "NFC",
    [Feature_VideoPlayback]         = "VideoPlayback",
    [Feature_Suspend]               = "Suspend",
    [Feature_Reboot]                = "Reboot",
};

/** Key names used by ssu-sysinfo
 */
static const char * const hw_feature_name_lut[Feature_Count] =
{
    [Feature_Invalid]               = "Feature_Invalid",
    [Feature_Microphone1]           = "Feature_Microphone1",
    [Feature_Microphone2]           = "Feature_Microphone2",
    [Feature_BackCamera]            = "Feature_BackCamera",
    [Feature_BackCameraFlashlight]  = "Feature_BackCameraFlashlight",
    [Feature_DisplayBacklight]      = "Feature_DisplayBacklight",
    [Feature_Battery]               = "Feature_Battery",
    [Feature_Bluetooth]             = "Feature_Bluetooth",
    [Feature_CellularData]          = "Feature_CellularData",
    [Feature_CellularVoice]         = "Feature_CellularVoice",
    [Feature_CompassSensor]         = "Feature_CompassSensor",
    [Feature_FMRadioReceiver]       = "Feature_FMRadioReceiver",
    [Feature_FrontCamera]           = "Feature_FrontCamera",
    [Feature_FrontCameraFlashlight] = "Feature_FrontCameraFlashlight",
    [Feature_GPS]                   = "Feature_GPS",
    [Feature_CellInfo]              = "Feature_CellInfo",
    [Feature_AccelerationSensor]    = "Feature_AccelerationSensor",
    [Feature_GyroSensor]            = "Feature_GyroSensor",
    [Feature_CoverSensor]           = "Feature_CoverSensor",
    [Feature_FingerprintSensor]     = "Feature_FingerprintSensor",
    [Feature_Headset]               = "Feature_Headset",
    [Feature_HardwareKeys]          = "Feature_HardwareKeys",
    [Feature_Display]               = "Feature_Display",
    [Feature_NotificationLED]       = "Feature_NotificationLED",
    [Feature_ButtonBacklight]       = "Feature_ButtonBacklight",
    [Feature_LightSensor]           = "Feature_LightSensor",
    [Feature_Loudspeaker]           = "Feature_Loudspeaker",
    [Feature_TheOtherHalf]          = "Feature_TheOtherHalf",
    [Feature_ProximitySensor]       = "Feature_ProximitySensor",
    [Feature_AudioPlayback]         = "Feature_AudioPlayback",
    [Feature_MemoryCardSlot]        = "Feature_MemoryCardSlot",
    [Feature_SIMCardSlot]           = "Feature_SIMCardSlot",
    [Feature_StereoLoudspeaker]     = "Feature_StereoLoudspeaker",
    [Feature_TouchScreen]           = "Feature_TouchScreen",
    [Feature_TouchScreenSelfTest]   = "Feature_TouchScreenSelfTest",
    [Feature_USBCharging]           = "Feature_USBCharging",
    [Feature_USBOTG]                = "Feature_USBOTG",
    [Feature_Vibrator]              = "Feature_Vibrator",
    [Feature_WLAN]                  = "Feature_WLAN",
    [Feature_NFC]                   = "Feature_NFC",
    [Feature_VideoPlayback]         = "Feature_VideoPlayback",
    [Feature_Suspend]               = "Feature_Suspend",
    [Feature_Reboot]                = "Feature_Reboot",
};

/* Feature availability to use when CSD config line is missing
 */
static const bool hw_feature_fallback_lut[Feature_Count] =
{
    [Feature_Invalid]               = false,
    [Feature_Microphone1]           = false,
    [Feature_Microphone2]           = false,
    [Feature_BackCamera]            = false,
    [Feature_BackCameraFlashlight]  = false,
    [Feature_DisplayBacklight]      = false,
    [Feature_Battery]               = false,
    [Feature_Bluetooth]             = false,
    [Feature_CellularData]          = false,
    [Feature_CellularVoice]         = false,
    [Feature_CompassSensor]         = false,
    [Feature_FMRadioReceiver]       = false,
    [Feature_FrontCamera]           = false,
    [Feature_FrontCameraFlashlight] = false,
    [Feature_GPS]                   = false,
    [Feature_CellInfo]              = false,
    [Feature_AccelerationSensor]    = false,
    [Feature_GyroSensor]            = false,
    [Feature_CoverSensor]           = false,
    [Feature_FingerprintSensor]     = false,
    [Feature_Headset]               = false,
    [Feature_HardwareKeys]          = false,
    [Feature_Display]               = false,
    [Feature_NotificationLED]       = false,
    [Feature_ButtonBacklight]       = false,
    [Feature_LightSensor]           = false,
    [Feature_Loudspeaker]           = false,
    [Feature_TheOtherHalf]          = false,
    [Feature_ProximitySensor]       = false,
    [Feature_AudioPlayback]         = false,
    [Feature_MemoryCardSlot]        = false,
    [Feature_SIMCardSlot]           = false,
    [Feature_StereoLoudspeaker]     = false,
    [Feature_TouchScreen]           = false,
    [Feature_TouchScreenSelfTest]   = false,
    [Feature_USBCharging]           = false,
    [Feature_USBOTG]                = false,
    [Feature_Vibrator]              = false,
    [Feature_WLAN]                  = false,
    [Feature_NFC]                   = false,
    [Feature_VideoPlayback]         = false,
    [Feature_Suspend]               = true,
    [Feature_Reboot]                = true,
};

bool
hw_feature_is_valid(hw_feature_t id)
{
    return (Feature_Invalid < id) && (id < Feature_Count);
}

bool
hw_feature_get_fallback(hw_feature_t id)
{
    return hw_feature_is_valid(id) ? hw_feature_fallback_lut[id] : false;
}

const char *
hw_feature_to_string(hw_feature_t id)
{
    if( !hw_feature_is_valid(id) )
        id = Feature_Invalid;

    return hw_feature_name_lut[id];
}

hw_feature_t
hw_feature_from_string(const char *name)
{
    hw_feature_t res = Feature_Invalid;

    if( !name )
        goto EXIT;

    for( hw_feature_t i = 0; i < Feature_Count; ++i ) {
        if( !strcasecmp(hw_feature_name_lut[i], name) ) {
            res = i;
            break;
        }
    }

EXIT:
    return res;
}

const char *
hw_feature_to_csd_key(hw_feature_t id)
{
    if( !hw_feature_is_valid(id) )
        id = Feature_Invalid;

    return hw_feature_csd_key_lut[id];
}

const char **
hw_feature_names(void)
{
    size_t       n = Feature_Count;
    const char **v = xcalloc(n, sizeof *v);
    size_t       k = 0;

    for( size_t i = Feature_Invalid + 1; i < n; ++i )
        v[k++] = hw_feature_name_lut[i];
    v[k] = 0;

    return v;
}
