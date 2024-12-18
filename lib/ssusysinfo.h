/** @file ssusysinfo.h
 *
 * ssu-sysinfo - Library API functions
 * <p>
 * Copyright (c) 2016 - 2022 Jolla Ltd.
 * <p>
 * @author Simo Piiroinen <simo.piiroinen@jolla.com>
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

#ifndef SSUSYSINFO_H_
# define SSUSYSINFO_H_

# include <stdbool.h>
# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# elif 0
}
# endif

/* ========================================================================= *
 * TYPES
 * ========================================================================= */

/** Opaque SSU configuration object structure
 *
 * Can be created/initialized with ssusysinfo_create() function and
 * should be released with ssusysinfo_delete() function.
 */
typedef struct ssusysinfo_t ssusysinfo_t;

/** SSU device mode bitfields
 *
 * @note This must be kept in sync with Ssu:DeviceMode C++ enum.
 */
typedef enum  {
    /** Disable automagic repository management */
    SSU_DEVICE_MODE_DISABLE_REPO_MANAGER = 1<<0,
    /** Enable RnD mode for device */
    SSU_DEVICE_MODE_RND                  = 1<<1,
    /** Enable Release mode
     *
     * @note The release mode bit is useful only for enabling *also*
     *       relase repositories in a device that is in rnd mode. Otherwise
     *       not having rnd mode bit set implies release mode.
     */
    SSU_DEVICE_MODE_RELEASE              = 1<<2,
    /** Disable strict mode (i.e., keep unmanaged repositories) */
    SSU_DEVICE_MODE_LENIENT              = 1<<3,
    /** Do repo isolation and similar bits important for updating devices */
    SSU_DEVICE_MODE_UPDATE               = 1<<4,
    /** Do repo isolation, but keep store repository enabled */
    SSU_DEVICE_MODE_APP_INSTALL          = 1<<5,
} ssu_device_mode_t;

/* ========================================================================= *
 * FUNCTIONS
 * ========================================================================= */

# pragma GCC visibility push(default)

/* -- ssusysinfo -- */

/** Create SSU configuration object
 *
 * Parses SSU configuration files and returns handle
 * that can be used for querying values.
 *
 * @return ssusysinfo object pointer
 */
ssusysinfo_t *ssusysinfo_create             (void);

/** Delete SSU configuration object
 *
 * @param self ssusysinfo object pointer, or NULL
 */
void          ssusysinfo_delete             (ssusysinfo_t *self);

/** Type agnostic callback for deleting SSU configuration objects
 *
 * @param self ssusysinfo object as void pointer, or NULL
 */
void          ssusysinfo_delete_cb          (void *self);

/** Force realoading of SSU configuration files
 *
 * @param self ssusysinfo object pointer
 */
void          ssusysinfo_reload             (ssusysinfo_t *self);

/** Query device model
 *
 * Try to find out ond what kind of system this is running.
 *
 * Uses flag file / cpuinfo content heuristics or looks it up
 * from /etc/hw-release file.
 *
 * Returns values such as:
 *   "SbJ"
 *   "tbj"
 *   "l500d"
 *   "tk7001"
 *   "SDK"
 *   "SDK Target"
 *   "UNKNOWN"
 *
 * Should be functionally equivalent with Qt based libssu method:
 *   SsuDeviceInfo::deviceModel()
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */
const char   *ssusysinfo_device_model       (ssusysinfo_t *self);

/** Query device base model
 *
 * For variant devices #ssusysinfo_device_model() returns the
 * variant name. The name of the base model can be queried
 * with this function.
 *
 * Lookup is done from [variants] section defined in board mappings
 *
 * Should be functionally equivalent with:
 *   SsuDeviceInfo::deviceVariant(false)
 *
 * If the device is not an variant and there is no base model,
 * returns "UNKNOWN" - otherwise return values are similar as
 * what can be expected from #ssusysinfo_device_model().
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */

const char   *ssusysinfo_device_base_model     (ssusysinfo_t *self);

/** Query device designation
 *
 * Type designation, like NCC-1701.
 *
 * Uses ssusysinfo_device_model() to locate board mapping section and
 * returns value of key "deviceDesignation".
 *
 * Returns values such as:
 *   "JP-1301"
 *   "JT-1501"
 *   "Aqua Fish"
 *   "TK7001"
 *   "UNKNOWN"
 *
 * Should be functionally equivalent with Qt based libssu method:
 *   SsuDeviceInfo::displayName(Ssu::DeviceDesignation)
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */
const char   *ssusysinfo_device_designation (ssusysinfo_t *self);

/** Query device manufacturer
 *
 * Manufacturer, like ACME Corp.
 *
 * Uses ssusysinfo_device_model() to locate board mapping section and
 * returns value of key "deviceManufacturer".
 *
 *
 * Returns values such as:
 *   "Jolla"
 *   "Intex"
 *   "Turing Robotic Industries"
 *   "UNKNOWN"
 *
 * Should be functionally equivalent with Qt based libssu method:
 *   SsuDeviceInfo::displayName(Ssu::DeviceManufacturer)
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */
const char   *ssusysinfo_device_manufacturer(ssusysinfo_t *self);

/** Query device pretty name
 *
 * Marketed device name, like Pogoblaster 3000.
 *
 * Uses ssusysinfo_device_model() to locate board mapping section and
 * returns value of key "prettyModel".
 *
 * Returns values such as:
 *   "Jolla"
 *   "Jolla Tablet"
 *   "Intex Aqua Fish"
 *   "Turing Phone"
 *   "UNKNOWN"
 *
 * Should be functionally equivalent with Qt based libssu method:
 *   SsuDeviceInfo::displayName(Ssu::Ssu::DeviceModel)
 *
 * @param self ssusysinfo object pointer
 *
 * @return always returns non-null c-string
 */
const char   *ssusysinfo_device_pretty_name (ssusysinfo_t *self);

/** Query ssu config version number
 *
 * Currently fetches "configVersion" value from "General" section in ssu.ini.
 *
 * This value is used internally by SSU to deal with configuration file format
 * changes and thus is unlikely to be of interest to other processes.
 *
 * @return config version number
 */
int ssusysinfo_ssu_config_version(ssusysinfo_t *self);

/** Query ssu registration status
 *
 * Currently fetches "registered" value from "General" section in ssu.ini.
 *
 * @return true if device is registered, false otherwise
 */
bool ssusysinfo_ssu_registered(ssusysinfo_t *self);

/** Query ssu device mode setting
 *
 * Translates "deviceMode" value from "General" section in ssu.ini file
 * into a number.
 *
 * @return device mode bitmask
 */
ssu_device_mode_t ssusysinfo_ssu_device_mode(ssusysinfo_t *self);

/** Query ssu in rnd mode
 *
 * Checks if return value from #ssusysinfo_ssu_device_mode()
 * includes also #SSU_DEVICE_MODE_RND bit.
 *
 * @return true if device is in rnd mode, false otherwise
 */
bool ssusysinfo_ssu_in_rnd_mode(ssusysinfo_t *self);

/** Query ssu architecture setting
 *
 * Currently fethes "arch" value from "General" section in ssu.ini.
 *
 * Expected values: "armv7hl", "amd64", ...
 *
 * @return architecture name
 */
const char *ssusysinfo_ssu_arch(ssusysinfo_t *self);

/** Query ssu brand setting
 *
 * Currently fetches "brand" value from "General" section in ssu.ini.
 *
 * Expected values: "jolla", "<customer_specific_name>", ...
 *
 * @return brand name, or "UNKNOWN" if not specified
 */
const char *ssusysinfo_ssu_brand(ssusysinfo_t *self);

/** Query ssu flavour setting
 *
 * Currently fetches "flavour" value from "General" section in ssu.ini.
 *
 * Expected values: "release", "testing", "devel", ...
 *
 * @note Flavor is meaningful only in rnd mode.
 *
 * @see #ssusysinfo_ssu_in_rnd_mode()
 *
 * @return flavour name
 */
const char *ssusysinfo_ssu_flavour(ssusysinfo_t *self);

/** Query ssu domain setting
 *
 * Currently fetches "domain" value from "General" section in ssu.ini.
 *
 * Expected values: "sales", "jolla", "<customer_name>"
 *
 * @return domain name
 */
const char *ssusysinfo_ssu_domain(ssusysinfo_t *self);

/** Query ssu release version
 *
 * Returns rnd / sales release version depending on device mode.
 *
 * Expected values: "1.2.3.4", "latest", ...
 *
 * @see #ssusysinfo_ssu_in_rnd_mode()
 * @see #ssusysinfo_ssu_def_release()
 * @see #ssusysinfo_ssu_rnd_release()
 *
 * @return release version string
 */
const char *ssusysinfo_ssu_release(ssusysinfo_t *self);

/** Query ssu "sales" release version
 *
 * Currently fetches "release" value from "General" section in ssu.ini.
 *
 * @note This release version is used when device is not in rnd mode.
 *
 * @see #ssusysinfo_ssu_in_rnd_mode()
 * @see #ssusysinfo_ssu_release()
 *
 * @return release version string
 */
const char *ssusysinfo_ssu_def_release(ssusysinfo_t *self);

/** Query ssu "rnd" release version
 *
 * Currently fetches "rndRelease" value from "General" section in ssu.ini.
 *
 * @note This release version is used when device is in rnd mode.
 *
 * @see #ssusysinfo_ssu_in_rnd_mode()
 * @see #ssusysinfo_ssu_release()
 *
 * @return release version string
 */
const char *ssusysinfo_ssu_rnd_release(ssusysinfo_t *self);

/** Query ssu enabled repositories setting
 *
 * Currently fetches "enabled-repos" value from "General" section in ssu.ini.
 *
 * Expected values: "tools, store, something_else", ...
 *
 * Used for adding custom/user repositories.
 *
 * @return list of repository names
 */
const char *ssusysinfo_ssu_enabled_repos(ssusysinfo_t *self);

/** Query ssu disabled repositories setting
 *
 * Currently fetches "disabled-repos" value from "General" section in ssu.ini.
 *
 * Expected values: "home, something_else", ...
 *
 * Used for blacklisting repositories.
 *
 * @return list of repository names
 */
const char *ssusysinfo_ssu_disabled_repos(ssusysinfo_t *self);

/** Query ssu last credential update timestamp
 *
 * Translates "lastCredentialsUpdate" value from "General" section in ssu.ini file
 * into a ISO-8601 compatible time value ("yyyy-mm-ddThh:mm:ss±hh:mm").
 *
 * @return last credential update time
 */
const char *ssusysinfo_ssu_last_credentials_update(ssusysinfo_t *self);

/** Query ssu credentials scope setting
 *
 * Currently fetches "credentials-scope" value from "General" section in ssu.ini.
 *
 * Expected values: "jolla", ...
 *
 * @return scope name
 */
const char *ssusysinfo_ssu_credentials_scope(ssusysinfo_t *self);

/** Query ssu jolla credentials url setting
 *
 * Currently fetches "credentials-url-jolla" value from "General" section in ssu.ini.
 *
 * @return url
 */
const char *ssusysinfo_ssu_credentials_url_jolla(ssusysinfo_t *self);

/** Query ssu store credentials url setting
 *
 * Currently fetches "credentials-url-store" value from "General" section in ssu.ini.
 *
 * @return url
 */
const char *ssusysinfo_ssu_credentials_url_store(ssusysinfo_t *self);

/** Query ssu default rnd domain setting
 *
 * Currently fetches "default-rnd-domain" value from "General" section in ssu.ini.
 *
 * Expected values: "jolla", ...
 *
 * Used as fallback domain in registration.
 *
 * @return domain name
 */
const char *ssusysinfo_ssu_default_rnd_domain(ssusysinfo_t *self);

/** Query ssu home url setting
 *
 * Currently fetches "home-url" value from "General" section in ssu.ini.
 *
 * Used for testing / automation.
 *
 * @return url
 */
const char *ssusysinfo_ssu_home_url(ssusysinfo_t *self);

/** Query OS name
 *
 * @since ssu-sysinfo 1.4.0
 *
 * Currently fetches "NAME=<name>" record from /etc/os-release file.
 *
 * Expected values: "Sailfish OS", ...
 *
 * @return os name
 */
const char *ssusysinfo_os_name(ssusysinfo_t *self);

/** Query OS version number
 *
 * @since ssu-sysinfo 1.4.0
 *
 * Currently fetches "VERSION_ID=<version>" record from /etc/os-release file.
 *
 * Expected values: "4.2.0.10", ...
 *
 * @return os version
 */
const char *ssusysinfo_os_version(ssusysinfo_t *self);

/** Query OS version description
 *
 * @since ssu-sysinfo 1.4.0
 *
 * Currently fetches "VERSION=<version_description>" record from /etc/os-release file.
 *
 * Expected values: "4.2.0.10 (Verla)", "4.2.0.11 (Verla) (devel)", ...
 *
 * @return os version description
 */
const char *ssusysinfo_os_pretty_version(ssusysinfo_t *self);

/** Query hw version number
 *
 * @since ssu-sysinfo 1.4.0
 *
 * Currently fetches "VERSION_ID=<version>" record from /etc/hw-release file.
 *
 * Expected values: "0.0.4.338", ...
 *
 * @return hw version
 */
const char *ssusysinfo_hw_version(ssusysinfo_t *self);

/** Query hw version description
 *
 * @since ssu-sysinfo 1.4.0
 *
 * Currently fetches "VERSION=<version_description>" record from /etc/hw-release file.
 *
 * Expected values: "0.0.4.338", "0.0.4.338 (devel)", ...
 *
 * @return hw version description
 */
const char *ssusysinfo_hw_pretty_version(ssusysinfo_t *self);

/** Query circuit board version description
 *
 * @since ssu-sysinfo 1.5.0
 *
 * Currently fetches version string from /sys/firmware/devicetree/base/model
 *
 * Expected values: "Spreadtrum UMS9230 1H10 SoC", ...
 *
 * @return hw version description, or "UNKNOWN" if not available
 */
const char *ssusysinfo_board_version(ssusysinfo_t *self);

/** HW features available on the device
 *
 * The original and primary use for the hw feature configuration is
 * to define what hw tests should be made available in the CSD test
 * application.
 *
 * As "a hw feature should be tested" generally means that such feature
 * is available, the csd config has secondary uses for example in
 * settings application where items that are not applicable to the
 * device can be hidden from UI.
 */
typedef enum {
    /** Placeholder value for error situations etc */
    Feature_Invalid,

    /** Has primary microphone */
    Feature_Microphone1,

    /** Has secondary microphone */
    Feature_Microphone2,

    /** Has back facing camera */
    Feature_BackCamera,

    /** Has flashlight for back facing camera */
    Feature_BackCameraFlashlight,

    /** Has controllable display brightness */
    Feature_DisplayBacklight,

    /** Has chargeable battery */
    Feature_Battery,

    /** Has bluetooth radio */
    Feature_Bluetooth,

    /** Has cellular data capability */
    Feature_CellularData,

    /** Has cellular voice capability */
    Feature_CellularVoice,

    /** Has electronic compass sensor */
    Feature_CompassSensor,

    /** Has FM radio receiver */
    Feature_FMRadioReceiver,

    /** Has front facing camera */
    Feature_FrontCamera,

    /** Has flashlight for front facing camera */
    Feature_FrontCameraFlashlight,

    /** Has GPS receiver */
    Feature_GPS,

    /** Can provide details of connected cells */
    Feature_CellInfo,

    /** Has accelerometer */
    Feature_AccelerationSensor,

    /** Has gyroscope */
    Feature_GyroSensor,

    /** Has display cover sensor */
    Feature_CoverSensor,

    /** Has fingerprint sensor */
    Feature_FingerprintSensor,

    /** Supports pluggable headset */
    Feature_Headset,

    /** Has special purpose hardware keys */
    Feature_HardwareKeys,

    /** Has display */
    Feature_Display,

    /** Has notification LED */
    Feature_NotificationLED,

    /** Has separate backlight for buttons */
    Feature_ButtonBacklight,

    /** Has ambient light sensor */
    Feature_LightSensor,

    /** Has loudspeaker */
    Feature_Loudspeaker,

    /** Supports The-Other-Half covers */
    Feature_TheOtherHalf,

    /** Has proximity sensor */
    Feature_ProximitySensor,

    /** Can do audio playback */
    Feature_AudioPlayback,

    /** Has SD card slot */
    Feature_MemoryCardSlot,

    /** Has SIM card slot(s) */
    Feature_SIMCardSlot,

    /** Has stereo loudspeaker */
    Feature_StereoLoudspeaker,

    /** Has display with touch input */
    Feature_TouchScreen,

    /** Can perform touch input self test */
    Feature_TouchScreenSelfTest,

    /** Supports USB charging */
    Feature_USBCharging,

    /** Supports USB On-The-Go */
    Feature_USBOTG,

    /** Has vibrator */
    Feature_Vibrator,

    /** Has WLAN functionality */
    Feature_WLAN,

    /** Has NFC functionality */
    Feature_NFC,

    /** Can do video playback */
    Feature_VideoPlayback,

    /** Device can be suspended */
    Feature_Suspend,

    /** Device can be rebooted */
    Feature_Reboot,

    /** Device supports network tethering of bluetooth devices */
    Feature_BluetoothTethering,

    /** Number of known hw features */
    Feature_Count
} hw_feature_t;

/** Get array of supported hw features
 *
 * @param self ssusysinfo object pointer
 *
 * @return zero/Feature_Invalid terminated array, or NULL
 */
hw_feature_t *ssusysinfo_get_hw_features(ssusysinfo_t *self);

/** Check if a hw feature is supported
 *
 * @param self ssusysinfo object pointer
 * @param id   hw feature enum value
 *
 * @return true if the feature is supported, false otherwise
 */
bool ssusysinfo_has_hw_feature(ssusysinfo_t *self, hw_feature_t id);

/** Convert hw feature enum value to string
 *
 * @param id  hw feature enum value
 *
 * @return name of the hw feature enum value, or NULL in case of errors
 */
const char *ssusysinfo_hw_feature_to_name(hw_feature_t id);

/** Convert string to hw feature enum value
 *
 * @param name  hw feature name
 *
 * @return enum value, or Feature_Invalid in case of errors
 */
hw_feature_t ssusysinfo_hw_feature_from_name(const char *name);

/** Get array of hw feature names
 *
 * The array contents are const strings, but caller must
 * release the array itself with free().
 *
 * @return NULL terminated array of names
 */
const char **ssusysinfo_hw_feature_names(void);

/** HW keys/buttons present on the device
 *
 * The original and primary use for the hw key configuration is
 * to define what buttons the user should be able to press during
 * CSD key verification test.
 *
 * As secondary use this can be used for example to determine
 * whether alien dalvik needs to utilize virtual android buttons
 * or not.
 *
 * As CSD is Qt/QML application, the available hw keys are defined
 * as a list of Qt::Key enumeration values (from qnamespace.h).
 *
 * However, to avoid qt build dependencies, a more abstract integer
 * type is used here.
 */
typedef uint32_t hw_key_t;

/** Get array of available hw keys
 *
 * @param self ssusysinfo object pointer
 *
 * @return zeroterminated array, or NULL
 */
hw_key_t *ssusysinfo_get_hw_keys(ssusysinfo_t *self);

/** Check if a hw key is available
 *
 * @param self ssusysinfo object pointer
 * @param id   hw key enum value
 *
 * @return true if the key is available, false otherwise
 */
bool ssusysinfo_has_hw_key(ssusysinfo_t *self, hw_key_t id);

/** Convert hw key enum value to string
 *
 * @param id  hw key enum value
 *
 * @return name of the hw key enum value, or NULL in case of errors
 */
const char *ssusysinfo_hw_key_to_name(hw_key_t id);

/** Convert string to hw key enum value
 *
 * @param name  hw key name
 *
 * @return enum value, or zero in case of errors
 */
hw_key_t ssusysinfo_hw_key_from_name(const char *name);

/** Get array of hw key names
 *
 * The array contents are const strings, but caller must
 * release the array itself with free().
 *
 * @return NULL terminated array of names
 */
const char **ssusysinfo_hw_key_names(void);

# pragma GCC visibility pop

# ifdef __cplusplus
};
# endif

#endif /* SSUSYSINFO_H_ */
