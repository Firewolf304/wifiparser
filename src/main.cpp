
//#include <linux/wireless.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <sys/ioctl.h>
//#include <net/if.h>
//#include <arpa/inet.h>

#include "../include/include.hpp"
#include "../include/custom.hpp"

static void _dummy(const gchar *log_domain,
                   GLogLevelFlags log_level,
                   const gchar *message,
                   gpointer user_data )

{
    /* Dummy does nothing */
    return ;
}
using format=firewolf::wifi::parsing_libnm;
GCancellable *cancel1 = g_cancellable_new();
static void done (GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *error = nullptr;
    GMainLoop * main_loop = reinterpret_cast<GMainLoop *>(user_data);
    if(!nm_client_check_connectivity_finish(reinterpret_cast<NMClient *>(source_object), res,&error)) {
        std::cerr << "Connection failed:" << error->message << std::endl;
        g_error_free(error);
    }
    else {
        std::cerr << "connected" << std::endl;
        g_clear_error(&error);
        g_main_loop_quit(main_loop);
    }
}
static void check_auth (GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *error = nullptr;
    GMainLoop * main_loop = reinterpret_cast<GMainLoop *>(user_data);
    if (!nm_client_add_and_activate_connection_finish(reinterpret_cast<NMClient *>(source_object), res,&error)) {
        std::cerr << "Connection failed:" << error->message << std::endl;
        g_error_free(error);
    } else {
        std::cerr << "Connection success:" << std::endl;
        if(nm_client_connectivity_check_get_enabled(reinterpret_cast<NMClient *>(source_object))) {
            std::cerr << "connecting" << std::endl;
            GMainLoop *loop = g_main_loop_new(nullptr, false);
            nm_client_check_connectivity_async(reinterpret_cast<NMClient *>(source_object), nullptr, done, loop);
            g_main_loop_run(loop);
            g_clear_error(&error);
            g_main_loop_quit(main_loop);
        }
    }

}
static void check_auth2 (GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *error = nullptr;

    if (!nm_client_activate_connection_finish(reinterpret_cast<NMClient *>(source_object), res,&error)) {
        std::cerr << "Connection failed:" << error->message << std::endl;
        g_error_free(error);
    } else {
        std::cerr << "Connection success:" << std::endl;
        if(nm_client_connectivity_check_get_enabled(reinterpret_cast<NMClient *>(source_object))) {
            std::cerr << "connecting" << std::endl;
            GMainLoop *loop = g_main_loop_new(nullptr, false);
            nm_client_check_connectivity_async(reinterpret_cast<NMClient *>(source_object), nullptr, done, loop);
            g_error_free(error);
            g_main_loop_run(loop);
        }
    }
}
int main() {
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK, _dummy, nullptr);
    std::cout << "Hello, World!" << std::endl;
    std::string iface = "wlp3s0";
    std::string ssid = "testwifi";
    std::string password = "123123";

    /*::wifi::parsing_iwlib parse;
    std::cout << parse.get_device_info();

    for(auto d : parse.run_scan()) {
        char buff[128];
        iw_print_key(buff, sizeof(buff), d->b.key, d->b.key_size,d->b.key_flags);
        std::cout << d->b.essid << " / " << buff << " / " << iw_sawap_ntop(&d->ap_addr, buff) << " / ";
        iw_print_freq_value(buff, sizeof(buff), d->b.freq);
        std::cout << buff << " / ";
        iw_print_stats(buff, sizeof(buff), &d->stats.qual, &parse.range, 1);
        std::cout << buff << "/ \n";
        //std::cout <<  << " / \n";
    }*/

// Connecty by libnm
    firewolf::wifi::parsing_libnm libnm;
    /*auto aps = libnm.get_access_points_fixed();
    for(auto ap : aps) {
        std::cout << libnm.get_access_point_full_info(libnm.tools.nm_access_point_convert_to_smart_point(ap),
                                                       { format::SSID,
                                                         format::SPACE,
                                                         format::BRACKET,
                                                         format::BSSID,
                                                         format::BRACKET,
                                                         format::SPACE,
                                                         format::WPA_TYPE,
                                                         format::DEF,
                                                         format::RSN_FLAGS,
                                                         format::SPACE,
                                                         format::POWER,
                                                         format::SLASH,
                                                         format::MAX_POWER,
                                                         format::SPACE,
                                                         format::MAX_BITRATE
                                                       }
                                                       ) << std::endl;

    }*/
    auto aps = libnm.get_access_points_large();
    NMDeviceWifi * dev = libnm.get_best_device_wifi();
    //NMDeviceWifi * dev = libnm.get_device_wifi_by_name(iface);
    NMAccessPoint * point = std::find_if( aps.begin(), aps.end(), [ssid](auto it) -> bool {
        return std::get<std::string>(it.second["SSID"]) == ssid;
    } )->first;
    libnm.simple_connection(iface, password, libnm.tools.nm_access_point_convert_to_smart_point(point));

    /*firewolf::wifi::parsing_libnm::APConf conf;
    conf.get_all(libnm, libnm.tools.nm_access_point_convert_to_smart_point( point ));
    //auto data = libnm.get_access_point_full_info_map(libnm.tools.nm_access_point_convert_to_smart_point(aps[0]));
    if(dev) {
        NMConnection *connection = nm_simple_connection_new();
        NMDeviceWifi *device;
        NMAccessPoint *ap;
        NMSetting *s_wireless = nm_setting_wireless_new ();
        NMSetting *s_8021x = nm_setting_802_1x_new();
        NMSetting  *s_sec = nm_setting_wireless_security_new();

        //std::cout << std::get<std::string>(save) << std::endl;


        for(auto d : conf.info) {
            auto val = std::get<format::setting_type>(d.second);
            if(val != format::setting_none) {
                std::cout << std::get<std::string>(d.second) << std::endl;
                switch(val) {
                    case firewolf::wifi::parsing_libnm::setting_wireless: {
                        switch(std::get<format::value_type>(d.second)) {
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
                        switch(std::get<format::value_type>(d.second)) {
                            case firewolf::wifi::parsing_libnm::value_int:
                                g_object_set(s_sec, std::get<const char *>(d.second), std::get<int>(d.second), nullptr);
                                break;
                            case firewolf::wifi::parsing_libnm::value_gbyte:
                                g_object_set(s_sec, std::get<const char *>(d.second), std::get<GBytes *>(d.second), nullptr);
                                break;
                            case firewolf::wifi::parsing_libnm::value_string:
                                g_object_set(s_sec, std::get<const char *>(d.second), std::get<std::string>(d.second).c_str(), nullptr);
                                break;
                        }
                    } break;
                    case firewolf::wifi::parsing_libnm::setting_802_11x: {
                        switch(std::get<format::value_type>(d.second)) {
                            case firewolf::wifi::parsing_libnm::value_int:
                                g_object_set(s_8021x, std::get<const char *>(d.second), std::get<int>(d.second), nullptr);
                                break;
                            case firewolf::wifi::parsing_libnm::value_gbyte:
                                g_object_set(s_8021x, std::get<const char *>(d.second), std::get<GBytes *>(d.second), nullptr);
                                break;
                            case firewolf::wifi::parsing_libnm::value_string:
                                g_object_set(s_8021x, std::get<const char *>(d.second), std::get<std::string>(d.second).c_str(), nullptr);
                                break;
                        }
                    } break;
                }
            }
        }

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

        std::string pathObj = nm_object_get_path(NM_OBJECT(point));
        nm_client_add_and_activate_connection_async(libnm.client, connection, NM_DEVICE(dev), pathObj.c_str(), cancel1, check_auth, loop);
        //nm_client_activate_connection_async(libnm.client, connection, NM_DEVICE(dev), pathObj.c_str(), cancel, check_auth, loop);
        g_main_loop_run(loop);
        g_main_loop_unref(loop);
    }
    else {
        return -1;
    }*/
    /*g_object_set(s_wireless, NM_SETTING_WIRELESS_BSSID,std::get<GBytes *>(conf.info ["SSID"]), nullptr);
        g_object_set(s_wireless, NM_SETTING_WIRELESS_BSSID,std::get<std::string>(conf.info["BSSID"]).c_str(), nullptr);
        g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,"wpa-802-1x", nullptr);
        g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_PSK,"412587412587", nullptr);*/

// Connect by dbus for fedora
    //dbus_optimizer dbus;
    //dbus.wifiStatus(true);
    //dbus.wifiTool.get_access_points();

/* // Wifi enabled status changer
    DBusError dbus_error;
    DBusConnection * dbus_conn = nullptr;
    DBusMessage * dbus_msg;
    DBusMessage * dbus_reply = nullptr;
    DBusMessageIter args;
    std::string dbus_result;
    dbus_error_init(&dbus_error);

    if((dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error)) == nullptr) {
        std::cerr << "Error init connect: " << dbus_error.name << " => " << dbus_error.message << std::endl;
        return -1;
    }
    if ( nullptr == (dbus_msg = dbus_message_new_method_call( "org.freedesktop.NetworkManager",
                                                              "/org/freedesktop/NetworkManager",
                                                              "org.freedesktop.DBus.Properties",
                                                              "Set")) ) {
        dbus_connection_unref(dbus_conn);
        std::cerr << "Error dbus message method call: " << dbus_error.name << " => " << dbus_error.message << std::endl;
    }
    dbus_message_iter_init_append(dbus_msg, &args);
    std::string name = "org.freedesktop.NetworkManager";
    dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING, &name);
    std::string method = "WirelessEnabled";
    dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING, &method);
    dbus_bool_t value = true;
        DBusMessageIter subMes;
        dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &subMes);
    dbus_message_iter_append_basic(&subMes, DBUS_TYPE_BOOLEAN, &value);
    dbus_message_iter_close_container(&args, &subMes);

    DBusPendingCall * call;
    dbus_reply = dbus_connection_send_with_reply_and_block(dbus_conn, dbus_msg, -1, &dbus_error);
    if(dbus_reply == nullptr) {
        dbus_connection_unref(dbus_conn);
        std::cerr << "Error dbus get reply: " << dbus_error.name << " => " << dbus_error.message << std::endl;
        dbus_error_free(&dbus_error);
        return -2;
    }
    std::cout << "OK" << std::endl;*/
    return 0;
}
