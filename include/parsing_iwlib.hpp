//
// Created by firewolf on 26.01.24.
//

#include "include.hpp"
namespace firewolf::wifi {
    class devices {
    public:
        enum method {
            method_netlink = 0,
            method_iwlib = 1,
            method_getifaddrs = 2,
        };
        method selected;

        devices(method method) { this->selected = method; }

        std::vector<std::string> get() {
            switch (this->selected) {
                case method::method_netlink: {
                    return netlink_method();
                }
                case method::method_iwlib: {
                    return iwlib_method();
                }
                case method::method_getifaddrs : {
                    return default_method();
                }
                default: {
                    return default_method();
                }
            }
        }

        devices();

        std::vector<std::string> get(method method) {
            switch (method) {
                case method::method_netlink: {
                    return netlink_method();
                }
                case method::method_iwlib: {
                    return iwlib_method();
                }
                case method::method_getifaddrs : {
                    return default_method();
                }
                default: {
                    return default_method();
                }

            }
        }

        std::vector<std::string> default_method() {
            auto check_error_negative = [this](int code, const std::string &message) {
                if (code < 0) {
                    std::perror(message.data());
                    return true;
                }
                return false;
            };
            std::vector<std::string> ret;
            struct ifaddrs *addrs;
            struct ifaddrs *tmp;
            getifaddrs(&addrs);
            tmp = addrs;
            for (; tmp; tmp = tmp->ifa_next) {
                if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET) {
                    ret.push_back(tmp->ifa_name);
                }
            }
            freeifaddrs(addrs);
            return ret;
        }

        std::vector<std::string> iwlib_method() {
            auto check_error_negative = [this](int code, const std::string &message) {
                if (code < 0) {
                    std::perror(message.data());
                    return true;
                }
                return false;
            };
            std::vector<std::string> ret;
            wireless_scan_head head;
            wireless_scan *result;
            int socket = iw_sockets_open();
            //iw_enum_devices(socket, &head);


        }

        std::vector<std::string> netlink_method() {
            auto check_error_negative = [this](int code, const std::string &message) {
                if (code < 0) {
                    std::perror(message.data());
                    return true;
                }
                return false;
            };

            std::vector<std::string> ret;
            /*nl_sock *socket;
            nl_cache *cache;
            rtnl_link *linker;
            auto stop = [this, socket, cache, ret]() {
                nl_cache_free(cache);
                nl_socket_free(socket);
            };
            socket = nl_socket_alloc();
            if (check_error_negative(genl_connect(socket), "Error connect genl")) {
                stop();
            }
            if (check_error_negative(rtnl_link_alloc_cache(socket, AF_UNSPEC, &cache) * -1, "Error get cache")) {
                stop();
            }
            //nl_cache_foreach(cache, linker)

            stop();*/
            return ret;
        }

        char **argv; // крайняя мера для method_iwlib, не хочет жрать vector с string или char*
    };
    class parsing_iwlib {
    public:
        parsing_iwlib() {
            this->interface = this->get_best_device();
            this->socket = iw_sockets_open();
            if (this->socket < 0) {
                std::cerr << "Error open socket: ";
                perror("");
                delete this;
            }
            if (iw_get_range_info(this->socket, this->interface.c_str(), &this->range) < 0) {
                std::cerr << "Error getting range: ";
                perror("");
                delete this;
            }
        }

        parsing_iwlib(std::string &iface) {
            this->interface = interface;
            this->socket = iw_sockets_open();
            if (this->socket < 0) {
                std::cerr << "Error open socket: ";
                perror("");
                delete this;
            }
            if (iw_get_range_info(this->socket, this->interface.c_str(), &this->range) < 0) {
                std::cerr << "Error getting range: ";
                perror("");
                delete this;
            }
        }

        parsing_iwlib(int socket) {
            this->socket = socket;
        }
        std::string get_best_device() {
            std::string ret;
            devices dev(devices::method::method_getifaddrs);
            std::vector<std::string> devicesList = dev.get();
            int sock = iw_sockets_open();

            for(auto ifname : devicesList) {
                //cout << ifname << ": ";
                wireless_info info;
                if (iw_get_basic_config(sock, ifname.c_str(), reinterpret_cast<wireless_config *>(&info)) < 0 && info.has_range < 0 ) {
                    //std::cout << "fail" << std::endl;
                }
                else {
                    return ifname;}

            }
            iw_sockets_close(sock);
            return ret;
        }

        std::string get_device_info(std::string interface, std::string mac_format = ":") {
            std::stringstream ret;
            std::strcpy(this->device.ifr_ifrn.ifrn_name, interface.data());
            if (ioctl(this->socket, SIOCGIFHWADDR, &this->device) == 0) {
                ret << "Name: " << this->device.ifr_ifrn.ifrn_name << std::endl;
                ret << "\tMAC: " << std::dec << std::setw(2) << std::setfill('0') << std::hex;
                for (int i = 0; i < 6; i++) {
                    ret << static_cast<int>( reinterpret_cast<char>(this->device.ifr_ifru.ifru_addr.sa_data[i] ) &
                                             0xFF )
                        << ((i < 5) ? mac_format : "");
                }
                ret << std::endl;
            }
            return ret.str();
        }

        std::string get_device_info(const char *interface, std::string mac_format = ":") {
            std::stringstream ret;
            std::strcpy(this->device.ifr_ifrn.ifrn_name, interface);
            if (ioctl(this->socket, SIOCGIFHWADDR, &this->device) == 0) {
                ret << "Name: " << this->device.ifr_ifrn.ifrn_name << std::endl;
                ret << "\tMAC: " << std::dec << std::setw(2) << std::setfill('0') << std::hex;
                for (int i = 0; i < 6; i++) {
                    ret << static_cast<int>( reinterpret_cast<char>(this->device.ifr_ifru.ifru_addr.sa_data[i] ) &
                                             0xFF )
                        << ((i < 5) ? mac_format : "");
                }
                ret << std::endl;
            }
            return ret.str();
        }

        std::string get_device_info(std::string mac_format = ":") {
            std::stringstream ret;
            std::strcpy(this->device.ifr_ifrn.ifrn_name, this->interface.data());
            if (ioctl(this->socket, SIOCGIFHWADDR, &this->device) == 0) {
                ret << "Name: " << this->device.ifr_ifrn.ifrn_name << std::endl;
                ret << "\tMAC: " << std::dec << std::setw(2) << std::setfill('0') << std::hex;
                for (int i = 0; i < 6; i++) {
                    ret << static_cast<int>( reinterpret_cast<char>(this->device.ifr_ifru.ifru_addr.sa_data[i] ) &
                                             0xFF )
                        << ((i < 5) ? mac_format : "");
                }
                ret << std::endl;
            } else {
                ret << "error";
            }
            return ret.str();
        }

        std::vector<wireless_scan *> run_scan() {
            std::vector<wireless_scan *> ret;
            if (iw_scan(this->socket, this->interface.data(), this->range.we_version_compiled, &this->head) < 0) {
                std::cerr << "Error iw_scan: ";
                perror("");
                delete this;
            }
            for (wireless_scan *headder = this->head.result; headder != NULL; headder = headder->next) {
                ret.push_back(headder);
            }
            return ret;
        }


        ~parsing_iwlib() {
            iw_sockets_close(this->socket);
        }

        std::string interface = "wlp2s0";
        iwrange range;
        ifreq device;
    private:
        int socket = 0;
        wireless_scan_head head;
        wireless_scan *result;

    };
}