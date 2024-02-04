//
// Created by firewolf304 on 29.01.24.
//

#ifndef WIFIPARSER_PARSING_LIBNM_HPP
#define WIFIPARSER_PARSING_LIBNM_HPP
#include "include.hpp"
namespace firewolf::wifi {
    struct APConf;


    class parsing_libnm{
        class utils {
        public:
            static std::string gbyteArray_to_string( GByteArray * array ) {
                std::string text(array->data, array->data + array->len);
                return text;
            }
            static std::string gbytes_to_string( GBytes * array ) {
                if(array) {
                    //std::cout << g_bytes_get_size(array) << std::endl;
                    g_bytes_ref(array);
                    auto result = g_bytes_unref_to_array(array);
                    if(result) {
                        std::string text(result->data, result->data + result->len);
                        g_byte_array_free(result, false);
                        return text;
                    }
                }
                return "";
            }
            static std::string gbytes_to_string( std::shared_ptr<GBytes *> array ) {
                if( *array) {
                    g_bytes_ref(*array);
                    //std::cout << g_bytes_get_size(*array) << std::endl;
                    auto result = g_bytes_unref_to_array(*array);
                    if(result) {
                        std::string text(result->data, result->data + result->len);
                        g_byte_array_free(result, false);
                        return text;
                    }
                }
                return "";
            }
            std::shared_ptr<NMAccessPoint *> nm_access_point_convert_to_smart_point(NMAccessPoint * point) {
                return std::move(std::make_shared<NMAccessPoint *>( point));
            }
            std::shared_ptr<GBytes *> gbytes_get_ssid_smart_point(std::shared_ptr<NMAccessPoint *> point) {
                return std::move(std::make_shared<GBytes *>( nm_access_point_get_ssid(*point)));
            }
        };
    public:
        utils tools;
        std::shared_ptr<NMDevice*> device;
        NMClient *client;
        GError *error = nullptr;
        parsing_libnm() {
            signal(SIGSEGV, error_dumper);
            client = nm_client_new(nullptr, &error);
            if (!client) {
                g_error_free(error);
                throw std::runtime_error("Error: Could not connect to NetworkManager: " + (std::string)error->message);
            }
            sleep(1);
        }
        ~parsing_libnm(){
            if(this->client)
                g_object_unref(this->client);
            g_clear_error(&this->error);
            glib_autoptr_clear_GError(this->error);
        }
        std::vector<gpointer> get_devices() {
            if (!this->client) {
                throw std::runtime_error("Client closed");
            }
            std::unique_ptr<const GPtrArray*> devices_ptr = std::make_unique<const GPtrArray*>(nm_client_get_devices(client));
            auto ret = std::vector<gpointer>((*devices_ptr)->pdata, (*devices_ptr)->pdata + (int)(*devices_ptr)->len);
            return ret;
        }
        std::vector<NMDevice *> get_devices_fixed() {
            auto row = get_devices();
            std::vector<NMDevice * > mass;
            for(auto data : row) {
                mass.push_back( static_cast<NMDevice *>(data) );
            }
            return mass;
        }
        std::vector<gpointer> get_devices(NMClient * client_ptr) {
            if (!client_ptr) {
                throw std::runtime_error("Client closed");
            }
            std::unique_ptr<const GPtrArray*> devices_ptr = std::make_unique<const GPtrArray*>(nm_client_get_devices(client_ptr));
            auto ret = std::vector<gpointer>((*devices_ptr)->pdata, (*devices_ptr)->pdata + (int)(*devices_ptr)->len);
            return ret;
        }
        std::vector<NMDevice *> get_devices_fixed(NMClient * client_ptr) {
            auto row = get_devices(client_ptr);
            std::vector<NMDevice * > mass;
            for(auto data : row) {
                mass.push_back( static_cast<NMDevice *>(data) );
            }
            return mass;
        }

        std::shared_ptr<NMDevice * > get_device_by_name_smart(std::string name ) const {
            return std::make_shared<NMDevice *>( nm_client_get_device_by_iface(this->client, name.c_str()) );
        }

        NMDevice * get_device_by_name(std::string & name ) const {
            return ( nm_client_get_device_by_iface(this->client, name.c_str()) );
        }
        std::shared_ptr<NMDeviceWifi * > get_device_wifi_by_name_smart(std::string name ) const {
            return std::make_shared<NMDeviceWifi *>( NM_DEVICE_WIFI(nm_client_get_device_by_iface(this->client, name.c_str())) );
        }
        NMDeviceWifi * get_device_wifi_by_name(std::string  name ) const {
            return NM_DEVICE_WIFI(nm_client_get_device_by_iface(this->client, name.c_str()));
        }
        std::vector<gpointer> get_access_points( std::shared_ptr<NMDevice*> device ) {
            auto endPoints = nm_device_wifi_get_access_points(NM_DEVICE_WIFI(*device) );
            auto AccessPoints = std::vector<gpointer>(endPoints->pdata, endPoints->pdata + endPoints->len);
            return AccessPoints;
        }
        std::vector<gpointer> get_access_points() {
            this->device = get_best_device_smart();
            if(this->device != nullptr) {
                return get_access_points(device);
            }
            throw std::runtime_error("No wifi device");
        }
        std::shared_ptr<NMDevice * > get_best_device_smart() {
            if(!this->client) {
                throw std::runtime_error("Client closed");
            }
            auto devices = get_devices();
            for(auto device : devices) {
                auto dev = static_cast<NMDevice *>(device);
                if(dev && NM_IS_DEVICE_WIFI(dev)) {
                    return std::make_shared<NMDevice *>(dev);
                }
            }
            return nullptr;
        }
        std::shared_ptr<NMDeviceWifi *> get_best_device_wifi_smart() {
            std::shared_ptr<NMDevice *> dev = get_best_device_smart();
            return std::make_shared<NMDeviceWifi *>( NM_DEVICE_WIFI(*dev) );
        }
        NMDeviceWifi * get_best_device_wifi() {
            std::shared_ptr<NMDevice *> dev = get_best_device_smart();
            return NM_DEVICE_WIFI(*dev) ;
        }
        std::vector<NMAccessPoint *> get_access_points_fixed( std::shared_ptr<NMDevice*> device ) {
            auto endPoints = nm_device_wifi_get_access_points(NM_DEVICE_WIFI(*device) );
            auto AccessPoints = std::vector<gpointer>(endPoints->pdata, endPoints->pdata + endPoints->len);
            std::vector<NMAccessPoint *> ret;
            for(auto ap : AccessPoints) {
                ret.push_back(static_cast<NMAccessPoint * >(ap));
            }
            return ret;
        }
        std::vector<NMAccessPoint *> get_access_points_fixed() {
            std::shared_ptr<NMDevice*> device = get_best_device_smart();
            if(device != nullptr) {
                return get_access_points_fixed(device);
            }
            throw std::runtime_error("No wifi device");
        }
        enum full_info_output : int{
            // ssid string
            SSID = 0,
            // mac string
            BSSID = 1,
            // flasg string
            FLAGS = 2,
            // freq guint32
            FREQ = 3,
            // mode string
            MODE = 4,
            // rsn string
            RSN_FLAGS = 5,
            // security string
            SEC_TYPE = 6,
            // wpa string
            WPA_TYPE = 7,
            // signal strength guint32
            POWER = 8,
            // delimiter '|'
            DEL = 9,
            // equal sign '='
            EQUAL = 10,
            // bracket '[' or ']'
            BRACKET = 11,
            // space ' '
            SPACE = 12,
            // hyphen '-'
            DEF = 13,
            // max power
            MAX_POWER = 14,
            // slash '/'
            SLASH = 15,
            // max bitrate guint32
            MAX_BITRATE = 16,
        };

        std::string get_access_point_full_info(std::shared_ptr< NMAccessPoint *> point, std::vector<full_info_output> format = {}) {
            std::string ssid = tools.gbytes_to_string( tools.gbytes_get_ssid_smart_point( point) );
            std::string bssid = (std::string)nm_access_point_get_bssid(*point);
            NM80211ApFlags flags_enum = nm_access_point_get_flags(*point);
            std::string flags = "";
            if(flags_enum & NM_802_11_AP_FLAGS_NONE) {
                flags = "NONE";
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_PRIVACY) {
                flags = "PRIVACY";
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_WPS) {
                flags = "WPS";
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_WPS_PBC) {
                flags = "WPS_PBC";
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_WPS_PIN) {
                flags = "WPS_PIN";
            }

            guint32 freq = nm_access_point_get_frequency(*point);
            NM80211Mode mode_enum =  nm_access_point_get_mode(*point);
            std::string mode = "";
            if (mode_enum == NM_802_11_MODE_ADHOC) {
                mode = "ADHOC";
            }
            else if (mode_enum & NM_802_11_MODE_AP) {
                mode = "AP";
            }
            else if (mode_enum & NM_802_11_MODE_INFRA) {
                mode = "INFRA";
            }
            else if (mode_enum & NM_802_11_MODE_MESH) {
                mode = "MESH";
            }
            else if (mode_enum & NM_802_11_MODE_UNKNOWN) {
                mode = "UNKNOWN";
            }
            NM80211ApSecurityFlags rsn_flags_enum = nm_access_point_get_rsn_flags(*point);
            std::string rsn_flags = "";
            if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_PSK ) {
                rsn_flags = "PSK";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_802_1X ) {
                rsn_flags = "802_1X";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192 ) {
                rsn_flags = "EAP_SUITE_B_192";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_OWE ) {
                rsn_flags = "OWE";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM ) {
                rsn_flags = "OWE_TM";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_SAE ) {
                rsn_flags = "SAE";
            }
            else {
                rsn_flags = "UNKNOW";
            }

            guint32 power = nm_access_point_get_strength(*point);
            guint32 max_bitrate = nm_access_point_get_max_bitrate(*point);
            NM80211ApSecurityFlags sec_type_enum = nm_access_point_get_wpa_flags(*point);
            std::string sec_type = "";
            if (sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP40) {
                sec_type = "WEP40";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP104) {
                sec_type = "WEP104";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_TKIP) {
                sec_type = "TKIP";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_CCMP) {
                sec_type = "CCMP";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_WEP40) {
                sec_type = "WEP40";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_WEP104) {
                sec_type = "WEP104";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_TKIP) {
                sec_type = "TKIP";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_CCMP) {
                sec_type = "CCMP";
            }
            else {
                sec_type = "UNKNOW";
            }

            std::string WPA_type = "";
            if (sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP40 || sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP104 ||
                rsn_flags_enum & NM_802_11_AP_SEC_PAIR_WEP40 || rsn_flags_enum & NM_802_11_AP_SEC_PAIR_WEP104) {
                WPA_type = "WEP";
            } else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_TKIP || sec_type_enum & NM_802_11_AP_SEC_PAIR_CCMP ||
                    rsn_flags_enum & NM_802_11_AP_SEC_PAIR_TKIP || rsn_flags_enum & NM_802_11_AP_SEC_PAIR_CCMP) {
                WPA_type = "WPA/WPA2";
            } else {
                WPA_type = "UNKNOW";
            }
            if(format.empty()) {
                auto ret = ssid + " (" + bssid + ") => [" + sec_type + " = " + WPA_type + "-" + rsn_flags + "]";
                return ret;
            }
            else {
                std::string ret = "";
                int bracket = 0;
                for(auto type : format) {
                    switch(type) {
                        case SSID: {
                            if(ssid.empty()) {
                                ret += "[null]";
                            }
                            else{
                                ret += ssid;
                            }
                        } break;
                        case BSSID:
                            ret += bssid; break;
                        case FLAGS:
                            ret += flags; break;
                        case FREQ:
                            ret += std::to_string(freq); break;
                        case MODE:
                            ret += mode; break;
                        case RSN_FLAGS:
                            ret += rsn_flags; break;
                        case SEC_TYPE:
                            ret += sec_type; break;
                        case WPA_TYPE:
                            ret += WPA_type; break;
                        case POWER:
                            ret += std::to_string(power); break;
                        case DEL:
                            ret += "|"; break;
                        case EQUAL:
                            ret += "=";break;
                        case BRACKET:
                        {
                            if(bracket <= 0) {
                                ret += "[";
                                bracket++;
                            }
                            else {
                                ret+= "]";
                                bracket--;
                            }
                        } break;
                        case SPACE:
                            ret += " "; break;
                        case DEF:
                            ret += "-"; break;
                        case SLASH:
                            ret += "/"; break;
                        case MAX_POWER:
                            ret += "100"; break;
                        case MAX_BITRATE:
                            ret += std::to_string(max_bitrate); break;
                    }
                }
                return ret;
            }

        }
        std::vector<std::string> get_access_point_full_info_vector(std::shared_ptr< NMAccessPoint *> point, std::vector<full_info_output> format = {}) {
            std::string ssid = tools.gbytes_to_string( tools.gbytes_get_ssid_smart_point( point) );
            std::string bssid = (std::string)nm_access_point_get_bssid(*point);
            NM80211ApFlags flags_enum = nm_access_point_get_flags(*point);
            std::string flags = "";
            if(flags_enum & NM_802_11_AP_FLAGS_NONE) {
                flags = "NONE";
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_PRIVACY) {
                flags = "PRIVACY";
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_WPS) {
                flags = "WPS";
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_WPS_PBC) {
                flags = "WPS_PBC";
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_WPS_PIN) {
                flags = "WPS_PIN";
            }

            guint32 freq = nm_access_point_get_frequency(*point);
            NM80211Mode mode_enum =  nm_access_point_get_mode(*point);
            std::string mode = "";
            if (mode_enum == NM_802_11_MODE_ADHOC) {
                mode = "ADHOC";
            }
            else if (mode_enum & NM_802_11_MODE_AP) {
                mode = "AP";
            }
            else if (mode_enum & NM_802_11_MODE_INFRA) {
                mode = "INFRA";
            }
            else if (mode_enum & NM_802_11_MODE_MESH) {
                mode = "MESH";
            }
            else if (mode_enum & NM_802_11_MODE_UNKNOWN) {
                mode = "UNKNOWN";
            }
            NM80211ApSecurityFlags rsn_flags_enum = nm_access_point_get_rsn_flags(*point);
            std::string rsn_flags = "";
            if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_PSK ) {
                rsn_flags = "PSK";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_802_1X ) {
                rsn_flags = "802_1X";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192 ) {
                rsn_flags = "EAP_SUITE_B_192";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_OWE ) {
                rsn_flags = "OWE";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM ) {
                rsn_flags = "OWE_TM";
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_SAE ) {
                rsn_flags = "SAE";
            }
            else {
                rsn_flags = "UNKNOW";
            }

            guint32 power = nm_access_point_get_strength(*point);
            guint32 max_bitrate = nm_access_point_get_max_bitrate(*point);
            NM80211ApSecurityFlags sec_type_enum = nm_access_point_get_wpa_flags(*point);
            std::string sec_type = "";
            if (sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP40) {
                sec_type = "WEP40";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP104) {
                sec_type = "WEP104";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_TKIP) {
                sec_type = "TKIP";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_CCMP) {
                sec_type = "CCMP";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_WEP40) {
                sec_type = "WEP40";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_WEP104) {
                sec_type = "WEP104";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_TKIP) {
                sec_type = "TKIP";
            }

            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_CCMP) {
                sec_type = "CCMP";
            }
            else {
                sec_type = "UNKNOW";
            }

            std::string WPA_type = "";
            if (sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP40 || sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP104 ||
                rsn_flags_enum & NM_802_11_AP_SEC_PAIR_WEP40 || rsn_flags_enum & NM_802_11_AP_SEC_PAIR_WEP104) {
                WPA_type = "WEP";
            } else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_TKIP || sec_type_enum & NM_802_11_AP_SEC_PAIR_CCMP ||
                       rsn_flags_enum & NM_802_11_AP_SEC_PAIR_TKIP || rsn_flags_enum & NM_802_11_AP_SEC_PAIR_CCMP) {
                WPA_type = "WPA/WPA2";
            } else {
                WPA_type = "UNKNOW";
            }
            if(format.empty()) {
                std::vector<std::string> ret = {ssid, "(", bssid, ")", "=>", "[", sec_type, "=", WPA_type, "-", rsn_flags, "]" };
                return ret;
            }
            else {
                std::vector<std::string> ret;
                int bracket = 0;
                for(auto type : format) {
                    switch(type) {
                        case SSID: {
                            if(ssid.empty()) {
                                ret.push_back("[null]");
                            }
                            else{
                                ret.push_back(ssid);
                            }
                        } break;
                        case BSSID:
                            ret.push_back(bssid); break;
                        case FLAGS:
                            ret.push_back(flags); break;
                        case FREQ:
                            ret.push_back(std::to_string(freq)); break;
                        case MODE:
                            ret.push_back(mode); break;
                        case RSN_FLAGS:
                            ret.push_back(rsn_flags); break;
                        case SEC_TYPE:
                            ret.push_back(sec_type); break;
                        case WPA_TYPE:
                            ret.push_back(WPA_type); break;
                        case POWER:
                            ret.push_back(std::to_string(power)); break;
                        case DEL:
                            ret.push_back("|"); break;
                        case EQUAL:
                            ret.push_back("=");break;
                        case BRACKET:
                        {
                            if(bracket <= 0) {
                                ret.push_back("[");
                                bracket++;
                            }
                            else {
                                ret.push_back("]");
                                bracket--;
                            }
                        } break;
                        case SPACE:
                            ret.push_back(" "); break;
                        case DEF:
                            ret.push_back("-"); break;
                        case SLASH:
                            ret.push_back("/"); break;
                        case MAX_POWER:
                            ret.push_back("100"); break;
                        case MAX_BITRATE:
                            ret.push_back( std::to_string(max_bitrate) ); break;
                    }
                }
                return ret;
            }

        }

        enum setting_type {
            setting_none = -1,
            setting_wireless = 0,
            setting_wireless_secutiry = 1,
            setting_802_11x = 2
        };
        enum value_type {
            value_none = -1,
            value_int = 0,
            value_gbyte = 1,
            value_string = 2
        };

        /*
         Map parameters:
             name, <string_name, gbyte_value, value, parameter_value, setting_type, value_type>
         Values:
             SSID = string, GBytes
             BSSID = string
             FREQ = guint32
             POWER = guint32
             MAX_BITRATE = guint32
             FLAGS = string, int
             MODE = string, int
             RSN = string, int
             SEC_TYPE = string, int
             WPA_TYPE = string
             https://developer-old.gnome.org/libnm/stable/NMSettingWirelessSecurity.html
         */
        typedef std::map<std::string, std::tuple<std::string, GBytes *, int, const char *, setting_type, value_type>> large_value;
        [[maybe_unused]] large_value get_access_point_full_info_map(std::shared_ptr< NMAccessPoint *> point) {
            std::map<std::string, std::tuple<std::string, GBytes*, int, const char *, setting_type, value_type>> ret;
            ret["SSID"] = std::make_tuple(tools.gbytes_to_string( nm_access_point_get_ssid(*point) ), nm_access_point_get_ssid(*point), -1, NM_SETTING_WIRELESS_SSID, setting_wireless, value_gbyte);
            //ret["SSID"].emplace<4>(2);
            //ret["SSID"].emplace<5>( "ssid" );
            ret["BSSID"] = std::make_tuple((std::string)nm_access_point_get_bssid(*point), nullptr, -1, NM_SETTING_WIRELESS_BSSID, setting_wireless, value_string);
            //ret["SSID"].emplace<4>(1);
            //ret["BSSID"].emplace<5>( "bssid" );

            ret["FREQ"] = std::make_tuple("", nullptr, nm_access_point_get_frequency(*point), nullptr, setting_none, value_int);
            ret["POWER"] = std::make_tuple("", nullptr, nm_access_point_get_strength(*point), nullptr, setting_none, value_int);
            ret["MAX_BITRATE"] = std::make_tuple("", nullptr, nm_access_point_get_max_bitrate(*point), nullptr, setting_none, value_int);
            NM80211ApFlags flags_enum = nm_access_point_get_flags(*point);
            if(flags_enum & NM_802_11_AP_FLAGS_NONE) {
                ret["FLAGS"] = std::make_tuple("none", nullptr, flags_enum, nullptr,  setting_none, value_int);
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_PRIVACY) {
                ret["FLAGS"] = std::make_tuple("privacy", nullptr, flags_enum, nullptr, setting_none, value_int);
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_WPS) {
                ret["FLAGS"] = std::make_tuple("wps", nullptr, flags_enum, nullptr, setting_none, value_int);
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_WPS_PBC) {
                ret["FLAGS"] = std::make_tuple("wps-pbc", nullptr, flags_enum, nullptr, setting_none, value_int);
            }
            else if(flags_enum & NM_802_11_AP_FLAGS_WPS_PIN) {
                ret["FLAGS"] = std::make_tuple("wps-pin", nullptr, flags_enum, nullptr, setting_none, value_int);
            }
            NM80211Mode mode_enum =  nm_access_point_get_mode(*point);
            if (mode_enum & NM_802_11_MODE_ADHOC) {
                ret["MODE"] = std::make_tuple("adhoc", nullptr, mode_enum, nullptr, setting_wireless, value_string);
            }
            else if (mode_enum & NM_802_11_MODE_AP) {
                ret["MODE"] = std::make_tuple("ap", nullptr, mode_enum, nullptr, setting_wireless, value_string);
            }
            else if (mode_enum & NM_802_11_MODE_INFRA) {
                ret["MODE"] = std::make_tuple("infra", nullptr, mode_enum, nullptr, setting_wireless, value_string);
            }
            else if (mode_enum & NM_802_11_MODE_MESH) {
                ret["MODE"] = std::make_tuple("mesh", nullptr, mode_enum, nullptr, setting_wireless, value_string);
            }
            else if (mode_enum & NM_802_11_MODE_UNKNOWN) {
                ret["MODE"] = std::make_tuple("unknow", nullptr, mode_enum, nullptr, setting_wireless, value_string);
            }

            NM80211ApSecurityFlags rsn_flags_enum = nm_access_point_get_rsn_flags(*point);
            if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_PSK ) {
                ret["RSN"] = std::make_tuple("wpa-psk", nullptr, rsn_flags_enum, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, setting_wireless_secutiry, value_string);
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_802_1X ) {
                ret["RSN"] = std::make_tuple("wpa-802-1x", nullptr, rsn_flags_enum, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, setting_wireless_secutiry, value_string);
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192 ) {
                ret["RSN"] = std::make_tuple("wpa-eap-suite-b-192", nullptr, rsn_flags_enum, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, setting_wireless_secutiry, value_string);
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_OWE ) {
                ret["RSN"] = std::make_tuple("owe", nullptr, rsn_flags_enum, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, setting_wireless_secutiry, value_string);
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM ) {
                ret["RSN"] = std::make_tuple("owe-tm", nullptr, rsn_flags_enum, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, setting_wireless_secutiry, value_string);
            }
            else if(rsn_flags_enum & NM_802_11_AP_SEC_KEY_MGMT_SAE ) {
                ret["RSN"] = std::make_tuple("sae", nullptr, rsn_flags_enum, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, setting_wireless_secutiry, value_string);
            }
            else {
                ret["RSN"] = std::make_tuple("none", nullptr, rsn_flags_enum, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, setting_wireless_secutiry, value_string);
            }
            NM80211ApSecurityFlags sec_type_enum = nm_access_point_get_wpa_flags(*point);
            /*if (sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP40) {
                ret["SEC_TYPE"] = std::make_tuple("pair_wep40", nullptr, sec_type_enum, NM_SETTING_WIRELESS_SECURITY_PAIRWISE, setting_none, value_int);
            }
            else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP104) {
                ret["SEC_TYPE"] = std::make_tuple("pair_wep104", nullptr, sec_type_enum, NM_SETTING_WIRELESS_SECURITY_PAIRWISE, setting_none, value_int);
            }*/
            if (sec_type_enum & NM_802_11_AP_SEC_PAIR_TKIP) {
                ret["SEC_TYPE"] = std::make_tuple("tkip", nullptr, sec_type_enum, NM_SETTING_WIRELESS_SECURITY_PAIRWISE, setting_none, value_int);
            }
            else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_CCMP) {
                ret["SEC_TYPE"] = std::make_tuple("ccmp", nullptr, sec_type_enum, NM_SETTING_WIRELESS_SECURITY_PAIRWISE, setting_none, value_int);
            }
            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_WEP40) {
                ret["SEC_TYPE"] = std::make_tuple("wep40", nullptr, sec_type_enum, NM_SETTING_WIRELESS_SECURITY_GROUP, setting_none, value_int);
            }
            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_WEP104) {
                ret["SEC_TYPE"] = std::make_tuple("wep104", nullptr, sec_type_enum, NM_SETTING_WIRELESS_SECURITY_GROUP, setting_none, value_int);
            }
            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_TKIP) {
                ret["SEC_TYPE"] = std::make_tuple("tkip", nullptr, sec_type_enum, NM_SETTING_WIRELESS_SECURITY_GROUP, setting_none, value_int);
            }
            else if (sec_type_enum & NM_802_11_AP_SEC_GROUP_CCMP) {
                ret["SEC_TYPE"] = std::make_tuple("ccmp", nullptr, sec_type_enum, NM_SETTING_WIRELESS_SECURITY_GROUP, setting_none, value_int);
            }
            else {
                ret["SEC_TYPE"] = std::make_tuple("none", nullptr, sec_type_enum, nullptr, setting_none, value_int);
            }
            if (sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP40 || sec_type_enum & NM_802_11_AP_SEC_PAIR_WEP104 ||
                rsn_flags_enum & NM_802_11_AP_SEC_PAIR_WEP40 || rsn_flags_enum & NM_802_11_AP_SEC_PAIR_WEP104) {
                ret["WPA_TYPE"] = std::make_tuple("wep", nullptr, -1, nullptr, setting_none, value_int);
            } else if (sec_type_enum & NM_802_11_AP_SEC_PAIR_TKIP || sec_type_enum & NM_802_11_AP_SEC_PAIR_CCMP ||
                       rsn_flags_enum & NM_802_11_AP_SEC_PAIR_TKIP || rsn_flags_enum & NM_802_11_AP_SEC_PAIR_CCMP) {
                ret["WPA_TYPE"] = std::make_tuple("wpa/wpa2", nullptr, -1, nullptr, setting_none, value_int);
            } else {
                ret["WPA_TYPE"] = std::make_tuple("unknow", nullptr, -1, nullptr, setting_none, value_int);
            }
            return ret;
        }
        std::string get_access_point_full_info(NMAccessPoint * point, std::vector<full_info_output> format = {}) {
            return get_access_point_full_info( std::shared_ptr<NMAccessPoint *>(&point), format );
        }
        struct _dependencies {
            bool s_wireless = false,
                    s_8021x = false,
                    s_sec = false;
        };
        struct APConf {
            _dependencies dependencies;
            std::map<std::string, std::tuple<std::string, GBytes *, int, const char *, parsing_libnm::setting_type, parsing_libnm::value_type>> info;
            //get all data using default pattern
            void get_all(parsing_libnm & libnm, std::shared_ptr<_NMAccessPoint *> point) {
                this->info = libnm.get_access_point_full_info_map(point);
                if(std::get<std::string>(this->info["RSN"]) == "wpa-psk" ) {
                    dependencies = {true, false, true};
                }
                else if(std::get<std::string>(this->info["RSN"]) == "wpa-802-1x" ) {
                    dependencies = {true, true, false};
                }
                else if(std::get<std::string>(this->info["RSN"]) == "none" ) {
                    dependencies = {true, false, false};
                }
            }

        };
        std::vector<std::pair<NMAccessPoint *, large_value>> get_access_points_large() {
            std::vector<std::pair<NMAccessPoint *, large_value>> ret;
            auto aps = get_access_points_fixed();
            for(auto d : aps ) {
                ret.push_back({d, get_access_point_full_info_map(tools.nm_access_point_convert_to_smart_point(d)) });
            }
            return ret;
        }
        void simple_connection(std::string iface, std::string password, std::shared_ptr<NMAccessPoint *> point) {
            std::shared_ptr<NMDeviceWifi *> dev = this->get_device_wifi_by_name_smart(iface);
            NMConnection *connection = nm_simple_connection_new();
            NMSetting *s_wireless = nm_setting_wireless_new ();
            NMSetting *s_8021x = nm_setting_802_1x_new();
            NMSetting  *s_sec = nm_setting_wireless_security_new();
            APConf conf;
            conf.get_all(*this, this->tools.nm_access_point_convert_to_smart_point( *point ));
            if(dev) {
                NMConnection *connection = nm_simple_connection_new();
                NMDeviceWifi *device;
                NMAccessPoint *ap;
                NMSetting *s_wireless = nm_setting_wireless_new ();
                NMSetting *s_8021x = nm_setting_802_1x_new();
                NMSetting  *s_sec = nm_setting_wireless_security_new();

                //std::cout << std::get<std::string>(save) << std::endl;


                for(auto d : conf.info) {
                    auto val = std::get<setting_type>(d.second);
                    if(val != setting_none) {
                        //std::cout << std::get<std::string>(d.second) << std::endl;
                        switch(val) {
                            case firewolf::wifi::parsing_libnm::setting_wireless: {
                                switch(std::get<value_type>(d.second)) {
                                    case firewolf::wifi::parsing_libnm::value_int:
                                        g_object_set(s_wireless, std::get<const char *>(d.second), std::get<int>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_gbyte:
                                        g_object_set(s_wireless, std::get<const char *>(d.second), std::get<GBytes *>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_string:
                                        g_object_set(s_wireless, std::get<const char *>(d.second), std::get<std::string>(d.second).c_str(), nullptr);
                                        break;
                                }
                            } break;
                            case firewolf::wifi::parsing_libnm::setting_wireless_secutiry: {
                                switch(std::get<value_type>(d.second)) {
                                    case firewolf::wifi::parsing_libnm::value_int:
                                        g_object_set(s_sec, std::get<const char *>(d.second), std::get<int>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_gbyte:
                                        g_object_set(s_sec, std::get<const char *>(d.second), std::get<GBytes *>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_string:
                                        g_object_set(s_sec, std::get<const char *>(d.second), std::get<std::string>(d.second).c_str(), nullptr);
                                        break;
                                    case value_none:
                                        break;
                                }
                            } break;
                            case firewolf::wifi::parsing_libnm::setting_802_11x: {
                                switch(std::get<value_type>(d.second)) {
                                    case firewolf::wifi::parsing_libnm::value_int:
                                        g_object_set(s_8021x, std::get<const char *>(d.second), std::get<int>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_gbyte:
                                        g_object_set(s_8021x, std::get<const char *>(d.second), std::get<GBytes *>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_string:
                                        g_object_set(s_8021x, std::get<const char *>(d.second), std::get<std::string>(d.second).c_str(), nullptr);
                                        break;
                                    case value_none:
                                        break;
                                }
                            } break;
                        }
                    }
                }
                /*g_object_set(s_wireless, NM_SETTING_WIRELESS_BSSID,std::get<GBytes *>(conf.info ["SSID"]), nullptr);
                g_object_set(s_wireless, NM_SETTING_WIRELESS_BSSID,std::get<std::string>(conf.info["BSSID"]).c_str(), nullptr);
                g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,"wpa-802-1x", nullptr);
                g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_PSK,"412587412587", nullptr);*/
                //std::cout << (conf.dependencies.s_wireless ? "true" : "false") << " " << (conf.dependencies.s_8021x ? "true" : "false") << " " << (conf.dependencies.s_sec ? "true" : "false") << std::endl;
                if(conf.dependencies.s_wireless) {
                    nm_connection_add_setting (connection, s_wireless);
                }
                if(conf.dependencies.s_8021x) {
                    g_object_set(s_sec, NM_SETTING_802_1X_PASSWORD, password.c_str(), nullptr);
                    nm_connection_add_setting(connection, s_8021x);
                }
                if(conf.dependencies.s_sec) {
                    g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_PSK, password.c_str(), nullptr);
                    nm_connection_add_setting(connection, s_sec);
                }
                GMainLoop *loop = g_main_loop_new(nullptr, false);

                std::string pathObj = nm_object_get_path(NM_OBJECT(*point));
                nm_client_add_and_activate_connection_async( this->client, connection, NM_DEVICE(*dev), pathObj.c_str(), nullptr, check_auth, loop);
                //nm_client_activate_connection_async(libnm.client, connection, NM_DEVICE(dev), pathObj.c_str(), cancel, check_auth, loop);
                g_main_loop_run(loop);
                g_main_loop_unref(loop);
            }
            else {
                return;
            }
        }
        void simple_connection(std::string password, std::shared_ptr<NMAccessPoint *> point) {
            std::shared_ptr<NMDeviceWifi *> dev = this->get_best_device_wifi_smart();
            this->simple_connection(dev, password, point);
        }
        void simple_connection(std::shared_ptr<NMDeviceWifi *> dev, std::string password, std::shared_ptr<NMAccessPoint *> point) {
            NMConnection *connection = nm_simple_connection_new();
            NMSetting *s_wireless = nm_setting_wireless_new ();
            NMSetting *s_8021x = nm_setting_802_1x_new();
            NMSetting  *s_sec = nm_setting_wireless_security_new();
            APConf conf;
            conf.get_all(*this, this->tools.nm_access_point_convert_to_smart_point( *point ));
            if(dev) {
                NMConnection *connection = nm_simple_connection_new();
                NMDeviceWifi *device;
                NMAccessPoint *ap;
                NMSetting *s_wireless = nm_setting_wireless_new ();
                NMSetting *s_8021x = nm_setting_802_1x_new();
                NMSetting  *s_sec = nm_setting_wireless_security_new();

                //std::cout << std::get<std::string>(save) << std::endl;


                for(auto d : conf.info) {
                    auto val = std::get<setting_type>(d.second);
                    if(val != setting_none) {
                        //std::cout << std::get<std::string>(d.second) << std::endl;
                        switch(val) {
                            case firewolf::wifi::parsing_libnm::setting_wireless: {
                                switch(std::get<value_type>(d.second)) {
                                    case firewolf::wifi::parsing_libnm::value_int:
                                        g_object_set(s_wireless, std::get<const char *>(d.second), std::get<int>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_gbyte:
                                        g_object_set(s_wireless, std::get<const char *>(d.second), std::get<GBytes *>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_string:
                                        g_object_set(s_wireless, std::get<const char *>(d.second), std::get<std::string>(d.second).c_str(), nullptr);
                                        break;
                                }
                            } break;
                            case firewolf::wifi::parsing_libnm::setting_wireless_secutiry: {
                                switch(std::get<value_type>(d.second)) {
                                    case firewolf::wifi::parsing_libnm::value_int:
                                        g_object_set(s_sec, std::get<const char *>(d.second), std::get<int>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_gbyte:
                                        g_object_set(s_sec, std::get<const char *>(d.second), std::get<GBytes *>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_string:
                                        g_object_set(s_sec, std::get<const char *>(d.second), std::get<std::string>(d.second).c_str(), nullptr);
                                        break;
                                    case value_none:
                                        break;
                                }
                            } break;
                            case firewolf::wifi::parsing_libnm::setting_802_11x: {
                                switch(std::get<value_type>(d.second)) {
                                    case firewolf::wifi::parsing_libnm::value_int:
                                        g_object_set(s_8021x, std::get<const char *>(d.second), std::get<int>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_gbyte:
                                        g_object_set(s_8021x, std::get<const char *>(d.second), std::get<GBytes *>(d.second), nullptr);
                                        break;
                                    case firewolf::wifi::parsing_libnm::value_string:
                                        g_object_set(s_8021x, std::get<const char *>(d.second), std::get<std::string>(d.second).c_str(), nullptr);
                                        break;
                                    case value_none:
                                        break;
                                }
                            } break;
                        }
                    }
                }
                /*g_object_set(s_wireless, NM_SETTING_WIRELESS_BSSID,std::get<GBytes *>(conf.info ["SSID"]), nullptr);
                g_object_set(s_wireless, NM_SETTING_WIRELESS_BSSID,std::get<std::string>(conf.info["BSSID"]).c_str(), nullptr);
                g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,"wpa-802-1x", nullptr);
                g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_PSK,"412587412587", nullptr);*/
                //std::cout << (conf.dependencies.s_wireless ? "true" : "false") << " " << (conf.dependencies.s_8021x ? "true" : "false") << " " << (conf.dependencies.s_sec ? "true" : "false") << std::endl;
                if(conf.dependencies.s_wireless) {
                    nm_connection_add_setting (connection, s_wireless);
                }
                if(conf.dependencies.s_8021x) {
                    g_object_set(s_sec, NM_SETTING_802_1X_PASSWORD, password.c_str(), nullptr);
                    nm_connection_add_setting(connection, s_8021x);
                }
                if(conf.dependencies.s_sec) {
                    g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_PSK, password.c_str(), nullptr);
                    nm_connection_add_setting(connection, s_sec);
                }
                GMainLoop *loop = g_main_loop_new(nullptr, false);

                std::string pathObj = nm_object_get_path(NM_OBJECT(*point));
                nm_client_add_and_activate_connection_async( this->client, connection, NM_DEVICE(*dev), pathObj.c_str(), nullptr, check_auth, loop);
                //nm_client_activate_connection_async(libnm.client, connection, NM_DEVICE(dev), pathObj.c_str(), cancel, check_auth, loop);
                g_main_loop_run(loop);
                g_main_loop_unref(loop);
            }
            else {
                return;
            }
        }
        struct specific_data {
            std::shared_ptr<GMainLoop *> loop;
            std::shared_ptr<NMDeviceWifi *> dev;
            std::shared_ptr<NMAccessPoint *> point;
            std::shared_ptr<NMConnection *> conn;
            std::string password;
            std::shared_ptr<bool> verified;
        };
        void spam_connestion() {
            //bruteforse conf
            int len = 5;
            int low_char = 48;
            int max_char = 57;
            int step = 10;
            //conn conf
            std::string iface = "wlp3s0";
            std::string ssid = "Test_test_2g";
            //std::string password(5, static_cast<char>(low_char));
            std::string password = "12345678";

            auto aps = get_access_points_large(); // access points data
            NMDeviceWifi * dev = get_best_device_wifi();
            //NMDeviceWifi * dev = libnm.get_device_wifi_by_name(iface);

            NMAccessPoint * point;
            try {
                point = std::find_if( aps.begin(), aps.end(), [ssid](auto it) -> bool {
                    return std::get<std::string>(it.second["SSID"]) == ssid;
                } )->first;
            } catch (std::runtime_error & e ) { throw std::logic_error("No access point list"); }
            APConf conf;
            conf.get_all(*this, this->tools.nm_access_point_convert_to_smart_point( point ));
            if(dev) {

                NMDeviceWifi *device;
                NMAccessPoint *ap;

                NMSetting *s_actiovate = nm_setting_connection_new();

                //std::cout << std::get<std::string>(save) << std::endl;


                /*g_object_set(s_wireless, NM_SETTING_WIRELESS_BSSID,std::get<GBytes *>(conf.info ["SSID"]), nullptr);
                g_object_set(s_wireless, NM_SETTING_WIRELESS_BSSID,std::get<std::string>(conf.info["BSSID"]).c_str(), nullptr);
                g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,"wpa-802-1x", nullptr);
                g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_PSK,"412587412587", nullptr);*/
                //std::cout << (conf.dependencies.s_wireless ? "true" : "false") << " " << (conf.dependencies.s_8021x ? "true" : "false") << " " << (conf.dependencies.s_sec ? "true" : "false") << std::endl;
                //      NM_SETTINGS_CONNECTION_FLAG_UNSAVED
                //g_object_set(s_actiovate, NM_SETTING_CONNECTION_SETTING_NAME, ssid.c_str(), nullptr);




                //std::cout << "verify: " << nm_connection_verify(connection, &this->error) << std::endl;

                std::string pathObj = point ? nm_object_get_path(NM_OBJECT(point)) : "/";
                //nm_client_add_connection_async(this->client, connection, true, nullptr, check_add, loop);



                //nm_client_add_and_activate_connection_async( this->client, connection, NM_DEVICE(dev), pathObj.c_str(), nullptr, check_auth, loop);
                auto connect = [this, dev, conf, &point]( std::string password) -> bool {
                    NMConnection *connection = nm_simple_connection_new();
                    signal(SIGTRAP, firewolf::wifi::parsing_libnm::error_dumper);
                    signal(SIGSEGV, firewolf::wifi::parsing_libnm::error_dumper);
                    GMainLoop *loop = g_main_loop_new(nullptr, false);
                    bool verified = false;
                    specific_data user_data = {
                            std::move(std::make_shared<GMainLoop *>( loop)),
                            std::move(std::make_shared<NMDeviceWifi *>( dev)),
                            std::move(std::make_shared<NMAccessPoint *>( point)),
                            std::move(std::make_shared<NMConnection *>( connection)),
                            password,
                            std::move(std::make_shared<bool>(verified))
                    };

                    GVariantBuilder * builder = g_variant_builder_new (G_VARIANT_TYPE_VARDICT);
                    g_variant_builder_add (builder, "{sv}", "bind-activation", g_variant_new_string ("dbus-client"));
                    g_variant_builder_add (builder, "{sv}", "persist", g_variant_new_string ("volatile"));
                    NMSetting *s_wireless = nm_setting_wireless_new ();
                    NMSetting *s_8021x = nm_setting_802_1x_new();
                    NMSetting  *s_sec = nm_setting_wireless_security_new();
                    for (auto d: conf.info) {
                        auto val = std::get<setting_type>(d.second);
                        if (val != setting_none) {
                            //std::cout << std::get<std::string>(d.second) << std::endl;
                            switch (val) {
                                case firewolf::wifi::parsing_libnm::setting_wireless: {
                                    switch (std::get<value_type>(d.second)) {
                                        case firewolf::wifi::parsing_libnm::value_int:
                                            g_object_set(s_wireless, std::get<const char *>(d.second),
                                                         std::get<int>(d.second), nullptr);
                                            break;
                                        case firewolf::wifi::parsing_libnm::value_gbyte:
                                            g_object_set(s_wireless, std::get<const char *>(d.second),
                                                         std::get<GBytes *>(d.second), nullptr);
                                            break;
                                        case firewolf::wifi::parsing_libnm::value_string:
                                            g_object_set(s_wireless, std::get<const char *>(d.second),
                                                         std::get<std::string>(d.second).c_str(), nullptr);
                                            break;
                                    }
                                }
                                    break;
                                case firewolf::wifi::parsing_libnm::setting_wireless_secutiry: {
                                    switch (std::get<value_type>(d.second)) {
                                        case firewolf::wifi::parsing_libnm::value_int:
                                            g_object_set(s_sec, std::get<const char *>(d.second),
                                                         std::get<int>(d.second), nullptr);
                                            break;
                                        case firewolf::wifi::parsing_libnm::value_gbyte:
                                            g_object_set(s_sec, std::get<const char *>(d.second),
                                                         std::get<GBytes *>(d.second), nullptr);
                                            break;
                                        case firewolf::wifi::parsing_libnm::value_string:
                                            g_object_set(s_sec, std::get<const char *>(d.second),
                                                         std::get<std::string>(d.second).c_str(), nullptr);
                                            break;
                                        case value_none:
                                            break;
                                    }
                                }
                                    break;
                                case firewolf::wifi::parsing_libnm::setting_802_11x: {
                                    switch (std::get<value_type>(d.second)) {
                                        case firewolf::wifi::parsing_libnm::value_int:
                                            g_object_set(s_8021x, std::get<const char *>(d.second),
                                                         std::get<int>(d.second), nullptr);
                                            break;
                                        case firewolf::wifi::parsing_libnm::value_gbyte:
                                            g_object_set(s_8021x, std::get<const char *>(d.second),
                                                         std::get<GBytes *>(d.second), nullptr);
                                            break;
                                        case firewolf::wifi::parsing_libnm::value_string:
                                            g_object_set(s_8021x, std::get<const char *>(d.second),
                                                         std::get<std::string>(d.second).c_str(), nullptr);
                                            break;
                                        case value_none:
                                            break;
                                    }
                                }
                                    break;
                            }
                        }
                    }
                    try {

                        if (conf.dependencies.s_wireless) {
                            nm_connection_add_setting(connection, s_wireless);
                        }
                        if (conf.dependencies.s_8021x) {
                            g_object_set(s_sec, NM_SETTING_802_1X_PASSWORD, password.c_str(), nullptr);
                            nm_connection_add_setting(connection, s_8021x);
                        }
                        if (conf.dependencies.s_sec) {
                            g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_PSK, password.c_str(), nullptr);
                            nm_connection_add_setting(connection, s_sec);
                        }

                        GVariant *options = g_variant_builder_end (builder);


                        nm_client_add_and_activate_connection2(
                                this->client, connection, NM_DEVICE(dev),
                                "/", options, nullptr, check_auth2, static_cast<void*>(&user_data) );
                        g_main_loop_run(loop);
                        auto state = nm_device_get_state(NM_DEVICE(dev));
                        std::cout << "Returned : " << *user_data.verified << std::endl;
                        nm_client_deactivate_connection(client, nm_device_get_active_connection(NM_DEVICE(dev)),
                                                        nullptr, nullptr);
                        g_main_loop_unref(loop);
                        g_object_unref(connection);
                    }
                    catch (std::runtime_error & e) {  }
                    return *user_data.verified;
                };
                bool stop = false;
                while(!stop) {
                    int check = 0;
                    for(auto iter = password.end()-1; iter != password.begin()-1; iter--) {
                        if(std::distance(password.begin(), iter) -1 > -1 ) {
                            if(*iter > max_char) {
                                *iter = low_char;
                                *(iter-1) += 1;
                                check++;
                            }
                        }
                        else {
                            if(*iter > max_char && check == len-1) {
                                stop = true;
                            }
                        }
                    }
                    if(!stop) {
                        std::cout << password << std::endl;
                        stop = connect(password);
                    }
                    if(password == "12345680") {
                        break;
                    }
                    *(password.end()-1) += 1;
                    sleep(0);
                }


            }
            else {
                return;
            }
            /*bool stop = false;
            while(!stop) {
                int check = 0;
                for(auto iter = password.end()-1; iter != password.begin()-1; iter--) {
                    if(std::distance(password.begin(), iter) -1 > -1 ) {
                        if(*iter > max_char) {
                            *iter = low_char;
                            *(iter-1) += 1;
                            check++;
                        }
                    }
                    else {
                        if(*iter > max_char && check == len-1) {
                            stop = true;
                        }
                    }
                }
                if(!stop) {
                    std::cout << password << std::endl;
                }
                *(password.end()-1) += 1;
                sleep(0);
            }*/

            //std::string brute = "00000";

        }
    private:
        static void done (GObject *source_object, GAsyncResult *res, gpointer user_data) {
            auto client = reinterpret_cast<NMClient *>(source_object);
            GError *error = nullptr;
            auto data = reinterpret_cast<specific_data*>(user_data);
            GMainLoop * main_loop = reinterpret_cast<GMainLoop *>(*data->loop);
            while(nm_device_get_state(NM_DEVICE(*data->dev)) == NM_DEVICE_STATE_CONFIG);

            if( nm_client_check_connectivity_finish(client, res,&error) == NMConnectivityState::NM_CONNECTIVITY_LIMITED ||
                nm_device_get_state(NM_DEVICE(*data->dev)) == NM_DEVICE_STATE_NEED_AUTH) {
                std::cerr << "Connection failed:" << error->message << std::endl;
            }
            else {
                std::cerr << "connected" << std::endl;
                *data->verified = true;
            }
            g_error_free(error);
            g_clear_error(&error);
            g_main_loop_quit(main_loop);
        }
        static void check_add (GObject *source_object, GAsyncResult *res, gpointer user_data) {
            GError *error = nullptr;
            GMainLoop * main_loop = reinterpret_cast<GMainLoop *>(user_data);
            std::cout << "Added" << std::endl;
            g_main_loop_quit(main_loop);
        }
        static void check_auth (GObject *source_object, GAsyncResult *res, gpointer user_data) {
            GError *error = nullptr;
            GMainLoop * main_loop = reinterpret_cast<GMainLoop *>(user_data);
            if (!nm_client_add_and_activate_connection_finish(reinterpret_cast<NMClient *>(source_object), res,&error)) {
                std::cerr << "Connection failed:" << error->message << std::endl;
            } else {
                std::cerr << "Connection success:" << std::endl;
                if(nm_client_connectivity_check_get_enabled(reinterpret_cast<NMClient *>(source_object))) {
                    std::cerr << "connecting" << std::endl;

                    GMainLoop * loop = g_main_loop_new(nullptr, false);
                    nm_client_check_connectivity_async(reinterpret_cast<NMClient *>(source_object), nullptr, done, loop);
                    g_main_loop_run(loop);
                    g_object_unref(loop);
                }
            }
            g_error_free(error);
            g_clear_error(&error);
            g_main_loop_quit(main_loop);

        }
        static void check_auth2 (GObject *source_object, GAsyncResult *res, gpointer user_data) {
            GError *error = nullptr;
            auto client = reinterpret_cast<NMClient *>(source_object);
            auto user = reinterpret_cast<specific_data*>(user_data);
            GMainLoop * main_loop = reinterpret_cast<GMainLoop *>(*user->loop);
            GVariant    *out_result;
            if (nm_client_add_and_activate_connection2_finish(client, res, &out_result ,&error) == nullptr) {
                std::cerr << "Connection failed:" << error->message << std::endl;
                g_error_free(error);
            } else {
                std::cerr << "Connection success:" << std::endl;
                if(nm_client_connectivity_check_get_enabled(client)) {
                    std::cerr << "connecting" << std::endl;
                    //GMainLoop *loop = g_main_loop_new(nullptr, false);
                    //try {
                        //specific_data data = { std::move(std::make_shared<GMainLoop *>( loop)), user->dev, user->point, user->conn, user->password, user->verified };
                        //nm_client_check_connectivity_async(reinterpret_cast<NMClient *>(source_object), nullptr, done, static_cast<void*>(&data));
                        //g_main_loop_run(loop);
                    //} catch (std::exception & e){}
                    bool activate = false;


                    bool stop = false;

                    while(true) {
                        bool ver = (bool)nm_connection_verify(NM_CONNECTION(client), &error);
                        NMState client_state = nm_client_get_state(client);
                        NMDeviceState dev_state = nm_device_get_state(NM_DEVICE(*user->dev));
                        NMDeviceStateReason dev_state_reason = nm_device_get_state_reason(NM_DEVICE(*user->dev));
                        NMActiveConnectionState ac_state = nm_active_connection_get_state(NM_ACTIVE_CONNECTION(client));
                        NMActivationStateFlags ac_state_flags = nm_active_connection_get_state_flags(NM_ACTIVE_CONNECTION(client));
                        NMActiveConnectionStateReason ac_state_reason = nm_active_connection_get_state_reason(NM_ACTIVE_CONNECTION(client));
                        NMTernary client_per_state = nm_client_get_permissions_state(client);
                        int a = 0;
                    }
                    if(activate) {
                        std::cerr << "connected" << std::endl;
                        *user->verified = true;
                    }
                    else {
                        std::cerr << "not connected" << std::endl;
                    }

                    std::cout << "Exit\n";
                    //g_object_unref(loop);
                }
            }

            g_clear_error(&error);
            g_main_loop_quit(main_loop);

        }
        static void error_dumper(int code) { throw std::runtime_error("Something wrong, status code: " + std::to_string(code)); }
    };

}
#endif //WIFIPARSER_PARSING_LIBNM_HPP
