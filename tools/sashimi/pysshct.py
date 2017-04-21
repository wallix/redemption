# -*- coding: utf-8 -*-

import ctypes
import ctypes.util

from ctypes import CFUNCTYPE, c_ulong, c_ulonglong, c_int, c_char_p, c_uint16, py_object, c_uint, c_uint32, c_uint64, c_float, POINTER, Structure

c_void_p = c_uint64

######### Define structs and callback types ############

# Define prototypes of callbacks.
#fonction_cb = CFUNCTYPE(c_int                    ## return value
#                , POINTER(c_int), POINTER(c_int) ## arguments
#                )
# a return type of None means void


#the event_callback used with poll events
event_cb = CFUNCTYPE(c_int, c_int, c_int, py_object)


CB_EVENT_FUNCS = {
        'event_cb': lambda fd, revent, userargp:
                        userargp.pollEvent(fd, revent)
}

class ssh_server_callbacks_struct(Structure):
    _fields_ = [
        ('size', c_ulong),
        ('userdata',  py_object),

        ('connect_status_server_cb',
            CFUNCTYPE(None,
                py_object, c_float)),
        ('global_request_server_cb',
            CFUNCTYPE(None,
                c_void_p, c_int, c_int, c_char_p, c_uint32, py_object)),
        ('auth_password_server_cb',
            CFUNCTYPE(c_int,
                c_char_p, c_char_p, py_object)),
        ('auth_none_server_cb',
            CFUNCTYPE(c_int,
                c_char_p, py_object)),
        ('auth_interactive_server_cb',
            CFUNCTYPE(c_int,
                c_char_p, c_int, py_object)),
        ('auth_gssapi_mic_server_cb',
            CFUNCTYPE(c_int,
                c_char_p, c_char_p, py_object)),
        ('auth_pubkey_server_cb',
            CFUNCTYPE(c_int,
                c_char_p, c_void_p, c_int, py_object)),
        ('service_request_server_cb', 
            CFUNCTYPE(c_int,
                c_char_p, py_object)),
        ('channel_open_request_session_server_cb', 
            CFUNCTYPE(c_void_p,
                py_object)),
        ('channel_open_request_direct_tcpip_server_cb', 
            CFUNCTYPE(c_void_p,
                c_char_p, c_uint16, c_char_p, c_uint16, py_object)),
        ('channel_open_request_forwarded_tcpip_function', 
            CFUNCTYPE(c_void_p,
                c_void_p, c_char_p, c_uint16, c_char_p, c_uint16, py_object))
    ]

CB_SERVER_SESSION_FUNCS = {
    'connect_status_server_cb': (
        lambda userargp, statusp:
            userargp.cb_connectStatus(statusp)
    ),
    'global_request_server_cb': (
        lambda sessionp, gtype, want_reply, bind_address, bind_port, userargp:
            userargp.cb_globalRequest(gtype, want_reply, bind_address, bind_port)
    ),
    'auth_password_server_cb': (
        lambda userp, passwordp, userargp:
            userargp.cb_authPassword(userp, str(passwordp))
    ),
    'auth_none_server_cb': (
        lambda userp, userargp:
            userargp.cb_authNone(userp)
    ),
    'auth_interactive_server_cb': (
        lambda userp, kbdint_response, userargp:
            userargp.cb_authInteractive(userp, kbdint_response)
    ),
    'auth_gssapi_mic_server_cb': (
        lambda userp, principalp, userargp:
           userargp.cb_authGssapiMic(userp, str(principalp))
    ),
    'auth_pubkey_server_cb': (
        lambda userp, pubkeyp, sig, userargp:
            userargp.cb_authPubkey(userp, pubkeyp, sig)
    ),
    'service_request_server_cb': (
        lambda servicep, userargp:
            userargp.cb_serviceRequest(servicep)
    ),
    'channel_open_request_session_server_cb': (
        lambda userargp:
            userargp.cb_channelOpenSessionRequest()
    ),
    'channel_open_request_direct_tcpip_server_cb': (
        lambda  destination, destination_port,
                originator, originator_port,
                userargp:
            userargp.cb_channelOpenDirectTCPIPRequest(
                        destination, destination_port, originator, originator_port)
    ),
    'channel_open_request_forwarded_tcpip_function': (
        lambda sessionp,
                destination, destination_port,
                originator, originator_port,
                userargp:
            userargp.channelOpenForwardedTCPIPRequest(
                    destination, destination_port, originator, originator_port)
    )
}


class ssh_client_callbacks_struct(Structure):
    _fields_ = [
        ('size', c_ulong),
        ('userdata', py_object),
        ('auth_none_reply_client_cb',
            CFUNCTYPE(None,
                c_int, py_object)),
        ('auth_function',
            CFUNCTYPE(c_int,
                c_char_p, c_char_p, c_uint32, c_int, c_int, py_object)),
        ('connect_status_function', 
            CFUNCTYPE(None,
                py_object, c_float)),
        ('global_request_function', 
            CFUNCTYPE(None,
                c_void_p, c_int, c_int, c_char_p, c_uint32, py_object)),
        ('channel_open_request_x11_function', 
            CFUNCTYPE(c_void_p,
                c_void_p, c_char_p, c_int, c_uint32, c_uint32, c_uint32, py_object)),
        ('channel_open_request_auth_agent_function', 
            CFUNCTYPE(c_void_p,
                c_void_p, py_object))
    ]

CB_CLIENT_SESSION_FUNCS = {
    'auth_none_reply_client_cb': (
        lambda status, userargp:
            userargp.cb_auth_none_reply_client(status)
    ),

    'auth_function': (
        lambda promptp, bufp, length, echo, verify, userargp:
            userargp.auth(promptp, bufp, length, echo, verify)
    ),
    'connect_status_function': (
        lambda userargp, statusp:
            userargp.connectStatus(statusp)
    ),
    'global_request_function': (
        lambda sessionp, gtype, want_reply, bind_address, bind_port, userargp:
            userargp.globalRequest(gtype, want_reply, bind_address, bind_port)
    ),
    'channel_open_request_x11_function': (
        lambda sessionp, originator, originator_port, sender, window, packet_size, userargp:
            userargp.channelOpenX11Request(str(originator), originator_port, sender, window, packet_size)
    ),
    'channel_open_request_auth_agent_function': (
        lambda sessionp, userargp:
            userargp.channelOpenAuthAgentRequest()
    )
}

class ssh_channel_callbacks_struct(Structure):
    _fields_ = [
        ('size', c_ulong),
        ('userdata', py_object),
        ('channel_data_function',
            CFUNCTYPE(c_int,
                c_void_p, c_void_p, c_void_p, c_uint, c_int, py_object)),
        ('channel_eof_function', 
            CFUNCTYPE(None,
                c_void_p, c_void_p, py_object)),
        ('channel_close_function', 
            CFUNCTYPE(None,
                c_void_p, c_void_p, py_object)),
        ('channel_signal_function', 
            CFUNCTYPE(None,
                c_void_p, c_void_p, c_char_p, py_object)),
        ('channel_exit_status_function', 
            CFUNCTYPE(None,
                c_void_p, c_void_p, c_int, py_object)),
        ('channel_exit_signal_function', 
            CFUNCTYPE(None, 
                c_void_p, c_void_p, c_char_p, c_int, c_char_p, c_char_p, py_object)),
        ('channel_pty_request_function', 
            CFUNCTYPE(c_int, 
                c_void_p, c_void_p, c_char_p, c_int, c_int, c_int, c_int, py_object)),
        ('channel_shell_request_function', 
            CFUNCTYPE(c_int, 
                c_void_p, c_void_p, py_object)),
        ('channel_auth_agent_req_function', 
            CFUNCTYPE(None,
                c_void_p, c_void_p, py_object)),
        ('channel_x11_req_function', 
            CFUNCTYPE(None, 
                c_void_p, c_void_p, c_int, c_char_p, c_char_p, c_uint, py_object)),
        ('channel_pty_window_change_function', 
            CFUNCTYPE(c_int, 
                c_void_p, c_void_p, c_int, c_int, c_int, c_int, py_object)),
        ('channel_exec_request_function', 
            CFUNCTYPE(c_int, 
                c_void_p, c_void_p, c_char_p, py_object)),
        ('channel_env_request_function', 
            CFUNCTYPE(c_int, 
                c_void_p, c_void_p, c_char_p, c_char_p, py_object)),
        ('channel_subsystem_request_function', 
            CFUNCTYPE(c_int, 
                c_void_p, c_void_p, c_char_p, py_object)),
        ('channel_open_x11_server_status_function', 
            CFUNCTYPE(None,
                c_void_p, c_void_p, c_int, py_object))
        ]

CB_CHANNEL_FUNCS = {
    'channel_data_function': (
        lambda sessionp, channelp, datap, length, is_stderr, userargp:
            (userargp.cb_data_stdout(ctypes.string_at(datap, length))
            if not is_stderr
            else userargp.cb_data_stderr(ctypes.string_at(datap, length)))
    ),

    'channel_eof_function': (
        lambda sessionp, channelp, userargp:
            userargp.cb_eof()
    ),

    'channel_close_function': (
        lambda sessionp, channelp, userargp:
            userargp.cb_close()
    ),

    'channel_signal_function': (
        lambda sessionp, channelp, signalp, userargp:
            userargp.cb_signal(signalp)
    ),

    'channel_exit_signal_function': (
        lambda sessionp, channelp, signalp, corep, errmsgp, langp, userargp:
            userargp.cb_exitSignal(signalp, corep, errmsgp, langp)
    ),

    'channel_exit_status_function': (
        lambda sessionp, channelp, status, userargp:
            userargp.cb_exitStatus(status)
    ),

    'channel_pty_request_function': (
        lambda sessionp, channelp, term, width, height, pxwidth, pxheight, userargp:
            userargp.cb_requestPty(term, width, height, pxwidth, pxheight)
    ),

    'channel_shell_request_function': (
        lambda sessionp, channelp, userargp:
            userargp.cb_requestShell()
    ),

    'channel_auth_agent_req_function': (
        lambda sessionp, channelp, userargp:
            userargp.cb_requestAuthAgent()
    ),

    'channel_x11_req_function': (
        lambda sessionp, channelp, single_connection, auth_protocol, auth_cookie, screen_number, userargp:
            userargp.cb_requestX11(single_connection, auth_protocol, auth_cookie, screen_number)
    ),

    'channel_pty_window_change_function': (
        lambda sessionp, channelp, width, height, pxwidth, pxheight, userargp:
            userargp.cb_ptyWindowChange(width, height, pxwidth, pxheight)
    ),

    'channel_exec_request_function': (
        lambda sessionp, channelp, command, userargp:
            userargp.cb_requestExec(command)
    ),

    'channel_env_request_function': (
        lambda sessionp, channelp, env_name, env_value, userargp:
            userargp.cb_requestEnv(env_name, env_value)
    ),

    'channel_subsystem_request_function': (
        lambda sessionp, channelp, subsystem, userargp:
            userargp.cb_requestSubsystem(subsystem)
    ),
    'channel_open_x11_server_status_function': (
        lambda sessionp, channelp, status, userargp:
            userargp.cb_open_x11_server_status(status)
    )
}


try:
    libpath = ctypes.util.find_library('sashimi')
    lib = ctypes.CDLL(libpath)

    # Methods called by SSH clients
    # =============================
    lib.ssh_event_x11_requested_channel_client.argtypes = [c_void_p, c_void_p, c_uint, c_uint, c_uint]
    lib.ssh_event_x11_requested_channel_client.restype = None

    lib.ssh_event_x11_requested_channel_client.restype = None
    lib.ssh_event_x11_requested_channel_failure_client.argtypes = [c_void_p, c_uint]

    lib.ssh_channel_change_pty_size_client.argtypes = [c_void_p, c_void_p, c_int, c_int]
    lib.ssh_channel_change_pty_size_client.restype = c_int

    lib.ssh_channel_close_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_close_client.restype = c_int

    lib.ssh_channel_free_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_free_client.restype = None

    lib.ssh_channel_is_closed_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_is_closed_client.restype = c_int

    lib.ssh_channel_is_eof_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_is_eof_client.restype = c_int

    lib.ssh_channel_is_open_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_is_open_client.restype = c_int

    lib.ssh_channel_open_forward_client.argtypes = [c_void_p, c_void_p, c_char_p, c_int, c_char_p, c_int]
    lib.ssh_channel_open_forward_client.restype = c_int

    lib.ssh_channel_request_auth_agent_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_request_auth_agent_client.restype = c_int

    lib.ssh_channel_request_env_client.argtypes = [c_void_p, c_void_p, c_char_p, c_char_p]
    lib.ssh_channel_request_env_client.restype = c_int

    lib.ssh_channel_request_exec_client.argtypes = [c_void_p, c_void_p, c_char_p]
    lib.ssh_channel_request_exec_client.restype = c_int

    lib.ssh_channel_request_pty_size_client.argtypes = [c_void_p, c_void_p, c_char_p, c_int, c_int]
    lib.ssh_channel_request_pty_size_client.restype = c_int

    lib.ssh_channel_request_send_signal_client.argtypes = [c_void_p, c_void_p, c_char_p]
    lib.ssh_channel_request_send_signal_client.restype = c_int

    lib.ssh_channel_request_shell_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_request_shell_client.restype = c_int

    lib.ssh_channel_request_subsystem_client.argtypes = [c_void_p, c_void_p, c_char_p]
    lib.ssh_channel_request_subsystem_client.restype = c_int

    lib.ssh_channel_request_x11_client.argtypes = [c_void_p, c_void_p, c_int, c_char_p, c_char_p, c_int]
    lib.ssh_channel_request_x11_client.restype = c_int

    lib.ssh_channel_send_eof_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_send_eof_client.restype = c_int

    lib.ssh_channel_write_client.argtypes = [c_void_p, c_void_p, c_char_p, c_uint]
    lib.ssh_channel_write_client.restype = c_int

    lib.ssh_disconnect_client.argtypes = [c_void_p]
    lib.ssh_disconnect_client.restype = None

    lib.ssh_event_set_fd_client.argtypes = [c_void_p, c_int, event_cb, py_object]
    lib.ssh_event_set_fd_client.restype = c_int

    lib.ssh_event_set_session_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_event_set_session_client.restype = None

    lib.ssh_get_server_publickey_hash_value_client.argtypes = [c_void_p, c_int, c_void_p, c_ulong, c_void_p]
    lib.ssh_get_server_publickey_hash_value_client.restype = c_int

    lib.ssh_gssapi_set_creds_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_gssapi_set_creds_client.restype = None

    lib.ssh_new_client_session.argtypes = [POINTER(ssh_client_callbacks_struct), c_void_p, c_char_p, c_char_p, c_char_p, c_char_p, c_char_p, c_void_p]
    lib.ssh_new_client_session.restypes = c_void_p

    lib.ssh_sessionchannel_open_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_sessionchannel_open_client.restype = c_int

    lib.ssh_set_agent_channel_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_set_agent_channel_client.restype = c_int

    lib.ssh_userauth_agent_client.argtypes = [c_void_p, c_void_p, c_char_p, c_void_p]
    lib.ssh_userauth_agent_client.restype = c_int

    lib.ssh_userauth_gssapi_client.argtypes = [c_void_p, c_void_p]
    lib.ssh_userauth_gssapi_client.restypes = c_int

    lib.ssh_userauth_kbdint_client.argtypes = [c_void_p, c_char_p, c_char_p, c_void_p]
    lib.ssh_userauth_kbdint_client.restype = c_int

    lib.ssh_userauth_kbdint_getnprompts_client.argtypes = [c_void_p]
    lib.ssh_userauth_kbdint_getnprompts_client.restype = c_int

    lib.ssh_userauth_kbdint_getprompt_client.argtypes = [c_void_p, c_uint, c_char_p, c_void_p]
    lib.ssh_userauth_kbdint_getprompt_client.restype = c_char_p

    lib.ssh_userauth_kbdint_setanswer_client.argtypes = [c_void_p, c_uint, c_char_p]
    lib.ssh_userauth_kbdint_setanswer_client.restype = c_int

    lib.ssh_userauth_list_client.argtypes = [c_void_p, c_char_p, c_void_p]
    lib.ssh_userauth_list_client.restype = c_int

    lib.ssh_userauth_none_client.argtypes = [c_void_p, c_char_p, c_void_p]
    lib.ssh_userauth_none_client.restype = c_int

    lib.ssh_userauth_password_client.argtypes = [c_void_p, c_char_p, c_char_p, c_void_p]
    lib.ssh_userauth_password_client.restype = c_int

    # Methods called by SSH servers
    # =============================
   
    lib.ssh_channel_close_server.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_close_server.restype = c_int

    lib.ssh_channel_free_server.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_free_server.restype = None

    lib.ssh_channel_is_closed_server.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_is_closed_server.restype = c_int

    lib.ssh_channel_is_eof_server.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_is_eof_server.restype = c_int

    lib.ssh_channel_is_open_server.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_is_open_server.restype = c_int

    lib.ssh_channel_open_auth_agent_server.argtypes = [
        c_void_p, c_void_p]
    lib.ssh_channel_open_auth_agent_server.restype = c_int

    lib.ssh_channel_open_reverse_forward_server.argtypes = [
        c_void_p, c_char_p, c_int, c_char_p, c_int]
    lib.ssh_channel_open_reverse_forward_server.restype = c_int

    lib.ssh_channel_open_x11_server.argtypes = [
        c_void_p, c_void_p, c_char_p, c_int]
    lib.ssh_channel_open_x11_server.restype = c_int

    lib.ssh_channel_request_send_exit_signal_server.argtypes = [
        c_void_p, c_void_p, c_char_p, c_int, c_char_p, c_char_p]
    lib.ssh_channel_request_send_exit_signal_server.restype = c_int

    lib.ssh_channel_request_send_exit_status_server.argtypes = [
        c_void_p, c_void_p, c_int]
    lib.ssh_channel_request_send_exit_status_server.restype = c_int

    lib.ssh_channel_send_eof_server.argtypes = [c_void_p, c_void_p]
    lib.ssh_channel_send_eof_server.restype = c_int

    lib.ssh_channel_write_server.argtypes = [
        c_void_p, c_void_p, c_char_p, c_uint]
    lib.ssh_channel_write_server.restype = c_int

    lib.ssh_channel_write_stderr_client.argtypes = [
        c_void_p, c_void_p, c_void_p, c_uint]
    lib.ssh_channel_write_stderr_client.restype = c_int

    lib.ssh_channel_write_stderr_server.argtypes = [
        c_void_p, c_void_p, c_void_p, c_uint]
    lib.ssh_channel_write_stderr_server.restype = c_int

    lib.ssh_disconnect_server.argtypes = [c_void_p]
    lib.ssh_disconnect_server.restype = None

    lib.ssh_event_set_session_server.argtypes = [c_void_p, c_void_p]
    lib.ssh_event_set_session_server.restype = None

    lib.ssh_forward_cancel_server.argtypes = [c_void_p, c_char_p, c_int]
    lib.ssh_forward_cancel_server.restype = c_int

    # => bound port should be there only after callback
    # int ssh_forward_listen_server(ssh_session_struct *, const char *, int port, int *bound_port);
    lib.ssh_forward_listen_server.argtypes = [
        c_void_p, c_char_p, c_int, c_void_p] 
    lib.ssh_forward_listen_server.restype = c_int
    
    lib.ssh_gssapi_get_creds_server.argtypes = [c_void_p]
    lib.ssh_gssapi_get_creds_server.restype = c_void_p
    
    
    lib.ssh_start_new_server_session.argtypes = [
        POINTER(ssh_server_callbacks_struct), 
        c_uint64, c_int, c_char_p, c_int]
    lib.ssh_start_new_server_session.restypes = c_void_p
    
    lib.ssh_userauth_kbdint_getanswer_server.argtypes = [
        c_void_p, c_uint, c_void_p]
    lib.ssh_userauth_kbdint_getanswer_server.restype = c_char_p
    
    lib.ssh_userauth_kbdint_getnanswers_server.argtypes = [
        c_void_p, c_void_p]
    lib.ssh_userauth_kbdint_getnanswers_server.restype = c_int
    
    lib.ssh_userauth_kbdint_settmpprompt_server.argtypes = [
        c_void_p, c_char_p, c_char_p, c_char_p, c_uint, c_void_p]
    lib.ssh_userauth_kbdint_settmpprompt_server.restype = c_int
    
    lib.ssh_userauth_kbdint_settmpprompts_server.argtypes = [
        c_void_p, c_char_p, c_char_p, c_uint, c_void_p, c_uint]
    lib.ssh_userauth_kbdint_settmpprompts_server.restypes = c_int

    # Methods not classified
    # ============================
    lib.ssh_new_channel.argtypes = [c_void_p, POINTER(ssh_channel_callbacks_struct)]
    lib.ssh_new_channel.restypes = c_void_p

    lib.ssh_set_blocking.argtypes = [c_void_p, c_int]
    lib.ssh_set_blocking.restype = None

    lib.ssh_event_dopoll.argtypes = [c_void_p, c_int]
    lib.ssh_event_dopoll.restype = c_int

    lib.ssh_new_poll_ctx.argtypes = []
#    lib.ssh_new_poll_ctx.restypes = c_void_p
    lib.ssh_new_poll_ctx.restypes = c_uint64

    lib.ssh_free.argtypes = [c_void_p]
    lib.ssh_free.restype = None

    lib.ssh_new_error.argtypes = []
    lib.ssh_new_error.restype = c_void_p

    lib.ssh_free_error.argtypes = [c_void_p]
    lib.ssh_free_error.restype = None

    lib.ssh_get_error.argtypes = [c_void_p]
    lib.ssh_get_error.restype = c_char_p

    lib.ssh_get_error_code.argtypes = [c_void_p]
    lib.ssh_get_error_code.restype = c_int

    lib.ssh_set_auth_methods_server.argtypes = [c_void_p, c_int]
    lib.ssh_set_auth_methods_server.restype = c_int

# 0.6.0 Deprecated
    lib.ssh_key_free.argtypes = [c_void_p]
    lib.ssh_key_free.restype = None

except AttributeError as e:
    lib = None
    raise ImportError('ssh shared library error (%s)' % e)
except Exception as e:
    lib = None
    import traceback
    raise ImportError('ssh shared library not found.\n'
                      'may be sashimi library is not installed .\n %s\n'
                      % traceback.format_exc(e))

# **** KEY TYPES for start server session
RSAKEY = 0
DSAKEY = 1
ECDSAKEY = 2
#****/ERROR RETURN CODES/*****
SSH_OK = 0
SSH_ERROR = -1
SSH_AGAIN = -2
SSH_EOF = -127
#****/SSH_LOG/*****
SSH_LOG_NOLOG = 0
SSH_LOG_WARNING = 1
SSH_LOG_PROTOCOL = 2
SSH_LOG_PACKET = 3
SSH_LOG_FUNCTIONS = 4
SSH_LOG_NONE = 0
SSH_LOG_WARN = 1
SSH_LOG_INFO = 2
SSH_LOG_DEBUG = 3
SSH_LOG_TRACE = 4
#****/SOCKET TYPE/*****
SSH_INVALID_SOCKET = -1
#*****/offsets of methods/*****
SSH_KEX = 0
SSH_HOSTKEYS = 1
SSH_CRYPT_C_S = 2
SSH_CRYPT_S_C = 3
SSH_MAC_C_S = 4
SSH_MAC_S_C = 5
SSH_COMP_C_S = 6
SSH_COMP_S_C = 7
SSH_LANG_C_S = 8
SSH_LANG_S_C = 9
SSH_CRYPT = 2
SSH_MAC = 3
SSH_COMP = 4
SSH_LANG = 5
#*****/SSH_AUTH/******
SSH_AUTH_SUCCESS = 0
SSH_AUTH_DENIED = 1
SSH_AUTH_PARTIAL = 2
SSH_AUTH_INFO = 3
SSH_AUTH_AGAIN = -2
SSH_AUTH_ERROR = -1
#*****/SSH_CHANNEL/******
SSH_CHANNEL_UNKNOWN = 0
SSH_CHANNEL_SESSION = 1
SSH_CHANNEL_DIRECT_TCPIP = 2
SSH_CHANNEL_FORWARDED_TCPIP = 3
SSH_CHANNEL_X11 = 4
SSH_CHANNEL_AUTH_AGENT = 5
#*****/SSH_REQUEST******
SSH_REQUEST_AUTH = 1
SSH_REQUEST_CHANNEL_OPEN = 2
SSH_REQUEST_CHANNEL = 3
SSH_REQUEST_SERVICE = 4
SSH_REQUEST_GLOBAL = 5
#****/AUTH FLAGS/*****
SSH_AUTH_METHOD_UNKNOWN = 0
SSH_AUTH_METHOD_NONE = 0x0001
SSH_AUTH_METHOD_PASSWORD = 0x0002
SSH_AUTH_METHOD_PUBLICKEY = 0x0004
SSH_AUTH_METHOD_HOSTBASED = 0x0008
SSH_AUTH_METHOD_INTERACTIVE = 0x0010
SSH_AUTH_METHOD_GSSAPI_MIC = 0x0020
SSH_CHANNEL_REQUEST_UNKNOWN = 0
SSH_CHANNEL_REQUEST_PTY = 1
SSH_CHANNEL_REQUEST_EXEC = 2
SSH_CHANNEL_REQUEST_SHELL = 3
SSH_CHANNEL_REQUEST_ENV = 4
SSH_CHANNEL_REQUEST_SUBSYSTEM = 5
SSH_CHANNEL_REQUEST_WINDOW_CHANGE = 6
SSH_CHANNEL_REQUEST_X11 = 7
SSH_CHANNEL_REQUEST_AUTH_AGENT = 8
SSH_GLOBAL_REQUEST_UNKNOWN = 0
SSH_GLOBAL_REQUEST_TCPIP_FORWARD = 1
SSH_GLOBAL_REQUEST_CANCEL_TCPIP_FORWARD = 2
SSH_PUBLICKEY_STATE_ERROR = -1
SSH_PUBLICKEY_STATE_NONE = 0
SSH_PUBLICKEY_STATE_VALID = 1
SSH_PUBLICKEY_STATE_WRONG = 2
SSH_SERVER_ERROR = -1,
SSH_SERVER_NOT_KNOWN = 0
SSH_SERVER_KNOWN_OK = 1
SSH_SERVER_KNOWN_CHANGED = 2
SSH_SERVER_FOUND_OTHER = 3
SSH_SERVER_FILE_NOT_FOUND = 4
MD5_DIGEST_LEN = 16
#****/ERROR/*****
SSH_NO_ERROR = 0
SSH_REQUEST_DENIED = 1
SSH_FATAL = 2
SSH_EINTR = 3
#****/TYPES FOR KEYS/****
SSH_KEYTYPE_UNKNOWN = 0
SSH_KEYTYPE_DSS = 1
SSH_KEYTYPE_RSA = 2
SSH_KEYTYPE_RSA1 = 3
SSH_KEYTYPE_ECDSA = 4
SSH_KEY_CMP_PUBLIC = 0
SSH_KEY_CMP_PRIVATE = 1

SSH_BIND_OPTIONS_BINDADDR = 0
SSH_BIND_OPTIONS_BINDPORT = 1
SSH_BIND_OPTIONS_BINDPORT_STR = 2
SSH_BIND_OPTIONS_HOSTKEY = 3
SSH_BIND_OPTIONS_DSAKEY = 4
SSH_BIND_OPTIONS_RSAKEY = 5
SSH_BIND_OPTIONS_BANNER = 6
SSH_BIND_OPTIONS_LOG_VERBOSITY = 7
SSH_BIND_OPTIONS_LOG_VERBOSITY_STR = 8
#****/CALLBACKS/****
SSH_SOCKET_FLOW_WRITEWILLBLOCK = 1
SSH_SOCKET_FLOW_WRITEWONTBLOCK = 2
SSH_SOCKET_EXCEPTION_EOF = 1
SSH_SOCKET_EXCEPTION_ERROR = 2
SSH_SOCKET_CONNECTED_OK = 1
SSH_SOCKET_CONNECTED_ERROR = 2
SSH_SOCKET_CONNECTED_TIMEOUT = 3
#*****/PUBKEY HASH TYPES/*****
SSH_PUBLICKEY_HASH_SHA1 = 0
SSH_PUBLICKEY_HASH_MD5 = 1
#*****/SSH SESSION STATUS FLAGS/*****
SSH_CLOSED = 0x01
SSH_READ_PENDING = 0x02
SSH_CLOSED_ERROR = 0x04
SSH_WRITE_PENDING = 0x08
# Python 2.7 compatibility layer

def buildCallbacks(api_cb_struct, cb_methods, userdata):
    callbacks = api_cb_struct()
    callbacks.size = ctypes.sizeof(api_cb_struct)
    callbacks.userdata = userdata

    for field, ftype in api_cb_struct._fields_[2:]:
        print("buildcallbacks", field, ftype)
        setattr(callbacks, field,
            ftype(cb_methods[field])
            if cb_methods[field] is not None
            else ctypes.cast(None, ftype))
    print("building callbacks done")
    return callbacks


