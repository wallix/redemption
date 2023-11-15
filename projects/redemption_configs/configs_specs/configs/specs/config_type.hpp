/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "configs/enumeration.hpp"

namespace cfg_specs {

inline void config_type_definition(type_enumerations & e)
{
    using Opt = type_enumerations::DisplayNameOption;
    auto withNameWhenDdescription = Opt::WithNameWhenDdescription;
    auto withoutNameWhenDescription = Opt::WithoutNameWhenDescription;

    e.enumeration_list("ModuleName", withNameWhenDdescription)
      .value("UNKNOWN")
      .value("login")
      .value("selector")
      .value("confirm")
      .value("link_confirm")
      .value("challenge")
      .value("valid")
      .value("transitory")
      .value("close")
      .value("close_back")
      .value("interactive_target")
      .value("RDP")
      .value("VNC")
      .value("INTERNAL")
      .value("waitinfo")
      .value("bouncer2")
      .value("autotest")
      .value("widgettest")
      .value("card")
    ;

    e.enumeration_flags("CaptureFlags", withNameWhenDdescription, "Specifies the type of data to be captured:")
      .value("none")
      .value("png")
      .value("wrm", "Session recording file.")
      .value("video").exclude()
      .value("ocr")
    ;

    e.enumeration_list("RdpSecurityEncryptionLevel", withNameWhenDdescription)
      .value("none")
      .value("low")
      .value("medium")
      .value("high")
    ;

    e.enumeration_list("Language", withNameWhenDdescription)
      .value("en")
      .value("fr")
    ;

    e.enumeration_list("ClipboardEncodingType", withNameWhenDdescription)
      .value("utf8").alias("utf-8")
      .value("latin1")
    ;

    e.enumeration_flags("KeyboardLogFlags", withoutNameWhenDescription)
      .value("none")
      .value("session_log", "keyboard log in session log")
      .value("wrm", "keyboard log in recorded sessions")
    ;

    e.enumeration_flags("ClipboardLogFlags", withoutNameWhenDescription)
      .value("none")
      .value("wrm", "clipboard log in recorded sessions")
      .value("meta", "clipboard log in recorded meta")
    ;

    e.enumeration_flags("FileSystemLogFlags", withoutNameWhenDescription)
      .value("none")
      .value("wrm", "(redirected) file system log in recorded sessions")
      .value("meta", "(redirected) file system log in recorded meta")
    ;

    e.enumeration_set("ColorDepth", withoutNameWhenDescription, "Specifies the maximum color resolution (color depth) for client connection session:")
      .value("depth8", 8, "8-bit")
      .value("depth15", 15, "15-bit 555 RGB mask")
      .value("depth16", 16, "16-bit 565 RGB mask")
      .value("depth24", 24, "24-bit RGB mask")
      .value("depth32", 32, "32-bit RGB mask + alpha")
    ;

    e.enumeration_flags("ServerNotification", withNameWhenDdescription)
      .value("nobody")
      .value("syslog", "message sent to syslog")
      .value("user", "User notified (through proxy interface)")
      .value("admin", "admin notified (Bastion notification)")
    ;

    e.enumeration_list("ServerCertCheck", withoutNameWhenDescription, "Behavior of certificates check.", "System errors like FS access rights issues or certificate decode are always check errors leading to connection rejection.")
      .value("fails_if_no_match_or_missing", "fails if certificates doesn't match or miss.")
      .value("fails_if_no_match_and_succeed_if_no_know", "fails if certificate doesn't match, succeed if no known certificate.")
      .value("succeed_if_exists_and_fails_if_missing", "succeed if certificates exists (not checked), fails if missing.")
      .value("always_succeed", "always succeed.")
    ;

    e.enumeration_list("TraceType", withoutNameWhenDescription, "Session record options.", "When session records are encrypted, they can be read only by the WALLIX Bastion where they have been generated.")
      .value("localfile", "No encryption (faster).")
      .value("localfile_hashed", "No encryption, with checksum.")
      .value("cryptofile", "Encryption enabled.")
    ;

    e.enumeration_list("KeyboardInputMaskingLevel", withoutNameWhenDescription)
      .value("unmasked", "keyboard input are not masked")
      .value("password_only", "only passwords are masked")
      .value("password_and_unidentified", "passwords and unidentified texts are masked")
      .value("fully_masked", "keyboard inputs are not logged")
    ;

    e.enumeration_list("SessionProbeOnLaunchFailure", withNameWhenDdescription, "Behavior on failure to launch Session Probe.")
      .value("ignore_and_continue", "The metadata collected is not essential for us. Instead, we prefer to minimize the impact on the user experience. The Session Probe launch will be in best-effort mode. The prevailing duration is defined by the 'Launch fallback timeout' instead of the 'Launch timeout'.")
      .value("disconnect_user", "This is the recommended setting. If the target meets all the technical prerequisites, there is no reason for the Session Probe not to launch. All that remains is to adapt the value of 'Launch timeout' to the performance of the target.")
      .value("retry_without_session_probe", "We wish to be able to recover the behavior of Bastion 5 when the Session Probe does not launch. The prevailing duration is defined by the 'Launch fallback timeout' instead of the 'Launch timeout'.")
    ;

    e.enumeration_list("VncBogusClipboardInfiniteLoop", withNameWhenDdescription)
      .value("delayed", "Clipboard processing is deferred and, if necessary, the token is left with the client.")
      .value("duplicated", "When 2 identical requests are received, the second is ignored. This can block clipboard data reception until a clipboard event is triggered on the server when the client clipboard is blocked, and vice versa.")
      .value("continued", "No special processing is done, the proxy always responds immediately.")
    ;

    e.enumeration_list("ColorDepthSelectionStrategy", withoutNameWhenDescription, "The method by which the proxy RDP establishes criteria on which to chosse a color depth for Session recording file (wrm):")
      .value("depth24", "24-bit")
      .value("depth16", "16-bit")
    ;

    e.enumeration_list("WrmCompressionAlgorithm", withoutNameWhenDescription, "The compression method of Session recording file (wrm):")
      .value("no_compression")
      .value("gzip", "GZip: Files are better compressed, but this takes more time and CPU load")
      .value("snappy", "Snappy: Faster than GZip, but files are less compressed")
    ;

    e.enumeration_list("RdpCompression", withoutNameWhenDescription, "Specifies the highest RDP compression support available")
      .value("none", "The RDP bulk compression is disabled")
      .value("rdp4", "RDP 4.0 bulk compression")
      .value("rdp5", "RDP 5.0 bulk compression")
      .value("rdp6", "RDP 6.0 bulk compression")
      .value("rdp6_1", "RDP 6.1 bulk compression")
    ;

    e.enumeration_set("OcrVersion", withNameWhenDdescription)
      .value("v1", 1)
      .value("v2", 2)
    ;

    e.enumeration_list("OcrLocale", withoutNameWhenDescription)
      .value("latin", "Recognizes Latin characters")
      .value("cyrillic", "Recognizes Latin and Cyrillic characters")
    ;

    e.enumeration_list("SessionProbeOnKeepaliveTimeout", withNameWhenDdescription)
      .value("ignore_and_continue", "Designed to minimize the impact on the user experience if the Session Probe is unstable. It should not be used when Session Probe is working well. An attacker can take advantage of this setting by simulating a Session Probe crash in order to bypass the surveillance.")
      .value("disconnect_user", "Legacy behavior. It’s a choice that gives more security, but the impact on the user experience seems disproportionate. The RDP session can be closed (resulting in the permanent loss of all its unsaved elements) if the 'End disconnected session' parameter (or an equivalent setting at the RDS-level) is enabled.")
      .value("freeze_connection_and_wait", "This is the recommended setting. User actions will be blocked until contact with the Session Probe (reply to KeepAlive message or something else) is resumed.")
    ;

    e.enumeration_list("SmartVideoCropping", withNameWhenDdescription)
      .value("disable", "When replaying the session video, the content of the RDP viewer matches the size of the client's desktop")
      .value("v1", "When replaying the session video, the content of the RDP viewer is restricted to the greatest area covered by the application during session")
      .value("v2", "When replaying the session video, the content of the RDP viewer is fully covered by the size of the greatest application window during session")
    ;

    e.enumeration_list("RdpModeConsole", withoutNameWhenDescription)
      .value("allow", "Forward Console mode request from client to the target.")
      .value("force", "Force Console mode on target regardless of client request.")
      .value("forbid", "Block Console mode request from client.")
    ;

    e.enumeration_flags("SessionProbeDisabledFeature", withoutNameWhenDescription)
      .value("none")
      .value("jab", "Java Access Bridge. General user activity monitoring in the Java applications (including detection of password fields).")
      .value("msaa", "MS Active Accessbility. General user activity monitoring (including detection of password fields). (legacy API)")
      .value("msuia", "MS UI Automation. General user activity monitoring (including detection of password fields). (new API)")
      .value("r1", "Reserved (do not use)").exclude()
      .value("edge_inspection", "Inspect Edge location URL. Basic web navigation monitoring.")
      .value("chrome_inspection", "Inspect Chrome Address/Search bar. Basic web navigation monitoring.")
      .value("firefox_inspection", "Inspect Firefox Address/Search bar. Basic web navigation monitoring.")
      .value("ie_monitoring", "Monitor Internet Explorer event. Advanced web navigation monitoring.")
      .value("group_membership", "Inspect group membership of user. User identity monitoring.")
    ;

    e.enumeration_list("RdpStoreFile", withNameWhenDdescription)
      .value("never", "Never store transferred files.")
      .value("always", "Always store transferred files.")
      .value("on_invalid_verification", "Transferred files are stored only if file verification is invalid. File verification by ICAP service must be enabled (in section file_verification).")
    ;

    e.enumeration_list("SessionProbeOnAccountManipulation", withNameWhenDdescription, "For targets running WALLIX BestSafe only.")
      .value("allow",  "User action will be accepted")
      .value("notify", "(Same thing as 'allow') ")
      .value("deny",   "User action will be rejected")
    ;

    e.enumeration_list("ClientAddressSent", withoutNameWhenDescription, "Client Address to send to target(in InfoPacket)")
      .value("no_address", "Send 0.0.0.0")
      .value("proxy", "Send proxy client address or target connexion")
      .value("front", "Send user client address of front connexion")
    ;

    e.enumeration_list("SessionProbeLogLevel", withNameWhenDdescription)
      .value("Off").exclude()
      .value("Fatal", "Designates very severe error events that will presumably lead the application to abort.")
      .value("Error", "Designates error events that might still allow the application to continue running.")
      .value("Info", "Designates informational messages that highlight the progress of the application at coarse-grained level.")
      .value("Warning", "Designates potentially harmful situations.")
      .value("Debug", "Designates fine-grained informational events that are mostly useful to debug an application.")
      .value("Detail", "Designates finer-grained informational events than Debug.")
    ;

    e.enumeration_list("ModRdpUseFailureSimulationSocketTransport", withNameWhenDdescription)
      .value("Off")
      .value("SimulateErrorRead")
      .value("SimulateErrorWrite")
    ;

    e.enumeration_list("LoginLanguage", withNameWhenDdescription)
      .value("Auto", "The language will be deduced according to the keyboard layout announced by the client")
      .value("EN")
      .value("FR")
    ;

    e.enumeration_list("VncTunnelingType", withNameWhenDdescription)
        .value("pxssh")
        .value("pexpect")
        .value("popen")
    ;

    e.enumeration_list("VncTunnelingCredentialSource", withNameWhenDdescription)
        .value("static_login")
        .value("scenario_account")
    ;

    e.enumeration_list("BannerType", withNameWhenDdescription)
      .value("info")
      .value("warn")
      .value("alert")
    ;

    e.enumeration_list("SessionProbeCPUUsageAlarmAction", withoutNameWhenDescription)
      .value("Restart", "Restart the Session Probe. May result in session disconnection due to loss of KeepAlive messages! Please refer to 'On keepalive timeout' parameter of current section and 'Allow multiple handshakes' parameter of 'Configuration options'.")
      .value("Stop", "Stop the Session Probe. May result in session disconnection due to loss of KeepAlive messages! Please refer to 'On keepalive timeout' parameter of current section.")
    ;

    e.enumeration_list("SessionProbeProcessCommandLineRetrieveMethod", withNameWhenDdescription)
      .value("windows_management_instrumentation", "Get command-line of processes via Windows Management Instrumentation. (Legacy method)")
      .value("windows_internals", "Calling internal system APIs to get the process command line. (More efficient but less stable)")
      .value("both", "First use internal system APIs call, if that fails, use Windows Management Instrumentation method.")
    ;

    e.enumeration_list("RdpSaveSessionInfoPDU", withoutNameWhenDescription)
        .value("Supported", "Windows")
        .value("UnsupportedOrUnknown", "Bastion, xrdp or others")
    ;

    e.enumeration_flags("SessionLogFormat", withNameWhenDdescription)
        .value("disabled")
        .value("SIEM")
        .value("ArcSight")
    ;
}

}
