    template<class Mod>
    class ModWithSocket final : private SocketTransport, public Mod
    {
        ModuleManager & mm;
        bool target_info_is_shown = false;

    public:
        template<class... Args>
        ModWithSocket(
            ModuleManager & mm, AuthApi & /*authentifier*/,
            const char * name, unique_fd sck, uint32_t verbose,
            std::string * error_message, sock_mod_barrier /*unused*/, Args && ... mod_args)
        : SocketTransport( name, std::move(sck)
                         , mm.ini.get<cfg::context::target_host>().c_str()
                         , mm.ini.get<cfg::context::target_port>()
                         , std::chrono::milliseconds(mm.ini.get<cfg::globals::mod_recv_timeout>())
                         , to_verbose_flags(verbose), error_message)
        , Mod(*this, std::forward<Args>(mod_args)...)
        , mm(mm)
        {
            this->mm.socket_transport = this;
        }

        ~ModWithSocket()
        {
            this->mm.socket_transport = nullptr;
            detail::log_proxy_target_disconnection(
                this->mm.ini.template get<cfg::context::auth_error_message>().c_str());
        }

        void display_osd_message(std::string const & message) override {
            this->mm.osd_message(message, true);
        }

        void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
        {
            //LOG(LOG_INFO, "mod_osd::rdp_input_scancode: keyCode=0x%X keyboardFlags=0x%04X this=<%p>", param1, param3, this);
            if (this->mm.mod_osd.try_input_scancode(param1, param2, param3, param4, keymap)) {
                this->target_info_is_shown = false;
                return ;
            }

            Mod::rdp_input_scancode(param1, param2, param3, param4, keymap);

            if (this->mm.ini.template get<cfg::globals::enable_osd_display_remote_target>() && (param1 == Keymap2::F12)) {
                bool const f12_released = (param3 & SlowPath::KBDFLAGS_RELEASE);
                if (this->target_info_is_shown && f12_released) {
                    // LOG(LOG_INFO, "Hide info");
                    this->mm.clear_osd_message();
                    this->target_info_is_shown = false;
                }
                else if (!this->target_info_is_shown && !f12_released) {
                    // LOG(LOG_INFO, "Show info");
                    std::string msg;
                    msg.reserve(64);
                    if (this->mm.ini.template get<cfg::client::show_target_user_in_f12_message>()) {
                        msg  = this->mm.ini.template get<cfg::globals::target_user>();
                        msg += "@";
                    }
                    msg += this->mm.ini.template get<cfg::globals::target_device>();
                    const uint32_t enddate = this->mm.ini.template get<cfg::context::end_date_cnx>();
                    if (enddate) {
                        const auto now = time(nullptr);
                        const auto elapsed_time = enddate - now;
                        // only if "reasonable" time
                        if (elapsed_time < 60*60*24*366L) {
                            msg += "  [";
                            add_time_before_closing(msg, elapsed_time, Translator(this->mm.ini));
                            msg += ']';
                        }
                    }
                    this->mm.osd_message(std::move(msg), false);
                    this->target_info_is_shown = true;
                }
            }
        }

        void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
        {
            if (this->mm.mod_osd.try_input_mouse(device_flags, x, y, keymap)) {
                this->target_info_is_shown = false;
                return ;
            }

            Mod::rdp_input_mouse(device_flags, x, y, keymap);
        }

        void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
            Mod::rdp_input_unicode(unicode, flag);
        }

        void rdp_input_invalidate(const Rect r) override
        {
            if (this->mm.mod_osd.try_input_invalidate(r)) {
                return ;
            }

            Mod::rdp_input_invalidate(r);
        }

        void rdp_input_invalidate2(array_view<Rect const> vr) override
        {
            if (this->mm.mod_osd.try_input_invalidate2(vr)) {
                return ;
            }

            Mod::rdp_input_invalidate2(vr);
        }
    };

