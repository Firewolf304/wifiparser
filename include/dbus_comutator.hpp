//
// Created by firewolf304 on 29.01.24.
//

#ifndef WIFIPARSER_DBUS_COMUTATOR_HPP
#define WIFIPARSER_DBUS_COMUTATOR_HPP
#include "include.hpp"
namespace firewolf::wifi {
    class dbus_optimizer {
    private:
        class dbus_basic_variables {
        public:
            DBusError dbus_error;
            DBusConnection *dbus_conn = nullptr;
            DBusMessage *dbus_msg = nullptr;
            DBusMessage *dbus_reply = nullptr;
            DBusMessageIter args;
            std::string dbus_result = "";

            dbus_basic_variables() {
                dbus_error_init(&dbus_error);
                if ((this->dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error)) == nullptr) {
                    std::cerr << "Error init connect: " << dbus_error.name << " => " << dbus_error.message << std::endl;
                    return;
                }
            }
        };

        class wifi_enabled_status_changer : dbus_basic_variables {
            bool status = false;
        public:
            [[maybe_unused]] explicit wifi_enabled_status_changer(bool status) : status(status) {}

            void operator()(bool status) {
                this->status = status;
                this->change_status();
            }

            void change_status(std::string iface = "org.freedesktop.DBus.Properties",
                               std::string dest = "org.freedesktop.NetworkManager",
                               std::string method = "Set",
                               std::string param = "WirelessEnabled",
                               std::string path = "/org/freedesktop/NetworkManager") {
                dbus_result = "";
                if (nullptr == (dbus_msg = dbus_message_new_method_call(dest.c_str(),
                                                                        path.c_str(),
                                                                        iface.c_str(),
                                                                        method.c_str()))) {
                    dbus_connection_unref(dbus_conn);
                    std::cerr << "Error dbus message param call: " << dbus_error.name << " => " << dbus_error.message
                              << std::endl;
                }
                dbus_message_iter_init_append(dbus_msg, &args);
                dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &dest);
                dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &param);
                DBusMessageIter subMes;
                dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &subMes);
                dbus_bool_t value = this->status;
                dbus_message_iter_append_basic(&subMes, DBUS_TYPE_BOOLEAN, &value);
                dbus_message_iter_close_container(&args, &subMes);

                DBusPendingCall *call;
                dbus_reply = dbus_connection_send_with_reply_and_block(dbus_conn, dbus_msg, -1, &dbus_error);
                if (dbus_reply == nullptr) {
                    dbus_connection_unref(dbus_conn);
                    std::cerr << "Error dbus get reply: " << dbus_error.name << " => " << dbus_error.message
                              << std::endl;
                    dbus_error_free(&dbus_error);
                    return;
                }
                dbus_error_free(&dbus_error);
                dbus_connection_unref(dbus_conn);
                dbus_message_unref(dbus_reply);
                dbus_message_unref(dbus_msg);
            };

            [[maybe_unused]] explicit wifi_enabled_status_changer() = default;

            ~wifi_enabled_status_changer() {}
        };

        class wifi_tool : dbus_basic_variables {
        public:
            wifi_tool() {}

            ~wifi_tool() {}

            void
            get_access_points(std::string iface = "org.freedesktop.NetworkManager.Device.Wireless.GetAllAccessPoints",
                              std::string dest = "org.freedesktop.NetworkManager",
                              std::string method = "",
                              std::string param = "WirelessEnabled",
                              std::string path = "/org/freedesktop/NetworkManager/Devices/3") {
                dbus_result = "";
                if (nullptr == (dbus_msg = dbus_message_new_method_call(dest.c_str(),
                                                                        path.c_str(),
                                                                        iface.c_str(),
                                                                        method.c_str()))) {
                    dbus_connection_unref(dbus_conn);
                    std::cerr << "Error dbus message method call: " << dbus_error.name << " => " << dbus_error.message
                              << std::endl;
                }

                dbus_error_free(&dbus_error);
                dbus_connection_unref(dbus_conn);
                dbus_message_unref(dbus_reply);
                dbus_message_unref(dbus_msg);
            }
        };

    public:
        dbus_optimizer() {}

        wifi_enabled_status_changer wifiStatus;
        wifi_tool wifiTool;


    };
}
#endif //WIFIPARSER_DBUS_COMUTATOR_HPP
