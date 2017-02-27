/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (c) 2014-2016 by Christophe Grosjean, Meng Tan
   Author(s): Christophe Grosjean

   Adapted from parts of the SSH Library
   Copyright (c) 2003-2009 by Aris Adamantiadis
*/

#include "system/ssl_md5.hpp"
#include "cxx/cxx.hpp"


// les cibles devraient toutes avoir une session cible, même celles gérées par FD => remplace fd_poll
void do_fd_target_event(ssh_poll_handle_fd_struct * fd_poll, int revents)
{
    syslog(LOG_INFO, "%s --- fd=%d revent=%u", __FUNCTION__, fd_poll->x_fd, revents);

    if (!fd_poll->lock){
        syslog(LOG_INFO, "%s polling CLIENT (FD) not locked", __FUNCTION__);
        /* avoid having any event caught during callback */
        fd_poll->lock = 1;
        int ret = fd_poll->pw_cb(fd_poll->x_fd, revents, fd_poll->pw_userdata);
        fd_poll->lock = (ret == SSH_ERROR)?1:0;
    }
}


REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")


// ==================================== SSH_MSG_DISCONNECT ================================

//    [RFC4253] 11.1.  Disconnection Message
//    ======================================

//          byte      SSH_MSG_DISCONNECT
//          uint32    reason code
//          string    description in ISO-10646 UTF-8 encoding [RFC3629]
//          string    language tag [RFC3066]

//       This message causes immediate termination of the connection.  All
//       implementations MUST be able to process this message; they SHOULD be
//       able to send this message.

//       The sender MUST NOT send or receive any data after this message, and
//       the recipient MUST NOT accept any data after receiving this message.
//       The Disconnection Message 'description' string gives a more specific
//       explanation in a human-readable form.  The Disconnection Message
//       'reason code' gives the reason in a more machine-readable format
//       (suitable for localization), and can have the values as displayed in
//       the table below.  Note that the decimal representation is displayed
//       in this table for readability, but the values are actually uint32
//       values.

//               Symbolic name                                reason code
//               -------------                                -----------
//          SSH_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT             1
//          SSH_DISCONNECT_PROTOCOL_ERROR                          2
//          SSH_DISCONNECT_KEY_EXCHANGE_FAILED                     3
//          SSH_DISCONNECT_RESERVED                                4
//          SSH_DISCONNECT_MAC_ERROR                               5
//          SSH_DISCONNECT_COMPRESSION_ERROR                       6
//          SSH_DISCONNECT_SERVICE_NOT_AVAILABLE                   7
//          SSH_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED          8
//          SSH_DISCONNECT_HOST_KEY_NOT_VERIFIABLE                 9
//          SSH_DISCONNECT_CONNECTION_LOST                        10
//          SSH_DISCONNECT_BY_APPLICATION                         11
//          SSH_DISCONNECT_TOO_MANY_CONNECTIONS                   12
//          SSH_DISCONNECT_AUTH_CANCELLED_BY_USER                 13
//          SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE         14
//          SSH_DISCONNECT_ILLEGAL_USER_NAME                      15

//       If the 'description' string is displayed, the control character
//       filtering discussed in [SSH-ARCH] should be used to avoid attacks by
//       sending terminal control characters.

//       Requests for assignments of new Disconnection Message 'reason code'
//       values (and associated 'description' text) in the range of 0x00000010
//       to 0xFDFFFFFF MUST be done through the IETF CONSENSUS method, as
//       described in [RFC2434].  The Disconnection Message 'reason code'
//       values in the range of 0xFE000000 through 0xFFFFFFFF are reserved for
//       PRIVATE USE.  As noted, the actual instructions to the IANA are in
//       [SSH-NUMBERS].

/**
 * @brief Disconnect from a session (client or server).
 * The session can then be reused to open a new session.
 *
 * @param[in]  client_session  The SSH session to use.
 */
void ssh_disconnect_client(SshClientSession * client_session) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    if (client_session->socket != NULL && client_session->socket->fd_in != INVALID_SOCKET) {
        client_session->out_buffer->out_uint8(SSH_MSG_DISCONNECT);
        client_session->out_buffer->out_uint32_be(SSH2_DISCONNECT_BY_APPLICATION);
        client_session->out_buffer->out_length_prefixed_cstr("Bye Bye");
        // TODO: we are not sending the packet... unlikely correct
        // we should send the disconnect message then wait for socket close
    }

    client_session->opts.fd = INVALID_SOCKET;
    client_session->session_state = SSH_SESSION_STATE_DISCONNECTED;

    if(client_session->current_crypto){
        delete client_session->current_crypto;
        client_session->current_crypto = nullptr;
    }
    if(client_session->in_buffer){
        client_session->in_buffer->buffer_reinit();
    }
    if(client_session->out_buffer){
        client_session->out_buffer->buffer_reinit();
    }
    client_session->auth_methods = 0;
}

// ==================================== SSH_MSG_UNIMPLEMENTED ================================

//  [RFC4253] 11.4.  Reserved Messages
//  ==================================

//   An implementation MUST respond to all unrecognized messages with an
//   SSH_MSG_UNIMPLEMENTED message in the order in which the messages were
//   received.  Such messages MUST be otherwise ignored.  Later protocol
//   versions may define other meanings for these message types.

//      byte      SSH_MSG_UNIMPLEMENTED
//      uint32    packet sequence number of rejected message

inline void ssh_send_unimplemented_client(SshClientSession * client_session)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_ERR,"Can't do anything with packet type %d",client_session->in_packet_type);
    client_session->out_buffer->out_uint8(SSH_MSG_UNIMPLEMENTED);
    client_session->out_buffer->out_uint32_be(client_session->recv_seq-1);
    client_session->packet_send();
}

static inline void handle_ssh_packet_unimplemented_client(SshClientSession * client_session, ssh_buffer_struct* packet, error_struct & error)
{
    (void)packet;
    (void)error;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t seq = client_session->in_buffer->in_uint32_be();
    syslog(LOG_ERR, "Received SSH_MSG_UNIMPLEMENTED (sequence number %d)", seq);
}

// [RFC4253] 11.3.  Debug Message
// ==============================

//      byte      SSH_MSG_DEBUG
//      boolean   always_display
//      string    message in ISO-10646 UTF-8 encoding [RFC3629]
//      string    language tag [RFC3066]

//   All implementations MUST understand this message, but they are
//   allowed to ignore it.  This message is used to transmit information
//   that may help debugging.  If 'always_display' is TRUE, the message
//   SHOULD be displayed.  Otherwise, it SHOULD NOT be displayed unless
//   debugging information has been explicitly requested by the user.

//   The 'message' doesn't need to contain a newline.  It is, however,
//   allowed to consist of multiple lines separated by CRLF (Carriage
//   Return - Line Feed) pairs.

//   If the 'message' string is displayed, the terminal control character
//   filtering discussed in [SSH-ARCH] should be used to avoid attacks by
//   sending terminal control characters.

/**
 * @brief Send a debug message
 *
 * @param[in] client_session          The SSH session
 * @param[in] message          Data to be sent
 * @param[in] always_display   Message SHOULD be displayed by the server. It
 *                             SHOULD NOT be displayed unless debugging
 *                             information has been explicitly requested.
 *
 * @return                     SSH_OK on success, SSH_ERROR otherwise.
 */

inline int ssh_send_debug_client(SshClientSession * client_session, const char *message, int always_display)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    client_session->out_buffer->out_uint8(SSH_MSG_DEBUG);
    client_session->out_buffer->out_uint8(always_display);
    client_session->out_buffer->out_length_prefixed_cstr(message);
    /* Empty language tag */
    client_session->out_buffer->out_uint32_be(0);
    client_session->packet_send();
    return SSH_OK;
}



static inline void handle_ssh_packet_disconnect_client(SshClientSession * client_session, ssh_buffer_struct* packet, error_struct & error)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t code = packet->in_uint32_be();
    // TODO: error should be read directly from stream
    // and should be filtered to avoid some attacks
    char * tmp_error = packet->in_strdup_cstr();
    syslog(LOG_INFO, "Received SSH_MSG_DISCONNECT %d", code);
    ssh_set_error(error,  SSH_FATAL, "Received SSH_MSG_DISCONNECT: %d (%s)", code, tmp_error);
    // TODO: this message must have matching callback in API
    delete [] tmp_error;
    client_session->socket->close();
    client_session->session_state = SSH_SESSION_STATE_ERROR;
    /* TODO: handle a graceful disconnect */
}

//SSH_PACKET_CALLBACK(ssh_packet_service_request)
// SSH_MSG_SERVICE_REQUEST
static inline void handle_ssh_packet_service_request_client(SshClientSession * client_session, ssh_buffer_struct* packet, error_struct & error)
{
    (void)client_session;
    (void)error;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    // SSH_REQUEST_SERVICE
    char * service = packet->in_strdup_cstr();
    syslog(LOG_INFO, "Received a SERVICE_REQUEST for service %s", service);
    delete service;
}

static inline int handle_ssh_packet_service_accept_client(SshClientSession * client_session, uint8_t type, ssh_buffer_struct* packet, void *user)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    (void)packet;
    (void)type;
    (void)user;

    client_session->auth_service_state = SSH_AUTH_SERVICE_ACCEPTED;
    syslog(LOG_INFO, "Received SSH_MSG_SERVICE_ACCEPT");

    return SSH_PACKET_USED;
}

// ================================= SSH_MSG_KEXINIT =================================
static int dh_handshake_client(SshClientSession * client_session, error_struct & error);

//    [RFC4253] 7.  Key Exchange
//    ==========================

//       Key exchange (kex) begins by each side sending name-lists of
//       supported algorithms.  Each side has a preferred algorithm in each
//       category, and it is assumed that most implementations, at any given
//       time, will use the same preferred algorithm.  Each side MAY guess
//       which algorithm the other side is using, and MAY send an initial key
//       exchange packet according to the algorithm, if appropriate for the
//       preferred method.

//       The guess is considered wrong if:

//       o  the kex algorithm and/or the host key algorithm is guessed wrong
//          (server and client have different preferred algorithm), or

//       o  if any of the other algorithms cannot be agreed upon (the
//          procedure is defined below in Section 7.1).

//       Otherwise, the guess is considered to be right, and the
//       optimistically sent packet MUST be handled as the first key exchange
//       packet.

//       However, if the guess was wrong, and a packet was optimistically sent
//       by one or both parties, such packets MUST be ignored (even if the
//       error in the guess would not affect the contents of the initial
//       packet(s)), and the appropriate side MUST send the correct initial
//       packet.

//       A key exchange method uses explicit server authentication if the key
//       exchange messages include a signature or other proof of the server's
//       authenticity.  A key exchange method uses implicit server
//       authentication if, in order to prove its authenticity, the server
//       also has to prove that it knows the shared secret, K, by sending a
//       message and a corresponding MAC that the client can verify.

//       The key exchange method defined by this document uses explicit server
//       authentication.  However, key exchange methods with implicit server
//       authentication MAY be used with this protocol.  After a key exchange
//       with implicit server authentication, the client MUST wait for a
//       response to its service request message before sending any further
//       data.

//    7.1.  Algorithm Negotiation

//       Key exchange begins by each side sending the following packet:

//          byte         SSH_MSG_KEXINIT
//          byte[16]     cookie (random bytes)
//          name-list    kex_algorithms
//          name-list    server_host_key_algorithms
//          name-list    encryption_algorithms_client_to_server
//          name-list    encryption_algorithms_server_to_client
//          name-list    mac_algorithms_client_to_server
//          name-list    mac_algorithms_server_to_client
//          name-list    compression_algorithms_client_to_server
//          name-list    compression_algorithms_server_to_client
//          name-list    languages_client_to_server
//          name-list    languages_server_to_client
//          boolean      first_kex_packet_follows
//          uint32       0 (reserved for future extension)

//       Each of the algorithm name-lists MUST be a comma-separated list of
//       algorithm names (see Algorithm Naming in [SSH-ARCH] and additional
//       information in [SSH-NUMBERS]).  Each supported (allowed) algorithm
//       MUST be listed in order of preference, from most to least.

//       The first algorithm in each name-list MUST be the preferred (guessed)
//       algorithm.  Each name-list MUST contain at least one algorithm name.

//          cookie
//             The 'cookie' MUST be a random value generated by the sender.
//             Its purpose is to make it impossible for either side to fully
//             determine the keys and the session identifier.

//          kex_algorithms
//             Key exchange algorithms were defined above.  The first
//             algorithm MUST be the preferred (and guessed) algorithm.  If
//             both sides make the same guess, that algorithm MUST be used.
//             Otherwise, the following algorithm MUST be used to choose a key
//             exchange method: Iterate over client's kex algorithms, one at a
//             time.  Choose the first algorithm that satisfies the following
//             conditions:

//             +  the server also supports the algorithm,

//             +  if the algorithm requires an encryption-capable host key,
//                there is an encryption-capable algorithm on the server's
//                server_host_key_algorithms that is also supported by the
//                client, and

//             +  if the algorithm requires a signature-capable host key,
//                there is a signature-capable algorithm on the server's
//                server_host_key_algorithms that is also supported by the
//                client.

//          If no algorithm satisfying all these conditions can be found, the
//          connection fails, and both sides MUST disconnect.

//          server_host_key_algorithms
//             A name-list of the algorithms supported for the server host
//             key.  The server lists the algorithms for which it has host
//             keys; the client lists the algorithms that it is willing to
//             accept.  There MAY be multiple host keys for a host, possibly
//             with different algorithms.

//             Some host keys may not support both signatures and encryption
//             (this can be determined from the algorithm), and thus not all
//             host keys are valid for all key exchange methods.

//             Algorithm selection depends on whether the chosen key exchange
//             algorithm requires a signature or an encryption-capable host
//             key.  It MUST be possible to determine this from the public key
//             algorithm name.  The first algorithm on the client's name-list
//             that satisfies the requirements and is also supported by the
//             server MUST be chosen.  If there is no such algorithm, both
//             sides MUST disconnect.

//          encryption_algorithms
//             A name-list of acceptable symmetric encryption algorithms (also
//             known as ciphers) in order of preference.  The chosen
//             encryption algorithm to each direction MUST be the first
//             algorithm on the client's name-list that is also on the
//             server's name-list.  If there is no such algorithm, both sides
//             MUST disconnect.

//             Note that "none" must be explicitly listed if it is to be
//             acceptable.  The defined algorithm names are listed in Section
//             6.3.

//          mac_algorithms
//             A name-list of acceptable MAC algorithms in order of
//             preference.  The chosen MAC algorithm MUST be the first
//             algorithm on the client's name-list that is also on the
//             server's name-list.  If there is no such algorithm, both sides
//             MUST disconnect.

//             Note that "none" must be explicitly listed if it is to be
//             acceptable.  The MAC algorithm names are listed in Section 6.4.

//          compression_algorithms
//             A name-list of acceptable compression algorithms in order of
//             preference.  The chosen compression algorithm MUST be the first
//             algorithm on the client's name-list that is also on the
//             server's name-list.  If there is no such algorithm, both sides
//             MUST disconnect.

//             Note that "none" must be explicitly listed if it is to be
//             acceptable.  The compression algorithm names are listed in
//             Section 6.2.

//          languages
//             This is a name-list of language tags in order of preference
//             [RFC3066].  Both parties MAY ignore this name-list.  If there
//             are no language preferences, this name-list SHOULD be empty as
//             defined in Section 5 of [SSH-ARCH].  Language tags SHOULD NOT
//             be present unless they are known to be needed by the sending
//             party.

//          first_kex_packet_follows
//             Indicates whether a guessed key exchange packet follows.  If a
//             guessed packet will be sent, this MUST be TRUE.  If no guessed
//             packet will be sent, this MUST be FALSE.

//             After receiving the SSH_MSG_KEXINIT packet from the other side,
//             each party will know whether their guess was right.  If the
//             other party's guess was wrong, and this field was TRUE, the
//             next packet MUST be silently ignored, and both sides MUST then
//             act as determined by the negotiated key exchange method.  If
//             the guess was right, key exchange MUST continue using the
//             guessed packet.

//       After the SSH_MSG_KEXINIT message exchange, the key exchange
//       algorithm is run.  It may involve several packet exchanges, as
//       specified by the key exchange method.

//       Once a party has sent a SSH_MSG_KEXINIT message for key exchange or
//       re-exchange, until it has sent a SSH_MSG_NEWKEYS message (Section
//       7.3), it MUST NOT send any messages other than:

//       o  Transport layer generic messages (1 to 19) (but
//          SSH_MSG_SERVICE_REQUEST and SSH_MSG_SERVICE_ACCEPT MUST NOT be
//          sent);

//       o  Algorithm negotiation messages (20 to 29) (but further
//          SSH_MSG_KEXINIT messages MUST NOT be sent);

//       o  Specific key exchange method messages (30 to 49).

//       The provisions of Section 11 apply to unrecognized messages.

//       Note, however, that during a key re-exchange, after sending a
//       SSH_MSG_KEXINIT message, each party MUST be prepared to process an
//       arbitrary number of messages that may be in-flight before receiving a
//       SSH_MSG_KEXINIT message from the other party.

//    7.2.  Output from Key Exchange

//       The key exchange produces two values: a shared secret K, and an
//       exchange hash H.  Encryption and authentication keys are derived from
//       these.  The exchange hash H from the first key exchange is
//       additionally used as the session identifier, which is a unique
//       identifier for this connection.  It is used by authentication methods
//       as a part of the data that is signed as a proof of possession of a
//       private key.  Once computed, the session identifier is not changed,
//       even if keys are later re-exchanged.

//       Each key exchange method specifies a hash function that is used in
//       the key exchange.  The same hash algorithm MUST be used in key
//       derivation.  Here, we'll call it HASH.

//       Encryption keys MUST be computed as HASH, of a known value and K, as
//       follows:

//       o  Initial IV client to server: HASH(K || H || "A" || session_id)
//          (Here K is encoded as mpint and "A" as byte and session_id as raw
//          data.  "A" means the single character A, ASCII 65).

//       o  Initial IV server to client: HASH(K || H || "B" || session_id)

//       o  Encryption key client to server: HASH(K || H || "C" || session_id)

//       o  Encryption key server to client: HASH(K || H || "D" || session_id)

//       o  Integrity key client to server: HASH(K || H || "E" || session_id)

//       o  Integrity key server to client: HASH(K || H || "F" || session_id)

//       Key data MUST be taken from the beginning of the hash output.  As
//       many bytes as needed are taken from the beginning of the hash value.
//       If the key length needed is longer than the output of the HASH, the
//       key is extended by computing HASH of the concatenation of K and H and
//       the entire key so far, and appending the resulting bytes (as many as
//       HASH generates) to the key.  This process is repeated until enough
//       key material is available; the key is taken from the beginning of
//       this value.  In other words:

//          K1 = HASH(K || H || X || session_id)   (X is e.g., "A")
//          K2 = HASH(K || H || K1)
//          K3 = HASH(K || H || K1 || K2)
//          ...
//          key = K1 || K2 || K3 || ...

//       This process will lose entropy if the amount of entropy in K is
//       larger than the internal state size of HASH.

//    7.3.  Taking Keys Into Use

//       Key exchange ends by each side sending an SSH_MSG_NEWKEYS message.
//       This message is sent with the old keys and algorithms.  All messages
//       sent after this message MUST use the new keys and algorithms.

//       When this message is received, the new keys and algorithms MUST be
//       used for receiving.

//       The purpose of this message is to ensure that a party is able to
//       respond with an SSH_MSG_DISCONNECT message that the other party can
//       understand if something goes wrong with the key exchange.

//          byte      SSH_MSG_NEWKEYS

//    8.  Diffie-Hellman Key Exchange

//       The Diffie-Hellman (DH) key exchange provides a shared secret that
//       cannot be determined by either party alone.  The key exchange is
//       combined with a signature with the host key to provide host
//       authentication.  This key exchange method provides explicit server
//       authentication as defined in Section 7.

//       The following steps are used to exchange a key.  In this, C is the
//       client; S is the server; p is a large safe prime; g is a generator
//       for a subgroup of GF(p); q is the order of the subgroup; V_S is S's
//       identification string; V_C is C's identification string; K_S is S's
//       public host key; I_C is C's SSH_MSG_KEXINIT message and I_S is S's
//       SSH_MSG_KEXINIT message that have been exchanged before this part
//       begins.

//       1. C generates a random number x (1 < x < q) and computes
//          e = g^x mod p.  C sends e to S.

//       2. S generates a random number y (0 < y < q) and computes
//          f = g^y mod p.  S receives e.  It computes K = e^y mod p,
//          H = hash(V_C || V_S || I_C || I_S || K_S || e || f || K)
//          (these elements are encoded according to their types; see below),
//          and signature s on H with its private host key.  S sends
//          (K_S || f || s) to C.  The signing operation may involve a
//          second hashing operation.

//       3. C verifies that K_S really is the host key for S (e.g., using
//          certificates or a local database).  C is also allowed to accept
//          the key without verification; however, doing so will render the
//          protocol insecure against active attacks (but may be desirable for
//          practical reasons in the short term in many environments).  C then
//          computes K = f^x mod p, H = hash(V_C || V_S || I_C || I_S || K_S
//          || e || f || K), and verifies the signature s on H.

//       Values of 'e' or 'f' that are not in the range [1, p-1] MUST NOT be
//       sent or accepted by either side.  If this condition is violated, the
//       key exchange fails.

//       This is implemented with the following messages.  The hash algorithm
//       for computing the exchange hash is defined by the method name, and is
//       called HASH.  The public key algorithm for signing is negotiated with
//       the SSH_MSG_KEXINIT messages.

//       First, the client sends the following:

//          byte      SSH_MSG_KEXDH_INIT
//          mpint     e

//       The server then responds with the following:

//          byte      SSH_MSG_KEXDH_REPLY
//          string    server public host key and certificates (K_S)
//          mpint     f
//          string    signature of H

//       The hash H is computed as the HASH hash of the concatenation of the
//       following:

//          string    V_C, the client's identification string (CR and LF
//                    excluded)
//          string    V_S, the server's identification string (CR and LF
//                    excluded)
//          string    I_C, the payload of the client's SSH_MSG_KEXINIT
//          string    I_S, the payload of the server's SSH_MSG_KEXINIT
//          string    K_S, the host key
//          mpint     e, exchange value sent by the client
//          mpint     f, exchange value sent by the server
//          mpint     K, the shared secret

//       This value is called the exchange hash, and it is used to
//       authenticate the key exchange.  The exchange hash SHOULD be kept
//       secret.

//       The signature algorithm MUST be applied over H, not the original
//       data.  Most signature algorithms include hashing and additional
//       padding (e.g., "ssh-dss" specifies SHA-1 hashing).  In that case, the
//       data is first hashed with HASH to compute H, and H is then hashed
//       with SHA-1 as part of the signing operation.

//    8.1.  diffie-hellman-group1-sha1

//       The "diffie-hellman-group1-sha1" method specifies the Diffie-Hellman
//       key exchange with SHA-1 as HASH, and Oakley Group 2 [RFC2409] (1024-
//       bit MODP Group).  This method MUST be supported for interoperability
//       as all of the known implementations currently support it.  Note that
//       this method is named using the phrase "group1", even though it
//       specifies the use of Oakley Group 2.

//    8.2.  diffie-hellman-group14-sha1

//       The "diffie-hellman-group14-sha1" method specifies a Diffie-Hellman
//       key exchange with SHA-1 as HASH and Oakley Group 14 [RFC3526] (2048-
//       bit MODP Group), and it MUST also be supported.

//    9.  Key Re-Exchange

//       Key re-exchange is started by sending an SSH_MSG_KEXINIT packet when
//       not already doing a key exchange (as described in Section 7.1).  When
//       this message is received, a party MUST respond with its own
//       SSH_MSG_KEXINIT message, except when the received SSH_MSG_KEXINIT
//       already was a reply.  Either party MAY initiate the re-exchange, but
//       roles MUST NOT be changed (i.e., the server remains the server, and
//       the client remains the client).

//       Key re-exchange is performed using whatever encryption was in effect
//       when the exchange was started.  Encryption, compression, and MAC
//       methods are not changed before a new SSH_MSG_NEWKEYS is sent after
//       the key exchange (as in the initial key exchange).  Re-exchange is
//       processed identically to the initial key exchange, except for the
//       session identifier that will remain unchanged.  It is permissible to
//       change some or all of the algorithms during the re-exchange.  Host
//       keys can also change.  All keys and initialization vectors are
//       recomputed after the exchange.  Compression and encryption contexts
//       are reset.

//       It is RECOMMENDED that the keys be changed after each gigabyte of
//       transmitted data or after each hour of connection time, whichever
//       comes sooner.  However, since the re-exchange is a public key
//       operation, it requires a fair amount of processing power and should
//       not be performed too often.

//       More application data may be sent after the SSH_MSG_NEWKEYS packet
//       has been sent; key exchange does not affect the protocols that lie
//       above the SSH transport layer.

// -------------------------------------------------------------

//   After the SSH_MSG_KEXINIT message exchange, the key exchange
//   algorithm is run.  It may involve several packet exchanges, as
//   specified by the key exchange method.

//   Once a party has sent a SSH_MSG_KEXINIT message for key exchange or
//   re-exchange, until it has sent a SSH_MSG_NEWKEYS message (Section
//   7.3), it MUST NOT send any messages other than:

//   o  Transport layer generic messages (1 to 19) (but
//      SSH_MSG_SERVICE_REQUEST and SSH_MSG_SERVICE_ACCEPT MUST NOT be
//      sent);

//   o  Algorithm negotiation messages (20 to 29) (but further
//      SSH_MSG_KEXINIT messages MUST NOT be sent);

//   o  Specific key exchange method messages (30 to 49).

//   The provisions of Section 11 apply to unrecognized messages.

//   Note, however, that during a key re-exchange, after sending a
//   SSH_MSG_KEXINIT message, each party MUST be prepared to process an
//   arbitrary number of messages that may be in-flight before receiving a
//   SSH_MSG_KEXINIT message from the other party.


// SSH_PACKET_CALLBACK(ssh_packet_kexinit)
// TODO: do one function for server the other for client
static inline int ssh_packet_kexinit_client(SshClientSession * client_session, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    if (client_session->session_state == SSH_SESSION_STATE_AUTHENTICATED){
        syslog(LOG_WARNING, "Other side initiating key re-exchange");
    }
    else if(client_session->session_state != SSH_SESSION_STATE_INITIAL_KEX){
        ssh_set_error(client_session->error,  SSH_FATAL,"SSH_KEXINIT received in wrong state");
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    if (packet->buffer_get_data(client_session->next_crypto->server_kex.cookie,16) != 16) {
        ssh_set_error(client_session->error,  SSH_FATAL, "ssh_packet_kexinit_client: no cookie in packet");
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    syslog(LOG_INFO,"Initialising server methods");

    client_session->next_crypto->server_kex.methods[SSH_KEX] = packet->in_strdup_cstr();
    client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS] = packet->in_strdup_cstr();
    client_session->next_crypto->server_kex.methods[SSH_CRYPT_C_S] = packet->in_strdup_cstr();
    client_session->next_crypto->server_kex.methods[SSH_CRYPT_S_C] = packet->in_strdup_cstr();
    client_session->next_crypto->server_kex.methods[SSH_MAC_C_S] = packet->in_strdup_cstr();
    client_session->next_crypto->server_kex.methods[SSH_MAC_S_C] = packet->in_strdup_cstr();
    client_session->next_crypto->server_kex.methods[SSH_COMP_C_S] = packet->in_strdup_cstr();
    client_session->next_crypto->server_kex.methods[SSH_COMP_S_C] = packet->in_strdup_cstr();
    client_session->next_crypto->server_kex.methods[SSH_LANG_S_C] = packet->in_strdup_cstr();
    client_session->next_crypto->server_kex.methods[SSH_LANG_C_S] = packet->in_strdup_cstr();


    client_session->session_state = SSH_SESSION_STATE_KEXINIT_RECEIVED;
    client_session->dh_handshake_state = DH_STATE_INIT;
    client_session->client_callbacks->connect_status_function(client_session->client_callbacks->userdata, 0.6f);

    syslog(LOG_INFO, "kex algos: %s",
        client_session->next_crypto->server_kex.methods[SSH_KEX].cstr());
    syslog(LOG_INFO, "server host key algo: %s",
        client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS].cstr());
    syslog(LOG_INFO, "encryption client->server: %s",
        client_session->next_crypto->server_kex.methods[SSH_CRYPT_C_S].cstr());
    syslog(LOG_INFO, "encryption server->client: %s",
        client_session->next_crypto->server_kex.methods[SSH_CRYPT_S_C].cstr());
    syslog(LOG_INFO, "mac algo client->server: %s",
        client_session->next_crypto->server_kex.methods[SSH_MAC_C_S].cstr());
    syslog(LOG_INFO, "mac algo server->client: %s",
        client_session->next_crypto->server_kex.methods[SSH_MAC_S_C].cstr());
    syslog(LOG_INFO, "compression algo client->server: %s",
        client_session->next_crypto->server_kex.methods[SSH_COMP_C_S].cstr());
    syslog(LOG_INFO, "compression algo server->client: %s",
        client_session->next_crypto->server_kex.methods[SSH_COMP_S_C].cstr());
    syslog(LOG_INFO, "languages client->server: %s",
        client_session->next_crypto->server_kex.methods[SSH_LANG_C_S].cstr());
    syslog(LOG_INFO, "languages server->client: %s",
        client_session->next_crypto->server_kex.methods[SSH_LANG_S_C].cstr());

    RAND_pseudo_bytes(client_session->next_crypto->client_kex.cookie, 16);

    memset(client_session->next_crypto->client_kex.methods, 0, KEX_METHODS_SIZE * sizeof(char **));

    /* NOTE: This is a fixed API and the index is defined by ssh_kex_types_e */
    const char *default_methods[] = {
      "curve25519-sha256@libssh.org,ecdh-sha2-nistp256,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1",
      "ecdsa-sha2-nistp256,ssh-rsa,ssh-dss",
      "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1",
      "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1",
      "hmac-sha1",
      "hmac-sha1",
      "none",
      "none",
      "",
      "",
      nullptr
    };

    syslog(LOG_INFO,"Initialising client methods");

    client_session->next_crypto->client_kex.methods[SSH_KEX] =
        (client_session->opts.wanted_methods[SSH_KEX] != nullptr)
            ?client_session->opts.wanted_methods[SSH_KEX]
            :default_methods[SSH_KEX];
    client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS] =
        (client_session->opts.wanted_methods[SSH_HOSTKEYS] != nullptr)
            ?client_session->opts.wanted_methods[SSH_HOSTKEYS]
            :default_methods[SSH_HOSTKEYS];
    client_session->next_crypto->client_kex.methods[SSH_CRYPT_C_S] =
        (client_session->opts.wanted_methods[SSH_CRYPT_C_S] != nullptr)
            ?client_session->opts.wanted_methods[SSH_CRYPT_C_S]
            :default_methods[SSH_CRYPT_C_S];
    client_session->next_crypto->client_kex.methods[SSH_CRYPT_S_C] =
        (client_session->opts.wanted_methods[SSH_CRYPT_S_C] != nullptr)
            ?client_session->opts.wanted_methods[SSH_CRYPT_S_C]
            :default_methods[SSH_CRYPT_S_C];
    client_session->next_crypto->client_kex.methods[SSH_MAC_C_S] =
        (client_session->opts.wanted_methods[SSH_MAC_C_S] != nullptr)
            ?client_session->opts.wanted_methods[SSH_MAC_C_S]
            :default_methods[SSH_MAC_C_S];
    client_session->next_crypto->client_kex.methods[SSH_MAC_S_C] =
        (client_session->opts.wanted_methods[SSH_MAC_S_C] != nullptr)
            ?client_session->opts.wanted_methods[SSH_MAC_S_C]
            :default_methods[SSH_MAC_S_C];
    client_session->next_crypto->client_kex.methods[SSH_COMP_C_S] =
        (client_session->opts.wanted_methods[SSH_COMP_C_S] != nullptr)
            ?client_session->opts.wanted_methods[SSH_COMP_C_S]
            :default_methods[SSH_COMP_C_S];
    client_session->next_crypto->client_kex.methods[SSH_COMP_S_C] =
        (client_session->opts.wanted_methods[SSH_COMP_S_C] != nullptr)
            ?client_session->opts.wanted_methods[SSH_COMP_S_C]
            :default_methods[SSH_COMP_S_C];
    client_session->next_crypto->client_kex.methods[SSH_LANG_S_C] =
        (client_session->opts.wanted_methods[SSH_LANG_S_C] != nullptr)
            ?client_session->opts.wanted_methods[SSH_LANG_S_C]
            :default_methods[SSH_LANG_S_C];
    client_session->next_crypto->client_kex.methods[SSH_LANG_C_S] =
        (client_session->opts.wanted_methods[SSH_LANG_C_S] != nullptr)
            ?client_session->opts.wanted_methods[SSH_LANG_C_S]
            :default_methods[SSH_LANG_C_S];

    client_session->next_crypto->kex_methods[SSH_KEX] = find_matching(
        client_session->next_crypto->server_kex.methods[SSH_KEX].cstr(),
        client_session->next_crypto->client_kex.methods[SSH_KEX].cstr(),
        ',');

    if (client_session->next_crypto->kex_methods[SSH_KEX].size == 0){
        ssh_set_error(client_session->error, SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
            "kex algos",
            client_session->next_crypto->server_kex.methods[SSH_KEX].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_KEX].cstr());
        client_session->socket->close();
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    // TODO: create a general purpose method to find a value in a string keyed dictionnary
    // and use it there and whenever necessary
    if(strcmp(client_session->next_crypto->kex_methods[SSH_KEX].cstr(), "diffie-hellman-group1-sha1") == 0){
      client_session->next_crypto->kex_type = SSH_KEX_DH_GROUP1_SHA1;
    }
    else if(strcmp(client_session->next_crypto->kex_methods[SSH_KEX].cstr(), "diffie-hellman-group14-sha1") == 0){
      client_session->next_crypto->kex_type = SSH_KEX_DH_GROUP14_SHA1;
    }
    else if(strcmp(client_session->next_crypto->kex_methods[SSH_KEX].cstr(), "ecdh-sha2-nistp256") == 0){
      client_session->next_crypto->kex_type = SSH_KEX_ECDH_SHA2_NISTP256;
    }
    else if(strcmp(client_session->next_crypto->kex_methods[SSH_KEX].cstr(), "curve25519-sha256@libssh.org") == 0){
      client_session->next_crypto->kex_type = SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG;
    }

    client_session->next_crypto->kex_methods[SSH_HOSTKEYS] = find_matching(
        client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS].cstr(),
        client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS].cstr(),
        ',');

    if (client_session->next_crypto->kex_methods[SSH_HOSTKEYS].size == 0){
        ssh_set_error(client_session->error, SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
            "server host key algo",
            client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS].cstr());
        client_session->socket->close();
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    client_session->next_crypto->kex_methods[SSH_CRYPT_C_S] = find_matching(
            client_session->next_crypto->server_kex.methods[SSH_CRYPT_C_S].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_CRYPT_C_S].cstr(),
            ',');

    if (client_session->next_crypto->kex_methods[SSH_CRYPT_C_S].size == 0){
        ssh_set_error(client_session->error, SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
            "encryption client->server",
            client_session->next_crypto->server_kex.methods[SSH_CRYPT_C_S].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_CRYPT_C_S].cstr());
        client_session->socket->close();
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }
    client_session->next_crypto->kex_methods[SSH_CRYPT_S_C] = find_matching(
        client_session->next_crypto->server_kex.methods[SSH_CRYPT_S_C].cstr(),
        client_session->next_crypto->client_kex.methods[SSH_CRYPT_S_C].cstr(),
        ',');

    if (client_session->next_crypto->kex_methods[SSH_CRYPT_S_C].size == 0){
        ssh_set_error(client_session->error, SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
            "encryption server->client",
            client_session->next_crypto->server_kex.methods[SSH_CRYPT_S_C].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_CRYPT_S_C].cstr());
        client_session->socket->close();
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    client_session->next_crypto->kex_methods[SSH_MAC_C_S] = find_matching(
        client_session->next_crypto->server_kex.methods[SSH_MAC_C_S].cstr(),
        client_session->next_crypto->client_kex.methods[SSH_MAC_C_S].cstr(),
        ',');

    if (client_session->next_crypto->kex_methods[SSH_MAC_C_S].size == 0){
        ssh_set_error(client_session->error, SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
            "mac algo client->server",
            client_session->next_crypto->server_kex.methods[SSH_MAC_C_S].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_MAC_C_S].cstr());
        client_session->socket->close();
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    client_session->next_crypto->kex_methods[SSH_MAC_S_C] = find_matching(
        client_session->next_crypto->server_kex.methods[SSH_MAC_S_C].cstr(),
        client_session->next_crypto->client_kex.methods[SSH_MAC_S_C].cstr(),
        ',');

    if (client_session->next_crypto->kex_methods[SSH_MAC_S_C].size == 0){
        ssh_set_error(client_session->error, SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
            "mac algo server->client",
            client_session->next_crypto->server_kex.methods[SSH_MAC_S_C].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_MAC_S_C].cstr());
        client_session->socket->close();
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }
    client_session->next_crypto->kex_methods[SSH_COMP_C_S] = find_matching(
        client_session->next_crypto->server_kex.methods[SSH_COMP_C_S].cstr(),
        client_session->next_crypto->client_kex.methods[SSH_COMP_C_S].cstr(),
        ',');

    if (client_session->next_crypto->kex_methods[SSH_COMP_C_S].size == 0){
        ssh_set_error(client_session->error,SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
            "compression algo client->server",
            client_session->next_crypto->server_kex.methods[SSH_COMP_C_S].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_COMP_C_S].cstr());
        client_session->socket->close();
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }
    client_session->next_crypto->kex_methods[SSH_COMP_S_C] = find_matching(
        client_session->next_crypto->server_kex.methods[SSH_COMP_S_C].cstr(),
        client_session->next_crypto->client_kex.methods[SSH_COMP_S_C].cstr(),
        ',');

    if (client_session->next_crypto->kex_methods[SSH_COMP_S_C].size == 0){
        ssh_set_error(client_session->error,SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
            "compression algo server->client",
            client_session->next_crypto->server_kex.methods[SSH_COMP_S_C].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_COMP_S_C].cstr());
        client_session->socket->close();
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }
    client_session->next_crypto->kex_methods[SSH_LANG_C_S] = find_matching(
        client_session->next_crypto->server_kex.methods[SSH_LANG_C_S].cstr(),
        client_session->next_crypto->client_kex.methods[SSH_LANG_C_S].cstr(),
        ',');

    client_session->next_crypto->kex_methods[SSH_LANG_S_C] = find_matching(
        client_session->next_crypto->server_kex.methods[SSH_LANG_S_C].cstr(),
        client_session->next_crypto->client_kex.methods[SSH_LANG_S_C].cstr(),
        ',');

    client_session->out_buffer->out_uint8(SSH_MSG_KEXINIT);
    client_session->out_buffer->out_blob(client_session->next_crypto->client_kex.cookie, 16);

    syslog(LOG_INFO, "kex algos: %s",
        client_session->next_crypto->client_kex.methods[SSH_KEX].cstr());
    syslog(LOG_INFO, "server host key algo: %s",
        client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS].cstr());
    syslog(LOG_INFO, "encryption client->server: %s",
        client_session->next_crypto->client_kex.methods[SSH_CRYPT_C_S].cstr());
    syslog(LOG_INFO, "encryption server->client: %s",
        client_session->next_crypto->client_kex.methods[SSH_CRYPT_S_C].cstr());
    syslog(LOG_INFO, "mac algo client->server: %s",
        client_session->next_crypto->client_kex.methods[SSH_MAC_C_S].cstr());
    syslog(LOG_INFO, "mac algo server->client: %s",
        client_session->next_crypto->client_kex.methods[SSH_MAC_S_C].cstr());
    syslog(LOG_INFO, "compression algo client->server: %s",
        client_session->next_crypto->client_kex.methods[SSH_COMP_C_S].cstr());
    syslog(LOG_INFO, "compression algo server->client: %s",
        client_session->next_crypto->client_kex.methods[SSH_COMP_S_C].cstr());
    syslog(LOG_INFO, "languages client->server: %s",
        client_session->next_crypto->client_kex.methods[SSH_LANG_C_S].cstr());
    syslog(LOG_INFO, "languages server->client: %s",
        client_session->next_crypto->client_kex.methods[SSH_LANG_S_C].cstr());

    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_KEX]);
    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS]);
    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_CRYPT_C_S]);
    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_CRYPT_S_C]);
    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_MAC_C_S]);
    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_MAC_S_C]);
    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_COMP_C_S]);
    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_COMP_S_C]);
    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_LANG_C_S]);
    client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_LANG_S_C]);
    client_session->out_buffer->out_uint8(0);
    client_session->out_buffer->out_uint32_be(0);

    client_session->packet_send();

    client_session->client_callbacks->connect_status_function(client_session->client_callbacks->userdata, 0.8f);
    client_session->session_state = SSH_SESSION_STATE_DH;
    if (dh_handshake_client(client_session, client_session->error) == SSH_ERROR) {
        client_session->socket->close();
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }
    if(client_session->dh_handshake_state == DH_STATE_FINISHED){
        client_session->client_callbacks->connect_status_function(client_session->client_callbacks->userdata, 1.0f);
        client_session->connected = 1;
        if (client_session->flags & SSH_SESSION_FLAG_AUTHENTICATED){
            client_session->session_state = SSH_SESSION_STATE_AUTHENTICATED;
        }
        else {
            client_session->session_state = SSH_SESSION_STATE_AUTHENTICATING;
        }
    }
    return SSH_PACKET_USED;
}

// ========================= END SSH_MSG_CHANNEL_FAILURE ===============================

/** @internal
 * @brief launches the DH handshake state machine
 * @param client_session session handle
 * @returns SSH_OK or SSH_ERROR
 * @warning this function returning is no proof that DH handshake is
 * completed
 */
static int dh_handshake_client(SshClientSession * client_session, error_struct & error)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    switch (client_session->dh_handshake_state) {
    case DH_STATE_INIT:
        switch(client_session->next_crypto->kex_type){
        case SSH_KEX_DH_GROUP1_SHA1:
        {
            client_session->next_crypto->x = BN_new();
            BN_rand(client_session->next_crypto->x, 128, 0, -1);

            // TODO: see how to simplify bignum ctx allocation
            BN_CTX* ctx = BN_CTX_new();
            client_session->next_crypto->e = BN_new();
            BIGNUM * g = BN_new();
            /* G is defined as 2 by the ssh2 standards */
            BN_set_word(g, 2);
            BIGNUM * p_group1 = BN_new();
            unsigned char p_group1_value[] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
            0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
            0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
            0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
            0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
            0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
            0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
            0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
            0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
            0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

            enum { P_GROUP1_LEN = 128 }; /* Size in bytes of the p number for group 1 */
            BN_bin2bn(p_group1_value, P_GROUP1_LEN, p_group1);
            BN_mod_exp(client_session->next_crypto->e, g, client_session->next_crypto->x, p_group1, ctx);
            BN_clear_free(p_group1);

            BN_clear_free(g);
            BN_CTX_free(ctx);

            {
                unsigned int len3 = BN_num_bytes(client_session->next_crypto->e);
                unsigned int bits3 = BN_num_bits(client_session->next_crypto->e);
                /* If the first bit is set we have a negative number, padding needed */
                int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(client_session->next_crypto->e, bits3 - 1))?1:0;
                SSHString num3(len3 + pad3);
                /* We have a negative number henceforth we need a leading zero */
                num3.data[0] = 0;
                BN_bn2bin(client_session->next_crypto->e, num3.data.get() + pad3);

                client_session->out_buffer->out_uint8(SSH_MSG_KEXDH_INIT);
                client_session->out_buffer->out_uint32_be(num3.size);
                client_session->out_buffer->out_blob(num3.data.get(), num3.size);
            }

            client_session->packet_send();
        }
        break;
        case SSH_KEX_DH_GROUP14_SHA1:
        {
            client_session->next_crypto->x = BN_new();
            BN_rand(client_session->next_crypto->x, 128, 0, -1);

            // TODO: see how to simplify bignum ctx allocation
            BN_CTX* ctx = BN_CTX_new();
            client_session->next_crypto->e = BN_new();
            BIGNUM * g = BN_new();
            /* G is defined as 2 by the ssh2 standards */
            BN_set_word(g, 2);
            BIGNUM * p_group14 = BN_new();
            unsigned char p_group14_value[] = {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
                0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
                0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
                0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
                0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
                0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
                0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
                0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
                0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05, 0x98, 0xDA, 0x48, 0x36,
                0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
                0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56,
                0x20, 0x85, 0x52, 0xBB, 0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
                0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04, 0xF1, 0x74, 0x6C, 0x08,
                0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
                0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2,
                0xEC, 0x07, 0xA2, 0x8F, 0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9,
                0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18, 0x39, 0x95, 0x49, 0x7C,
                0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
                0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAC, 0xAA, 0x68, 0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF};
            enum { P_GROUP14_LEN = 256 }; /* Size in bytes of the p number for group 14 */
            BN_bin2bn(p_group14_value, P_GROUP14_LEN, p_group14);
            BN_mod_exp(client_session->next_crypto->e, g, client_session->next_crypto->x, p_group14, ctx);
            BN_clear_free(p_group14);

            BN_clear_free(g);
            BN_CTX_free(ctx);

            {
                unsigned int len3 = BN_num_bytes(client_session->next_crypto->e);
                unsigned int bits3 = BN_num_bits(client_session->next_crypto->e);
                /* If the first bit is set we have a negative number, padding needed */
                int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(client_session->next_crypto->e, bits3 - 1))?1:0;
                SSHString num3(len3 + pad3);
                /* We have a negative number henceforth we need a leading zero */
                num3.data[0] = 0;
                BN_bn2bin(client_session->next_crypto->e, num3.data.get() + pad3);

                client_session->out_buffer->out_uint8(SSH_MSG_KEXDH_INIT);
                client_session->out_buffer->out_uint32_be(num3.size);
                client_session->out_buffer->out_blob(num3.data.get(), num3.size);
            }

            client_session->packet_send();
        }
        break;
        case SSH_KEX_ECDH_SHA2_NISTP256:
        {
            // TODO move that to ecdh.init
            BN_CTX* ctx = BN_CTX_new();
            client_session->next_crypto->ecdh.privkey = EC_KEY_new_by_curve_name(NISTP256);
            if (client_session->next_crypto->ecdh.privkey == nullptr) {
                BN_CTX_free(ctx);
                return SSH_ERROR;
            }
            const EC_GROUP *group = EC_KEY_get0_group(client_session->next_crypto->ecdh.privkey);
            EC_KEY_generate_key(client_session->next_crypto->ecdh.privkey);

            const EC_POINT * pubkey = EC_KEY_get0_public_key(client_session->next_crypto->ecdh.privkey);
            int len = EC_POINT_point2oct(group, pubkey, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, ctx);

            client_session->next_crypto->ecdh.client_pubkey = SSHString(len);
            EC_POINT_point2oct(group, pubkey, POINT_CONVERSION_UNCOMPRESSED,
                               client_session->next_crypto->ecdh.client_pubkey.data.get(), len, ctx);
            BN_CTX_free(ctx);

            client_session->out_buffer->out_uint8(SSH_MSG_KEX_ECDH_INIT);
            client_session->out_buffer->out_uint32_be(client_session->next_crypto->ecdh.client_pubkey.size);
            client_session->out_buffer->out_blob(client_session->next_crypto->ecdh.client_pubkey.data.get(),
                                          client_session->next_crypto->ecdh.client_pubkey.size);

            client_session->packet_send();
        }
        break;
        case SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG:
            client_session->next_crypto->curve_25519.init(client_session->next_crypto->curve_25519.client_pubkey);

            client_session->out_buffer->out_uint8(SSH_MSG_KEX_ECDH_INIT);
            client_session->out_buffer->out_uint32_be(CURVE25519_PUBKEY_SIZE);
            client_session->out_buffer->out_blob(client_session->next_crypto->curve_25519.client_pubkey, CURVE25519_PUBKEY_SIZE);

            client_session->packet_send();

        break;
        default:
            return SSH_ERROR;
        }

        client_session->dh_handshake_state = DH_STATE_INIT_SENT;
        REDEMPTION_CXX_FALLTHROUGH;
    case DH_STATE_INIT_SENT:
        /* wait until ssh_packet_dh_reply_client is called */
        break;
    case DH_STATE_NEWKEYS_SENT:
        /* wait until ssh_packet_newkeys is called */
        break;
    case DH_STATE_FINISHED:
      return SSH_OK;
    default:
      ssh_set_error(error, SSH_FATAL, "Invalid state in dh_handshake(): %d",
          client_session->dh_handshake_state);

      return SSH_ERROR;
  }

  return SSH_OK;
}


static void ssh_connection_callback_client(SshClientSession * client_session, error_struct & error){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    switch(client_session->session_state){
        case SSH_SESSION_STATE_NONE:
        case SSH_SESSION_STATE_CONNECTING:
        case SSH_SESSION_STATE_SOCKET_CONNECTED:
            break;
        case SSH_SESSION_STATE_INITIAL_KEX:
        /* TODO: This state should disappear in favor of get_key handle */
            break;
        case SSH_SESSION_STATE_KEXINIT_RECEIVED:
        {
            client_session->client_callbacks->connect_status_function(client_session->client_callbacks->userdata, 0.6f);

            if (client_session->next_crypto->server_kex.methods[SSH_KEX].size == 0){

                syslog(LOG_INFO, "%s [D] Initializing server kex methods", __FUNCTION__);

                RAND_pseudo_bytes(client_session->next_crypto->server_kex.cookie, 16);

                client_session->next_crypto->server_kex.methods[SSH_KEX] = "curve25519-sha256@libssh.org,ecdh-sha2-nistp256,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1";
                client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS] = "ecdsa-sha2-nistp256,ssh-rsa,ssh-dss";

                client_session->next_crypto->server_kex.methods[SSH_CRYPT_C_S] = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";
                client_session->next_crypto->server_kex.methods[SSH_CRYPT_S_C] = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";
                client_session->next_crypto->server_kex.methods[SSH_MAC_C_S] = "hmac-sha1";
                client_session->next_crypto->server_kex.methods[SSH_MAC_S_C] = "hmac-sha1";
                client_session->next_crypto->server_kex.methods[SSH_COMP_C_S] = "none,zlib,zlib@openssh.com";
                client_session->next_crypto->server_kex.methods[SSH_COMP_S_C] = "none,zlib,zlib@openssh.com";
                client_session->next_crypto->server_kex.methods[SSH_LANG_S_C] = "";
                client_session->next_crypto->server_kex.methods[SSH_LANG_C_S] = "";
            }
            else {
                syslog(LOG_INFO, "%s [D] Server kex methods already initialized", __FUNCTION__);
            }


            syslog(LOG_INFO, "kex algos: %s",
                client_session->next_crypto->server_kex.methods[SSH_KEX].cstr());
            syslog(LOG_INFO, "server host key algo: %s",
                client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS].cstr());
            syslog(LOG_INFO, "encryption client->server: %s",
                client_session->next_crypto->server_kex.methods[SSH_CRYPT_C_S].cstr());
            syslog(LOG_INFO, "encryption server->client: %s",
                client_session->next_crypto->server_kex.methods[SSH_CRYPT_S_C].cstr());
            syslog(LOG_INFO, "mac algo client->server: %s",
                client_session->next_crypto->server_kex.methods[SSH_MAC_C_S].cstr());
            syslog(LOG_INFO, "mac algo server->client: %s",
                client_session->next_crypto->server_kex.methods[SSH_MAC_S_C].cstr());
            syslog(LOG_INFO, "compression algo client->server: %s",
                client_session->next_crypto->server_kex.methods[SSH_COMP_C_S].cstr());
            syslog(LOG_INFO, "compression algo server->client: %s",
                client_session->next_crypto->server_kex.methods[SSH_COMP_S_C].cstr());
            syslog(LOG_INFO, "languages client->server: %s",
                client_session->next_crypto->server_kex.methods[SSH_LANG_C_S].cstr());
            syslog(LOG_INFO, "languages server->client: %s",
                client_session->next_crypto->server_kex.methods[SSH_LANG_S_C].cstr());


            client_session->next_crypto->kex_methods[SSH_KEX] = find_matching(
                client_session->next_crypto->server_kex.methods[SSH_KEX].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_KEX].cstr(),
                ',');

            if (client_session->next_crypto->kex_methods[SSH_KEX].size == 0){
                ssh_set_error(client_session->error, SSH_FATAL,
                    "kex error : no match for method %s: server [%s], client [%s]",
                    "kex algos",
                    client_session->next_crypto->server_kex.methods[SSH_KEX].cstr(),
                    client_session->next_crypto->client_kex.methods[SSH_KEX].cstr());
                client_session->socket->close();
                client_session->session_state = SSH_SESSION_STATE_ERROR;
                return;
            }

            // TODO: create a general purpose method to find a value in a string keyed dictionnary
            // and use it there and whenever necessary
            if(strcmp(client_session->next_crypto->kex_methods[SSH_KEX].cstr(), "diffie-hellman-group1-sha1") == 0){
              client_session->next_crypto->kex_type = SSH_KEX_DH_GROUP1_SHA1;
            }
            else if(strcmp(client_session->next_crypto->kex_methods[SSH_KEX].cstr(), "diffie-hellman-group14-sha1") == 0){
              client_session->next_crypto->kex_type = SSH_KEX_DH_GROUP14_SHA1;
            }
            else if(strcmp(client_session->next_crypto->kex_methods[SSH_KEX].cstr(), "ecdh-sha2-nistp256") == 0){
              client_session->next_crypto->kex_type = SSH_KEX_ECDH_SHA2_NISTP256;
            }
            else if(strcmp(client_session->next_crypto->kex_methods[SSH_KEX].cstr(), "curve25519-sha256@libssh.org") == 0){
              client_session->next_crypto->kex_type = SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG;
            }

            client_session->next_crypto->kex_methods[SSH_HOSTKEYS] = find_matching(
                client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS].cstr(),
                ',');

        syslog(LOG_INFO,"%s [B] Setting HOSKTEYS: server=%s client=%s -> methods=%s",
            __FUNCTION__,
            client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS].cstr(),
            client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS].cstr(),
            client_session->next_crypto->kex_methods[SSH_HOSTKEYS].cstr()
        );


            syslog(LOG_ERR, "%s HOSTKEYS %s %s",
                __FUNCTION__,
                client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS].cstr());

            if (client_session->next_crypto->kex_methods[SSH_HOSTKEYS].size == 0){
                ssh_set_error(client_session->error, SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
                    "server host key algo",
                    client_session->next_crypto->server_kex.methods[SSH_HOSTKEYS].cstr(),
                    client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS].cstr());
                client_session->socket->close();
                client_session->session_state = SSH_SESSION_STATE_ERROR;
                return;
            }

            client_session->next_crypto->kex_methods[SSH_CRYPT_C_S] = find_matching(
                    client_session->next_crypto->server_kex.methods[SSH_CRYPT_C_S].cstr(),
                    client_session->next_crypto->client_kex.methods[SSH_CRYPT_C_S].cstr(),
                    ',');

            if (client_session->next_crypto->kex_methods[SSH_CRYPT_C_S].size == 0){
                ssh_set_error(client_session->error, SSH_FATAL,
                    "kex error : no match for method %s: server [%s], client [%s]",
                    "encryption client->server",
                    client_session->next_crypto->server_kex.methods[SSH_CRYPT_C_S].cstr(),
                    client_session->next_crypto->client_kex.methods[SSH_CRYPT_C_S].cstr());
                client_session->socket->close();
                client_session->session_state = SSH_SESSION_STATE_ERROR;
                return;
            }
            client_session->next_crypto->kex_methods[SSH_CRYPT_S_C] = find_matching(
                client_session->next_crypto->server_kex.methods[SSH_CRYPT_S_C].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_CRYPT_S_C].cstr(),
                ',');

            if (client_session->next_crypto->kex_methods[SSH_CRYPT_S_C].size == 0){
                ssh_set_error(client_session->error, SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
                    "mac algo server->client",
                    client_session->next_crypto->server_kex.methods[SSH_CRYPT_S_C].cstr(),
                    client_session->next_crypto->client_kex.methods[SSH_CRYPT_S_C].cstr());
                client_session->socket->close();
                client_session->session_state = SSH_SESSION_STATE_ERROR;
                return;
            }

            client_session->next_crypto->kex_methods[SSH_MAC_C_S] = find_matching(
                client_session->next_crypto->server_kex.methods[SSH_MAC_C_S].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_MAC_C_S].cstr(),
                ',');

            if (client_session->next_crypto->kex_methods[SSH_MAC_C_S].size == 0){
                ssh_set_error(client_session->error, SSH_FATAL,
                    "kex error : no match for method %s: server [%s], client [%s]",
                    "mac algo client->server",
                    client_session->next_crypto->server_kex.methods[SSH_MAC_C_S].cstr(),
                    client_session->next_crypto->client_kex.methods[SSH_MAC_C_S].cstr());
                client_session->socket->close();
                client_session->session_state = SSH_SESSION_STATE_ERROR;
                return;
            }

            client_session->next_crypto->kex_methods[SSH_MAC_S_C] = find_matching(
                client_session->next_crypto->server_kex.methods[SSH_MAC_S_C].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_MAC_S_C].cstr(),
                ',');

            if (client_session->next_crypto->kex_methods[SSH_MAC_S_C].size == 0){
                ssh_set_error(client_session->error, SSH_FATAL,
                    "kex error : no match for method %s: server [%s], client [%s]",
                    "mac algo server->client",
                    client_session->next_crypto->server_kex.methods[SSH_MAC_S_C].cstr(),
                    client_session->next_crypto->client_kex.methods[SSH_MAC_S_C].cstr());
                client_session->socket->close();
                client_session->session_state = SSH_SESSION_STATE_ERROR;
                return;
            }
            client_session->next_crypto->kex_methods[SSH_COMP_C_S] = find_matching(
                client_session->next_crypto->server_kex.methods[SSH_COMP_C_S].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_COMP_C_S].cstr(),
                ',');

            if (client_session->next_crypto->kex_methods[SSH_COMP_C_S].size == 0){
                ssh_set_error(client_session->error,SSH_FATAL,
                    "kex error : no match for method %s: server [%s], client [%s]",
                    "compression algo client->server",
                    client_session->next_crypto->server_kex.methods[SSH_COMP_C_S].cstr(),
                    client_session->next_crypto->client_kex.methods[SSH_COMP_C_S].cstr());
                client_session->socket->close();
                client_session->session_state = SSH_SESSION_STATE_ERROR;
                return;
            }
            client_session->next_crypto->kex_methods[SSH_COMP_S_C] = find_matching(
                client_session->next_crypto->server_kex.methods[SSH_COMP_S_C].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_COMP_S_C].cstr(),
                ',');

            if (client_session->next_crypto->kex_methods[SSH_COMP_S_C].size == 0){
                ssh_set_error(client_session->error,SSH_FATAL,
                    "kex error : no match for method %s: server [%s], client [%s]",
                    "compression algo server->client",
                    client_session->next_crypto->server_kex.methods[SSH_COMP_S_C].cstr(),
                    client_session->next_crypto->client_kex.methods[SSH_COMP_S_C].cstr());
                client_session->socket->close();
                client_session->session_state = SSH_SESSION_STATE_ERROR;
                return;
            }
            client_session->next_crypto->kex_methods[SSH_LANG_C_S] = find_matching(
                client_session->next_crypto->server_kex.methods[SSH_LANG_C_S].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_LANG_C_S].cstr(),
                ',');

            client_session->next_crypto->kex_methods[SSH_LANG_S_C] = find_matching(
                client_session->next_crypto->server_kex.methods[SSH_LANG_S_C].cstr(),
                client_session->next_crypto->client_kex.methods[SSH_LANG_S_C].cstr(),
                ',');

            client_session->out_buffer->out_uint8(SSH_MSG_KEXINIT);
            client_session->out_buffer->out_blob(client_session->next_crypto->client_kex.cookie, 16);

            syslog(LOG_INFO, "kex algos: %s",
                client_session->next_crypto->client_kex.methods[SSH_KEX].cstr());
            syslog(LOG_INFO, "server host key algo: %s",
                client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS].cstr());
            syslog(LOG_INFO, "encryption client->server: %s",
                client_session->next_crypto->client_kex.methods[SSH_CRYPT_C_S].cstr());
            syslog(LOG_INFO, "encryption server->client: %s",
                client_session->next_crypto->client_kex.methods[SSH_CRYPT_S_C].cstr());
            syslog(LOG_INFO, "mac algo client->server: %s",
                client_session->next_crypto->client_kex.methods[SSH_MAC_C_S].cstr());
            syslog(LOG_INFO, "mac algo server->client: %s",
                client_session->next_crypto->client_kex.methods[SSH_MAC_S_C].cstr());
            syslog(LOG_INFO, "compression algo client->server: %s",
                client_session->next_crypto->client_kex.methods[SSH_COMP_C_S].cstr());
            syslog(LOG_INFO, "compression algo server->client: %s",
                client_session->next_crypto->client_kex.methods[SSH_COMP_S_C].cstr());
            syslog(LOG_INFO, "languages client->server: %s",
                client_session->next_crypto->client_kex.methods[SSH_LANG_C_S].cstr());
            syslog(LOG_INFO, "languages server->client: %s",
                client_session->next_crypto->client_kex.methods[SSH_LANG_S_C].cstr());


            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_KEX]);
            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_HOSTKEYS]);
            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_CRYPT_C_S]);
            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_CRYPT_S_C]);
            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_MAC_C_S]);
            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_MAC_S_C]);
            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_COMP_C_S]);
            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_COMP_S_C]);
            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_LANG_C_S]);
            client_session->out_buffer->out_sshstring(client_session->next_crypto->client_kex.methods[SSH_LANG_S_C]);
            client_session->out_buffer->out_uint8(0);
            client_session->out_buffer->out_uint32_be(0);

            client_session->packet_send();

            client_session->client_callbacks->connect_status_function(client_session->client_callbacks->userdata, 0.8f);
            client_session->session_state = SSH_SESSION_STATE_DH;
            if (dh_handshake_client(client_session, error) == SSH_ERROR) {
                client_session->socket->close();
                client_session->session_state = SSH_SESSION_STATE_ERROR;
                return;
            }
            if(client_session->dh_handshake_state==DH_STATE_FINISHED){
                client_session->client_callbacks->connect_status_function(client_session->client_callbacks->userdata, 1.0f);
                client_session->connected = 1;
                if (client_session->flags & SSH_SESSION_FLAG_AUTHENTICATED){
                    client_session->session_state = SSH_SESSION_STATE_AUTHENTICATED;
                }
                else {
                    client_session->session_state = SSH_SESSION_STATE_AUTHENTICATING;
                }
            }
        }
        break;
        case SSH_SESSION_STATE_DH:
        {
            if(client_session->dh_handshake_state==DH_STATE_FINISHED){
                client_session->client_callbacks->connect_status_function(client_session->client_callbacks->userdata, 1.0f);
                client_session->connected = 1;
                if (client_session->flags & SSH_SESSION_FLAG_AUTHENTICATED){
                    client_session->session_state = SSH_SESSION_STATE_AUTHENTICATED;
                }
                else {
                    client_session->session_state = SSH_SESSION_STATE_AUTHENTICATING;
                }
            }
        }
        break;
        case SSH_SESSION_STATE_AUTHENTICATING:
            break;
        case SSH_SESSION_STATE_ERROR:
            client_session->socket->close();
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return;
        case SSH_SESSION_STATE_DISCONNECTED:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_AUTHENTICATED:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_BANNER_RECEIVED:
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            ssh_set_error(error, SSH_FATAL,"Invalid state %d",client_session->session_state);
    }
    return;
}




static void handle_signal_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    char * sig = packet->in_strdup_cstr();
    syslog(LOG_INFO,
      "SSH_MSG_CHANNEL_REQUEST '%s' <%s> for channel %s wr=%d",
      "signal", sig, channel->show(), want_reply);

    if (channel->callbacks && channel->callbacks->channel_signal_function) {
        channel->callbacks->channel_signal_function(client_session,
                                                    channel,
                                                    sig,
                                                    channel->callbacks->userdata);
    }
    delete sig;
}

static inline void handle_exit_signal_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    char * sig = packet->in_strdup_cstr();
    uint8_t i = packet->in_uint8();
    const char *core = i?"(core dumped)":"";
    char * errmsg = packet->in_strdup_cstr();
    char * lang = packet->in_strdup_cstr();

    syslog(LOG_INFO,
      "SSH_MSG_CHANNEL_REQUEST '%s' <%s %s %s %s> for channel %s wr=%d",
      "exit-signal", sig, core, errmsg, lang, channel->show(), want_reply);

    if (channel->callbacks && channel->callbacks->channel_exit_signal_function) {
        channel->callbacks->channel_exit_signal_function(client_session,
                                                         channel,
                                                         sig, i, errmsg, lang,
                                                         channel->callbacks->userdata);
    }
    delete lang;
    delete errmsg;
    delete sig;
}


static inline void handle_auth_agent_req_at_openssh_dot_com_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO,
      "SSH_MSG_CHANNEL_REQUEST '%s' <> for channel %s wr=%d",
      "auth-agent-req@openssh.com", channel->show(), want_reply);

    if (channel->callbacks && channel->callbacks->channel_auth_agent_req_function) {
        channel->callbacks->channel_auth_agent_req_function(client_session, channel,
                                                            channel->callbacks->userdata);
        if (want_reply) {
            syslog(LOG_INFO,"Responding to Openssh's auth-agent-req");
            client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_SUCCESS);
            client_session->out_buffer->out_uint32_be(channel->remote_channel);
            client_session->packet_send();
        }
    }
}


static inline void handle_pty_req_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
{
    (void)client_session;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    char * TERM = packet->in_strdup_cstr();
    uint32_t width = packet->in_uint32_be();
    uint32_t height = packet->in_uint32_be();
    uint32_t pxwidth = packet->in_uint32_be();
    uint32_t pxheight = packet->in_uint32_be();
    char * modes = packet->in_strdup_cstr();

    syslog(LOG_INFO,
      "SSH_MSG_CHANNEL_REQUEST '%s' <%s, %d, %d, %d, %d, %s> for channel %s wr=%d",
      "pty-req", TERM,
      static_cast<int>(width), static_cast<int>(height),
      static_cast<int>(pxwidth), static_cast<int>(pxheight), modes,
      channel->show(), want_reply);

    delete TERM;
    delete modes;
}


//6.7.  Window Dimension Change Message
//---------------------------------------

//   When the window (terminal) size changes on the client side, it MAY
//   send a message to the other side to inform it of the new dimensions.

//      byte      SSH_MSG_CHANNEL_REQUEST
//      uint32    recipient channel
//      string    "window-change"
//      boolean   FALSE
//      uint32    terminal width, columns
//      uint32    terminal height, rows
//      uint32    terminal width, pixels
//      uint32    terminal height, pixels

//   A response SHOULD NOT be sent to this message.

static inline void handle_window_change_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
{
    (void)client_session;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t width = packet->in_uint32_be();
    uint32_t height = packet->in_uint32_be();
    uint32_t pxwidth = packet->in_uint32_be();
    uint32_t pxheight = packet->in_uint32_be();

    syslog(LOG_INFO,
      "SSH_MSG_CHANNEL_REQUEST '%s' <%d, %d, %d, %d> for channel %s wr=%d",
      "window-change", static_cast<int>(width), static_cast<int>(height), static_cast<int>(pxwidth), static_cast<int>(pxheight),
      channel->show(), want_reply);
}


static inline void handle_subsystem_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    char *subsystem = packet->in_strdup_cstr();

    syslog(LOG_INFO,
      "SSH_MSG_CHANNEL_REQUEST '%s' <%s> for channel %s wr=%d",
      "subsystem", subsystem, channel->show(), want_reply);

    // SSH_REQUEST_CHANNEL SSH_CHANNEL_REQUEST_SUBSYSTEM
    int rc = -1;
    if (want_reply) {
        syslog(LOG_INFO, "Sending a channel_request success to channel %d", channel->remote_channel);
        client_session->out_buffer->out_uint8((rc == 0)?SSH_MSG_CHANNEL_SUCCESS:SSH_MSG_CHANNEL_FAILURE);
        client_session->out_buffer->out_uint32_be(channel->remote_channel);
        client_session->packet_send();
    }
    else {
        syslog(LOG_INFO, "The client doesn't want to know if the request succeeded");
    }
    delete subsystem;
}


static inline void handle_shell_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
{
    (void)client_session;
    (void)packet;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO,
      "SSH_MSG_CHANNEL_REQUEST '%s' <> for channel %s wr=%d",
      "shell", channel->show(), want_reply);
}



static inline void handle_exec_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
{
    (void)client_session;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    char * command = packet->in_strdup_cstr();

    syslog(LOG_INFO,
      "SSH_MSG_CHANNEL_REQUEST '%s' <%s> for channel %s wr=%d",
      "exec", command, channel->show(), want_reply);

    delete command;
}



// 6.4.  Environment Variable Passing
// ----------------------------------

//   Environment variables may be passed to the shell/command to be
//   started later.  Uncontrolled setting of environment variables in a
//   privileged process can be a security hazard.  It is recommended that
//   implementations either maintain a list of allowable variable names or
//   only set environment variables after the server process has dropped
//   sufficient privileges.

//      byte      SSH_MSG_CHANNEL_REQUEST
//      uint32    recipient channel
//      string    "env"
//      boolean   want reply
//      string    variable name
//      string    variable value



static inline void handle_env_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
{
    (void)client_session;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    char * var_name = packet->in_strdup_cstr();
    char * var_value = packet->in_strdup_cstr();

    syslog(LOG_INFO,
      "SSH_MSG_CHANNEL_REQUEST '%s' <%s=%s> for channel %s wr=%d",
      "env", var_name, var_value, channel->show(), want_reply);

    delete var_name;
    delete var_value;
}


static inline void handle_x11_req_request_client(SshClientSession * client_session, ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
{
    (void)client_session;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO,
      "Received a %s channel_request for channel (%d:%d) (want_reply=%d)",
      "x11-req", channel->local_channel, channel->remote_channel, want_reply);

    /* uint8_t x11_single_connection = */ packet->in_uint8();
    char * x11_auth_protocol = packet->in_strdup_cstr();
    char * x11_auth_cookie = packet->in_strdup_cstr();
    // TODO: why no network order ?
    /* uint32_t x11_screen_number = */ packet->in_uint32_le();
    delete x11_auth_protocol;
    delete x11_auth_cookie;
}


void SshClientSession::handle_channel_rcv_request_client(ssh_buffer_struct* packet, error_struct & error)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
//    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    uint32_t chan = packet->in_uint32_be();
    ssh_channel channel = this->ssh_channel_from_local(chan);
    if (channel == nullptr) {
        ssh_set_error(error,  SSH_FATAL,
                      "Server specified invalid channel %lu",
                      static_cast<long unsigned int>(ntohl(chan)));
        syslog(LOG_INFO,"%s", ssh_get_error(&error));
        return;
    }

    const char * request = packet->in_strdup_cstr();
    uint8_t request_code = get_request_code(request);
    if (request_code == REQUEST_STRING_UNKNOWN){
      syslog(LOG_INFO, "%s --- Unknown channel request %s %s",
        __FUNCTION__, request, channel->show());
      return;
    }
    delete request;

    uint8_t want_reply = packet->in_uint8();

    switch (request_code){
    case REQUEST_STRING_EXIT_STATUS:
        this->handle_exit_status_request_client(channel, want_reply, packet);
        break;
    case REQUEST_STRING_SIGNAL:
        handle_signal_request_client(this, channel, want_reply, packet);
        break;
    case REQUEST_STRING_EXIT_SIGNAL:
        handle_exit_signal_request_client(this, channel, want_reply, packet);
        break;
    case REQUEST_STRING_AUTH_AGENT_REQ_AT_OPENSSH_DOT_COM:
        handle_auth_agent_req_at_openssh_dot_com_request_client(this, channel, want_reply);
        break;
    case REQUEST_STRING_PTY_REQ:
        handle_pty_req_request_client(this, channel, want_reply, packet);
        break;
    case REQUEST_STRING_WINDOW_CHANGE:
        handle_window_change_request_client(this, channel, want_reply, packet);
        break;
    case REQUEST_STRING_SUBSYSTEM:
        handle_subsystem_request_client(this, channel, want_reply, packet);
        break;
    case REQUEST_STRING_SHELL:
        handle_shell_request_client(this, channel, want_reply, packet);
        break;
    case REQUEST_STRING_EXEC:
        handle_exec_request_client(this, channel, want_reply, packet);
        break;
    case REQUEST_STRING_ENV:
        handle_env_request_client(this, channel, want_reply, packet);
        break;
    case REQUEST_STRING_X11_REQ:
        handle_x11_req_request_client(this, channel, want_reply, packet);
        break;
    default:
        ;
    }
}


static inline void handle_channel_open_session_request_client(SshClientSession * client_session, ssh_buffer_struct * packet)
{
    (void)client_session;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_SESSION;
    syslog(LOG_INFO, "Clients wants to open a %s channel", "session");
    /* uint32_t sender = */ packet->in_uint32_be();
    /* uint32_t window = */ packet->in_uint32_be();
    /* uint32_t packet_size = */ packet->in_uint32_be();
}


inline void handle_channel_open_direct_tcpip_request_client(SshClientSession * client_session, ssh_buffer_struct * packet)
{
    (void)client_session;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_DIRECT_TCPIP
    syslog(LOG_INFO, "Clients wants to open a %s channel", "direct-tcpip");
    /* uint32_t sender = */ packet->in_uint32_be();
    /* uint32_t window = */ packet->in_uint32_be();
    /* uint32_t packet_size = */ packet->in_uint32_be();
    char * destination = packet->in_strdup_cstr();
    /* uint32_t destination_port = */ packet->in_uint32_be();
    char * originator = packet->in_strdup_cstr();
    /* uint32_t originator_port = */ packet->in_uint32_be();

    // Why leave other side of callback create channel ?
    // We initialize all relevant channel informations here anyway
    delete originator;
    delete destination;
}


static inline void handle_channel_open_forwarded_tcpip_request_client(SshClientSession * client_session, ssh_buffer_struct * packet)
{
    (void)client_session;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_FORWARDED_TCPIP
    syslog(LOG_INFO, "Clients wants to open a %s channel", "forwarded-tcpip");
    /* uint32_t sender = */      (void)packet->in_uint32_be();
    /* uint32_t window = */      (void)packet->in_uint32_be();
    /* uint32_t packet_size = */ (void)packet->in_uint32_be();
    char * destination = packet->in_strdup_cstr();
    /* uint32_t destination_port = */ (void)packet->in_uint32_be();
    char * originator = packet->in_strdup_cstr();
    /* uint32_t originator_port = */ (void)packet->in_uint32_be();
    delete originator;
    delete destination;
}


static inline void handle_channel_open_x11_request_client(SshClientSession * client_session, ssh_buffer_struct * packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_X11
    syslog(LOG_INFO, "Clients wants to open a %s channel", "x11");
    uint32_t sender = packet->in_uint32_be();
    uint32_t window = packet->in_uint32_be();
    uint32_t packet_size = packet->in_uint32_be();
    char * originator = packet->in_strdup_cstr();
    uint32_t originator_port = packet->in_uint32_be();
    syslog(LOG_INFO, "Calling server callback x11");

    client_session->client_callbacks->channel_open_request_x11_function(
            client_session,
            originator,
            originator_port,
            sender,
            window,
            packet_size,
            client_session->client_callbacks->userdata);

    syslog(LOG_INFO, "Call to server callback x11 done");

    delete originator;
}


static inline void handle_channel_open_auth_agent_request_client(SshClientSession * client_session, ssh_buffer_struct * packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_X11
    syslog(LOG_INFO, "Client wants to open a %s channel", "auth-agent@openssh.com");
    uint32_t sender = packet->in_uint32_be();
    uint32_t window = packet->in_uint32_be();
    uint32_t packet_size = packet->in_uint32_be();
    syslog(LOG_INFO, "Calling client callback auth_agent");
    ssh_channel channel = client_session->client_callbacks->channel_open_request_auth_agent_function(
            client_session,
            client_session->client_callbacks->userdata);

    if (channel != nullptr) {
        syslog(LOG_INFO,
                "Auth Agent Accepting a channel request_open for chan %d",
                channel->remote_channel);

        channel->local_channel = client_session->new_channel_id();
        channel->local_window = 32000;
        channel->local_maxpacket = CHANNEL_TOTAL_PACKET_SIZE;
        channel->remote_channel = sender;
        channel->remote_window = window;
        channel->remote_maxpacket = packet_size;
        channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN;

        client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_CONFIRMATION);
        client_session->out_buffer->out_uint32_be(sender);
        client_session->out_buffer->out_uint32_be(channel->local_channel);
        client_session->out_buffer->out_uint32_be(channel->local_window);
        client_session->out_buffer->out_uint32_be(channel->local_maxpacket);
    }
    else {
            syslog(LOG_INFO, "Auth Agent Refusing a channel");

            client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_FAILURE);
            client_session->out_buffer->out_uint32_be(sender);
            client_session->out_buffer->out_uint32_be(SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED);
            client_session->out_buffer->out_uint32_be(0); /* reason is an empty string */
            client_session->out_buffer->out_uint32_be(0); /* language too */
    }
    client_session->packet_send();
}


//5.1.  Opening a Channel

//   When either side wishes to open a new channel, it allocates a local
//   number for the channel.  It then sends the following message to the
//   other side, and includes the local channel number and initial window
//   size in the message.

//      byte      SSH_MSG_CHANNEL_OPEN
//      string    channel type in US-ASCII only
//      uint32    sender channel
//      uint32    initial window size
//      uint32    maximum packet size
//      ....      channel type specific data follows

//   The 'channel type' is a name, as described in [SSH-ARCH] and
//   [SSH-NUMBERS], with similar extension mechanisms.  The 'sender
//   channel' is a local identifier for the channel used by the sender of
//   this message.  The 'initial window size' specifies how many bytes of
//   channel data can be sent to the sender of this message without
//   adjusting the window.  The 'maximum packet size' specifies the
//   maximum size of an individual data packet that can be sent to the
//   sender.  For example, one might want to use smaller packets for
//   interactive connections to get better interactive response on slow
//   links.

//   The remote side then decides whether it can open the channel, and
//   responds with either SSH_MSG_CHANNEL_OPEN_CONFIRMATION or
//   SSH_MSG_CHANNEL_OPEN_FAILURE.

//      byte      SSH_MSG_CHANNEL_OPEN_CONFIRMATION
//      uint32    recipient channel
//      uint32    sender channel
//      uint32    initial window size
//      uint32    maximum packet size
//      ....      channel type specific data follows

//   The 'recipient channel' is the channel number given in the original
//   open request, and 'sender channel' is the channel number allocated by
//   the other side.

//      byte      SSH_MSG_CHANNEL_OPEN_FAILURE
//      uint32    recipient channel
//      uint32    reason code
//      string    description in ISO-10646 UTF-8 encoding [RFC3629]
//      string    language tag [RFC3066]

//   If the recipient of the SSH_MSG_CHANNEL_OPEN message does not support
//   the specified 'channel type', it simply responds with
//   SSH_MSG_CHANNEL_OPEN_FAILURE.  The client MAY show the 'description'
//   string to the user.  If this is done, the client software should take
//   the precautions discussed in [SSH-ARCH].

//   The SSH_MSG_CHANNEL_OPEN_FAILURE 'reason code' values are defined in
//   the following table.  Note that the values for the 'reason code' are
//   given in decimal format for readability, but they are actually uint32
//   values.

//             Symbolic name                           reason code
//             -------------                           -----------
//            SSH_OPEN_ADMINISTRATIVELY_PROHIBITED          1
//            SSH_OPEN_CONNECT_FAILED                       2
//            SSH_OPEN_UNKNOWN_CHANNEL_TYPE                 3
//            SSH_OPEN_RESOURCE_SHORTAGE                    4

//   Requests for assignments of new SSH_MSG_CHANNEL_OPEN 'reason code'
//   values (and associated 'description' text) in the range of 0x00000005
//   to 0xFDFFFFFF MUST be done through the IETF CONSENSUS method, as
//   described in [RFC2434].  The IANA will not assign Channel Connection
//   Failure 'reason code' values in the range of 0xFE000000 to
//   0xFFFFFFFF.  Channel Connection Failure 'reason code' values in that
//   range are left for PRIVATE USE, as described in [RFC2434].

//   While it is understood that the IANA will have no control over the
//   range of 0xFE000000 to 0xFFFFFFFF, this range will be split in two
//   parts and administered by the following conventions.

//   o  The range of 0xFE000000 to 0xFEFFFFFF is to be used in conjunction
//      with locally assigned channels.  For example, if a channel is
//      proposed with a 'channel type' of "example_session@example.com",
//      but fails, then the response will contain either a 'reason code'
//      assigned by the IANA (as listed above and in the range of
//      0x00000001 to 0xFDFFFFFF) or a locally assigned value in the range
//      of 0xFE000000 to 0xFEFFFFFF.  Naturally, if the server does not
//      understand the proposed 'channel type', even if it is a locally
//      defined 'channel type', then the 'reason code' MUST be 0x00000003,
//      as described above, if the 'reason code' is sent.  If the server
//      does understand the 'channel type', but the channel still fails to
//      open, then the server SHOULD respond with a locally assigned
//      'reason code' value consistent with the proposed, local 'channel
//      type'.  It is assumed that practitioners will first attempt to use
//      the IANA assigned 'reason code' values and then document their
//      locally assigned 'reason code' values.

//   o  There are no restrictions or suggestions for the range starting
//      with 0xFF.  No interoperability is expected for anything used in
//      this range.  Essentially, it is for experimentation.


static inline int handle_channel_open_client(SshClientSession * client_session, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    char * type_c = packet->in_strdup_cstr();
    syslog(LOG_INFO, "%s --- channel type = %s", __FUNCTION__, type_c);
    uint8_t channel_open_code = get_channel_open_request_code(type_c);
    delete type_c;

    switch (channel_open_code) {
    case REQUEST_STRING_CHANNEL_OPEN_SESSION:
        syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_SESSION", __FUNCTION__);
        handle_channel_open_session_request_client(client_session, packet);
    break;
    case REQUEST_STRING_CHANNEL_OPEN_DIRECT_TCPIP:
        syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_DIRECT_TCPIP", __FUNCTION__);
        handle_channel_open_direct_tcpip_request_client(client_session, packet);
    break;
    case REQUEST_STRING_CHANNEL_OPEN_FORWARDED_TCPIP:
        syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_FORWARDED_TCPIP", __FUNCTION__);
        handle_channel_open_forwarded_tcpip_request_client(client_session, packet);
    break;
    case REQUEST_STRING_CHANNEL_OPEN_X11:
        syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_X11", __FUNCTION__);
        handle_channel_open_x11_request_client(client_session, packet);
    break;
    case REQUEST_STRING_CHANNEL_OPEN_AUTH_AGENT:
        syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_AUTH_AGENT", __FUNCTION__);
        handle_channel_open_auth_agent_request_client(client_session, packet);
    break;
    case REQUEST_STRING_CHANNEL_OPEN_UNKNOWN:
        syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_UNKNOWN", __FUNCTION__);
        REDEMPTION_CXX_FALLTHROUGH;
    default:
    break;
    }
    return SSH_PACKET_USED;
}


/*
 * draft-ietf-secsh-architecture-05.txt
 *
 *   Transport layer protocol:
 *
 *     1-19     Transport layer generic (e.g. disconnect, ignore, debug,
 *              etc)
 *     20-29    Algorithm negotiation
 *     30-49    Key exchange method specific (numbers can be reused for
 *              different authentication methods)
 *
 *   User authentication protocol:
 *
 *     50-59    User authentication generic
 *     60-79    User authentication method specific (numbers can be reused
 *              for different authentication methods)
 *
 *   Connection protocol:
 *
 *     80-89    Connection protocol generic
 *     90-127   Channel related messages
 *
 *   Reserved for client protocols:
 *
 *     128-191  Reserved
 *
 *   Local extensions:
 *
 *     192-255  Local extensions
 */

// SSH_PACKET_CALLBACK(ssh_packet_dh_reply)
static inline int ssh_packet_dh_reply_client(SshClientSession * client_session, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO,"Received SSH_KEXDH_REPLY");
    if(client_session->session_state != SSH_SESSION_STATE_DH
    && client_session->dh_handshake_state != DH_STATE_INIT_SENT){
        ssh_set_error(client_session->error,  SSH_FATAL,"ssh_packet_dh_reply_client called in wrong state : %d:%d",
            client_session->session_state, client_session->dh_handshake_state);
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    // client_session->next_crypto->
    switch(client_session->next_crypto->kex_type){
    case SSH_KEX_DH_GROUP1_SHA1:
    {
        if (sizeof(uint32_t) > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t tmp_server_pubkey_len = packet->in_uint32_be();
        if (tmp_server_pubkey_len > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        client_session->next_crypto->server_pubkey = SSHString(tmp_server_pubkey_len);
        packet->buffer_get_data(client_session->next_crypto->server_pubkey.data.get(),tmp_server_pubkey_len);

        if (sizeof(uint32_t) > packet->in_remain()){
            ssh_set_error(client_session->error, SSH_FATAL, "Cannot import f number");
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t f_len = packet->in_uint32_be();
        if (f_len > packet->in_remain()){
            ssh_set_error(client_session->error, SSH_FATAL, "Cannot import f number");
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint8_t * f = new uint8_t[f_len];
        packet->buffer_get_data(f, f_len);
        // TODO: is there error management for BN_bin2bn
        client_session->next_crypto->f = BN_bin2bn(f, f_len, nullptr);

        if (sizeof(uint32_t) > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t tmp_dh_server_signature_len = packet->in_uint32_be();
        if (tmp_dh_server_signature_len > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }

        SSHString tmp_dh_server_signature(tmp_dh_server_signature_len);
        packet->buffer_get_data(tmp_dh_server_signature.data.get(),tmp_dh_server_signature_len);

        client_session->next_crypto->dh_server_signature = std::move(tmp_dh_server_signature);

        BN_CTX* ctx = BN_CTX_new();
        client_session->next_crypto->k = BN_new();
        /* the server and clients don't use the same numbers */
        BIGNUM * p_group1 = BN_new();
        unsigned char p_group1_value[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
        0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
        0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
        0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
        0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
        0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
        0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
        0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
        0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
        0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        enum { P_GROUP1_LEN = 128 }; /* Size in bytes of the p number for group 1 */
        BN_bin2bn(p_group1_value, P_GROUP1_LEN, p_group1);
        BN_mod_exp(client_session->next_crypto->k, client_session->next_crypto->f, client_session->next_crypto->x, p_group1, ctx);
        BN_clear_free(p_group1);
        BN_CTX_free(ctx);
    }
    break;
    case SSH_KEX_DH_GROUP14_SHA1:
    {
        if (sizeof(uint32_t) > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t tmp_server_pubkey_len = packet->in_uint32_be();
        if (tmp_server_pubkey_len > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        SSHString tmp_server_pubkey(tmp_server_pubkey_len);
        packet->buffer_get_data(tmp_server_pubkey.data.get(),tmp_server_pubkey_len);

        client_session->next_crypto->server_pubkey = std::move(tmp_server_pubkey);

        if (sizeof(uint32_t) > packet->in_remain()){
            ssh_set_error(client_session->error, SSH_FATAL, "Cannot import f number");
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t f_len = packet->in_uint32_be();
        if (f_len > packet->in_remain()){
            ssh_set_error(client_session->error, SSH_FATAL, "Cannot import f number");
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint8_t * f = new uint8_t[f_len];
        packet->buffer_get_data(f, f_len);
        // TODO: is there error management for BN_bin2bn
        client_session->next_crypto->f = BN_bin2bn(f, f_len, nullptr);

        if (sizeof(uint32_t) > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t tmp_dh_server_signature_len = packet->in_uint32_be();
        if (tmp_dh_server_signature_len > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        SSHString tmp_dh_server_signature(tmp_dh_server_signature_len);
        packet->buffer_get_data(tmp_dh_server_signature.data.get(),tmp_dh_server_signature_len);

        client_session->next_crypto->dh_server_signature = std::move(tmp_dh_server_signature);

        BN_CTX* ctx = BN_CTX_new();
        client_session->next_crypto->k = BN_new();

        /* the server and clients don't use the same numbers */
        BIGNUM * p_group14 = BN_new();
        unsigned char p_group14_value[] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
            0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
            0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
            0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
            0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
            0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
            0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
            0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
            0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
            0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
            0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05, 0x98, 0xDA, 0x48, 0x36,
            0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
            0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56,
            0x20, 0x85, 0x52, 0xBB, 0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
            0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04, 0xF1, 0x74, 0x6C, 0x08,
            0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
            0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2,
            0xEC, 0x07, 0xA2, 0x8F, 0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9,
            0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18, 0x39, 0x95, 0x49, 0x7C,
            0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
            0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAC, 0xAA, 0x68, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF};

        enum { P_GROUP14_LEN = 256 }; /* Size in bytes of the p number for group 14 */
        BN_bin2bn(p_group14_value, P_GROUP14_LEN, p_group14);
        BN_mod_exp(client_session->next_crypto->k, client_session->next_crypto->f, client_session->next_crypto->x, p_group14, ctx);
        BN_clear_free(p_group14);
        BN_CTX_free(ctx);
    }
    break;
    case SSH_KEX_ECDH_SHA2_NISTP256:
    {
        if (sizeof(uint32_t) > packet->in_remain()) {
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t tmp_server_pubkey_len = packet->in_uint32_be();
        if (tmp_server_pubkey_len > packet->in_remain()) {
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        SSHString tmp_server_pubkey(tmp_server_pubkey_len);
        packet->buffer_get_data(tmp_server_pubkey.data.get(),tmp_server_pubkey_len);
        client_session->next_crypto->server_pubkey = std::move(tmp_server_pubkey);

        if (sizeof(uint32_t) > packet->in_remain()) {
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t tmp_ecdh_server_pubkey_len = packet->in_uint32_be();
        if (tmp_ecdh_server_pubkey_len > packet->in_remain()) {
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        client_session->next_crypto->ecdh.server_pubkey = SSHString(tmp_ecdh_server_pubkey_len);
        packet->buffer_get_data(client_session->next_crypto->ecdh.server_pubkey.data.get(),
                                client_session->next_crypto->ecdh.server_pubkey.size);

        if (sizeof(uint32_t) > packet->in_remain()) {
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t tmp_dh_server_signature_len = packet->in_uint32_be();
        if (tmp_dh_server_signature_len > packet->in_remain()) {
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        SSHString tmp_dh_server_signature(tmp_dh_server_signature_len);
        packet->buffer_get_data(tmp_dh_server_signature.data.get(),tmp_dh_server_signature_len);
        client_session->next_crypto->dh_server_signature = std::move(tmp_dh_server_signature);

        /* TODO: verify signature now instead of waiting for NEWKEYS */
        if (client_session->next_crypto->ecdh.build_k(client_session->next_crypto->k, client_session->next_crypto->ecdh.server_pubkey) < 0) {
            ssh_set_error(client_session->error, SSH_FATAL, "Cannot build k number");
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
    }
    break;
    case SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG:
    {
        /* this is the server host key */
        if (sizeof(uint32_t) > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        uint32_t tmp_server_pubkey_len = packet->in_uint32_be();
        if (tmp_server_pubkey_len > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        SSHString tmp_server_pubkey(tmp_server_pubkey_len);
        packet->buffer_get_data(tmp_server_pubkey.data.get(),tmp_server_pubkey_len);

        client_session->next_crypto->server_pubkey = std::move(tmp_server_pubkey);

        if (sizeof(uint32_t) > packet->in_remain()){
            ssh_set_error(client_session->error, SSH_FATAL, "No Q_S ECC point in packet");
            return SSH_ERROR;
        }
        uint32_t q_s_string_len = packet->in_uint32_be();
        if (q_s_string_len > packet->in_remain()){
            ssh_set_error(client_session->error, SSH_FATAL, "No Q_S ECC point in packet");
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }


        if (q_s_string_len != CURVE25519_PUBKEY_SIZE){
            ssh_set_error(client_session->error, SSH_FATAL, "Incorrect size for server Curve25519 public key: %d",
                    static_cast<int>(q_s_string_len));
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        packet->buffer_get_data(client_session->next_crypto->curve_25519.server_pubkey, CURVE25519_PUBKEY_SIZE);

        if (sizeof(uint32_t) > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t tmp_dh_server_signature_len = packet->in_uint32_be();
        if (tmp_dh_server_signature_len > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            client_session->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        SSHString tmp_dh_server_signature(tmp_dh_server_signature_len);
        packet->buffer_get_data(tmp_dh_server_signature.data.get(),tmp_dh_server_signature_len);

        client_session->next_crypto->dh_server_signature = std::move(tmp_dh_server_signature);
        client_session->next_crypto->curve_25519.build_k(client_session->next_crypto->k,
                                               client_session->next_crypto->curve_25519.server_pubkey);
    }
    break;
    default:
        ssh_set_error(client_session->error,  SSH_FATAL,"Wrong kex type in ssh_packet_dh_reply_client");
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    /* Send the MSG_NEWKEYS */
    client_session->out_buffer->out_uint8(SSH_MSG_NEWKEYS);
    client_session->packet_send();
    client_session->dh_handshake_state = DH_STATE_NEWKEYS_SENT;
    return SSH_PACKET_USED;
}

// SSH_PACKET_CALLBACK(ssh_packet_newkeys)
static inline int ssh_packet_newkeys_client(SshClientSession * client_session, error_struct & error)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO, "Received SSH_MSG_NEWKEYS");
    if (client_session->session_state != SSH_SESSION_STATE_DH
    && client_session->dh_handshake_state != DH_STATE_NEWKEYS_SENT){
        ssh_set_error(error,  SSH_FATAL,"ssh_packet_newkeys called in wrong state : %d:%d",
        client_session->session_state, client_session->dh_handshake_state);
        client_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    int rc = client_session->make_sessionid();

    if (rc != SSH_OK) {
        client_session->session_state=SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    /*
     * Set the cryptographic functions for the next crypto
     * (it is needed for generate_session_keys for key lengths)
     */
    client_session->next_crypto->out_cipher = cipher_new_by_name(client_session->next_crypto->kex_methods[SSH_CRYPT_C_S]);
    client_session->next_crypto->in_cipher = cipher_new_by_name(client_session->next_crypto->kex_methods[SSH_CRYPT_S_C]);

    /* compression */
    if (strcmp(client_session->next_crypto->kex_methods[SSH_COMP_C_S].cstr(), "zlib") == 0) {
        client_session->next_crypto->do_compress_out = 1;
    }
    if (strcmp(client_session->next_crypto->kex_methods[SSH_COMP_S_C].cstr(), "zlib") == 0) {
        client_session->next_crypto->do_compress_in = 1;
    }
    if (strcmp(client_session->next_crypto->kex_methods[SSH_COMP_C_S].cstr(), "zlib@openssh.com") == 0) {
        client_session->next_crypto->delayed_compress_out = 1;
    }
    if (strcmp(client_session->next_crypto->kex_methods[SSH_COMP_S_C].cstr(), "zlib@openssh.com") == 0) {
        client_session->next_crypto->delayed_compress_in = 1;
    }

    if (client_session->next_crypto->generate_session_keys_client(client_session->session_id, error) < 0) {
        client_session->session_state=SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }

    /* Verify the host's signature. FIXME do it sooner */
    SSHString sig_blob = std::move(client_session->next_crypto->dh_server_signature);

    /* get the server public key */
    ssh_buffer_struct buffer;
    buffer.out_blob(client_session->next_crypto->server_pubkey.data.get(),
                    client_session->next_crypto->server_pubkey.size);

    ssh_key_struct *key;
    if (ssh_pki_import_pubkey_blob(buffer, &key) < 0){
        return SSH_ERROR;
    }

    /* check if public key from server matches user preferences */
    // TODO: check that, we should always have some wanted method
    if (client_session->opts.wanted_methods[SSH_HOSTKEYS]) {
        const char *object = key->type_c();
        const char *remaining_list = client_session->opts.wanted_methods[SSH_HOSTKEYS];
        for (;;)
        {
            const char *a = strchr(remaining_list, ',');
            if (a == nullptr) { // last
                if (strcmp(remaining_list, object) != 0) {
                    // definitely not found
                    ssh_set_error(error,
                                  SSH_FATAL,
                                  "Public key from server (%s) doesn't match user "
                                  "preference (%s)",
                                  key->type_c(),
                                  client_session->opts.wanted_methods[SSH_HOSTKEYS]);
                    ssh_key_free(key);
                    return -1;
                }
                break;
            }
            // last
            if (strncmp(remaining_list, object, a - remaining_list) == 0) {
                break; // found
            }
            remaining_list = a + 1;
        }
    }

    rc = ssh_pki_signature_verify_blob(sig_blob,
                                   key,
                                   client_session->next_crypto->secret_hash,
                                   client_session->next_crypto->digest_len,
                                   client_session->error);
    /* Set the server public key type for known host checking */
    client_session->next_crypto->server_pubkey_type = key->type_c();

    ssh_key_free(key);
    memset(sig_blob.data.get(), 'X', sig_blob.size);
    if (rc == SSH_ERROR) {
        client_session->session_state=SSH_SESSION_STATE_ERROR;
        return SSH_PACKET_USED;
    }
    syslog(LOG_INFO,"Signature verified and valid");

    /*
     * Once we got SSH_MSG_NEWKEYS we can switch next_crypto and
     * current_crypto
     */
    if (client_session->current_crypto) {
        delete client_session->current_crypto;
        client_session->current_crypto = nullptr;
    }

    /* FIXME later, include a function to change keys */
    client_session->current_crypto = client_session->next_crypto;
    client_session->next_crypto = new ssh_crypto_struct();

    client_session->dh_handshake_state = DH_STATE_FINISHED;
    ssh_connection_callback_client(client_session, error);
    return SSH_PACKET_USED;
}

inline int ssh_auth_reply_denied_client(SshClientSession * client_session)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    char methods[256] = {0};
    uint32_t auth_methods_flags =
        (client_session->auth_methods == 0)?(SSH_AUTH_METHOD_PUBLICKEY|SSH_AUTH_METHOD_PASSWORD)
        :
        client_session->auth_methods & (
          SSH_AUTH_METHOD_PUBLICKEY
        | SSH_AUTH_METHOD_GSSAPI_MIC
        | SSH_AUTH_METHOD_INTERACTIVE
        | SSH_AUTH_METHOD_PASSWORD
        | SSH_AUTH_METHOD_HOSTBASED
        );

    bool coma = false;
    if (auth_methods_flags & SSH_AUTH_METHOD_PUBLICKEY){
        strcat(methods, "publickey");
        coma = true;
    }
    if (auth_methods_flags & SSH_AUTH_METHOD_GSSAPI_MIC){
        if (coma) { strcat(methods, ","); }
        strcat(methods, "gssapi-with-mic");
        coma = true;
    }
    if (auth_methods_flags & SSH_AUTH_METHOD_INTERACTIVE){
        if (coma) { strcat(methods, ","); }
        strcat(methods, "keyboard-interactive");
        coma = true;
    }
    if (auth_methods_flags & SSH_AUTH_METHOD_PASSWORD){
        if (coma) { strcat(methods, ","); }
        strcat(methods, "password");
        coma = true;
    }
    if (auth_methods_flags & SSH_AUTH_METHOD_HOSTBASED){
        if (coma) { strcat(methods, ","); }
        strcat(methods, "hostbased");
    }

    syslog(LOG_INFO, "Sending a auth failure. methods that can continue: %s", methods);
    client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_FAILURE);
    client_session->out_buffer->out_length_prefixed_cstr(methods);
    client_session->out_buffer->out_uint8(false);
    client_session->packet_send();

    return SSH_OK;
}



/** @internal
 * @brief handles an user authentication using GSSAPI
 */
static inline int ssh_gssapi_handle_userauth_client(SshClientSession * client_session, const char *user, uint32_t n_oid, SSHString * oids[]){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    char service_name[]="host";
    gss_buffer_desc name_buf;
    gss_name_t server_name; /* local server fqdn */
    OM_uint32 min_stat;
    unsigned int i;
    gss_OID_set supported; /* oids supported by server */
    gss_OID_set both_supported; /* oids supported by both client and server */
    gss_OID_set selected; /* oid selected for authentication */
    int present=0;
    int oid_count=0;
    struct gss_OID_desc_struct oid;

    gss_create_empty_oid_set(&min_stat, &both_supported);

    gss_indicate_mechs(&min_stat, &supported);
    for (i=0; i < supported->count; ++i){
        // TODO: avoid too long buffers, we can make this one static and truncate it
        char *hexa = new char[supported->elements[i].length * 3 + 1];
        size_t q = 0;
        size_t j = 0;
        for (q = 0; q < supported->elements[q].length; q++) {
            const uint8_t cl = reinterpret_cast<uint8_t *>(supported->elements[q].elements)[q] >> 4;
            const uint8_t ch = reinterpret_cast<uint8_t *>(supported->elements[q].elements)[q] & 0x0F;
            hexa[j] = (ch < 10?'0':'a')+ch;
            hexa[j+1] = (cl < 10?'0':'a')+cl;
            hexa[j+2] = ':';
            j+= 3;
        }
        hexa[j>0?(j-1):0] = 0;
        syslog(LOG_DEBUG, "Supported mech %d: %s\n", i, hexa);
        delete[] hexa;
    }

    for (i=0 ; i< n_oid ; ++i){
        syslog(LOG_INFO,"GSSAPI: i=%u n_oid=%u", i, n_oid);
        unsigned char *oid_s = oids[i]->data.get();
        size_t len = oids[i]->size;
        syslog(LOG_INFO,"GSSAPI: oid_len=%d %u %u %u", static_cast<int>(len), SSH_OID_TAG, oid_s[0], oid_s[1]);
        if(len < 2){
            syslog(LOG_WARNING,"GSSAPI: received invalid OID 1");
            continue;
        }
        if(oid_s[0] != SSH_OID_TAG){
            syslog(LOG_WARNING,"GSSAPI: received invalid OID 2");
            continue;
        }
        if(static_cast<size_t>(oid_s[1]) != len - 2){
            syslog(LOG_WARNING,"GSSAPI: received invalid OID 3");
            continue;
        }
        oid.elements = &oid_s[2];
        oid.length = len - 2;
        gss_test_oid_set_member(&min_stat,&oid,supported,&present);
        if(present){
            gss_add_oid_set_member(&min_stat,&oid,&both_supported);
            oid_count++;
        }
    }
    syslog(LOG_INFO,"GSSAPI: n_oid loop done i=%u", i);

    gss_release_oid_set(&min_stat, &supported);
    if (oid_count == 0){
        syslog(LOG_INFO,"GSSAPI: no OID match");
        ssh_auth_reply_denied_client(client_session);
        gss_release_oid_set(&min_stat, &both_supported);
        return SSH_OK;
    }
    /* from now we have room for context */
    if (client_session->gssapi == nullptr){
        client_session->gssapi = new ssh_gssapi_struct;
    }

    name_buf.value = service_name;
    name_buf.length = strlen(static_cast<const char*>(name_buf.value)) + 1;
    OM_uint32 maj_stat2 = gss_import_name(&min_stat, &name_buf, GSS_C_NT_HOSTBASED_SERVICE, &server_name);
    if (maj_stat2 != GSS_S_COMPLETE) {
        syslog(LOG_WARNING, "importing name %d, %d", maj_stat2, min_stat);
        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, maj_stat2, GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_WARNING, "GSSAPI(%s): %s", "importing name", static_cast<char *>(buffer.value));
        return -1;
    }

    OM_uint32 maj_stat3 = gss_acquire_cred(&min_stat, server_name, 0,
            both_supported, GSS_C_ACCEPT,
            &client_session->gssapi->server_creds, &selected, nullptr);
    gss_release_name(&min_stat, &server_name);
    gss_release_oid_set(&min_stat, &both_supported);

    if (maj_stat3 != GSS_S_COMPLETE) {
        syslog(LOG_WARNING, "error acquiring credentials %d, %d", maj_stat3, min_stat);
        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, maj_stat3, GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_WARNING, "GSSAPI(%s): %s", "acquiring creds", static_cast<char *>(buffer.value));


        ssh_auth_reply_denied_client(client_session);
        return SSH_ERROR;
    }

    syslog(LOG_INFO, "acquiring credentials %d, %d", maj_stat3, min_stat);

    /* finding which OID from client we selected */
    for (i=0 ; i< n_oid ; ++i){
        unsigned char *oid_s =oids[i]->data.get();
        size_t len = oids[i]->size;
        if (len < 2){
            syslog(LOG_WARNING,"GSSAPI: received invalid OID 1");
            continue;
        }
        if (oid_s[0] != SSH_OID_TAG){
            syslog(LOG_WARNING,"GSSAPI: received invalid OID 2");
            continue;
        }
        if (oid_s[1] != len - 2){
            syslog(LOG_WARNING,"GSSAPI: received invalid OID 3");
            continue;
        }

        oid.elements = &oid_s[2];
        oid.length = len - 2;
        gss_test_oid_set_member(&min_stat, &oid, selected, &present);
        if(present){
            syslog(LOG_INFO, "Selected oid %d", i);
            break;
        }
    }
    client_session->gssapi->mech.length = oid.length;
    client_session->gssapi->mech.elements = malloc(oid.length);

    // TODO : check memory allocation
    memcpy(client_session->gssapi->mech.elements, oid.elements, oid.length);
    gss_release_oid_set(&min_stat, &selected);
    client_session->gssapi->user = strdup(user);
    client_session->gssapi->service = service_name;
    client_session->gssapi->state = SSH_GSSAPI_STATE_RCV_TOKEN;

    client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_GSSAPI_RESPONSE);
    client_session->out_buffer->out_uint32_be(oids[i]->size);
    client_session->out_buffer->out_blob(oids[i]->data.get(), oids[i]->size);
    client_session->packet_send();

    return SSH_OK;
}


/** @internal
 * @frees a gssapi context
 */
static void ssh_gssapi_free_client(SshClientSession * client_session){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    OM_uint32 min;
    if (client_session->gssapi == nullptr)
        return;
    free(client_session->gssapi->user);
    client_session->gssapi->user = nullptr;
    free(client_session->gssapi->mech.elements);
    client_session->gssapi->mech.elements = nullptr;
    gss_release_cred(&min,&client_session->gssapi->server_creds);
    if (client_session->gssapi->client.creds !=
                    client_session->gssapi->client.client_deleg_creds) {
        gss_release_cred(&min, &client_session->gssapi->client.creds);
    }
    free(client_session->gssapi);
    client_session->gssapi = nullptr;
}

static inline ssh_buffer_struct* ssh_gssapi_build_mic_client(SshClientSession * client_session, ssh_buffer_struct * mic_buffer){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    SSHString str(static_cast<uint32_t>(client_session->current_crypto->digest_len));
    memcpy(str.data.get(), client_session->session_id, client_session->current_crypto->digest_len);

    mic_buffer->out_uint32_be(str.size);
    mic_buffer->out_blob(str.data.get(), str.size);

    mic_buffer->out_uint8(SSH_MSG_USERAUTH_REQUEST);
    mic_buffer->out_length_prefixed_cstr(client_session->gssapi->user);
    mic_buffer->out_length_prefixed_cstr("ssh-connection");
    mic_buffer->out_length_prefixed_cstr("gssapi-with-mic");

    return mic_buffer;
}

// SSH_PACKET_CALLBACK(ssh_packet_userauth_gssapi_mic)
static inline int ssh_packet_userauth_gssapi_mic_client(SshClientSession * client_session, uint8_t type, ssh_buffer_struct* packet, void *user)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    OM_uint32 maj_stat, min_stat;
    gss_buffer_desc mic_buf = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc mic_token_buf = GSS_C_EMPTY_BUFFER;

    (void)user;
    (void)type;
    ssh_buffer_struct * mic_buffer = nullptr;

    syslog(LOG_INFO,"Received SSH_MSG_USERAUTH_GSSAPI_MIC");

    if (sizeof(uint32_t) > packet->in_remain()) {
        ssh_auth_reply_denied_client(client_session);
        ssh_gssapi_free_client(client_session);
        if (mic_buffer != nullptr) {
            delete mic_buffer;
        }
        return SSH_PACKET_USED;
    }
    uint32_t mic_token_len = packet->in_uint32_be();
    if (mic_token_len > packet->in_remain()) {
        ssh_auth_reply_denied_client(client_session);
        ssh_gssapi_free_client(client_session);
        if (mic_buffer != nullptr) {
            delete mic_buffer;
        }
        return SSH_PACKET_USED;
    }
    SSHString mic_token(mic_token_len);
    packet->buffer_get_data(mic_token.data.get(),mic_token_len);


    if (client_session->gssapi == nullptr
    || client_session->gssapi->state != SSH_GSSAPI_STATE_RCV_MIC) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Received SSH_MSG_USERAUTH_GSSAPI_MIC in invalid state");
        ssh_auth_reply_denied_client(client_session);
        ssh_gssapi_free_client(client_session);
        if (mic_buffer != nullptr) {
            delete mic_buffer;
        }
        return SSH_PACKET_USED;
    }
    mic_buffer = new ssh_buffer_struct;
    if (ssh_gssapi_build_mic_client(client_session, mic_buffer) == nullptr){
        // TODO : check memory allocation
        ssh_auth_reply_denied_client(client_session);
        ssh_gssapi_free_client(client_session);
        if (mic_buffer != nullptr) {
            delete mic_buffer;
        }
        return SSH_PACKET_USED;
    }
    mic_buf.length = mic_buffer->in_remain();
    mic_buf.value = mic_buffer->get_pos_ptr();
    mic_token_buf.length = mic_token.size;
    mic_token_buf.value = mic_token.data.get();

    maj_stat = gss_verify_mic(&min_stat, client_session->gssapi->ctx, &mic_buf, &mic_token_buf, nullptr);

    {
        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, maj_stat, GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_INFO, "GSSAPI(%s): %s", "verifying MIC", static_cast<char *>(buffer.value));
    }

    {
        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, min_stat, GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_INFO, "GSSAPI(%s): %s", "verifying MIC (min stat)", static_cast<char *>(buffer.value));
    }


    if (maj_stat == GSS_S_DEFECTIVE_TOKEN || GSS_ERROR(maj_stat)) {
        ssh_auth_reply_denied_client(client_session);
        ssh_gssapi_free_client(client_session);
        if (mic_buffer != nullptr) {
            delete mic_buffer;
        }
        return SSH_PACKET_USED;
    }

    ssh_gssapi_free_client(client_session);
    if (mic_buffer != nullptr) {
        delete mic_buffer;
    }
    return SSH_PACKET_USED;
}


static int ssh_gssapi_send_auth_mic_client(SshClientSession * client_session, SSHString *oid_set[], int n_oid){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_REQUEST);
    client_session->out_buffer->out_length_prefixed_cstr(client_session->opts.username); /* username */
    client_session->out_buffer->out_length_prefixed_cstr("ssh-connection");  /* service */
    client_session->out_buffer->out_length_prefixed_cstr("gssapi-with-mic"); /* method */
    client_session->out_buffer->out_uint32_be(n_oid);

    int i = 0;
    for (;i < n_oid; ++i){
        client_session->out_buffer->out_uint32_be(oid_set[i]->size);
        client_session->out_buffer->out_blob(oid_set[i]->data.get(), oid_set[i]->size);
    }

    client_session->auth_state = SSH_AUTH_STATE_GSSAPI_REQUEST_SENT;
    client_session->packet_send();
    return SSH_OK;
}

static int ssh_gssapi_send_mic_client(SshClientSession * client_session){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    OM_uint32 maj_stat, min_stat;
    gss_buffer_desc mic_buf = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc mic_token_buf = GSS_C_EMPTY_BUFFER;

    syslog(LOG_INFO,"Sending SSH_MSG_USERAUTH_GSSAPI_MIC");

    ssh_buffer_struct * mic_buffer = new ssh_buffer_struct;
    if (ssh_gssapi_build_mic_client(client_session, mic_buffer) == nullptr){
        // TODO : check memory allocation
        delete mic_buffer;
        return SSH_ERROR;
    }
    mic_buf.length = mic_buffer->in_remain();
    mic_buf.value = mic_buffer->get_pos_ptr();

    maj_stat = gss_get_mic(&min_stat,client_session->gssapi->ctx, GSS_C_QOP_DEFAULT, &mic_buf, &mic_token_buf);
    if (GSS_ERROR(maj_stat)){
        delete mic_buffer;

        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_WARNING, "GSSAPI(%s): %s", "generating MIC", static_cast<char *>(buffer.value));

        return SSH_ERROR;
    }

    client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_GSSAPI_MIC);

    client_session->out_buffer->out_uint32_be(mic_token_buf.length);

    client_session->out_buffer->out_blob(reinterpret_cast<const uint8_t *>(mic_token_buf.value), mic_token_buf.length);
    delete mic_buffer;

    client_session->packet_send();
    return SSH_OK;
}


// SSH_PACKET_CALLBACK(ssh_packet_userauth_gssapi_token_client)
static inline int ssh_packet_userauth_gssapi_token_client(SshClientSession * client_session, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    OM_uint32 min_stat;
    gss_buffer_desc input_token;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    syslog(LOG_INFO,"Received SSH_MSG_USERAUTH_GSSAPI_TOKEN");
    if (!client_session->gssapi || client_session->auth_state != SSH_AUTH_STATE_GSSAPI_TOKEN){
        ssh_set_error(client_session->error,  SSH_FATAL, "Received SSH_MSG_USERAUTH_GSSAPI_TOKEN in invalid state");
        return SSH_PACKET_USED;
    }

    if (sizeof(uint32_t) > packet->in_remain()) {
        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, 0, GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_WARNING, "GSSAPI(%s): %s", "Gssapi error", static_cast<char *>(buffer.value));

        ssh_gssapi_free_client(client_session);
        client_session->gssapi = nullptr;
        return SSH_PACKET_USED;
    }
    uint32_t token_len = packet->in_uint32_be();
    if (token_len > packet->in_remain()) {
        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, 0, GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_WARNING, "GSSAPI(%s): %s", "Gssapi error", static_cast<char *>(buffer.value));
        ssh_gssapi_free_client(client_session);
        client_session->gssapi = nullptr;
        return SSH_PACKET_USED;
    }

    SSHString token(token_len);
    packet->buffer_get_data(token.data.get(),token_len);

    // TODO: avoid too long buffers, we can make this one static and truncate it
    char *hexa = new char[token_len * 3 + 1];
    size_t q = 0;
    size_t j = 0;
    for (q = 0; q < token_len; q++) {
        const uint8_t cl = token.data.get()[q] >> 4;
        const uint8_t ch = token.data.get()[q] & 0x0F;
        hexa[j] = (ch < 10?'0':'a')+ch;
        hexa[j+1] = (cl < 10?'0':'a')+cl;
        hexa[j+2] = ':';
        j+= 3;
    }
    hexa[j>0?(j-1):0] = 0;
    syslog(LOG_INFO, "GSSAPI Token : %s",hexa);
    delete[] hexa;

    input_token.length = token.size;
    input_token.value = token.data.get();
    OM_uint32 maj_stat1 = gss_init_sec_context(&min_stat,
                                    client_session->gssapi->client.creds,
                                    &client_session->gssapi->ctx,
                                    client_session->gssapi->client.server_name,
                                    client_session->gssapi->client.oid,
                                    client_session->gssapi->client.flags,
                                    0, nullptr, &input_token, nullptr,
                                    &output_token, nullptr, nullptr);

    gss_buffer_desc buffer;
    OM_uint32 dummy;
    OM_uint32 message_context;
    gss_display_status(&dummy, 0, GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
    syslog(LOG_WARNING, "GSSAPI(%s): %s", "accepting token", static_cast<char *>(buffer.value));

    if (GSS_ERROR(maj_stat1)){
        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, maj_stat1,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_INFO, "GSSAPI(%s): %s", "Gssapi error", static_cast<char *>(buffer.value));

        ssh_gssapi_free_client(client_session);
        client_session->gssapi=nullptr;
        return SSH_PACKET_USED;
    }

    if (output_token.length != 0){
        char *hexa = new char[output_token.length * 3 + 1];
        size_t q = 0;
        size_t j = 0;
        for (q = 0; q < output_token.length; q++) {
            const uint8_t cl = reinterpret_cast<uint8_t *>(output_token.value)[q] >> 4;
            const uint8_t ch = reinterpret_cast<uint8_t *>(output_token.value)[q] & 0x0F;
            hexa[j] = (ch < 10?'0':'a')+ch;
            hexa[j+1] = (cl < 10?'0':'a')+cl;
            hexa[j+2] = ':';
            j+= 3;
        }
        hexa[j>0?(j-1):0] = 0;
        syslog(LOG_INFO, "GSSAPI: sending token %s",hexa);
        delete[] hexa;

        SSHString token2(static_cast<uint32_t>(output_token.length));
        memcpy(token2.data.get(), output_token.value, output_token.length);
        client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_GSSAPI_TOKEN);
        client_session->out_buffer->out_uint32_be(token2.size);
        client_session->out_buffer->out_blob(token2.data.get(), token2.size);
        client_session->packet_send();
    }
    if(maj_stat1 == GSS_S_COMPLETE){
        client_session->auth_state = SSH_AUTH_STATE_NONE;
        ssh_gssapi_send_mic_client(client_session);
    }
    return SSH_PACKET_USED;
}


/**
 * @internal
 *
 * @brief Handle a SSH_MSG_MSG_USERAUTH_INFO_RESPONSE packet and queue a
 * SSH Message
 */

//SSH_PACKET_CALLBACK(ssh_packet_userauth_info_response)
static inline int ssh_packet_userauth_info_response_client(SshClientSession * client_session, ssh_buffer_struct* packet)
{
    // SSH_REQUEST_AUTH SSH_AUTH_METHOD_INTERACTIVE
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t i;

//    int kbdint_response = 1;
    uint32_t nanswers = packet->in_uint32_be();

    if (client_session->kbdint == nullptr) {
        syslog(LOG_INFO, "Warning: Got a keyboard-interactive "
                            "response but it seems we didn't send the request.");

        client_session->kbdint = new ssh_kbdint_struct;
    }

    syslog(LOG_INFO,"kbdint: %d answers", nanswers);
    if (nanswers > KBDINT_MAX_PROMPT) {
        ssh_set_error(client_session->error,  SSH_FATAL,
            "Too many answers received from client: %u (0x%.4x)",
            nanswers, nanswers);
        ssh_kbdint_free(client_session->kbdint);
        client_session->kbdint = nullptr;
        return SSH_PACKET_USED;
    }

    if(nanswers != client_session->kbdint->nprompts) {
        /* warn but let the application handle this case */
        syslog(LOG_INFO, "Warning: Number of prompts and answers"
                    " mismatch: p=%u a=%u", client_session->kbdint->nprompts, nanswers);
    }
    client_session->kbdint->nanswers = nanswers;
    client_session->kbdint->answers = static_cast<char**>(malloc(nanswers * sizeof(char *)));
    memset(client_session->kbdint->answers, 0, nanswers * sizeof(char *));

    for (i = 0; i < nanswers; i++) {
        client_session->kbdint->answers[i] = packet->in_strdup_cstr();
    }

    return SSH_PACKET_USED;
}


/**
 * @internal
 *
 * @brief Handles a SSH_USERAUTH_BANNER packet.
 *
 * This banner should be shown to user prior to authentication
 */
// TODO: we should have some callback to notify the server banner has been recieved
static inline int ssh_packet_userauth_banner_client(SshClientSession * client_session, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_DEBUG, "Received SSH_USERAUTH_BANNER packet");
    if (sizeof(uint32_t) > packet->in_remain()) {
        // ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    uint32_t banner_len = packet->in_uint32_be();
    if (banner_len > packet->in_remain()) {
        // ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    SSHString banner(banner_len);

    packet->buffer_get_data(banner.data.get(),banner_len);
    client_session->banner = std::move(banner);

  return SSH_PACKET_USED;
}


//    8.  Diffie-Hellman Key Exchange

//       The Diffie-Hellman (DH) key exchange provides a shared secret that
//       can not be determined by either party alone.  The key exchange is
//       combined with a signature with the host key to provide host
//       authentication.  This key exchange method provides explicit server
//       authentication as is defined in Section 7.

//       In the following description (C is the client, S is the server; p is
//       a large safe prime, g is a generator for a subgroup of GF(p), and q
//       is the order of the subgroup; V_S is S's version string; V_C is C's
//       version string; K_S is S's public host key; I_C is C's KEXINIT
//       message and I_S S's KEXINIT message which have been exchanged before
//       this part begins):

//       1.  C generates a random number x (1 < x < q) and computes e = g^x
//           mod p.  C sends "e" to S.
//       2.  S generates a random number y (0 < y < q) and computes f = g^y
//           mod p.  S receives "e".  It computes K = e^y mod p, H = hash(V_C
//           || V_S || I_C || I_S || K_S || e || f || K) (these elements are
//           encoded according to their types; see below), and signature s on
//           H with its private host key.  S sends "K_S || f || s" to C.  The
//           signing operation may involve a second hashing operation.
//       3.  C verifies that K_S really is the host key for S (e.g., using
//           certificates or a local database).  C is also allowed to accept
//           the key without verification; however, doing so will render the
//           protocol insecure against active attacks (but may be desirable
//           for practical reasons in the short term in many environments).  C
//           then computes K = f^x mod p, H = hash(V_C || V_S || I_C || I_S ||
//           K_S || e || f || K), and verifies the signature s on H.

//       Either side MUST NOT send or accept e or f values that are not in the
//       range [1, p-1].  If this condition is violated, the key exchange
//       fails.

//       This is implemented with the following messages.  The hash algorithm
//       for computing the exchange hash is defined by the method name, and is
//       called HASH.  The public key algorithm for signing is negotiated with
//       the KEXINIT messages.

//       First, the client sends the following:

//         byte      SSH_MSG_KEXDH_INIT
//         mpint     e


//       The server responds with the following:

//         byte      SSH_MSG_KEXDH_REPLY
//         string    server public host key and certificates (K_S)
//         mpint     f
//         string    signature of H

//       The hash H is computed as the HASH hash of the concatenation of the
//       following:

//         string    V_C, the client's version string (CR and NL excluded)
//         string    V_S, the server's version string (CR and NL excluded)
//         string    I_C, the payload of the client's SSH_MSG_KEXINIT
//         string    I_S, the payload of the server's SSH_MSG_KEXINIT
//         string    K_S, the host key
//         mpint     e, exchange value sent by the client
//         mpint     f, exchange value sent by the server
//         mpint     K, the shared secret

//       This value is called the exchange hash, and it is used to
//       authenticate the key exchange.  The exchange hash SHOULD be kept
//       secret.


//       The signature algorithm MUST be applied over H, not the original
//       data.  Most signature algorithms include hashing and additional
//       padding - for example, "ssh-dss" specifies SHA-1 hashing.  In that
//       case, the data is first hashed with HASH to compute H, and H is then
//       hashed with SHA-1 as part of the signing operation.


/**
 * @internal
 *
 * @brief Handle a SSH_CHANNEL_OPEN_FAILURE and set the state of the channel.
 */
// SSH_PACKET_CALLBACK(ssh_packet_channel_open_fail)
static inline int ssh_packet_channel_open_fail_client(SshClientSession * client_session, ssh_channel_struct * channel, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t code =  packet->in_uint32_be();

    if (sizeof(uint32_t) > packet->in_remain()) {
        // TODO: malformed, should be an error
        channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN_DENIED;
        return SSH_PACKET_USED;
    }
    uint32_t error_s_len = packet->in_uint32_be();
    if (error_s_len > packet->in_remain()) {
        // TODO: malformed, should be an error
        channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN_DENIED;
        return SSH_PACKET_USED;
    }
    SSHString error_s(error_s_len);
    packet->buffer_get_data(error_s.data.get(),error_s_len);

    ssh_set_error(client_session->error,  SSH_REQUEST_DENIED,
                  "Channel opening failure: channel %u error (%lu) %*s",
                  channel->local_channel,
                  static_cast<long unsigned int>(ntohl(code)),
                  error_s.size, error_s.data.get());
    channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN_DENIED;
    return SSH_PACKET_USED;
}


/* decompression */
static z_stream *initdecompress_client(SshClientSession * client_session)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int status;

    z_stream *stream = static_cast<z_stream_s*>(malloc(sizeof(z_stream)));
    if (stream == nullptr) {
        return nullptr;
    }
    memset(stream,0,sizeof(z_stream));

    status = inflateInit(stream);
    if (status != Z_OK) {
        free(stream);
        stream = nullptr;
        ssh_set_error(client_session->error,  SSH_FATAL,
            "Status = %d initiating inflate context!", status);
        return nullptr;
    }
    return stream;
}

static ssh_buffer_struct* gzip_decompress_client(SshClientSession * client_session, ssh_buffer_struct* source, size_t maxlen)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    z_stream *zin = static_cast<z_stream_s*>(client_session->current_crypto->compress_in_ctx);
    void *in_ptr = source->get_pos_ptr();
    unsigned long in_size = source->in_remain();
    unsigned char out_buf[GZIP_BLOCKSIZE] = {0};
    ssh_buffer_struct* dest = nullptr;
    unsigned long len;
    int status;

    if (zin == nullptr) {
        client_session->current_crypto->compress_in_ctx = initdecompress_client(client_session);
        zin = static_cast<z_stream_s*>(client_session->current_crypto->compress_in_ctx);
        if (zin == nullptr) {
          return nullptr;
        }
    }

    dest = new ssh_buffer_struct;
    if (dest == nullptr) {
        return nullptr;
    }

    zin->next_out = out_buf;
    zin->next_in = static_cast<uint8_t*>(in_ptr);
    zin->avail_in = in_size;

    do {
        zin->avail_out = GZIP_BLOCKSIZE;
        status = inflate(zin, Z_PARTIAL_FLUSH);
        if (status != Z_OK && status != Z_BUF_ERROR) {
            ssh_set_error(client_session->error,  SSH_FATAL,
              "status %d inflating zlib packet", status);
            delete dest;
            return nullptr;
        }

        len = GZIP_BLOCKSIZE - zin->avail_out;
        dest->out_blob(out_buf,len);
        if (dest->in_remain() > maxlen){
            /* Size of packet exceeded, avoid a denial of service attack */
            delete dest;
            return nullptr;
        }
        zin->next_out = out_buf;
    } while (zin->avail_out == 0);

    return dest;
}


static int decompress_buffer_client(SshClientSession * client_session,ssh_buffer_struct* buf, size_t maxlen)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    ssh_buffer_struct* dest = gzip_decompress_client(client_session,buf, maxlen);
    if (dest == nullptr) {
        return -1;
    }

    if (buf->buffer_reinit() < 0) {
        delete dest;
        return -1;
    }

    buf->out_blob(dest->get_pos_ptr(), dest->in_remain());

    delete dest;
    return 0;
}




/**
 * @internal
 *
 * @brief Send a global request (needed for forward listening) and wait for the
 * result.
 *
 * @param[in]  client_session  The SSH session handle.
 *
 * @param[in]  request  The type of request (defined in RFC).
 *
 * @param[in]  buffer   Additional data to put in packet.
 *
 * @param[in]  reply    Set if you expect a reply from server.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 */
static inline int global_request_client(SshClientSession * client_session, const char *request, ssh_buffer_struct* buffer, int reply) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO, "> GLOBAL_REQUEST %s", request);

    if (client_session->global_req_state == SSH_CHANNEL_REQ_STATE_NONE){
        client_session->out_buffer->out_uint8(SSH_MSG_GLOBAL_REQUEST);
        client_session->out_buffer->out_length_prefixed_cstr(request);
        client_session->out_buffer->out_uint8(reply != 0);

        if (buffer != nullptr) {
            client_session->out_buffer->out_blob(buffer->get_pos_ptr(), buffer->in_remain());
        }

        client_session->global_req_state = SSH_CHANNEL_REQ_STATE_PENDING;
        client_session->packet_send();

        syslog(LOG_INFO,
                "Sent a SSH_MSG_GLOBAL_REQUEST %s", request);

        if (reply == 0) {
            client_session->global_req_state = SSH_CHANNEL_REQ_STATE_NONE;

            return SSH_OK;
        }
    }

    if (client_session->session_state == SSH_SESSION_STATE_ERROR){
        client_session->global_req_state = SSH_CHANNEL_REQ_STATE_NONE;
        return SSH_ERROR;
    }

    while(1){
        if (client_session->global_req_state != SSH_CHANNEL_REQ_STATE_PENDING){
            break;
        }

        // Waiting for input
        dopoll(client_session->ctx, (client_session->flags&SSH_SESSION_FLAG_BLOCKING)?SSH_TIMEOUT_INFINITE:SSH_TIMEOUT_NONBLOCKING);

        if (client_session->session_state == SSH_SESSION_STATE_ERROR){
            client_session->global_req_state = SSH_CHANNEL_REQ_STATE_NONE;
            return SSH_ERROR;
        }

        if (!(client_session->flags&SSH_SESSION_FLAG_BLOCKING)){
            break;
        }
    }

    int rc = SSH_OK;
    switch(client_session->global_req_state){
    case SSH_CHANNEL_REQ_STATE_ACCEPTED:
        syslog(LOG_INFO, "Global request %s success",request);
        rc = SSH_OK;
        break;
    case SSH_CHANNEL_REQ_STATE_DENIED:
        syslog(LOG_INFO,
                "Global request %s failed", request);
        ssh_set_error(client_session->error, SSH_REQUEST_DENIED,
                      "Global request %s failed", request);
        rc = SSH_ERROR;
        break;
    case SSH_CHANNEL_REQ_STATE_ERROR:
    case SSH_CHANNEL_REQ_STATE_NONE:
        rc = SSH_ERROR;
        break;
    case SSH_CHANNEL_REQ_STATE_PENDING:
        return SSH_AGAIN;
    }
    client_session->global_req_state = SSH_CHANNEL_REQ_STATE_NONE;

    return rc;
}


/**
 * @brief Sends the "tcpip-forward" global request to ask the server to begin
 *        listening for inbound connections.
 *
 * @param[in]  client_session  The ssh session to send the request.
 *
 * @param[in]  address  The address to bind to on the server. Pass nullptr to bind
 *                      to all available addresses on all protocol families
 *                      supported by the server.
 *
 * @param[in]  port     The port to bind to on the server. Pass 0 to ask the
 *                      server to allocate the next available unprivileged port
 *                      number
 *
 * @param[in]  bound_port The pointer to get actual bound port. Pass nullptr to
 *                        ignore.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 **/
inline int ssh_forward_listen_client(SshClientSession * client_session, const char *address, int port, int *bound_port)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    // TODO: check how and when this function is called to ensure we are actually in NONE
    // state when calling it. See how to manage non blocking (in global request)
    if(client_session->global_req_state != SSH_CHANNEL_REQ_STATE_NONE){
        return SSH_ERROR;
    }

    ssh_buffer_struct* buffer = new ssh_buffer_struct;
    buffer->out_length_prefixed_cstr(address ? address : "");
    buffer->out_uint32_be(port);

    int rc = global_request_client(client_session, "tcpip-forward", buffer, 1);

    /* TODO: FIXME no guarantee the last packet we received contains
     * that info */
    if (rc == SSH_OK && port == 0 && bound_port) {
        *bound_port = client_session->in_buffer->in_uint32_be();
    }

    delete buffer;
    return rc;
}

/**
 * @brief Sends the "cancel-tcpip-forward" global request to ask the server to
 *        cancel the tcpip-forward request.
 *
 * @param[in]  client_session  The ssh session to send the request.
 *
 * @param[in]  address  The bound address on the server.
 *
 * @param[in]  port     The bound port on the server.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 */
inline int ssh_forward_cancel_client(SshClientSession * client_session, const char *address, int port)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    // TODO: check how and when this function is called to ensure we are actually in NONE
    // state when calling it. See how to manage non blocking (in global request)
    if(client_session->global_req_state != SSH_CHANNEL_REQ_STATE_NONE){
        return SSH_ERROR;
    }

    ssh_buffer_struct* buffer = new ssh_buffer_struct;
    buffer->out_length_prefixed_cstr(address ? address : "");
    buffer->out_uint32_be(port);

    int rc = global_request_client(client_session, "cancel-tcpip-forward", buffer, 1);

    delete buffer;
    return rc;
}


/**
 * @internal
 * @brief handles a SSH_USERAUTH_INFO_REQUEST packet, as used in
 *        keyboard-interactive authentication, and changes the
 *        authentication state.
 */
 // SSH_PACKET_CALLBACK(ssh_packet_userauth_info_request)
static inline int ssh_packet_userauth_info_request_client(SshClientSession * client_session, uint8_t type, ssh_buffer_struct* packet, void *user)
{
  syslog(LOG_INFO, "%s ---", __FUNCTION__);
  uint32_t i;
  (void)user;
  (void)type;


    /* name of the "asking" window showed to client */
    if (sizeof(uint32_t) > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
        return SSH_PACKET_USED;
    }
    uint32_t name_len = packet->in_uint32_be();
    if (name_len > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
        return SSH_PACKET_USED;
    }
    SSHString name(name_len);
    packet->buffer_get_data(name.data.get(), name_len);

    if (sizeof(uint32_t) > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
        return SSH_PACKET_USED;
    }
    uint32_t instruction_len = packet->in_uint32_be();
    if (instruction_len > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
        return SSH_PACKET_USED;
    }
    SSHString instruction(instruction_len);
    packet->buffer_get_data(instruction.data.get(), instruction_len);

    if (sizeof(uint32_t) > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
        return SSH_PACKET_USED;
    }
    uint32_t tmp_len = packet->in_uint32_be();
    if (tmp_len > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
        return SSH_PACKET_USED;
    }
    SSHString tmp(tmp_len);
    packet->buffer_get_data(tmp.data.get(), tmp_len);

    uint32_t nprompts = packet->in_uint32_be();

    if (client_session->kbdint == nullptr) {
        client_session->kbdint = new ssh_kbdint_struct();
    }
    else {
        ssh_kbdint_clean(client_session->kbdint);
    }

    client_session->kbdint->name = new char [name.size + 1];
    memcpy(client_session->kbdint->name, name.data.get(), name.size);
    client_session->kbdint->name[name.size] = 0;

    client_session->kbdint->instruction = new char[instruction.size + 1];
    memcpy(client_session->kbdint->instruction, instruction.data.get(), instruction.size);
    client_session->kbdint->instruction[instruction.size] = 0;

    syslog(LOG_DEBUG, "%d keyboard-interactive prompts", nprompts);
    if (nprompts > KBDINT_MAX_PROMPT) {
        ssh_set_error(client_session->error,  SSH_FATAL,
            "Too much prompts requested by the server: %u (0x%.4x)",
            nprompts, nprompts);
        ssh_kbdint_free(client_session->kbdint);
        client_session->kbdint = nullptr;

    return SSH_PACKET_USED;
    }

    client_session->kbdint->nprompts = nprompts;
    client_session->kbdint->nanswers = nprompts;
    client_session->kbdint->prompts = static_cast<char**>(malloc(nprompts * sizeof(char *)));
    // TODO : check memory allocation
    memset(client_session->kbdint->prompts, 0, nprompts * sizeof(char *));

    client_session->kbdint->echo = static_cast<unsigned char*>(malloc(nprompts));
    // TODO : check memory allocation
    memset(client_session->kbdint->echo, 0, nprompts);

    for (i = 0; i < nprompts; i++) {
        if (sizeof(uint32_t) > packet->in_remain()) {
            // TODO : check memory allocation
            client_session->kbdint->nprompts = i-1;
            ssh_kbdint_free(client_session->kbdint);
            client_session->kbdint = nullptr;
            return SSH_PACKET_USED;
        }
        uint32_t tmp2_len = packet->in_uint32_be();
        if (tmp2_len > packet->in_remain()) {
           // TODO : check memory allocation
            client_session->kbdint->nprompts = i-1;
            ssh_kbdint_free(client_session->kbdint);
            client_session->kbdint = nullptr;

            return SSH_PACKET_USED;
        }
        SSHString tmp2(tmp2_len);
        packet->buffer_get_data(tmp2.data.get(), tmp2_len);

        client_session->kbdint->echo[i] = packet->in_uint8();

        client_session->kbdint->prompts[i] = new char [tmp2.size + 1];
        // TODO : check memory allocation
        memcpy(client_session->kbdint->prompts[i], tmp2.data.get(), tmp2.size);
        client_session->kbdint->prompts[i][tmp2.size] = 0;

    }
    client_session->auth_state=SSH_AUTH_STATE_INFO;

    return SSH_PACKET_USED;
}


// SSH_PACKET_CALLBACK(ssh_packet_userauth_gssapi_response)
inline int ssh_packet_userauth_gssapi_response_client(SshClientSession * client_session, uint8_t type, ssh_buffer_struct* packet, void *user)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    gss_uint32 maj_stat, min_stat;
    gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    (void)type;
    (void)user;

    syslog(LOG_INFO, "Received SSH_USERAUTH_GSSAPI_RESPONSE");
    if (client_session->auth_state != SSH_AUTH_STATE_GSSAPI_REQUEST_SENT){
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid state in ssh_packet_userauth_gssapi_response");
        return SSH_PACKET_USED;
    }

    if (sizeof(uint32_t) > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "parse error");
        return SSH_PACKET_USED;
    }
    uint32_t oid_s_len = packet->in_uint32_be();
    if (oid_s_len > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "parse error");
        return SSH_PACKET_USED;
    }
    SSHString oid_s(oid_s_len);
    packet->buffer_get_data(oid_s.data.get(),oid_s_len);
    client_session->gssapi->client.oid = ssh_gssapi_oid_from_string(oid_s);

    if (!client_session->gssapi->client.oid) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid OID");
        return SSH_PACKET_USED;
    }

    client_session->gssapi->client.flags = GSS_C_MUTUAL_FLAG | GSS_C_INTEG_FLAG;
    if (client_session->opts.gss_delegate_creds) {
        client_session->gssapi->client.flags |= GSS_C_DELEG_FLAG;
    }

    /* prepare the first TOKEN response */
    maj_stat = gss_init_sec_context(&min_stat,
                                    client_session->gssapi->client.creds,
                                    &client_session->gssapi->ctx,
                                    client_session->gssapi->client.server_name,
                                    client_session->gssapi->client.oid,
                                    client_session->gssapi->client.flags,
                                    0, nullptr, &input_token, nullptr,
                                    &output_token, nullptr, nullptr);
    if(GSS_ERROR(maj_stat)){
        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_WARNING, "GSSAPI(%s): %s", "Initializing gssapi context", static_cast<char *>(buffer.value));

        return SSH_PACKET_USED;
    }
    if (output_token.length != 0){
        // TODO: avoid too long buffers, we can make this one static and truncate it
        char *hexa = new char[output_token.length * 3 + 1];
        size_t q = 0;
        size_t j = 0;
        for (q = 0; q < output_token.length; q++) {
            const uint8_t cl = reinterpret_cast<uint8_t *>(output_token.value)[q] >> 4;
            const uint8_t ch = reinterpret_cast<uint8_t *>(output_token.value)[q] & 0x0F;
            hexa[j] = (ch < 10?'0':'a')+ch;
            hexa[j+1] = (cl < 10?'0':'a')+cl;
            hexa[j+2] = ':';
            j+= 3;
        }
        hexa[j>0?(j-1):0] = 0;
        syslog(LOG_INFO, "GSSAPI: sending token %s",hexa);
        delete[] hexa;

        SSHString token(static_cast<uint32_t>(output_token.length));
        memcpy(token.data.get(), output_token.value, output_token.length);
        client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_GSSAPI_TOKEN);
        client_session->out_buffer->out_uint32_be(token.size);
        client_session->out_buffer->out_blob(token.data.get(), token.size);
        client_session->packet_send();
        client_session->auth_state = SSH_AUTH_STATE_GSSAPI_TOKEN;
    }
    return SSH_PACKET_USED;
}


 // SSH_PACKET_CALLBACK(ssh_packet_userauth_pk_ok)
static inline int ssh_packet_userauth_pk_ok_client(SshClientSession * client_session, uint8_t type, ssh_buffer_struct* packet, void *user)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int rc;

  syslog(LOG_INFO, "Received SSH_USERAUTH_PK_OK/INFO_REQUEST/GSSAPI_RESPONSE");

  if(client_session->auth_state==SSH_AUTH_STATE_KBDINT_SENT){
    /* Assuming we are in keyboard-interactive context */
    syslog(LOG_INFO,
            "keyboard-interactive context, assuming SSH_USERAUTH_INFO_REQUEST");
    rc=ssh_packet_userauth_info_request_client(client_session,type,packet,user);
  } else if (client_session->auth_state == SSH_AUTH_STATE_GSSAPI_REQUEST_SENT){
    rc = ssh_packet_userauth_gssapi_response_client(client_session, type, packet, user);
  } else {
    client_session->auth_state=SSH_AUTH_STATE_PK_OK;
    syslog(LOG_INFO, "Assuming SSH_USERAUTH_PK_OK");
    rc=SSH_PACKET_USED;
  }

  return rc;
}



/**
 * @internal
 *
 * @brief Handles a SSH_USERAUTH_SUCCESS packet.
 *
 * It is also used to communicate the new to the upper levels.
 */
 // SSH_PACKET_CALLBACK(ssh_packet_userauth_success)
static inline int ssh_packet_userauth_success_client(SshClientSession * client_session, uint8_t type, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
  (void)packet;
  (void)type;

  syslog(LOG_DEBUG, "Authentication successful");
  syslog(LOG_INFO, "Received SSH_USERAUTH_SUCCESS");

  client_session->auth_state=SSH_AUTH_STATE_SUCCESS;
  client_session->session_state=SSH_SESSION_STATE_AUTHENTICATED;
  client_session->flags |= SSH_SESSION_FLAG_AUTHENTICATED;

  if(client_session->current_crypto && client_session->current_crypto->delayed_compress_out){
      syslog(LOG_DEBUG, "Enabling delayed compression OUT");
      client_session->current_crypto->do_compress_out=1;
  }
  if(client_session->current_crypto && client_session->current_crypto->delayed_compress_in){
      syslog(LOG_DEBUG, "Enabling delayed compression IN");
      client_session->current_crypto->do_compress_in=1;
  }

  return SSH_PACKET_USED;
}


/**
 * @internal
 *
 * @brief Handles a SSH_USERAUTH_FAILURE packet.
 *
 * This handles the complete or partial authentication failure.
 */
 // SSH_PACKET_CALLBACK(ssh_packet_userauth_failure)
static inline int ssh_packet_userauth_failure_client(SshClientSession * client_session, uint8_t type, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    char *auth_methods = nullptr;
    uint8_t partial = 0;
    (void) type;

    if (sizeof(uint32_t) > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid USERAUTH_FAILURE msg");
        return SSH_PACKET_USED;
    }
    uint32_t auth_len = packet->in_uint32_be();
    if (auth_len > packet->in_remain()) {
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid USERAUTH_FAILURE msg");
        return SSH_PACKET_USED;
    }
    SSHString auth(auth_len);
    packet->buffer_get_data(auth.data.get(), auth_len);


    partial = packet->in_uint8();

    auth_methods = new char [auth.size + 1];
    memcpy(auth_methods, auth.data.get(), auth.size);
    auth_methods[auth.size] = 0;

    if (partial) {
        client_session->auth_state=SSH_AUTH_STATE_PARTIAL;
        syslog(LOG_INFO,
            "Partial success. Authentication that can continue: %s",
            auth_methods);
    }
    else {
        client_session->auth_state=SSH_AUTH_STATE_FAILED;
        syslog(LOG_INFO,
            "Access denied. Authentication that can continue: %s",
            auth_methods);
        ssh_set_error(client_session->error,  SSH_REQUEST_DENIED,
                "Access denied. Authentication that can continue: %s",
                auth_methods);

        client_session->auth_methods = 0;
    }
    if (strstr(auth_methods, "password") != nullptr) {
        client_session->auth_methods |= SSH_AUTH_METHOD_PASSWORD;
    }
    if (strstr(auth_methods, "keyboard-interactive") != nullptr) {
        client_session->auth_methods |= SSH_AUTH_METHOD_INTERACTIVE;
    }
    if (strstr(auth_methods, "publickey") != nullptr) {
        client_session->auth_methods |= SSH_AUTH_METHOD_PUBLICKEY;
    }
    if (strstr(auth_methods, "hostbased") != nullptr) {
        client_session->auth_methods |= SSH_AUTH_METHOD_HOSTBASED;
    }
    if (strstr(auth_methods, "gssapi-with-mic") != nullptr) {
          client_session->auth_methods |= SSH_AUTH_METHOD_GSSAPI_MIC;
    }

    free(auth_methods);
    auth_methods = nullptr;

    return SSH_PACKET_USED;
}

static inline int ssh2_msg_channel_open_confirmation_client(SshClientSession * client_session, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_WARNING,"Received SSH_MSG_CHANNEL_OPEN_CONFIRMATION");

    uint32_t chan = packet->in_uint32_be();
    ssh_channel channel = client_session->ssh_channel_from_local(chan);
    if(channel==nullptr){
        ssh_set_error(client_session->error,  SSH_FATAL,
                      "Unknown channel id %lu",
                      static_cast<long unsigned int>(chan));
        /* TODO: Set error marking in channel object */

        return SSH_PACKET_USED;
    }

    channel->remote_channel = packet->in_uint32_be();
    channel->remote_window = packet->in_uint32_be();
    channel->remote_maxpacket = packet->in_uint32_be();

    syslog(LOG_INFO,
            "Received a CHANNEL_OPEN_CONFIRMATION for channel %d:%d",
            channel->local_channel,
            channel->remote_channel);
    syslog(LOG_INFO,
            "Remote window : %lu, maxpacket : %lu",
            static_cast<long unsigned int>(channel->remote_window),
            static_cast<long unsigned int>(channel->remote_maxpacket));

    channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN;
    channel->flags &= ~SSH_CHANNEL_FLAG_NOT_BOUND;

    return SSH_PACKET_USED;
}


static inline int channel_rcv_data_stderr_client(SshClientSession * client_session, ssh_buffer_struct* packet, error_struct & error)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    uint32_t chan = packet->in_uint32_be();
    ssh_channel channel = client_session->ssh_channel_from_local(chan);
    if (channel == nullptr) {
        ssh_set_error(error, SSH_FATAL,
            "Server specified invalid channel %lu", static_cast<long unsigned int>(ntohl(chan)));
        syslog(LOG_INFO, "%s", ssh_get_error(&error));

        return SSH_PACKET_USED;
    }

    /* uint32 data type code. we can ignore it */
    /* uint32_t ignore = */ packet->in_uint32_be();

    if (sizeof(uint32_t) > packet->in_remain()) {
        return -1;
    }
    uint32_t str_len = packet->in_uint32_be();
    if (str_len > packet->in_remain()) {
        return -1;
    }
    SSHString str(str_len);
    packet->buffer_get_data(str.data.get(), str_len);

    // TODO: see that, we read full packet then drop what the local_window can't hold
    // as this looks really like a forbidden case we should probably close the connection
    if (str_len > channel->local_window) {
        syslog(LOG_INFO,
               "Data packet too big for our window(%u vs %d)",
               str_len,
               channel->local_window);
    }

    channel->stderr_buffer->out_blob(str.data.get(), str_len);

    if (str_len <= channel->local_window) {
        channel->local_window -= str_len;
    } else {
        channel->local_window = 0; /* buggy remote */
    }

    syslog(LOG_INFO,
            "Channel windows are now (local win=%d remote win=%d)",
            channel->local_window,
            channel->remote_window);

    if (channel->callbacks && channel->callbacks->channel_data_function){
        int used = channel->callbacks->channel_data_function(client_session,
                                                         channel,
                                                         channel->stderr_buffer->get_pos_ptr(),
                                                         channel->stderr_buffer->in_remain(),
                                                         1,
                                                         channel->callbacks->userdata);
        if(used > 0) {
            channel->stderr_buffer->in_skip_bytes(used);
        }
        if (channel->local_window + channel->stderr_buffer->in_remain() < WINDOWLIMIT) {
            if (WINDOWBASE > channel->local_window){
                /* WINDOW_ADJUST packet needs a relative increment rather than an absolute
                 * value, so we give here the missing bytes needed to reach new_window
                 */
                client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_WINDOW_ADJUST);
                client_session->out_buffer->out_uint32_be(channel->remote_channel);
                client_session->out_buffer->out_uint32_be(WINDOWBASE - channel->local_window);
                client_session->packet_send();
                channel->local_window = WINDOWBASE - channel->local_window ;
            }
        }
    }

    return SSH_PACKET_USED;
}

/* is_stderr is set to 1 if the data are extended, ie stderr */
// SSH_PACKET_CALLBACK(channel_rcv_data)
static inline int channel_rcv_data_client(SshClientSession * client_session, ssh_buffer_struct* packet, error_struct & error)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    uint32_t chan = packet->in_uint32_be();
    ssh_channel channel = client_session->ssh_channel_from_local(chan);
    if (channel == nullptr) {
        ssh_set_error(error, SSH_FATAL,
            "Server specified invalid channel %lu", static_cast<long unsigned int>(ntohl(chan)));
        syslog(LOG_INFO, "%s", ssh_get_error(&error));
        return SSH_PACKET_USED;
    }

    if (sizeof(uint32_t) > packet->in_remain()) {
        return -1;
    }
    uint32_t str_len = packet->in_uint32_be();
    if (str_len > packet->in_remain()) {
        return -1;
    }
    SSHString str(str_len);
    packet->buffer_get_data(str.data.get(), str_len);

    // TODO: see that, we read full packet then drop what the local_window can't hold
    // as this looks really like a forbidden case we should probably close the connection
    if (str_len > channel->local_window) {
        syslog(LOG_INFO,
               "Data packet too big for our window(%u vs %d)",
               str_len,
               channel->local_window);
    }

    channel->stdout_buffer->out_blob(str.data.get(), str_len);

    if (str_len <= channel->local_window) {
        channel->local_window -= str_len;
    } else {
        channel->local_window = 0; /* buggy remote */
    }

    syslog(LOG_INFO,
            "Channel windows are now (local win=%d remote win=%d)",
            channel->local_window,
            channel->remote_window);

    if (channel->callbacks && channel->callbacks->channel_data_function){
        int used = channel->callbacks->channel_data_function(client_session,
                                                         channel,
                                                         channel->stdout_buffer->get_pos_ptr(),
                                                         channel->stdout_buffer->in_remain(),
                                                         0,
                                                         channel->callbacks->userdata);
        if(used > 0) {
            channel->stdout_buffer->in_skip_bytes(used);
        }
        if (channel->local_window + channel->stdout_buffer->in_remain() < WINDOWLIMIT) {
            if (WINDOWBASE > channel->local_window){
                /* WINDOW_ADJUST packet needs a relative increment rather than an absolute
                 * value, so we give here the missing bytes needed to reach new_window
                 */
                client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_WINDOW_ADJUST);
                client_session->out_buffer->out_uint32_be(channel->remote_channel);
                client_session->out_buffer->out_uint32_be(WINDOWBASE - channel->local_window);
                client_session->packet_send();
                channel->local_window = WINDOWBASE - channel->local_window ;
            }
        }
    }

    return SSH_PACKET_USED;
}



// SSH_PACKET_CALLBACK(channel_rcv_close)
static inline int channel_rcv_close_client(SshClientSession * client_session, ssh_buffer_struct* packet)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    uint32_t chan = packet->in_uint32_be();
    ssh_channel channel = client_session->ssh_channel_from_local(chan);
    if (channel == nullptr) {
        ssh_set_error(client_session->error,  SSH_FATAL,
                      "Server specified invalid channel %lu",
                      static_cast<long unsigned int>(ntohl(chan)));
        syslog(LOG_INFO, "%s", ssh_get_error(&client_session->error));

        return SSH_PACKET_USED;
    }

    syslog(LOG_INFO, "Received close on channel (%d:%d)",
            channel->local_channel,
            channel->remote_channel);

    if ((channel->stdout_buffer && channel->stdout_buffer->in_remain() > 0) ||
        (channel->stderr_buffer && channel->stderr_buffer->in_remain() > 0)) {
        channel->delayed_close = 1;
    } else {
        channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_CLOSED;
    }
    if (channel->remote_eof == 0) {
        syslog(LOG_INFO,
                "Remote host not polite enough to send an eof before close");
    }
    channel->remote_eof = 1;

    /*
     * The remote eof doesn't break things if there was still data into read
     * buffer because the eof is ignored until the buffer is empty.
     */

    if(channel->callbacks && channel->callbacks->channel_close_function){
        channel->callbacks->channel_close_function(client_session,
                                                   channel,
                                                   channel->callbacks->userdata);
    }
    syslog(LOG_INFO,
            "Return from user channel_close function");

    channel->flags |= SSH_CHANNEL_FLAG_CLOSED_REMOTE;
    if(channel->flags & SSH_CHANNEL_FLAG_FREED_LOCAL){
        for (unsigned i = 0; i < client_session->nbchannels ; i++){
            if (client_session->channels[i] == channel){
                client_session->nbchannels--;
                client_session->channels[i] = client_session->channels[client_session->nbchannels];
                delete channel;
                break;
            }
        }
    }

    return SSH_PACKET_USED;
}

// SSH_PACKET_CALLBACK(channel_rcv_change_window)
static inline int channel_rcv_change_window_client(SshClientSession * client_session, ssh_buffer_struct* packet, error_struct & error)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t chan = packet->in_uint32_be();
    ssh_channel channel = client_session->ssh_channel_from_local(chan);
    if (channel == nullptr) {
        ssh_set_error(error, SSH_FATAL, "Server specified invalid channel %lu", static_cast<long unsigned int>(ntohl(chan)));
        syslog(LOG_INFO, "%s", ssh_get_error(&error));
        syslog(LOG_INFO, "Error getting a window adjust message: invalid packet");

        return SSH_PACKET_USED;
    }
    uint32_t bytes = packet->in_uint32_be();
    channel->remote_window += bytes;

    return SSH_PACKET_USED;
}

inline int ssh_packet_global_request_client(SshClientSession * client_session, ssh_buffer_struct* packet)
{
    // SSH_REQUEST_GLOBAL
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int rc = SSH_PACKET_USED;
    char * request = packet->in_strdup_cstr();
    uint8_t want_reply = packet->in_uint8();

    syslog(LOG_INFO, "UNKNOWN SSH_MSG_GLOBAL_REQUEST %s %d", request, want_reply);

    if (want_reply){
        client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_FAILURE);
    }
    delete request;
    return rc;
}

//  sshd has an application-level keepalive,
// controlled by ClientAliveInterval and ClientAliveCountMax.  It is these
// that generate the "keepalive@openssh.com" messages and these that you
// need to change in your case, (in addition to KeepAlive).

//    Later versions (3.8 and up) also have the client-side equivalent
// (ServerAliveInterval and ServerAliveCountMax).


/** @internal
 * @handles a data received event. It then calls the handlers for the different packet types
 * or and exception handler callback.
 * @param data pointer to the data received
 * @param receivedlen length of data received. It might not be enough for a complete packet
 * @returns number of bytes read and processed.
 */
int SshClientSession::handle_received_data_client(const void *data, size_t receivedlen)
{
    syslog(LOG_INFO, "%s ---", __PRETTY_FUNCTION__);
    syslog(LOG_INFO, "%s <<<<<<<<<<<<<<<<< %s ", __FUNCTION__, this->show());

    unsigned int blocksize = (this->current_crypto ?
                              this->current_crypto->in_cipher->blocksize : 8);
    int current_macsize = this->current_crypto ? MACSIZE : 0;
    unsigned char mac[30] = {0};
    char buffer[16] = {0};
    const uint8_t *packet;
    int to_be_read;
    int rc;
//    uint32_t compsize;
//    uint32_t payloadsize;
    size_t processed = 0; /* number of byte processed from the callback */
    uint8_t padding = 0;

    if (data == nullptr) {
        this->session_state= SSH_SESSION_STATE_ERROR;
        return 0;
    }

    if (this->session_state == SSH_SESSION_STATE_ERROR) {
        syslog(LOG_INFO, "%s session in error state: exiting ", __FUNCTION__);
        this->session_state = SSH_SESSION_STATE_ERROR;
        return 0;
    }

    switch(this->packet_state) {
        case PACKET_STATE_INIT:
            if (receivedlen < blocksize) {
                /*
                 * We didn't received enough data to read
                 * we need at least one block size, give up
                 */
                syslog(LOG_INFO, "%s Need more data (got %d, need %d)",
                     __FUNCTION__, static_cast<int>(receivedlen), static_cast<int>(blocksize));
                return 0;
            }
            this->in_packet_type = 0;
            this->in_packet_valid = 0;

            if (this->in_buffer) {
                delete this->in_buffer;
            }
            this->in_buffer = new ssh_buffer_struct;
            memcpy(buffer, data, blocksize);
            processed += blocksize;

            if (this->current_crypto) {
              if (packet_decrypt(*(this->current_crypto), buffer,
                  this->current_crypto->in_cipher->blocksize, this->error) < 0) {
                  this->in_packet_len = 0;
                 // TODO: check that suspicious. What will happen if decrypt fail ?
              }
            }
            {
                Parse p(reinterpret_cast<uint8_t *>(buffer));
                this->in_packet_len = p.in_uint32_be();
            }

            // TODO: why not reading from buffer instead of copying to in_buffer ?
            // (actually copying to in_buffer could be done inside packet_decrypt)
            this->in_buffer->out_blob(buffer, blocksize);

            if (this->in_packet_len > MAX_PACKET_LEN) {
                ssh_set_error(this->error,
                              SSH_FATAL,
                              "read_packet(): Packet len larger than allowed max (%.4x, %u > %u)",
                              this->in_packet_len, this->in_packet_len, MAX_PACKET_LEN);
                this->session_state= SSH_SESSION_STATE_ERROR;
                return processed;
            }
            this->packet_state = PACKET_STATE_SIZEREAD;
            REDEMPTION_CXX_FALLTHROUGH;
        case PACKET_STATE_SIZEREAD:

            // some cases are looking wrong here, seems like not crypted/crypted cases
            // are mixed. It works by sheer luck...
            to_be_read = this->in_packet_len - blocksize + sizeof(uint32_t) + current_macsize;
            /* if to_be_read is zero, the whole packet was blocksize bytes. */
            // TODO: I wonder what would happen is to_be_read is negative in above expression
            // => will be seen as a large positive number and accepted as valid ?
            if (to_be_read != 0) {
                if ((receivedlen - processed < static_cast<unsigned int>(to_be_read))
                || (to_be_read < current_macsize)
                ) {
                    /* give up, not enough data in buffer */
                    syslog(LOG_INFO,"packet: partial packet (read len) [len=%d] receivedlen=%u - process=%u < to_be_read=%u",
                        this->in_packet_len, static_cast<unsigned int>(receivedlen), static_cast<unsigned int>(processed), static_cast<unsigned int>(to_be_read));
                    return processed;
                }

                packet = static_cast<const uint8_t*>(data) + processed;
                this->in_buffer->out_blob(packet, to_be_read - current_macsize);
                processed += to_be_read - current_macsize;
            }

            if (this->current_crypto) {
                /*
                 * Decrypt the rest of the packet (blocksize bytes already
                 * have been decrypted)
                 */
                uint32_t buffer_len = this->in_buffer->in_remain();

                /* The following check avoids decrypting zero bytes */
                if (buffer_len > blocksize) {
                    uint8_t *payload = this->in_buffer->get_pos_ptr() + blocksize;
                    uint32_t plen = buffer_len - blocksize;

                    rc = packet_decrypt(*this->current_crypto, payload, plen, this->error);
                    if (rc < 0) {
                        syslog(LOG_INFO, "%s Decrypt error", __FUNCTION__);
                        ssh_set_error(this->error,  SSH_FATAL, "Decrypt error");
                        this->session_state= SSH_SESSION_STATE_ERROR;
                        return processed;
                    }
                }

                /* copy the last part from the incoming buffer */
                packet = static_cast<const uint8_t*>(data) + processed;
                memcpy(mac, packet, MACSIZE);

                rc = this->packet_hmac_verify(this->in_buffer, mac);
                if (rc < 0) {
                    syslog(LOG_INFO, "%s HMAC error", __FUNCTION__);
                    ssh_set_error(this->error, SSH_FATAL, "HMAC error");
                    this->session_state= SSH_SESSION_STATE_ERROR;
                    return processed;
                }
                processed += current_macsize;
            }

            /* skip the size field which has been processed before */
            this->in_buffer->in_skip_bytes(sizeof(uint32_t));
            padding = this->in_buffer->in_uint8();

            if (padding > this->in_buffer->in_remain()) {
                ssh_set_error(this->error,
                              SSH_FATAL,
                              "Invalid padding: %d (%d left)",
                              padding,
                              static_cast<uint32_t>(this->in_buffer->in_remain()));
                this->session_state= SSH_SESSION_STATE_ERROR;
                return processed;
            }
            this->in_buffer->buffer_pass_bytes_end(padding);
//            compsize = this->in_buffer->in_remain();

            if (this->current_crypto
                && this->current_crypto->do_compress_in
                && this->in_buffer->in_remain() > 0) {
                rc = decompress_buffer_client(this, this->in_buffer, MAX_PACKET_LEN);
                if (rc < 0) {
                    this->session_state= SSH_SESSION_STATE_ERROR;
                    return processed;
                }
            }
//            payloadsize = this->in_buffer->in_remain();
            this->recv_seq++;

            /*
             * We don't want to rewrite a new packet while still executing the
             * packet callbacks
             */

            this->in_packet_type = this->in_buffer->in_uint8();
            this->in_packet_valid = 1;
            this->in_packet_len = 0;

//            syslog(LOG_INFO,
//                    "====> packet: read type %hhd [len=%d, padding=%hhd, comp=%d, payload=%d]",
//                    this->in_packet_type, this->in_packet_len, padding, compsize, payloadsize);

            {
                syslog(LOG_INFO, "%s --- PACKET TYPE = %d", __FUNCTION__, this->in_packet_type);
                switch (this->in_packet_type) {
                case SSH_MSG_DISCONNECT:
                    syslog(LOG_INFO, "%s --- SSH_MSG_DISCONNECT", __FUNCTION__);
                    handle_ssh_packet_disconnect_client(this, this->in_buffer, this->error);
                break;
                case SSH_MSG_IGNORE:
                    syslog(LOG_INFO, "%s --- ssh2_msg_ignore", __FUNCTION__);
                break;
                case SSH_MSG_UNIMPLEMENTED:
                    syslog(LOG_INFO, "%s --- SSH_MSG_UNIMPLEMENTED", __FUNCTION__);
                    handle_ssh_packet_unimplemented_client(this, this->in_buffer, this->error);
                break;
                case SSH_MSG_DEBUG:
                    syslog(LOG_INFO, "%s --- ssh2_msg_debug", __FUNCTION__);
                break;
                case SSH_MSG_SERVICE_REQUEST:
                    syslog(LOG_INFO, "%s --- SSH_MSG_SERVICE_REQUEST", __FUNCTION__);
                    handle_ssh_packet_service_request_client(this, this->in_buffer, this->error);
                break;
                case SSH_MSG_SERVICE_ACCEPT:
                    syslog(LOG_INFO, "%s --- SSH_MSG_SERVICE_ACCEPT", __FUNCTION__);
                    // HERE
                    handle_ssh_packet_service_accept_client(this, this->in_packet_type, this->in_buffer, this);
                //    this->flags &= ~SSH_SESSION_FLAG_BLOCKING;
                break;


                case SSH_MSG_KEXINIT:
                    // Comment above means that messages are restricted until MSG_NEWKEYS have been received
                    syslog(LOG_INFO, "%s --- SSH_MSG_KEXINIT", __FUNCTION__);
                    ssh_packet_kexinit_client(this, this->in_buffer);
                break;
                case SSH_MSG_NEWKEYS:
                    syslog(LOG_INFO, "%s --- SSH_MSG_NEWKEYS", __FUNCTION__);
                    ssh_packet_newkeys_client(this, this->error);
                break;

//       First, the client sends the following:

//         byte      SSH_MSG_KEXDH_INIT
//         mpint     e

// Implies This message can be received Server side only
                case SSH_MSG_KEXDH_INIT:
                    syslog(LOG_INFO, "%s --- SSH_MSG_KEXDH_INIT", __FUNCTION__);
                    // ssh_packet_kexdh_init_client(this, this->in_buffer);
                break;

//       The server responds with the following:

//         byte      SSH_MSG_KEXDH_REPLY
//         string    server public host key and certificates (K_S)
//         mpint     f
//         string    signature of H

// Implies This message can be received Client side only

                case SSH_MSG_KEXDH_REPLY:
                    syslog(LOG_INFO, "%s --- SSH_MSG_KEXDH_REPLY", __FUNCTION__);
                    ssh_packet_dh_reply_client(this, this->in_buffer);
                break;

                case SSH_MSG_USERAUTH_REQUEST:
                    syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_REQUEST -- should never be received by client", __FUNCTION__);
                break;
                case SSH_MSG_USERAUTH_FAILURE:
                    syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_FAILURE", __FUNCTION__);
                    // HERE
                    ssh_packet_userauth_failure_client(this, this->in_packet_type, this->in_buffer);
                break;
                case SSH_MSG_USERAUTH_SUCCESS:
                    syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_SUCCESS", __FUNCTION__);
                    ssh_packet_userauth_success_client(this, this->in_packet_type, this->in_buffer);
                break;
                case SSH_MSG_USERAUTH_BANNER:
                    syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_BANNER", __FUNCTION__);
                    ssh_packet_userauth_banner_client(this, this->in_buffer);
                break;
                case SSH_MSG_USERAUTH_PK_OK:
                    syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_PK_OK", __FUNCTION__);
                    // HERE
                    ssh_packet_userauth_pk_ok_client(this, this->in_packet_type, this->in_buffer, this);
                break;
                case SSH_MSG_USERAUTH_INFO_RESPONSE:
//                case SSH_MSG_USERAUTH_GSSAPI_TOKEN:
                    // TODO: replace this with a better test where the accepted answer
                    // depends on the type of the previously sent request
                    if (this->gssapi != nullptr) {
                        syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_GSSAPI_TOKEN", __FUNCTION__);
                        ssh_packet_userauth_gssapi_token_client(this, this->in_buffer);
                    }
                    else {
                        syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_INFO_RESPONSE", __FUNCTION__);
                        ssh_packet_userauth_info_response_client(this, this->in_buffer);
                    }
                break;
                case SSH_MSG_USERAUTH_GSSAPI_MIC:
                    syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_GSSAPI_MIC", __FUNCTION__);
                    // HERE
                    ssh_packet_userauth_gssapi_mic_client(this, this->in_packet_type, this->in_buffer, this);
                break;
                case SSH_MSG_GLOBAL_REQUEST:
                    syslog(LOG_INFO, "%s --- SSH_MSG_GLOBAL_REQUEST", __FUNCTION__);
                    ssh_packet_global_request_client(this, this->in_buffer);
                break;
                case SSH_MSG_REQUEST_SUCCESS:
                    syslog(LOG_INFO, "%s --- SSH_MSG_REQUEST_SUCCESS", __FUNCTION__);
                    if(this->global_req_state != SSH_CHANNEL_REQ_STATE_PENDING){
                        syslog(LOG_ERR, "SSH_REQUEST_SUCCESS received in incorrect state %d",
                            this->global_req_state);
                    }
                    this->global_req_state=SSH_CHANNEL_REQ_STATE_ACCEPTED;
                break;
                case SSH_MSG_REQUEST_FAILURE:
                    syslog(LOG_INFO, "%s --- SSH_MSG_REQUEST_FAILURE", __FUNCTION__);
                    if(this->global_req_state != SSH_CHANNEL_REQ_STATE_PENDING){
                        syslog(LOG_ERR, "SSH_REQUEST_DENIED received in incorrect state %d",
                            this->global_req_state);
                    }
                    this->global_req_state = SSH_CHANNEL_REQ_STATE_DENIED;
                break;
                case SSH_MSG_CHANNEL_OPEN:
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_OPEN", __FUNCTION__);
                    if (this->session_state != SSH_SESSION_STATE_AUTHENTICATED){
                        ssh_set_error(this->error,  SSH_FATAL,
                        "Invalid state when receiving channel open request (must be authenticated)");
                        this->session_state = SSH_SESSION_STATE_ERROR;
                        return processed;
                    }
                    handle_channel_open_client(this, this->in_buffer);
                break;
                case SSH_MSG_CHANNEL_OPEN_CONFIRMATION:
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_OPEN_CONFIRMATION", __FUNCTION__);
                    ssh2_msg_channel_open_confirmation_client(this, this->in_buffer);
                break;
                case SSH_MSG_CHANNEL_OPEN_FAILURE:
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_OPEN_FAILURE", __FUNCTION__);
                    {
                        uint32_t chan = this->in_buffer->in_uint32_be();
                        ssh_channel channel = this->ssh_channel_from_local(chan);
                        if (channel == nullptr) {
                            ssh_set_error(this->error,  SSH_FATAL,
                                          "Server specified invalid channel %lu",
                                          static_cast<long unsigned int>(ntohl(chan)));
                            syslog(LOG_ERR,"Invalid channel in packet");
                        }
                        else {
                            ssh_packet_channel_open_fail_client(this, channel, this->in_buffer);
                        }
                }
                break;
                case SSH_MSG_CHANNEL_WINDOW_ADJUST:
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_WINDOW_ADJUST", __FUNCTION__);
                    channel_rcv_change_window_client(this, this->in_buffer, this->error);
                break;
                case SSH_MSG_CHANNEL_DATA:
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_DATA", __FUNCTION__);
                    channel_rcv_data_client(this, this->in_buffer, this->error);
                break;
                case SSH_MSG_CHANNEL_EXTENDED_DATA:
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_EXTENDED_DATA", __FUNCTION__);
                    channel_rcv_data_stderr_client(this, this->in_buffer, this->error);
                break;
                case SSH_MSG_CHANNEL_EOF:
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_EOF", __FUNCTION__);
                    this->handle_channel_rcv_eof_client(this->in_buffer);
                break;
                case SSH_MSG_CHANNEL_CLOSE:
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_CLOSE", __FUNCTION__);
                    channel_rcv_close_client(this, this->in_buffer);
//                    this->session_state = SSH_SESSION_STATE_ERROR; // breaks agent forwarding
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_CLOSE DONE", __FUNCTION__);
                break;
                case SSH_MSG_CHANNEL_REQUEST:
//                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_REQUEST", __FUNCTION__);
                    this->handle_channel_rcv_request_client(this->in_buffer, this->error);
                break;
                case SSH_MSG_CHANNEL_SUCCESS:
                {
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_SUCCESS", __FUNCTION__);
                    uint32_t chan = this->in_buffer->in_uint32_be();
                    ssh_channel channel = this->ssh_channel_from_local(chan);
                    if(channel->request_state != SSH_CHANNEL_REQ_STATE_PENDING){
                        syslog(LOG_ERR, "SSH_MSG_CHANNEL_SUCCESS received in incorrect state %d",
                                channel->request_state);
                    } else {
                        channel->request_state=SSH_CHANNEL_REQ_STATE_ACCEPTED;
                    }
                }
                break;
                case SSH_MSG_CHANNEL_FAILURE:
                {
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_FAILURE", __FUNCTION__);
                    uint32_t chan = this->in_buffer->in_uint32_be();
                    ssh_channel channel = this->ssh_channel_from_local(chan);
                    if(channel->request_state != SSH_CHANNEL_REQ_STATE_PENDING){
                        syslog(LOG_ERR, "SSH_MSG_CHANNEL_FAILURE received in incorrect state %d",
                                channel->request_state);
                    } else {
                        channel->request_state=SSH_CHANNEL_REQ_STATE_DENIED;
                    }
                }
                break;
                default:
                    ssh_send_unimplemented_client(this);
                break;
                }
            }
            syslog(LOG_INFO, "Packet used processed=%d receivedLen=%d",
                static_cast<int>(processed), static_cast<int>(receivedlen));

            this->packet_state = PACKET_STATE_INIT;
//            if (processed < receivedlen) {
//                /* Handle a potential packet left in socket buffer */
//                syslog(LOG_INFO,
//                        "Processing %" PRIdS " bytes left in socket buffer",
//                        receivedlen-processed);

//                packet = ((uint8_t*)data) + processed;

//                // Why a recursive call instead of a loop ?
//                rc = handle_received_data(packet, receivedlen - processed, session);
//                processed += rc;
//            }
//            syslog(LOG_INFO, "%s One ssh packet handled", __FUNCTION__);
            return processed;
//        case PACKET_STATE_PROCESSING:
//            syslog(LOG_INFO, "%s PACKET_STATE_PROCESSING ", __FUNCTION__);
//            syslog(LOG_ERR, "Nested packet processing. Delaying.");
//            return 0;
    }
    syslog(LOG_INFO, "%s OTHER ", __FUNCTION__);

    ssh_set_error(this->error,
                  SSH_FATAL,
                  "Invalid state into packet_read2(): %d",
                  this->packet_state);
    syslog(LOG_INFO, "%s OTHER.1 ", __FUNCTION__);
    this->session_state = SSH_SESSION_STATE_ERROR;
    return 0;
}


/**
 * @brief Add a fd to the event and assign it a callback,
 * when used in blocking mode.
 * @param ctx          access to polling context
 * @param fd           Socket that will be polled.
 * @param pw_cb        Function to be called if any of the events are set.
 *                     The prototype of cb is:
 *                      int (*ssh_event_callback)(socket_t fd, int revents,
 *                                                          void *userdata);
 * @param pw_userdata  Userdata to be passed to the callback function. nullptr if
 *                      not needed.
 *
 * @returns SSH_OK      on success
 *          SSH_ERROR   on failure
 */
int ssh_event_set_fd_client(ssh_poll_ctx_struct * ctx, socket_t fd, ssh_event_callback pw_cb, void *pw_userdata) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_WARNING, "ssh_event_add_fd = %u", fd);

    if(ctx == nullptr || pw_cb == nullptr || fd == INVALID_SOCKET) {
        syslog(LOG_WARNING, "ssh_event_add_fd failed = %u", fd);
        return SSH_ERROR;
    }

    syslog(LOG_WARNING, "new poll handle for poll_out FDWCALLBACK");
    ctx->fd_poll = new ssh_poll_handle_fd_struct(fd, pw_cb, pw_userdata);
    ctx->target_session = nullptr;

    return SSH_OK;
}


void ssh_event_set_session_client(ssh_poll_ctx_struct * ctx, SshClientSession * client_session) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    ctx->target_session = client_session;
}

/**
 * @internal
 * @brief Wait for a response of an authentication function.
 *
 * @param[in] client_session   The SSH session.
 *
 * @returns SSH_AUTH_SUCCESS Authentication success, or pubkey accepted
 *          SSH_AUTH_PARTIAL Authentication succeeded but another mean
 *                           of authentication is needed.
 *          SSH_AUTH_INFO    Data for keyboard-interactive
 *          SSH_AUTH_AGAIN   In nonblocking mode, call has to be made again
 *          SSH_AUTH_ERROR   Error during the process.
 */

static enum ssh_auth_e ssh_userauth_get_response_client(SshClientSession * client_session) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    enum ssh_auth_e rc = SSH_AUTH_ERROR;

    if ((client_session->flags & SSH_SESSION_FLAG_BLOCKING)){
        int timeout = TIMEOUT_DEFAULT_MS;
        struct timeval start;
        gettimeofday(&start, nullptr);

        while(client_session->auth_state == SSH_AUTH_STATE_NONE
            || client_session->auth_state == SSH_AUTH_STATE_KBDINT_SENT
            || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_REQUEST_SENT
            || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_TOKEN
            || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_MIC_SENT) {
            // Waiting for input
            dopoll(client_session->ctx, timeout);
            if (client_session->session_state == SSH_SESSION_STATE_ERROR) {
                return SSH_AUTH_ERROR;
            }
            struct timeval now;
            gettimeofday(&now, nullptr);
            long ms =   (now.tv_sec - start.tv_sec) * 1000
                    + (now.tv_usec < start.tv_usec) * 1000
                    + (now.tv_usec - start.tv_usec) / 1000;

            if (ms >= timeout)
            {
                if (client_session->auth_state == SSH_AUTH_STATE_NONE
                    || client_session->auth_state == SSH_AUTH_STATE_KBDINT_SENT
                    || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_REQUEST_SENT
                    || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_TOKEN
                    || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_MIC_SENT){
                  return SSH_AUTH_AGAIN;
                }
                break;
            }
            timeout -= ms;
        }
    }
    else {
        if(client_session->auth_state == SSH_AUTH_STATE_NONE
            || client_session->auth_state == SSH_AUTH_STATE_KBDINT_SENT
            || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_REQUEST_SENT
            || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_TOKEN
            || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_MIC_SENT) {
            // Waiting for input
            dopoll(client_session->ctx, SSH_TIMEOUT_NONBLOCKING);
            if (client_session->session_state == SSH_SESSION_STATE_ERROR) {
                return SSH_AUTH_ERROR;
            }
        }
        if (client_session->auth_state == SSH_AUTH_STATE_NONE
            || client_session->auth_state == SSH_AUTH_STATE_KBDINT_SENT
            || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_REQUEST_SENT
            || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_TOKEN
            || client_session->auth_state == SSH_AUTH_STATE_GSSAPI_MIC_SENT){
          return SSH_AUTH_AGAIN;
        }
    }

    switch(client_session->auth_state) {
        case SSH_AUTH_STATE_ERROR:
            rc = SSH_AUTH_ERROR;
            break;
        case SSH_AUTH_STATE_FAILED:
            rc = SSH_AUTH_DENIED;
            break;
        case SSH_AUTH_STATE_INFO:
            rc = SSH_AUTH_INFO;
            break;
        case SSH_AUTH_STATE_PARTIAL:
            rc = SSH_AUTH_PARTIAL;
            break;
        case SSH_AUTH_STATE_PK_OK:
        case SSH_AUTH_STATE_SUCCESS:
            rc = SSH_AUTH_SUCCESS;
            break;
        case SSH_AUTH_STATE_KBDINT_SENT:
        case SSH_AUTH_STATE_GSSAPI_REQUEST_SENT:
        case SSH_AUTH_STATE_GSSAPI_TOKEN:
        case SSH_AUTH_STATE_GSSAPI_MIC_SENT:
        case SSH_AUTH_STATE_NONE:
            /* not reached */
            rc = SSH_AUTH_ERROR;
            break;
    }

    return rc;
}

// [RFC4253] 10.  Service Request
// ==============================

//   After the key exchange, the client requests a service.  The service
//   is identified by a name.  The format of names and procedures for
//   defining new names are defined in [SSH-ARCH] and [SSH-NUMBERS].

//   Currently, the following names have been reserved:

//      ssh-userauth
//      ssh-connection

//   Similar local naming policy is applied to the service names, as is
//   applied to the algorithm names.  A local service should use the
//   PRIVATE USE syntax of "servicename@domain".

//      byte      SSH_MSG_SERVICE_REQUEST
//      string    service name

//   If the server rejects the service request, it SHOULD send an
//   appropriate SSH_MSG_DISCONNECT message and MUST disconnect.

//   When the service starts, it may have access to the session identifier
//   generated during the key exchange.

//   If the server supports the service (and permits the client to use
//   it), it MUST respond with the following:

//      byte      SSH_MSG_SERVICE_ACCEPT
//      string    service name

//   Message numbers used by services should be in the area reserved for
//   them (see [SSH-ARCH] and [SSH-NUMBERS]).  The transport level will
//   continue to process its own messages.

//   Note that after a key exchange with implicit server authentication,
//   the client MUST wait for a response to its service request message
//   before sending any further data.


/**
 * @internal
 *
 * @brief Request a service from the SSH server.
 *
 * Service requests are for example: ssh-userauth, ssh-connection, etc.
 *
 * @param  client_session      The session to use to ask for a service request.
 * @param  service      The service request.
 *
 * @return SSH_OK on success
 * @return SSH_ERROR on error
 * @return SSH_AGAIN No response received yet
 * @bug actually only works with ssh-userauth
 */
static inline int ssh_send_service_request_client(SshClientSession * client_session, const char *service) {
    // THIS FUNCTION IS CURRENTLY BLOCKING (expecting answer)

    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    client_session->out_buffer->out_uint8(SSH_MSG_SERVICE_REQUEST);
    client_session->out_buffer->out_length_prefixed_cstr(service);
    client_session->auth_service_state = SSH_AUTH_SERVICE_SENT;
    client_session->packet_send();

    // TODO: I Removed auth timeout support (failed to received accept in time), add it back later
    // also check the next message received is service acceptation or disconnect
    // (many message should never be received between SERVICE_REQUEST and ACCEPTED reply)
    return SSH_OK;
}




/**
 * @brief Get available authentication methods from the server.
 *
 * This requires the function ssh_userauth_none() to be called before the
 * methods are available. The server MAY return a list of methods that may
 * continue.
 *
 * @param[in] client_session   The SSH session.
 *
 * @param[in] error  Deprecated, don't care.
 *
 * @returns             A bitfield of the fllowing values:
 *                      - SSH_AUTH_METHOD_PASSWORD
 *                      - SSH_AUTH_METHOD_PUBLICKEY
 *                      - SSH_AUTH_METHOD_HOSTBASED
 *                      - SSH_AUTH_METHOD_INTERACTIVE
 *
 * @warning Other reserved flags may appear in future versions.
 * @see ssh_userauth_none()
 */
int ssh_userauth_list_client(SshClientSession * client_session, error_struct * error)
{
    (void)error;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    return client_session->auth_methods;
}

/**
 * @brief Try to authenticate with the given public key.
 *
 * To avoid unnecessary processing and user interaction, the following method
 * is provided for querying whether authentication using the 'pubkey' would
 * be possible.
 *
 * @param[in] client_session     The SSH session.
 *
 * @param[in] username    The username, this SHOULD be nullptr.
 *
 * @param[in] pubkey      The public key to try.
 *
 * @return  SSH_AUTH_ERROR:   A serious error happened.\n
 *          SSH_AUTH_DENIED:  The server doesn't accept that public key as an
 *                            authentication token. Try another key or another
 *                            method.\n
 *          SSH_AUTH_PARTIAL: You've been partially authenticated, you still
 *                            have to use another method.\n
 *          SSH_AUTH_SUCCESS: The public key is accepted
 *          SSH_AUTH_AGAIN:   In nonblocking mode, you've got to call this again
 *                            later.
 *
 * @note Most server implementations do not permit changing the username during
 * authentication. The username should only be set with ssh_options_set() only
 * before you connect to the server.
 */
ssh_auth_e ssh_userauth_try_publickey_client(SshClientSession * client_session, const char *username, const ssh_key_struct *pubkey)
{
    syslog(LOG_INFO, "%s %s ---", __FUNCTION__, client_session->show());
    ssh_auth_e rc = SSH_AUTH_ERROR;

    syslog(LOG_INFO, "%s A ---", __FUNCTION__);

    if (pubkey == nullptr || !(pubkey->flags & SSH_KEY_FLAG_PUBLIC)) {
        syslog(LOG_INFO, "%s Invalid pubkey", __FUNCTION__);
        ssh_set_error(client_session->error,  SSH_FATAL, "Invalid pubkey");
        return SSH_AUTH_ERROR;
    }
    syslog(LOG_INFO, "%s B ---", __FUNCTION__);

    switch(client_session->pending_call_state) {
        case SSH_PENDING_CALL_NONE:
            syslog(LOG_INFO, "%s C ---", __FUNCTION__);
            syslog(LOG_INFO, "%s SSH_PENDING_CALL_NONE", __FUNCTION__);
            switch (ssh_send_service_request_client(client_session, "ssh-userauth")){
            case SSH_AGAIN:
                syslog(LOG_INFO, "%s SSH_PENDING_CALL_NONE -> SSH_AGAIN", __FUNCTION__);
                return SSH_AUTH_AGAIN;
            case SSH_ERROR:
                syslog(LOG_WARNING, "Failed to request \"ssh-userauth\" service");
                syslog(LOG_INFO, "%s SSH_PENDING_CALL_NONE -> SSH_AUTH_ERROR", __FUNCTION__);
                return SSH_AUTH_ERROR;
            default:
                ;
            }
            syslog(LOG_INFO, "%s building msg SSH_MSG_USERAUTH_REQUEST", __FUNCTION__);

            /* request */
            client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_REQUEST);

            /* username */
            {
                const char * str_username = username?username:client_session->opts.username;
                // TODO : check username
                client_session->out_buffer->out_length_prefixed_cstr(str_username);
            }
            syslog(LOG_INFO, "%s D ---", __FUNCTION__);

            client_session->out_buffer->out_length_prefixed_cstr("ssh-connection"); /* service */
            client_session->out_buffer->out_length_prefixed_cstr("publickey"); /* method */

            /* private key? */
            client_session->out_buffer->out_uint8(0);

            /* algo */
            client_session->out_buffer->out_length_prefixed_cstr(pubkey->type_c());

            /* public key */
            {
                SSHString pubkey_blob(0);

                switch (pubkey->type) {
                    case SSH_KEYTYPE_DSS:
                    {
                        ssh_buffer_struct buffer;
                        buffer.out_length_prefixed_cstr(pubkey->type_c());
                        syslog(LOG_INFO, "%s SSH_KEYTYPE_DSS", __FUNCTION__);
                        buffer.out_bignum(pubkey->dsa->p); // p
                        buffer.out_bignum(pubkey->dsa->q); // q
                        buffer.out_bignum(pubkey->dsa->g); // g
                        buffer.out_bignum(pubkey->dsa->pub_key); // n
                        pubkey_blob = SSHString(static_cast<uint32_t>(buffer.in_remain()));
                        memcpy(pubkey_blob.data.get(), buffer.get_pos_ptr(), pubkey_blob.size);
                    }
                    break;
                    case SSH_KEYTYPE_RSA:
                    case SSH_KEYTYPE_RSA1:
                    {
                        ssh_buffer_struct buffer;
                        buffer.out_length_prefixed_cstr(pubkey->type_c());
                        syslog(LOG_INFO, "%s SSH_KEYTYPE_RSA", __FUNCTION__);
                        buffer.out_bignum(pubkey->rsa->e); // e
                        buffer.out_bignum(pubkey->rsa->n); // n
                        pubkey_blob = SSHString(static_cast<uint32_t>(buffer.in_remain()));
                        memcpy(pubkey_blob.data.get(), buffer.get_pos_ptr(), pubkey_blob.size);
                    }
                    break;
                    case SSH_KEYTYPE_ECDSA:
                    {
                        syslog(LOG_INFO, "%s SSH_KEYTYPE_ECDSA", __FUNCTION__);
                        ssh_buffer_struct buffer;
                        buffer.out_length_prefixed_cstr(pubkey->type_c());

                        buffer.out_length_prefixed_cstr(
                            (pubkey->ecdsa_nid == NID_X9_62_prime256v1) ? "nistp256" :
                            (pubkey->ecdsa_nid == NID_secp384r1)        ? "nistp384" :
                            (pubkey->ecdsa_nid == NID_secp521r1)        ? "nistp521" :
                            "unknown");

                        const EC_GROUP *g = EC_KEY_get0_group(pubkey->ecdsa);
                        const EC_POINT *p = EC_KEY_get0_public_key(pubkey->ecdsa);

                        size_t len_ec = EC_POINT_point2oct(g, p, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
                        if (len_ec == 0) {
                            return SSH_AUTH_ERROR;
                        }

                        SSHString e(static_cast<uint32_t>(len_ec));
                        if (e.size != EC_POINT_point2oct(g, p, POINT_CONVERSION_UNCOMPRESSED, e.data.get(), e.size, nullptr)){
                            return SSH_AUTH_ERROR;
                        }

                        buffer.out_uint32_be(e.size);
                        buffer.out_blob(e.data.get(), e.size);
                        pubkey_blob = SSHString(static_cast<uint32_t>(buffer.in_remain()));
                        memcpy(pubkey_blob.data.get(), buffer.get_pos_ptr(), pubkey_blob.size);
                    }
                    break;
                    case SSH_KEYTYPE_UNKNOWN:
                        syslog(LOG_INFO, "%s SSH_KEYTYPE_UNKNOWN", __FUNCTION__);
                }

                client_session->out_buffer->out_uint32_be(pubkey_blob.size);
                client_session->out_buffer->out_blob(pubkey_blob.data.get(), pubkey_blob.size);
            }
            client_session->auth_state = SSH_AUTH_STATE_NONE;
            client_session->pending_call_state = SSH_PENDING_CALL_AUTH_OFFER_PUBKEY;
            syslog(LOG_INFO, "%s building msg SSH_MSG_USERAUTH_REQUEST -> packetsend : SSH_PENDING_CALL_AUTH_OFFER_PUBKEY", __FUNCTION__);

            client_session->packet_send();
            syslog(LOG_INFO, "%s packet_send -> TO FALLBACK ---", __FUNCTION__);
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_OFFER_PUBKEY:
            syslog(LOG_INFO, "%s SSH_PENDING_CALL_OFFER_PUBKEY -> waiting response", __FUNCTION__);
            switch (ssh_userauth_get_response_client(client_session)){
            case SSH_AUTH_AGAIN:
                syslog(LOG_INFO, "%s SSH_PENDING_CALL_OFFER_PUBKEY -> AUTH_AGAIN", __FUNCTION__);
                return SSH_AUTH_AGAIN;
            case SSH_AUTH_SUCCESS:
                syslog(LOG_INFO, "%s SSH_PENDING_CALL_OFFER_PUBKEY -> got response AUTH_SUCCESS", __FUNCTION__);
                client_session->pending_call_state = SSH_PENDING_CALL_NONE;
                return SSH_AUTH_SUCCESS;
            case SSH_AUTH_ERROR:
                syslog(LOG_INFO, "%s SSH_PENDING_CALL_OFFER_PUBKEY -> got response AUTH_ERROR", __FUNCTION__);
                client_session->pending_call_state = SSH_PENDING_CALL_NONE;
                return SSH_AUTH_ERROR;
            case SSH_AUTH_DENIED:
                syslog(LOG_INFO, "%s SSH_PENDING_CALL_OFFER_PUBKEY -> got response AUTH_DENIED", __FUNCTION__);
                client_session->pending_call_state = SSH_PENDING_CALL_NONE;
                return SSH_AUTH_DENIED;
            case SSH_AUTH_PARTIAL:
                syslog(LOG_INFO, "%s SSH_PENDING_CALL_OFFER_PUBKEY -> got response AUTH_PARTIAL", __FUNCTION__);
                client_session->pending_call_state = SSH_PENDING_CALL_NONE;
                return SSH_AUTH_PARTIAL;
            case SSH_AUTH_INFO:
                syslog(LOG_INFO, "%s SSH_PENDING_CALL_OFFER_PUBKEY -> got response AUTH_INFO", __FUNCTION__);
                client_session->pending_call_state = SSH_PENDING_CALL_NONE;
                return SSH_AUTH_INFO;
            }
            break;
        case SSH_PENDING_CALL_AUTH_GSSAPI_MIC:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_KBDINT_SEND:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_KBDINT_INIT:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_AGENT:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_PUBKEY:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_PASSWORD:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_NONE:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_CONNECT:
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            syslog(LOG_INFO, "%s F ---", __FUNCTION__);
            syslog(LOG_INFO, "%s Wrong state during pending SSH call", __FUNCTION__);
            ssh_set_error(client_session->error, SSH_FATAL, "Wrong state during pending SSH call");
            syslog(LOG_INFO, "%s done SSH_ERROR ---", __FUNCTION__);
            return SSH_AUTH_ERROR;
    }
    syslog(LOG_INFO, "%s G ---", __FUNCTION__);
    return rc;
}

// [RFC-4252] 7.  Public Key Authentication Method: publickey
// ==========================================================

//       The only REQUIRED authentication 'method name' is public key
//       authentication.  All implementations MUST support this method;
//       however, not all users need to have public keys, and most local
//       policies are not likely to require public key authentication for all
//       users in the near future.

//       With this method, the possession of a private key serves as
//       authentication.  This method works by sending a 'signature' created
//       with a private key of the user.  The server MUST check that the key
//       is a valid authenticator for the user, and MUST check that the
//       'signature' is valid.  If both hold, the authentication request MUST
//       be accepted; otherwise it MUST be rejected.  (Note that the server
//       MAY require additional authentications after successful
//       authentication.)

//       Private keys are often stored in an encrypted form at the client
//       host, and the user must supply a passphrase before the signature can
//       be generated.  Even if they are not, the signing operation involves
//       some expensive computation.  To avoid unnecessary processing and user
//       interaction, the following message is provided for querying whether
//       authentication using the key would be acceptable.
//          byte      SSH_MSG_USERAUTH_REQUEST
//          string    user name in ISO-10646 UTF-8 encoding
//          string    service name in US-ASCII
//          string    "publickey"
//          boolean   FALSE
//          string    public key algorithm name
//          string    public key blob

//       Public key algorithms are defined in the transport layer
//       specification [SSH-TRANS].  The 'public key blob' may contain
//       certificates.

//       Any public key algorithm may be offered for use in authentication.
//       In particular, the list is not constrained by what was negotiated
//       during key exchange.  If the server does not support some algorithm,
//       it MUST simply reject the request.

//       The server MUST respond to this message with either
//       SSH_MSG_USERAUTH_FAILURE or with the following:

//         byte      SSH_MSG_USERAUTH_PK_OK
//         string    public key algorithm name from the request
//         string    public key blob from the request

//       To perform actual authentication, the client MAY then send a
//       signature generated using the private key.  The client MAY send the
//       signature directly without first verifying whether the key is
//       acceptable.  The signature is sent using the following packet:

//         byte      SSH_MSG_USERAUTH_REQUEST
//         string    user name
//         string    service
//         string    "publickey"
//         boolean   TRUE
//         string    public key algorithm name
//         string    public key to be used for authentication
//         string    signature

//       The value of 'signature' is a signature by the corresponding private
//       key over the following data, in the following order:

//         string    session identifier
//         byte      SSH_MSG_USERAUTH_REQUEST
//         string    user name
//         string    service
//         string    "publickey"
//         boolean   TRUE
//         string    public key algorithm name
//         string    public key to be used for authentication

//       When the server receives this message, it MUST check whether the
//       supplied key is acceptable for authentication, and if so, it MUST
//       check whether the signature is correct.

//       If both checks succeed, this method is successful.  Note that the
//       server may require additional authentications.  The server MUST
//       respond with SSH_MSG_USERAUTH_SUCCESS (if no more authentications are
//       needed), or SSH_MSG_USERAUTH_FAILURE (if the request failed, or more
//       authentications are needed).

//       The following method-specific message numbers are used by the
//       publickey authentication method.

//         SSH_MSG_USERAUTH_PK_OK              60

/**
 * @brief Try to do public key authentication with ssh agent.
 *
 * @param[in]  client_session  The ssh session to use.
 *
 * @param[in]  username The username, this SHOULD be nullptr.
 *
 * @return  SSH_AUTH_ERROR:   A serious error happened.\n
 *          SSH_AUTH_DENIED:  The server doesn't accept that public key as an
 *                            authentication token. Try another key or another
 *                            method.\n
 *          SSH_AUTH_PARTIAL: You've been partially authenticated, you still
 *                            have to use another method.\n
 *          SSH_AUTH_SUCCESS: The public key is accepted.
 *          SSH_AUTH_AGAIN:   In nonblocking mode, you've got to call this again
 *                            later.
 *
 * @note Most server implementations do not permit changing the username during
 * authentication. The username should only be set with ssh_options_set() only
 * before you connect to the server.
 */
int ssh_userauth_agent_client(SshClientSession * client_session, SshServerSession * front_session, const char *username, error_struct * error) {
    (void)error;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO, "%s session=%p front_session=%p username=%p agent=%p---",
        __FUNCTION__,
        static_cast<void*>(client_session),
        static_cast<void*>(front_session),
        reinterpret_cast<const void*>(username),
        static_cast<void*>(client_session->agent));

    if (!client_session->agent) {
        syslog(LOG_INFO, "No agent in session %s ---", __FUNCTION__);
        return SSH_AUTH_DENIED;
    }
    if (!client_session->agent->agent_is_running(client_session)) {
        syslog(LOG_INFO, "agent not running %s ---", __FUNCTION__);
        return SSH_AUTH_DENIED;
    }

    // C'est un automate d'etat a la con. On peut sortir de la boucle while et rentrer dedans
    // c'est dans ce cas que servent les tests sur agent_state et pubkey
    // on conserve juste les valeurs entre deux appels
    if (!client_session->agent_state){
        client_session->agent_state = new ssh_agent_state_struct;
    }
    if (client_session->agent_state->pubkey == nullptr){
        if (client_session->agent->channel){
            client_session->agent->ssh_agent_get_ident_count_channel_ssh2_server(front_session, front_session->error);
            if (client_session->agent->count > 0) {
                client_session->agent_state->pubkey = client_session->agent->ssh_agent_get_next_ident(&client_session->agent_state->comment);
                client_session->agent->count--;
            }
        }
    }
    while (client_session->agent_state->pubkey != nullptr) {
        switch (client_session->agent_state->state)
        {
        case SSH_AGENT_STATE_NONE:
            syslog(LOG_INFO, "SSH_AGENT_STATE_NONE %s ---", __FUNCTION__);
            syslog(LOG_DEBUG, "Trying identity %s", client_session->agent_state->comment);
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_AGENT_STATE_PUBKEY:
        {
            syslog(LOG_INFO, "SSH_AGENT_STATE_PUBKEY %s ---", __FUNCTION__);
            enum ssh_auth_e rc = ssh_userauth_try_publickey_client(client_session, username, client_session->agent_state->pubkey);
            switch (rc){
            case SSH_AUTH_ERROR:
                syslog(LOG_INFO, "%s SSH_AGENT_STATE : try_publickey said AUTH_ERROR ---", __FUNCTION__);
//                free(client_session->agent_state->comment);
                ssh_key_free(client_session->agent_state->pubkey);
                delete client_session->agent_state;
                client_session->agent_state = nullptr;
                return SSH_AUTH_ERROR;
            case SSH_AUTH_AGAIN:
                syslog(LOG_INFO, "%s SSH_AGENT_STATE : try_publickey said AUTH_AGAIN ---", __FUNCTION__);
                return SSH_AUTH_AGAIN;
            case SSH_AUTH_SUCCESS:
                syslog(LOG_INFO, "%s SSH_AGENT_STATE : try_publickey said AUTH_SUCCESS ---", __FUNCTION__);
                syslog(LOG_DEBUG, "Public key of %s accepted by server", client_session->agent_state->comment);
                client_session->agent_state->state = SSH_AGENT_STATE_AUTH;
                continue;
            break;
            case SSH_AUTH_INFO:
                syslog(LOG_INFO, "%s SSH_AGENT_STATE : try_publickey said AUTH_INFO ---", __FUNCTION__);
                client_session->agent_state->state = SSH_AGENT_STATE_PUBKEY;
                return SSH_AUTH_AGAIN;
            case SSH_AUTH_PARTIAL:
                syslog(LOG_INFO, "%s SSH_AGENT_STATE : try_publickey said AUTH_PARTIAL ---", __FUNCTION__);
                client_session->agent_state->state = SSH_AGENT_STATE_PUBKEY;
                return SSH_AUTH_AGAIN;
            case SSH_AUTH_DENIED:
                syslog(LOG_INFO, "%s SSH_AGENT_STATE : try_publickey said AUTH_DENIED ---", __FUNCTION__);
                syslog(LOG_DEBUG, "Public key of %s refused by server", client_session->agent_state->comment);
                free(client_session->agent_state->comment);
                ssh_key_free(client_session->agent_state->pubkey);
                if (client_session->agent->count > 0){
                    client_session->agent_state->pubkey = client_session->agent->ssh_agent_get_next_ident(&client_session->agent_state->comment);
                    client_session->agent->count--;
                }
                syslog(LOG_INFO, "Public key next_ident done");
                client_session->agent_state->state = SSH_AGENT_STATE_NONE;
            } // AUTH_RESULT
        }
        break;
        case SSH_AGENT_STATE_AUTH:
        {
            syslog(LOG_INFO, "SSH_AGENT_STATE_AUTH %s ---", __FUNCTION__);

            switch(client_session->pending_call_state) {
            case SSH_PENDING_CALL_NONE:
            {
                syslog(LOG_INFO, "A: %s %s--- ", __FUNCTION__, username);
                int rc = ssh_send_service_request_client(client_session, "ssh-userauth");
                if (rc == SSH_AGAIN) {
                    syslog(LOG_INFO, "B: %s ---", __FUNCTION__);
                    return SSH_AUTH_AGAIN;
                } else if (rc == SSH_ERROR) {
                    syslog(LOG_INFO, "C: %s ---", __FUNCTION__);
                    syslog(LOG_WARNING,
                        "Failed to request \"ssh-userauth\" service");
                    free(client_session->agent_state->comment);
                    ssh_key_free(client_session->agent_state->pubkey);
                    delete client_session->agent_state;
                    client_session->agent_state = nullptr;
                    return SSH_AUTH_ERROR;
                }

                const ssh_key_struct * pubkey = client_session->agent_state->pubkey;
                SSHString pubkey_blob(0);

                switch (pubkey->type) {
                    case SSH_KEYTYPE_DSS:
                    {
                        ssh_buffer_struct buffer;
                        buffer.out_length_prefixed_cstr(pubkey->type_c());
                        syslog(LOG_INFO, "%s SSH_KEYTYPE_DSS", __FUNCTION__);
                        buffer.out_bignum(pubkey->dsa->p); // p
                        buffer.out_bignum(pubkey->dsa->q); // q
                        buffer.out_bignum(pubkey->dsa->g); // g
                        buffer.out_bignum(pubkey->dsa->pub_key); // n
                        pubkey_blob = SSHString(static_cast<uint32_t>(buffer.in_remain()));
                        memcpy(pubkey_blob.data.get(), buffer.get_pos_ptr(), pubkey_blob.size);
                    }
                    break;
                    case SSH_KEYTYPE_RSA:
                    case SSH_KEYTYPE_RSA1:
                    {
                        ssh_buffer_struct buffer;
                        buffer.out_length_prefixed_cstr(pubkey->type_c());
                        syslog(LOG_INFO, "%s SSH_KEYTYPE_RSA", __FUNCTION__);
                        buffer.out_bignum(pubkey->rsa->e); // e
                        buffer.out_bignum(pubkey->rsa->n); // n
                        pubkey_blob = SSHString(static_cast<uint32_t>(buffer.in_remain()));
                        memcpy(pubkey_blob.data.get(), buffer.get_pos_ptr(), pubkey_blob.size);
                    }
                    break;
                    case SSH_KEYTYPE_ECDSA:
                    {
                        syslog(LOG_INFO, "%s SSH_KEYTYPE_ECDSA", __FUNCTION__);
                        ssh_buffer_struct buffer;
                        buffer.out_length_prefixed_cstr(pubkey->type_c());

                        buffer.out_length_prefixed_cstr(
                            (pubkey->ecdsa_nid == NID_X9_62_prime256v1) ? "nistp256" :
                            (pubkey->ecdsa_nid == NID_secp384r1)        ? "nistp384" :
                            (pubkey->ecdsa_nid == NID_secp521r1)        ? "nistp521" :
                            "unknown");

                        const EC_GROUP *g = EC_KEY_get0_group(pubkey->ecdsa);
                        const EC_POINT *p = EC_KEY_get0_public_key(pubkey->ecdsa);

                        size_t len_ec = EC_POINT_point2oct(g, p, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
                        if (len_ec == 0) {
                            return SSH_AUTH_ERROR;
                        }

                        SSHString e(static_cast<uint32_t>(len_ec));
                        if (e.size != EC_POINT_point2oct(g, p, POINT_CONVERSION_UNCOMPRESSED, e.data.get(), e.size, nullptr)){
                            return SSH_AUTH_ERROR;
                        }

                        buffer.out_uint32_be(e.size);
                        buffer.out_blob(e.data.get(), e.size);
                        pubkey_blob = SSHString(static_cast<uint32_t>(buffer.in_remain()));
                        memcpy(pubkey_blob.data.get(), buffer.get_pos_ptr(), pubkey_blob.size);
                    }
                    break;
                    case SSH_KEYTYPE_UNKNOWN:
                        syslog(LOG_INFO, "%s SSH_KEYTYPE_UNKNOWN", __FUNCTION__);
                }



                client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_REQUEST); /* request */
                client_session->out_buffer->out_length_prefixed_cstr(username); /* username */
                client_session->out_buffer->out_length_prefixed_cstr("ssh-connection"); /* service */
                client_session->out_buffer->out_length_prefixed_cstr("publickey"); /* method */
                client_session->out_buffer->out_uint8(1);                          /* signed */
                client_session->out_buffer->out_length_prefixed_cstr(client_session->agent_state->pubkey->type_c());
                /* public key */
                client_session->out_buffer->out_uint32_be(pubkey_blob.size);
                client_session->out_buffer->out_blob(pubkey_blob.data.get(), pubkey_blob.size);

                /* sign the buffer with the private key */
                struct ssh_crypto_struct *crypto = client_session->current_crypto
                                                 ? client_session->current_crypto
                                                 : client_session->next_crypto
                                                 ;

                // ssh_agent_sign_data_channel
                ssh_buffer_struct* reply;
                int type = SSH2_AGENT_FAILURE;
                int flags = 0;

                std::unique_ptr<ssh_buffer_struct> request(new ssh_buffer_struct);

                request->out_uint8(SSH2_AGENTC_SIGN_REQUEST);
                /* public key */
                request->out_uint32_be(pubkey_blob.size);
                request->out_blob(pubkey_blob.data.get(), pubkey_blob.size);

                ssh_buffer_struct sig_buf;
                sig_buf.out_uint32_be(crypto->digest_len);
                sig_buf.out_blob(client_session->session_id, crypto->digest_len);
                sig_buf.buffer_add_buffer(client_session->out_buffer);

                request->out_uint32_be(sig_buf.in_remain());
                request->buffer_add_buffer(&sig_buf);
                request->out_uint32_be(flags);

                hexdump(reinterpret_cast<char*>(request->get_pos_ptr()), request->in_remain(), 16);

                reply = new ssh_buffer_struct;

                /* send the request */
                if (client_session->agent->agent_talk_channel_server(front_session, &(*request), reply, front_session->error) < 0) {
                    syslog(LOG_INFO, "%s back from agent talk channel 1 error ---", __FUNCTION__);

                    delete reply;
                    free(client_session->agent_state->comment);
                    ssh_key_free(client_session->agent_state->pubkey);
                    delete client_session->agent_state;
                    client_session->agent_state = nullptr;
                    return SSH_AUTH_ERROR;
                }
                syslog(LOG_INFO, "%s back from agent talk channel 1 ---", __FUNCTION__);

                /* check if reply is valid */
                type = reply->in_uint8();

                syslog(LOG_INFO, "%s back from agent talk channel 2 type=%u ---", __FUNCTION__, type);

                switch (type){
                case SSH2_AGENT_SIGN_RESPONSE:
                break;
                case SSH_AGENT_FAILURE:
                case SSH_COM_AGENT2_FAILURE:
                case SSH2_AGENT_FAILURE:
                    syslog(LOG_WARNING, "Agent reports failure in signing the key");
                    free(client_session->agent_state->comment);
                    ssh_key_free(client_session->agent_state->pubkey);
                    delete client_session->agent_state;
                    client_session->agent_state = nullptr;
                    return SSH_AUTH_ERROR;
                default:
                    ssh_set_error(front_session->error, SSH_FATAL, "Bad authentication response: %d", type);
                    free(client_session->agent_state->comment);
                    ssh_key_free(client_session->agent_state->pubkey);
                    delete client_session->agent_state;
                    client_session->agent_state = nullptr;
                    return SSH_AUTH_ERROR;
                }

                if (sizeof(uint32_t) > reply->in_remain()) {
                    // ERRRRRRRRRRRRRRRRRRRRRRRRRR
                }
                uint32_t sig_blob_len = reply->in_uint32_be();
                if (sig_blob_len > reply->in_remain()) {
                    // ERRRRRRRRRRRRRRRRRRRRRRRRRR
                }
                SSHString sig_blob(sig_blob_len);
                reply->buffer_get_data(sig_blob.data.get(),sig_blob_len);

                syslog(LOG_INFO, "%s back from agent talk channel 3 sig_blob_len =%u ---", __FUNCTION__, sig_blob_len);


                delete reply;

                client_session->out_buffer->out_uint32_be(sig_blob.size);
                client_session->out_buffer->out_blob(sig_blob.data.get(), sig_blob.size);

                client_session->auth_state = SSH_AUTH_STATE_NONE;
                client_session->pending_call_state = SSH_PENDING_CALL_AUTH_AGENT;
                client_session->packet_send();
                rc = ssh_userauth_get_response_client(client_session);
                if (rc != SSH_AUTH_AGAIN) {
                    return rc;
                }
                client_session->pending_call_state = SSH_PENDING_CALL_NONE;
                syslog(LOG_INFO, "J: %s ---", __FUNCTION__);
                return rc;
            }
            case SSH_PENDING_CALL_AUTH_AGENT:
            {
                syslog(LOG_INFO, "K: %s ---", __FUNCTION__);
                int rc = ssh_userauth_get_response_client(client_session);
                client_session->pending_call_state = SSH_PENDING_CALL_NONE;
                syslog(LOG_INFO, "L: %s ---", __FUNCTION__);
                switch (rc){
                case SSH_AUTH_AGAIN:
                    return SSH_AUTH_AGAIN;
                case SSH_AUTH_ERROR:
                    free(client_session->agent_state->comment);
                    ssh_key_free(client_session->agent_state->pubkey);
                    delete client_session->agent_state;
                    client_session->agent_state = nullptr;
                    return SSH_AUTH_ERROR;
                case SSH_AUTH_SUCCESS:
                    free(client_session->agent_state->comment);
                    ssh_key_free(client_session->agent_state->pubkey);
                    delete client_session->agent_state;
                    client_session->agent_state = nullptr;
                    return SSH_AUTH_SUCCESS;
                break;
//                case SSH_ERROR:
                default:
                    syslog(LOG_INFO, "Server accepted public key but refused the signature");
                    if (client_session->agent->count > 0){
                        client_session->agent_state->pubkey = client_session->agent->ssh_agent_get_next_ident(&client_session->agent_state->comment);
                        client_session->agent->count--;
                    }
                    client_session->agent_state->state = SSH_AGENT_STATE_NONE;
                    continue;
                }
            }
            case SSH_PENDING_CALL_AUTH_NONE:
                REDEMPTION_CXX_FALLTHROUGH;
            case SSH_PENDING_CALL_AUTH_PASSWORD:
                REDEMPTION_CXX_FALLTHROUGH;
            case SSH_PENDING_CALL_AUTH_OFFER_PUBKEY:
                REDEMPTION_CXX_FALLTHROUGH;
            case SSH_PENDING_CALL_AUTH_PUBKEY:
                REDEMPTION_CXX_FALLTHROUGH;
            case SSH_PENDING_CALL_AUTH_KBDINT_INIT:
                REDEMPTION_CXX_FALLTHROUGH;
            case SSH_PENDING_CALL_AUTH_KBDINT_SEND:
                REDEMPTION_CXX_FALLTHROUGH;
            case SSH_PENDING_CALL_AUTH_GSSAPI_MIC:
                REDEMPTION_CXX_FALLTHROUGH;
            case SSH_PENDING_CALL_CONNECT:
                REDEMPTION_CXX_FALLTHROUGH;
            default:
                ;
            }
        }
        REDEMPTION_CXX_FALLTHROUGH;
        default:;
            syslog(LOG_INFO, "SSH_AGENT_STATE ???? DEFAULT %s ---", __FUNCTION__);

            // Should not happen
        }
    }

    delete client_session->agent_state;
    client_session->agent_state = nullptr;
    return SSH_AUTH_ERROR;
}


/**
 * @brief Try to authenticate by password.
 *
 * This authentication method is normally disabled on SSHv2 server. You should
 * use keyboard-interactive mode.
 *
 * The 'password' value MUST be encoded UTF-8.  It is up to the server how to
 * interpret the password and validate it against the password database.
 * However, if you read the password in some other encoding, you MUST convert
 * the password to UTF-8.
 *
 * @param[in] client_session   The ssh session to use.
 *
 * @param[in] username  The username, this SHOULD be nullptr.
 *
 * @param[in] password  The password to authenticate in UTF-8.
 *
 * @returns SSH_AUTH_ERROR:   A serious error happened.\n
 *          SSH_AUTH_DENIED:  Authentication failed: use another method\n
 *          SSH_AUTH_PARTIAL: You've been partially authenticated, you still
 *                            have to use another method\n
 *          SSH_AUTH_SUCCESS: Authentication success\n
 *          SSH_AUTH_AGAIN:   In nonblocking mode, you've got to call this again
 *                            later.
 *
 * @note Most server implementations do not permit changing the username during
 * authentication. The username should only be set with ssh_options_set() only
 * before you connect to the server.
 *
 * @see ssh_userauth_none()
 * @see ssh_userauth_kbdint()
 */
int ssh_userauth_password_client(SshClientSession * client_session,
                          const char *username,
                          const char *password,
                          error_struct * error) {
    (void)error;
    int rc;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    switch(client_session->pending_call_state) {
        case SSH_PENDING_CALL_NONE:
            break;
        case SSH_PENDING_CALL_AUTH_OFFER_PUBKEY:
            rc = ssh_userauth_get_response_client(client_session);
            if (rc != SSH_AUTH_AGAIN) {
                client_session->pending_call_state = SSH_PENDING_CALL_NONE;
            }
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_GSSAPI_MIC:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_KBDINT_SEND:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_KBDINT_INIT:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_AGENT:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_PUBKEY:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_PASSWORD:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_AUTH_NONE:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_PENDING_CALL_CONNECT:
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            ssh_set_error(client_session->error,
                          SSH_FATAL,
                          "Wrong state during pending SSH call");
            return SSH_ERROR;
    }

    rc = ssh_send_service_request_client(client_session, "ssh-userauth");
    if (rc == SSH_AGAIN) {
        return SSH_AUTH_AGAIN;
    } else if (rc == SSH_ERROR) {
        syslog(LOG_WARNING,
            "Failed to request \"ssh-userauth\" service");
        return SSH_AUTH_ERROR;
    }

    /* request */
    client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_REQUEST);

    /* username */
    {
        const char * str_username = username?username:client_session->opts.username;
        // TODO : check username
        client_session->out_buffer->out_length_prefixed_cstr(str_username);
    }

    /* service */
    client_session->out_buffer->out_length_prefixed_cstr("ssh-connection");

    /* method */
    client_session->out_buffer->out_length_prefixed_cstr("password");

    /* FALSE */
    client_session->out_buffer->out_uint8(0);

    /* password */
    client_session->out_buffer->out_length_prefixed_cstr(password);

    client_session->auth_state = SSH_AUTH_STATE_NONE;
    client_session->pending_call_state = SSH_PENDING_CALL_AUTH_OFFER_PUBKEY;
    client_session->packet_send();

    rc = ssh_userauth_get_response_client(client_session);
    if (rc != SSH_AUTH_AGAIN) {
        client_session->pending_call_state = SSH_PENDING_CALL_NONE;
    }

    return rc;
}

/*
 * This function sends the first packet as explained in RFC 3066 section 3.1.
 */
static int ssh_userauth_kbdint_init_client(SshClientSession * client_session,
                                    const char *username,
                                    const char *submethods)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int rc;
    switch (client_session->pending_call_state){
    case SSH_PENDING_CALL_NONE:
        rc = ssh_send_service_request_client(client_session, "ssh-userauth");
        if (rc == SSH_AGAIN) {
            return SSH_AUTH_AGAIN;
        }
        else if (rc == SSH_ERROR) {
            syslog(LOG_WARNING, "Failed to request \"ssh-userauth\" service");
            return SSH_AUTH_ERROR;
        }

        /* request */
        client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_REQUEST);

        /* username */
        {
            const char * str_username = username?username:client_session->opts.username;
            if (str_username == nullptr){
                client_session->out_buffer->buffer_reinit();
                return SSH_AUTH_ERROR;
            }
            client_session->out_buffer->out_length_prefixed_cstr(str_username);
        }
        /* service */
        client_session->out_buffer->out_length_prefixed_cstr("ssh-connection");
        /* method */
        client_session->out_buffer->out_length_prefixed_cstr("keyboard-interactive");
        /* lang string (ignore it) */
        client_session->out_buffer->out_length_prefixed_cstr("");
        /* submethods */
        client_session->out_buffer->out_length_prefixed_cstr(submethods?submethods:"");
        client_session->auth_state = SSH_AUTH_STATE_KBDINT_SENT;
        client_session->pending_call_state = SSH_PENDING_CALL_AUTH_KBDINT_INIT;

        syslog(LOG_DEBUG,
                "Sending keyboard-interactive init request");

        client_session->packet_send();
        REDEMPTION_CXX_FALLTHROUGH;
    // fallbak to next case
    case SSH_PENDING_CALL_AUTH_KBDINT_INIT:
        rc = ssh_userauth_get_response_client(client_session);
        if (rc != SSH_AUTH_AGAIN){
            client_session->pending_call_state = SSH_PENDING_CALL_NONE;
        }
        return rc;
    case SSH_PENDING_CALL_AUTH_OFFER_PUBKEY:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_AGENT:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_NONE:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_PASSWORD:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_GSSAPI_MIC:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_PUBKEY:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_KBDINT_SEND:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_CONNECT:
    REDEMPTION_CXX_FALLTHROUGH;
    default:
        ssh_set_error(client_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return SSH_AUTH_ERROR;
    }
    return SSH_AUTH_ERROR;
}


/**
 * @internal
 *
 * @brief Send the current challenge response and wait for a reply from the
 *        server.
 *
 * @returns SSH_AUTH_INFO if more info is needed
 * @returns SSH_AUTH_SUCCESS
 * @returns SSH_AUTH_FAILURE
 * @returns SSH_AUTH_PARTIAL
 */
static int ssh_userauth_kbdint_send_client(SshClientSession * client_session)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t i;
    int rc;
    if (client_session->pending_call_state == SSH_PENDING_CALL_AUTH_KBDINT_SEND){
        rc = ssh_userauth_get_response_client(client_session);
        if (rc != SSH_AUTH_AGAIN){
            client_session->pending_call_state = SSH_PENDING_CALL_NONE;
        }
        return rc;
    }
    if (client_session->pending_call_state != SSH_PENDING_CALL_NONE){
        ssh_set_error(client_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return SSH_ERROR;
    }
    client_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_INFO_RESPONSE);
    client_session->out_buffer->out_uint32_be(client_session->kbdint->nprompts);

    for (i = 0; i < client_session->kbdint->nprompts; i++) {
        client_session->out_buffer->out_length_prefixed_cstr(
            (client_session->kbdint->answers && client_session->kbdint->answers[i])?client_session->kbdint->answers[i]:"");
    }

    client_session->auth_state = SSH_AUTH_STATE_KBDINT_SENT;
    client_session->pending_call_state = SSH_PENDING_CALL_AUTH_KBDINT_SEND;
    ssh_kbdint_free(client_session->kbdint);
    client_session->kbdint = nullptr;

    syslog(LOG_DEBUG,
            "Sending keyboard-interactive response packet");

    client_session->packet_send();
    rc = ssh_userauth_get_response_client(client_session);
    if (rc != SSH_AUTH_AGAIN){
        client_session->pending_call_state = SSH_PENDING_CALL_NONE;
    }
    return rc;
}


/**
 * @brief Try to authenticate through the "keyboard-interactive" method.
 *
 * @param[in]  client_session  The ssh session to use.
 *
 * @param[in]  user     The username to authenticate. You can specify nullptr if
 *                      ssh_option_set_username() has been used. You cannot try
 *                      two different logins in a row.
 *
 * @param[in]  submethods Undocumented. Set it to nullptr.
 *
 * @returns SSH_AUTH_ERROR:   A serious error happened\n
 *          SSH_AUTH_DENIED:  Authentication failed : use another method\n
 *          SSH_AUTH_PARTIAL: You've been partially authenticated, you still
 *                            have to use another method\n
 *          SSH_AUTH_SUCCESS: Authentication success\n
 *          SSH_AUTH_INFO:    The server asked some questions. Use
 *                            ssh_userauth_kbdint_getnprompts() and such.\n
 *          SSH_AUTH_AGAIN:   In nonblocking mode, you've got to call this again
 *                            later.
 *
 * @see ssh_userauth_kbdint_getnprompts()
 * @see ssh_userauth_kbdint_getname()
 * @see ssh_userauth_kbdint_getinstruction()
 * @see ssh_userauth_kbdint_getprompt()
 * @see ssh_userauth_kbdint_setanswer()
 */
int ssh_userauth_kbdint_client(SshClientSession * client_session, const char *user, const char *submethods, error_struct * error) {
    (void)error;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int rc = SSH_AUTH_ERROR;

    if (client_session == nullptr) {
        return SSH_AUTH_ERROR;
    }

    if ((client_session->pending_call_state == SSH_PENDING_CALL_NONE && client_session->kbdint == nullptr) ||
            client_session->pending_call_state == SSH_PENDING_CALL_AUTH_KBDINT_INIT)
        rc = ssh_userauth_kbdint_init_client(client_session, user, submethods);
    else if (client_session->pending_call_state == SSH_PENDING_CALL_AUTH_KBDINT_SEND ||
            client_session->kbdint != nullptr) {
        /*
         * If we are at this point, it is because client_session->kbdint exists.
         * It means the user has set some information there we need to send
         * the server and then we need to ack the status (new questions or ok
         * pass in).
         * It is possible that client_session->kbdint is nullptr while we're waiting for
         * a reply, hence the test for the pending call.
         */
        rc = ssh_userauth_kbdint_send_client(client_session);
    } else {
        /* We are here because client_session->kbdint == nullptr & state != NONE.
         * This should not happen
         */
        rc = SSH_AUTH_ERROR;
        ssh_set_error(client_session->error, SSH_FATAL,"Invalid state in %s", __FUNCTION__);
    }
    return rc;
}

/**
 * @brief Get the number of prompts (questions) the server has given.
 *
 * Once you have called ssh_userauth_kbdint() and received SSH_AUTH_INFO return
 * code, this function can be used to retrieve information about the keyboard
 * interactive authentication questions sent by the remote host.
 *
 * @param[in]  client_session  The ssh session to use.
 *
 * @returns             The number of prompts.
 */
int ssh_userauth_kbdint_getnprompts_client(SshClientSession * client_session) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if(client_session==nullptr){
        return SSH_ERROR;
    }
    if(client_session->kbdint == nullptr) {
        ssh_set_error(client_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return SSH_ERROR;
    }
    return client_session->kbdint->nprompts;
}


/** @brief returns the OIDs of the mechs that have usable credentials
 */
static int ssh_gssapi_match_client(SshClientSession * client_session, gss_OID_set *valid_oids)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    OM_uint32 maj_stat, min_stat, lifetime;
    gss_OID_set actual_mechs;
    gss_buffer_desc namebuf;
    gss_name_t client_id = GSS_C_NO_NAME;
    gss_OID oid;
    unsigned int i;
    int ret;

    if (client_session->gssapi->client.client_deleg_creds == nullptr) {
        if (client_session->opts.gss_client_identity != nullptr) {
            namebuf.value = client_session->opts.gss_client_identity;
            namebuf.length = strlen(client_session->opts.gss_client_identity);

            maj_stat = gss_import_name(&min_stat, &namebuf,
                                       GSS_C_NT_USER_NAME, &client_id);
            if (GSS_ERROR(maj_stat)) {
                ret = SSH_ERROR;
                gss_release_name(&min_stat, &client_id);
                return ret;
            }
        }

        maj_stat = gss_acquire_cred(&min_stat, client_id, GSS_C_INDEFINITE,
                                    GSS_C_NO_OID_SET, GSS_C_INITIATE,
                                    &client_session->gssapi->client.creds,
                                    &actual_mechs, nullptr);
        if (GSS_ERROR(maj_stat)) {
            ret = SSH_ERROR;
            gss_release_name(&min_stat, &client_id);
            return ret;
        }
    } else {
        client_session->gssapi->client.creds =
                                    client_session->gssapi->client.client_deleg_creds;

        maj_stat = gss_inquire_cred(&min_stat, client_session->gssapi->client.creds,
                                    &client_id, nullptr, nullptr, &actual_mechs);
        if (GSS_ERROR(maj_stat)) {
            ret = SSH_ERROR;
            gss_release_name(&min_stat, &client_id);
            return ret;
        }
    }

    gss_create_empty_oid_set(&min_stat, valid_oids);

    /* double check each single cred */
    for (i = 0; i < actual_mechs->count; i++) {
        /* check lifetime is not 0 or skip */
        lifetime = 0;
        oid = &actual_mechs->elements[i];
        maj_stat = gss_inquire_cred_by_mech(&min_stat,
                                            client_session->gssapi->client.creds,
                                            oid, nullptr, &lifetime, nullptr, nullptr);

        if (maj_stat == GSS_S_COMPLETE && lifetime > 0) {
            gss_add_oid_set_member(&min_stat, oid, valid_oids);
            // TODO: avoid too long buffers, we can make this one static and truncate it
            char *hexa = new char[oid->length * 3 + 1];
            size_t q = 0;
            size_t j = 0;
            for (q = 0; q < oid->length; q++) {
                const uint8_t cl = reinterpret_cast<uint8_t *>(oid->elements)[q] >> 4;
                const uint8_t ch = reinterpret_cast<uint8_t *>(oid->elements)[q] & 0x0F;
                hexa[j] = (ch < 10?'0':'a')+ch;
                hexa[j+1] = (cl < 10?'0':'a')+cl;
                hexa[j+2] = ':';
                j+= 3;
            }
            hexa[j>0?(j-1):0] = 0;
            syslog(LOG_DEBUG, "GSSAPI valid oid %d : %s\n", i, hexa);
            delete[] hexa;
        }
    }

    ret = SSH_OK;

    gss_release_name(&min_stat, &client_id);
    return ret;
}


/**
 * @brief launches a gssapi-with-mic auth request
 * @returns SSH_AUTH_ERROR:   A serious error happened\n
 *          SSH_AUTH_DENIED:  Authentication failed : use another method\n
 *          SSH_AUTH_AGAIN:   In nonblocking mode, you've got to call this again
 *                            later.
 */
inline int ssh_gssapi_auth_mic_client(SshClientSession * client_session)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int i;
    gss_OID_set selected; /* oid selected for authentication */
    OM_uint32 maj_stat;
    OM_uint32 min_stat;
    char name_buf[256];
    gss_buffer_desc hostname;
    const char *gss_host = client_session->opts.host;

    if (client_session->gssapi == nullptr){
        client_session->gssapi = new ssh_gssapi_struct;
    }

    if (client_session->opts.gss_server_identity != nullptr) {
        gss_host = client_session->opts.gss_server_identity;
    }
    /* import target host name */
    snprintf(name_buf, sizeof(name_buf), "host@%s", gss_host);

    hostname.value = name_buf;
    hostname.length = strlen(name_buf) + 1;
    maj_stat = gss_import_name(&min_stat, &hostname,
                               GSS_C_NT_HOSTBASED_SERVICE,
                               &client_session->gssapi->client.server_name);
    if (maj_stat != GSS_S_COMPLETE) {
        syslog(LOG_WARNING, "importing name %d, %d", maj_stat, min_stat);
        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_WARNING, "GSSAPI(%s): %s", "importing name", static_cast<char *>(buffer.value));


        return SSH_PACKET_USED;
    }

    /* copy username */
    client_session->gssapi->user = strdup(client_session->opts.username);
    // TODO : check memory allocation

    syslog(LOG_INFO, "Authenticating with gssapi to host %s with user %s",
            client_session->opts.host, client_session->gssapi->user);
    int rc2 = ssh_gssapi_match_client(client_session, &selected);
    if (rc2 == SSH_ERROR) {
        return SSH_AUTH_DENIED;
    }

    int n_oids = selected->count;
    syslog(LOG_INFO, "Sending %d oids", n_oids);

    typedef SSHString * SSHString_pointer;
    SSHString_pointer * oids = new SSHString_pointer[n_oids];

    for (i=0; i<n_oids; ++i){
        oids[i] = new SSHString(selected->elements[i].length + 2);
        oids[i]->data[0] = SSH_OID_TAG;
        oids[i]->data[1] = selected->elements[i].length;
        memcpy(&(oids[i]->data[2]), selected->elements[i].elements, selected->elements[i].length);
    }

    int rc3 = ssh_gssapi_send_auth_mic_client(client_session, oids, n_oids);
    for (i = 0; i < n_oids; i++) {
        free(oids[i]);
    }
    free(oids);
    if (rc3 != SSH_ERROR) {
        return SSH_AUTH_AGAIN;
    }
    return SSH_AUTH_ERROR;
}


/**
 * @brief Try to authenticate through the "gssapi-with-mic" method.
 *
 * @param[in]  client_session  The ssh session to use.
 *
 * @returns SSH_AUTH_ERROR:   A serious error happened\n
 *          SSH_AUTH_DENIED:  Authentication failed : use another method\n
 *          SSH_AUTH_PARTIAL: You've been partially authenticated, you still
 *                            have to use another method\n
 *          SSH_AUTH_SUCCESS: Authentication success\n
 *          SSH_AUTH_AGAIN:   In nonblocking mode, you've got to call this again
 *                            later.
 */
int ssh_userauth_gssapi_client(SshClientSession * client_session, error_struct * error) {
    (void)error;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int rc = SSH_AUTH_DENIED;

    switch(client_session->pending_call_state) {
    case SSH_PENDING_CALL_NONE:
        rc = ssh_send_service_request_client(client_session, "ssh-userauth");
        if (rc == SSH_AGAIN) {
            return SSH_AUTH_AGAIN;
        } else if (rc == SSH_ERROR) {
            syslog(LOG_WARNING,
                "Failed to request \"ssh-userauth\" service");
            return SSH_AUTH_ERROR;
        }
        syslog(LOG_INFO, "Authenticating with gssapi-with-mic");
        client_session->auth_state = SSH_AUTH_STATE_NONE;
        client_session->pending_call_state = SSH_PENDING_CALL_AUTH_GSSAPI_MIC;
        rc = ssh_gssapi_auth_mic_client(client_session);

        if (rc == SSH_AUTH_ERROR || rc == SSH_AUTH_DENIED) {
            client_session->auth_state = SSH_AUTH_STATE_NONE;
            client_session->pending_call_state = SSH_PENDING_CALL_NONE;
            return rc;
        }
        REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_GSSAPI_MIC:
        rc = ssh_userauth_get_response_client(client_session);
        if (rc != SSH_AUTH_AGAIN) {
            client_session->pending_call_state = SSH_PENDING_CALL_NONE;
        }
        return rc;
    case SSH_PENDING_CALL_AUTH_OFFER_PUBKEY:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_AGENT:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_NONE:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_PASSWORD:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_PUBKEY:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_CONNECT:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_KBDINT_SEND:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_PENDING_CALL_AUTH_KBDINT_INIT:
    REDEMPTION_CXX_FALLTHROUGH;
    default:
        ssh_set_error(client_session->error,
                SSH_FATAL,
                "Wrong state during pending SSH call");
        return SSH_ERROR;
    }
    return SSH_AUTH_ERROR;
}


/**
 * @brief Set the answer for a question from a message block.
 *
 * If you have called ssh_userauth_kbdint() and got SSH_AUTH_INFO, this
 * function returns the questions from the server.
 *
 * @param[in]  client_session  The ssh session to use.
 *
 * @param[in]  i index  The number of the ith prompt.
 *
 * @param[in]  answer   The answer to give to the server. The answer MUST be
 *                      encoded UTF-8. It is up to the server how to interpret
 *                      the value and validate it. However, if you read the
 *                      answer in some other encoding, you MUST convert it to
 *                      UTF-8.
 *
 * @return              0 on success, < 0 on error.
 */
int ssh_userauth_kbdint_setanswer_client(SshClientSession * client_session, unsigned int i, const char *answer, error_struct * error)
{
    (void)error;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if (client_session == nullptr){
        return -1;
    }
    if (answer == nullptr
    || client_session->kbdint == nullptr
    || i >= client_session->kbdint->nprompts){
        ssh_set_error(client_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    if (client_session->kbdint->answers == nullptr) {
        client_session->kbdint->answers = static_cast<char**>(malloc(sizeof(char*) * client_session->kbdint->nprompts));
        // TODO : check memory allocation
        memset(client_session->kbdint->answers, 0, sizeof(char *) * client_session->kbdint->nprompts);
    }

    if (client_session->kbdint->answers[i]) {
        free(client_session->kbdint->answers[i]);
        client_session->kbdint->answers[i] = nullptr;
    }

    client_session->kbdint->answers[i] = strdup(answer);
    // TODO : check memory allocation
    return 0;
}


/**
 * @brief Get a prompt from a message block.
 *
 * Once you have called ssh_userauth_kbdint() and received SSH_AUTH_INFO return
 * code, this function can be used to retrieve information about the keyboard
 * interactive authentication questions sent by the remote host.
 *
 * @param[in]  client_session  The ssh session to use.
 *
 * @param[in]  i        The index number of the i'th prompt.
 *
 * @param[out] echo     This is an optional variable. You can obtain a
 *                      boolean if the user input should be echoed or
 *                      hidden. For passwords it is usually hidden.
 *
 * @returns             A pointer to the prompt. Do not free it.
 *
 * @code
 *   const char prompt;
 *   char echo;
 *
 *   prompt = ssh_userauth_kbdint_getprompt(session, 0, &echo);
 *   if (echo) ...
 * @endcode
 */
const char *ssh_userauth_kbdint_getprompt_client(SshClientSession * client_session, unsigned int i, char *echo, error_struct * error)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if (client_session == nullptr){
        return nullptr;
    }
    if (client_session->kbdint == nullptr) {
        ssh_set_error(*error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return nullptr;
    }
    if (i > client_session->kbdint->nprompts) {
        ssh_set_error(*error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return nullptr;
    }

    if (echo) {
        *echo = client_session->kbdint->echo[i];
    }

    return client_session->kbdint->prompts[i];
}




/**
 * @brief Open a session channel (suited for a shell, not TCP forwarding).
 *
 * @param[in]  channel  An allocated channel.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 *
 */

 // TODO: this is client session only ?

int ssh_sessionchannel_open_client(SshClientSession * client_session, ssh_channel channel)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int err = SSH_ERROR;

    if(channel == nullptr) {
        return SSH_ERROR;
    }

    if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_NOT_OPEN){
        return SSH_ERROR;
    }

    channel->channel_open(client_session->new_channel_id(),
        "session", CHANNEL_MAX_PACKET, CHANNEL_INITIAL_WINDOW, client_session->out_buffer);

    client_session->packet_send();
    if (client_session->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    /* wait until channel is opened by server */
    err = SSH_OK;
    while(1) {
        // Waiting for input
        dopoll(client_session->ctx, (client_session->flags & SSH_SESSION_FLAG_BLOCKING)
            ? SSH_TIMEOUT_INFINITE
            : SSH_TIMEOUT_NONBLOCKING);

        if (client_session->session_state == SSH_SESSION_STATE_ERROR){
            return SSH_ERROR;
        }
        if (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING) {
            err = SSH_AGAIN;
            if (!(client_session->flags&SSH_SESSION_FLAG_BLOCKING)){ break; }
            continue;
        }
        err = SSH_OK;
        break;
    }

    if(channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN){
        err=SSH_OK;
    }
    return err;
}



/**
 * @brief Open a TCP/IP forwarding channel.
 *
 * @param[in]  channel  An allocated channel.
 *
 * @param[in]  remotehost The remote host to connected (host name or IP).
 *
 * @param[in]  remoteport The remote port.
 *
 * @param[in]  sourcehost The numeric IP address of the machine from where the
 *                        connection request originates. This is mostly for
 *                        logging purposes.
 *
 * @param[in]  localport  The port on the host from where the connection
 *                        originated. This is mostly for logging purposes.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 *
 * @warning This function does not bind the local port and does not automatically
 *          forward the content of a socket to the channel. You still have to
 *          use channel_read and channel_write for this.
 */
int ssh_channel_open_forward_client(SshClientSession * client_session, ssh_channel channel, const char *remotehost, int remoteport, const char *sourcehost, int localport) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int err = SSH_ERROR;

    if(channel == nullptr) {
        return err;
    }

    if(remotehost == nullptr || sourcehost == nullptr) {
        ssh_set_error(client_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return err;
    }

    if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_NOT_OPEN){
        return SSH_ERROR;
    }
    channel->channel_open(client_session->new_channel_id(),
        "direct-tcpip", CHANNEL_MAX_PACKET, CHANNEL_INITIAL_WINDOW, client_session->out_buffer);

    client_session->out_buffer->out_length_prefixed_cstr(remotehost);
    client_session->out_buffer->out_uint32_be(remoteport);
    client_session->out_buffer->out_length_prefixed_cstr(sourcehost);
    client_session->out_buffer->out_uint32_be(localport);

    client_session->packet_send();

    if (client_session->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    /* wait until channel is opened by server */
    err = SSH_OK;
    while(1) {
        if (client_session->session_state == SSH_SESSION_STATE_ERROR){
            err = SSH_ERROR;
            break;
        }
        if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING){
            err = SSH_OK;
            break;
        }
        if (client_session->socket == nullptr) {
            syslog(LOG_WARNING, "handle_packets early exit : no socket");
            err = SSH_ERROR;
            break;
        }

        // Waiting for input
        dopoll(client_session->ctx, (client_session->flags&SSH_SESSION_FLAG_BLOCKING)
            ? SSH_TIMEOUT_INFINITE
            : SSH_TIMEOUT_NONBLOCKING);

        if (client_session->session_state == SSH_SESSION_STATE_ERROR) {
            break;
        }
        if (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING) {
            err = SSH_AGAIN;
        }
        if (!(client_session->flags&SSH_SESSION_FLAG_BLOCKING)){
            break;
        }
    }

    if(channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN){
        err=SSH_OK;
    }
    return err;
}



//5.3.  Closing a Channel

//   When a party will no longer send more data to a channel, it SHOULD
//   send SSH_MSG_CHANNEL_EOF.

//      byte      SSH_MSG_CHANNEL_EOF
//      uint32    recipient channel

//   No explicit response is sent to this message.  However, the
//   application may send EOF to whatever is at the other end of the
//   channel.  Note that the channel remains open after this message, and
//   more data may still be sent in the other direction.  This message
//   does not consume window space and can be sent even if no window space
//   is available.

//   When either party wishes to terminate the channel, it sends
//   SSH_MSG_CHANNEL_CLOSE.  Upon receiving this message, a party MUST
//   send back an SSH_MSG_CHANNEL_CLOSE unless it has already sent this
//   message for the channel.  The channel is considered closed for a
//   party when it has both sent and received SSH_MSG_CHANNEL_CLOSE, and
//   the party may then reuse the channel number.  A party MAY send
//   SSH_MSG_CHANNEL_CLOSE without having sent or received
//   SSH_MSG_CHANNEL_EOF.

//      byte      SSH_MSG_CHANNEL_CLOSE
//      uint32    recipient channel

//   This message does not consume window space and can be sent even if no
//   window space is available.

//   It is RECOMMENDED that all data sent before this message be delivered
//   to the actual destination, if possible.


int ssh_channel_send_eof_client(SshClientSession * client_session, ssh_channel channel){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int err = SSH_ERROR;

    if(channel == nullptr) {
        return err;
    }

    client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_EOF);
    client_session->out_buffer->out_uint32_be(channel->remote_channel);

    err = client_session->packet_send();
    syslog(LOG_INFO,
            "Sent a EOF on client channel (%d:%d)",
            channel->local_channel,
            channel->remote_channel);

    if (client_session->session_state == SSH_SESSION_STATE_ERROR){
        channel->local_eof = true;
        return SSH_ERROR;
    }

    err = SSH_OK;
    while(1){
        if (client_session->out_buffer->in_remain() == 0){
            break;
        }
        if (client_session->session_state == SSH_SESSION_STATE_ERROR) {
            break;
        }
        if (client_session->socket == nullptr) {
            syslog(LOG_WARNING, "handle_packets early exit : no socket");
            err = SSH_ERROR;
            break;
        }

        // Waiting for input
        dopoll(client_session->ctx, SSH_TIMEOUT_INFINITE);
        if (client_session->session_state == SSH_SESSION_STATE_ERROR) {
            syslog(LOG_WARNING, "handle_packets returned an error");
            break;
        }
    }
    channel->local_eof = true;
    return err;
}


/**
 * @brief Close a channel.
 *
 * This sends an end of file and then closes the channel. You won't be able
 * to recover any data the server was going to send or was in buffers.
 *
 * @param[in]  channel  The channel to close.
 *
 * @return              SSH_OK on success, SSH_ERROR if an error occurred.
 *
 * @see ssh_channel_free()
 * @see ssh_channel_is_eof()
 */
int ssh_channel_close_client(SshClientSession * client_session, ssh_channel channel){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    int err = 0;

    if(channel == nullptr) {
        return SSH_ERROR;
    }

    if (!channel->local_eof) {
        err = ssh_channel_send_eof_client(client_session, channel);
    }

    if (err != SSH_OK) {
        return err;
    }

    client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_CLOSE);
    client_session->out_buffer->out_uint32_be(channel->remote_channel);

    err = client_session->packet_send();
    syslog(LOG_INFO,
            "Sent a close on client channel (%d:%d)",
            channel->local_channel,
            channel->remote_channel);

    if(err == SSH_OK) {
        channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_CLOSED;
    }

    if (client_session->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    err = SSH_OK;
    while(1){
        if (client_session->socket->out_buffer->in_remain() == 0){
            break;
        }
        if (client_session->session_state == SSH_SESSION_STATE_ERROR) {
            break;
        }
        if (client_session->socket == nullptr) {
            syslog(LOG_WARNING, "handle_packets early exit : no socket");
            err = SSH_ERROR;
            break;
        }

        // Waiting for input
        dopoll(client_session->ctx, SSH_TIMEOUT_INFINITE);
        if (client_session->session_state == SSH_SESSION_STATE_ERROR) {
            break;
        }
    }
    return err;
}


/**
 * @brief Close and free a channel.
 *
 * @param[in]  channel  The channel to free.
 *
 * @warning Any data unread on this channel will be lost.
 */
void ssh_channel_free_client(SshClientSession * client_session, ssh_channel channel) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN) {
        ssh_channel_close_client(client_session, channel);
    }
    channel->flags |= SSH_CHANNEL_FLAG_FREED_LOCAL;

    /* The idea behind the flags is the following : it is well possible
     * that a client closes a channel that stills exists on the server side.
     * We definitively close the channel when we receive a close message *and*
     * the user closed it.
     */
    if((channel->flags & SSH_CHANNEL_FLAG_CLOSED_REMOTE)
       || (channel->flags & SSH_CHANNEL_FLAG_NOT_BOUND)){
        for (unsigned i = 0; i < client_session->nbchannels ; i++){
            if (client_session->channels[i] == channel){
                client_session->nbchannels--;
                client_session->channels[i] = client_session->channels[client_session->nbchannels];
                delete channel;
                break;
            }
        }
    }
}


int ssh_get_server_publickey_hash_value_client(const SshClientSession * client_session,
                                        enum ssh_publickey_hash_type type,
                                        unsigned char *buf,
                                        size_t *hlen,
                                        error_struct * error)
{
    (void)error;
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    ssh_key_struct *key;

    ssh_buffer_struct buffer;

    syslog(LOG_INFO, "%s --- A", __FUNCTION__);

    buffer.out_blob(client_session->current_crypto->server_pubkey.data.get(),
                    client_session->current_crypto->server_pubkey.size);

    syslog(LOG_INFO, "%s --- B", __FUNCTION__);

    ssh_pki_import_pubkey_blob(buffer, &key);

    syslog(LOG_INFO, "%s --- C", __FUNCTION__);


    SSHString blob(0);

    syslog(LOG_INFO, "%s --- D", __FUNCTION__);

    switch (key->type) {
        case SSH_KEYTYPE_DSS:
        {
            ssh_buffer_struct buffer;
            buffer.out_length_prefixed_cstr(key->type_c());
            syslog(LOG_INFO, "%s SSH_KEYTYPE_DSS", __FUNCTION__);
            buffer.out_bignum(key->dsa->p); // p
            buffer.out_bignum(key->dsa->q); // q
            buffer.out_bignum(key->dsa->g); // g
            buffer.out_bignum(key->dsa->pub_key); // n
            SSHString str(static_cast<uint32_t>(buffer.in_remain()));
            memcpy(str.data.get(), buffer.get_pos_ptr(), str.size);
            blob = std::move(str);
        }
        break;
        case SSH_KEYTYPE_RSA:
        case SSH_KEYTYPE_RSA1:
        {
            ssh_buffer_struct buffer;
            buffer.out_length_prefixed_cstr(key->type_c());
            syslog(LOG_INFO, "%s SSH_KEYTYPE_RSA", __FUNCTION__);
            buffer.out_bignum(key->rsa->e); // e
            buffer.out_bignum(key->rsa->n); // n
            SSHString str(static_cast<uint32_t>(buffer.in_remain()));
            memcpy(str.data.get(), buffer.get_pos_ptr(), str.size);
            blob = std::move(str);
        }
        break;
        case SSH_KEYTYPE_ECDSA:
        {
            syslog(LOG_INFO, "%s SSH_KEYTYPE_ECDSA", __FUNCTION__);
            ssh_buffer_struct buffer;
            buffer.out_length_prefixed_cstr(key->type_c());

            buffer.out_length_prefixed_cstr(
                (key->ecdsa_nid == NID_X9_62_prime256v1) ? "nistp256" :
                (key->ecdsa_nid == NID_secp384r1)        ? "nistp384" :
                (key->ecdsa_nid == NID_secp521r1)        ? "nistp521" :
                "unknown");

            const EC_GROUP *g = EC_KEY_get0_group(key->ecdsa);
            const EC_POINT *p = EC_KEY_get0_public_key(key->ecdsa);

            size_t len_ec = EC_POINT_point2oct(g, p, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
            if (len_ec == 0) {
                return SSH_ERROR;
            }

            SSHString e(static_cast<uint32_t>(len_ec));
            if (e.size != EC_POINT_point2oct(g, p, POINT_CONVERSION_UNCOMPRESSED, e.data.get(), e.size, nullptr)){
                return SSH_ERROR;
            }

            buffer.out_uint32_be(e.size);
            buffer.out_blob(e.data.get(), e.size);
            SSHString str(static_cast<uint32_t>(buffer.in_remain()));
            memcpy(str.data.get(), buffer.get_pos_ptr(), str.size);
            blob = std::move(str);
        }
        break;
        case SSH_KEYTYPE_UNKNOWN:
            syslog(LOG_INFO, "%s SSH_KEYTYPE_UNKNOWN", __FUNCTION__);
            return SSH_ERROR;
    }


    switch (type) {
    case SSH_PUBLICKEY_HASH_SHA1:
    {
        SslSha1 sha1;
        sha1.update(blob.data.get(), blob.size);
        sha1.final(buf);
        if (hlen){
            *hlen = SHA_DIGEST_LENGTH;
        }
    }
    break;
    case SSH_PUBLICKEY_HASH_MD5:
    {
        SslMd5 md5;
        md5.update(blob.data.get(), blob.size);
        md5.final(buf);
        if (hlen){
            *hlen = SslMd5::DIGEST_LENGTH;
        }
    }
    break;
    default:
        return SSH_ERROR;
    }
    free(key);
    return SSH_OK;
}

/** @brief sets the SSH agent channel.
 * The SSH agent channel will be used to authenticate this client using
 * an agent through a channel, from another session. The most likely use
 * is to implement SSH Agent forwarding into a SSH proxy.
 * @param[in] channel a SSH channel from another session.
 * @returns SSH_OK in case of success
 *          SSH_ERROR in case of an error
 */
int ssh_set_agent_channel_client(SshClientSession * client_session, ssh_channel channel)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if (!client_session){
        return SSH_ERROR;
    }
    if (!client_session->agent){
        syslog(LOG_INFO, "NO AGENT %s ---", __FUNCTION__);
        ssh_set_error(client_session->error,  SSH_REQUEST_DENIED, "Session has no active agent");
        return SSH_ERROR;
    }
    client_session->agent->set_channel(channel);
    return SSH_OK;
}


/**
 * @brief Set the forwadable ticket to be given to the server for authentication.
 *
 * @param[in] creds gssapi credentials handle.
 */
void ssh_gssapi_set_creds_client(SshClientSession * client_session, const ssh_gssapi_creds creds)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if (client_session->gssapi == nullptr){
        client_session->gssapi = new ssh_gssapi_struct;
    }
    client_session->gssapi->client.client_deleg_creds = static_cast<gss_cred_id_t>(creds);
}

int ssh_channel_request_env_client(SshClientSession * client_session, ssh_channel channel, const char *name, const char *value)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO, ">>>>>>>>>>>> CHANNEL_REQUEST_ENV <%s=%s> %s", name, value, channel->show());
    int rc = SSH_ERROR;

    if(channel == nullptr) {
        return SSH_ERROR;
    }
    if(name == nullptr || value == nullptr) {
        ssh_set_error(client_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return rc;
    }

    switch(channel->request_state){
    case SSH_CHANNEL_REQ_STATE_NONE:
    {
        uint8_t want_reply = 1;

        client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
        client_session->out_buffer->out_uint32_be(channel->remote_channel);
        client_session->out_buffer->out_length_prefixed_cstr("env");
        client_session->out_buffer->out_uint8(want_reply);

        client_session->out_buffer->out_length_prefixed_cstr(name);
        client_session->out_buffer->out_length_prefixed_cstr(value);

        client_session->packet_send();
        if (!want_reply) {
            channel->request_state = SSH_CHANNEL_REQ_STATE_NONE;
            return SSH_OK;
        }
        else {
            channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
            return channel->channel_request(client_session);
        }
    }
    break;
    case SSH_CHANNEL_REQ_STATE_PENDING:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_DENIED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ACCEPTED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ERROR:
    REDEMPTION_CXX_FALLTHROUGH;

    default:
    break;
    }

    return channel->channel_request(client_session);
}



/**
 * @brief Sends the "x11-req" channel request over an existing session channel.
 *
 * This will enable redirecting the display of the remote X11 applications to
 * local X server over an secure tunnel.
 *
 * @param[in]  channel  An existing session channel where the remote X11
 *                      applications are going to be executed.
 *
 * @param[in]  single_connection A boolean to mark only one X11 app will be
 *                               redirected.
 *
 * @param[in]  protocol A x11 authentication protocol. Pass nullptr to use the
 *                      default value MIT-MAGIC-COOKIE-1.
 *
 * @param[in]  cookie   A x11 authentication cookie. Pass nullptr to generate
 *                      a random cookie.
 *
 * @param[in] screen_number The screen number.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 */
int ssh_channel_request_x11_client(SshClientSession * client_session, ssh_channel channel, int single_connection, const char *protocol, const char *cookie, int screen_number)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO, ">>>>>>>>>>>> CHANNEL_REQUEST_X11 <%s %s> %s",
        protocol, cookie, channel->show());
    if(channel == nullptr) {
        return SSH_ERROR;
    }

    switch(channel->request_state){
    case SSH_CHANNEL_REQ_STATE_NONE:
    {
        uint8_t want_reply = 1;

        client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
        client_session->out_buffer->out_uint32_be(channel->remote_channel);
        client_session->out_buffer->out_length_prefixed_cstr("x11-req");
        client_session->out_buffer->out_uint8(want_reply);

        client_session->out_buffer->out_uint8(single_connection == 0 ? 0 : 1);
        client_session->out_buffer->out_length_prefixed_cstr(protocol ? protocol : "MIT-MAGIC-COOKIE-1");

        if (cookie) {
            client_session->out_buffer->out_length_prefixed_cstr(cookie);
        }
        else {
            // TODO: check that, randomness of that cookie is dubious
            static const char *hex = "0123456789abcdef";
            char newcookie[36];
            unsigned char rnd[16];
            int i;

            RAND_pseudo_bytes(rnd, sizeof(rnd));
            for (i = 0; i < 16; i++) {
                newcookie[i*2] = hex[rnd[i] & 0x0f];
                newcookie[i*2+1] = hex[rnd[i] >> 4];
            }
            newcookie[32] = '\0';
            client_session->out_buffer->out_length_prefixed_cstr(newcookie);
        }

        client_session->out_buffer->out_uint32_be(screen_number);

        client_session->packet_send();
        if (!want_reply) {
            channel->request_state = SSH_CHANNEL_REQ_STATE_NONE;
            return SSH_OK;
        }
        else {
            channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
            return channel->channel_request(client_session);
        }
    }
    break;
    case SSH_CHANNEL_REQ_STATE_PENDING:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_DENIED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ACCEPTED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ERROR:
    REDEMPTION_CXX_FALLTHROUGH;

    default:
    break;
    }
    return channel->channel_request(client_session);
}


/**
 * @brief Send a signal to remote process (as described in RFC 4254, section 6.9).
 *
 * Sends a signal 'sig' to the remote process.
 * Note, that remote system may not support signals concept.
 * In such a case this request will be silently ignored.
 * Only SSH-v2 is supported (I'm not sure about SSH-v1).
 *
 * OpenSSH doesn't support signals yet, see:
 * https://bugzilla.mindrot.org/show_bug.cgi?id=1424
 *
 * @param[in]  channel  The channel to send signal.
 *
 * @param[in]  sig      The signal to send (without SIG prefix)
 *                      \n\n
 *                      SIGABRT  -> ABRT \n
 *                      SIGALRM  -> ALRM \n
 *                      SIGFPE   -> FPE  \n
 *                      SIGHUP   -> HUP  \n
 *                      SIGILL   -> ILL  \n
 *                      SIGINT   -> INT  \n
 *                      SIGKILL  -> KILL \n
 *                      SIGPIPE  -> PIPE \n
 *                      SIGQUIT  -> QUIT \n
 *                      SIGSEGV  -> SEGV \n
 *                      SIGTERM  -> TERM \n
 *                      SIGUSR1  -> USR1 \n
 *                      SIGUSR2  -> USR2 \n
 *
 * @return              SSH_OK on success, SSH_ERROR if an error occurred
 *                      (including attempts to send signal via SSH-v1 session).
 */
int ssh_channel_request_send_signal_client(SshClientSession * client_session, ssh_channel channel, const char *sig)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO, ">>>>>>>>>>> CHANNEL_REQUEST_SEND_SIGNAL %s", channel->show());
    int rc = SSH_ERROR;

    if(channel == nullptr) {
        return SSH_ERROR;
    }
    if(sig == nullptr) {
        ssh_set_error(client_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return rc;
    }

    switch(channel->request_state){
    case SSH_CHANNEL_REQ_STATE_NONE:
    {
        uint8_t want_reply = 0;
        client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
        client_session->out_buffer->out_uint32_be(channel->remote_channel);
        client_session->out_buffer->out_length_prefixed_cstr("signal");
        client_session->out_buffer->out_uint8(want_reply);

        client_session->out_buffer->out_length_prefixed_cstr(sig);
        channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
        client_session->packet_send();
        if (!want_reply) {
            channel->request_state = SSH_CHANNEL_REQ_STATE_NONE;
            return SSH_OK;
        }
        else {
            channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
            return channel->channel_request(client_session);
        }
    }
    break;
    case SSH_CHANNEL_REQ_STATE_PENDING:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_DENIED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ACCEPTED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ERROR:
    REDEMPTION_CXX_FALLTHROUGH;

    default:
        break;
    }
    rc = channel->channel_request(client_session);

    return rc;
}

/**
 * @brief Change the size of the terminal associated to a channel.
 *
 * @param[in]  channel  The channel to change the size.
 * @param[in]  cols     The new number of columns.
 * @param[in]  rows     The new number of rows.
 * @return              SSH_OK on success, SSH_ERROR if an error occurred.
 *
 * @warning Do not call it from a signal handler if you are not sure any other
 *          libssh function using the same channel/session is running at same
 *          time (not 100% threadsafe).
 */
int ssh_channel_change_pty_size_client(SshClientSession * client_session, ssh_channel channel, int cols, int rows) {
    syslog(LOG_INFO, "%s --- (%d, %d)", __FUNCTION__, cols, rows);
    int rc = SSH_ERROR;

    switch(channel->request_state){
    case SSH_CHANNEL_REQ_STATE_NONE:
    {
        uint8_t want_reply = 0;

        client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
        client_session->out_buffer->out_uint32_be(channel->remote_channel);
        client_session->out_buffer->out_length_prefixed_cstr("window-change");
        client_session->out_buffer->out_uint8(want_reply);

        client_session->out_buffer->out_uint32_be(cols);
        client_session->out_buffer->out_uint32_be(rows);
        client_session->out_buffer->out_uint32_be(0);
        client_session->out_buffer->out_uint32_be(0);
        client_session->packet_send();
        if (!want_reply) {
            channel->request_state = SSH_CHANNEL_REQ_STATE_NONE;
            return SSH_OK;
        }
        else {
            channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
            return channel->channel_request(client_session);
        }
    }
    break;
    case SSH_CHANNEL_REQ_STATE_PENDING:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_DENIED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ACCEPTED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ERROR:
    REDEMPTION_CXX_FALLTHROUGH;
    default:
        break;
    }
    rc = channel->channel_request(client_session);
    return rc;
}


/**
 * @brief Run a shell command without an interactive shell.
 *
 * This is similar to 'sh -c command'.
 *
 * @param[in]  channel  The channel to execute the command.
 *
 * @param[in]  cmd      The command to execute
 *                      (e.g. "ls ~/ -al | grep -i reports").
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 *
 */
int ssh_channel_request_exec_client(SshClientSession * client_session, ssh_channel channel, const char *cmd) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO, ">>>>>>>>>>>> CHANNEL_REQUEST_EXEC <%s> %s", cmd, channel->show());
    int rc = SSH_ERROR;

    if(channel == nullptr) {
        return SSH_ERROR;
    }
    if(cmd == nullptr) {
        ssh_set_error(client_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return rc;
    }

    switch(channel->request_state){
    case SSH_CHANNEL_REQ_STATE_NONE:
    {
        uint8_t want_reply = 1;

        client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
        client_session->out_buffer->out_uint32_be(channel->remote_channel);
        client_session->out_buffer->out_length_prefixed_cstr("exec");
        client_session->out_buffer->out_uint8(want_reply);

        client_session->out_buffer->out_length_prefixed_cstr(cmd);
        client_session->packet_send();
        if (!want_reply) {
            channel->request_state = SSH_CHANNEL_REQ_STATE_NONE;
            return SSH_OK;
        }
        else {
            channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
            return channel->channel_request(client_session);
        }
    }
    break;
    case SSH_CHANNEL_REQ_STATE_PENDING:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_DENIED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ACCEPTED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ERROR:
    REDEMPTION_CXX_FALLTHROUGH;
    default:
        ;
    }

    return channel->channel_request(client_session);
}


/**
 * @brief Request a pty with a specific type and size.
 *
 * @param[in]  channel  The channel to sent the request.
 *
 * @param[in]  terminal The terminal type ("vt100, xterm,...").
 *
 * @param[in]  col      The number of columns.
 *
 * @param[in]  row      The number of rows.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 */
int ssh_channel_request_pty_size_client(SshClientSession * client_session, ssh_channel channel, const char *terminal, int col, int row)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    syslog(LOG_INFO, ">>>>>>>>>>>> CHANNEL_REQUEST_PTY_SIZE <%s, %d, %d> %s", terminal, col, row, channel->show());

    if(channel == nullptr) {
        return SSH_ERROR;
    }

    if(terminal == nullptr) {
        ssh_set_error(client_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return SSH_ERROR;
    }

    switch(channel->request_state){
    case SSH_CHANNEL_REQ_STATE_NONE:
    {
        uint8_t want_reply = 1;

        client_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
        client_session->out_buffer->out_uint32_be(channel->remote_channel);
        client_session->out_buffer->out_length_prefixed_cstr("pty-req");
        client_session->out_buffer->out_uint8(want_reply);

        client_session->out_buffer->out_length_prefixed_cstr(terminal);
        client_session->out_buffer->out_uint32_be(col);
        client_session->out_buffer->out_uint32_be(row);
        client_session->out_buffer->out_uint32_be(0);
        client_session->out_buffer->out_uint32_be(0);
        client_session->out_buffer->out_uint32_be(1);
        /* Add a 0byte string */
        client_session->out_buffer->out_uint8(0);

        client_session->packet_send();
        if (!want_reply) {
            channel->request_state = SSH_CHANNEL_REQ_STATE_NONE;
            return SSH_OK;
        }
        else {
            channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
            return channel->channel_request(client_session);
        }
    }
    break;
    case SSH_CHANNEL_REQ_STATE_PENDING:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_DENIED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ACCEPTED:
    REDEMPTION_CXX_FALLTHROUGH;
    case SSH_CHANNEL_REQ_STATE_ERROR:
    REDEMPTION_CXX_FALLTHROUGH;
    default:
    break;
    }
    return channel->channel_request(client_session);
}



/**
 * @brief Blocking write on a channel.
 *
 * @param[in]  channel  The channel to write to.
 *
 * @param[in]  data     A pointer to the data to write.
 *
 * @param[in]  len      The length of the buffer to write to.
 *
 * @return              The number of bytes written, SSH_ERROR on error.
 *
 */
int SshClientSession::ssh_channel_write_client(ssh_channel channel, const uint8_t *data, uint32_t len)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if(channel == nullptr) {
        syslog(LOG_WARNING, "Invalid channel");
        return SSH_ERROR;
    }
    if(data == nullptr) {
        syslog(LOG_WARNING, "Invalid data");
        return SSH_ERROR;
    }

    if (len > INT_MAX) {
        syslog(LOG_WARNING, "Length (%u) is bigger than INT_MAX", len);
        return SSH_ERROR;
    }

    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t origlen = len;

    /*    syslog(LOG_INFO, "channel write len=%d", len);*/
    /*
     * Handle the max packet len from remote side, be nice
     * 10 bytes for the headers
     */
    size_t maxpacketlen = channel->remote_maxpacket - 10;

    if (channel->local_eof) {
        ssh_set_error(this->error, SSH_REQUEST_DENIED,
                      "Can't write to channel %d:%d  after EOF was sent",
                      channel->local_channel,
                      channel->remote_channel);
        this->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_ERROR;
    }

    if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN
    || channel->delayed_close != 0) {
        syslog(LOG_INFO, "Remote channel is closed");
        ssh_set_error(this->error, SSH_REQUEST_DENIED, "Remote channel is closed");
        return SSH_ERROR;
    }

    if (this->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    while (len > 0) {
        while (channel->remote_window == 0) {
            if (this->session_state == SSH_SESSION_STATE_ERROR){
                syslog(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
                return SSH_ERROR;
            }
            syslog(LOG_WARNING, "Waiting for growing window Call to handle_packets session_state=%d channel_state=%d",
                this->session_state, static_cast<int>(channel->state));
            if (this->socket == nullptr){
                return SSH_ERROR;
            }

            // Waiting for input
            dopoll(this->ctx, (this->flags&SSH_SESSION_FLAG_BLOCKING)?SSH_TIMEOUT_INFINITE:SSH_TIMEOUT_NONBLOCKING);

            if (this->session_state == SSH_SESSION_STATE_ERROR
            || this->session_state == SSH_SESSION_STATE_ERROR){
                syslog(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
                return SSH_ERROR;
            }
        }

        uint32_t effectivelen = len > channel->remote_window ? channel->remote_window : len;
        if (effectivelen > maxpacketlen) {
            effectivelen = maxpacketlen;
        }

        // TODO: datas are buffered in channel buffer but sent only if remote window
        // has enough free space. Channels can say if they have enough room to store data.
        // construction of message and sending will be done later (in polling).

        this->out_buffer->out_uint8(SSH_MSG_CHANNEL_DATA);
        this->out_buffer->out_uint32_be(channel->remote_channel);
        this->out_buffer->out_uint32_be(effectivelen);
        this->out_buffer->out_blob(data, effectivelen);

        this->packet_send();

        syslog(LOG_INFO,
                "%s wrote %ld bytes", __FUNCTION__, static_cast<long int>(effectivelen));

        channel->remote_window -= effectivelen;
        len -= effectivelen;
        data += effectivelen;
    }
    return static_cast<int>(origlen - len);

}

/**
 * @brief Blocking write on a channel stderr.
 *
 * @param[in]  channel  The channel to write to.
 *
 * @param[in]  data     A pointer to the data to write.
 *
 * @param[in]  len      The length of the buffer to write to.
 *
 * @return              The number of bytes written, SSH_ERROR on error.
 *
 */
int SshClientSession::ssh_channel_write_stderr_client(ssh_channel channel, const uint8_t *data, uint32_t len)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if(channel == nullptr) {
        syslog(LOG_WARNING, "Invalid channel");
        return SSH_ERROR;
    }
    if(data == nullptr) {
        syslog(LOG_WARNING, "Invalid data");
        return SSH_ERROR;
    }

    if (len > INT_MAX) {
        syslog(LOG_WARNING, "Length (%u) is bigger than INT_MAX", len);
        return SSH_ERROR;
    }

    uint32_t origlen = len;

    /*    syslog(LOG_INFO, "channel write len=%d", len);*/
    /*
     * Handle the max packet len from remote side, be nice
     * 10 bytes for the headers
     */
    size_t maxpacketlen = channel->remote_maxpacket - 10;

    if (channel->local_eof) {
        ssh_set_error(this->error, SSH_REQUEST_DENIED,
                      "Can't write to channel %d:%d  after EOF was sent",
                      channel->local_channel,
                      channel->remote_channel);
        this->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_ERROR;
    }

    if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN
    || channel->delayed_close != 0) {
        syslog(LOG_INFO, "Remote channel is closed");
        ssh_set_error(this->error, SSH_REQUEST_DENIED, "Remote channel is closed");
        return SSH_ERROR;
    }

    if (this->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    while (len > 0) {
        while (channel->remote_window == 0) {
            if (this->session_state == SSH_SESSION_STATE_ERROR){
                syslog(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
                return SSH_ERROR;
            }
            syslog(LOG_WARNING, "Waiting for growing window Call to handle_packets session_state=%d channel_state=%d",
                this->session_state, static_cast<int>(channel->state));
            if (this->socket == nullptr){
                return SSH_ERROR;
            }

            // Waiting for input
            dopoll(this->ctx, (this->flags&SSH_SESSION_FLAG_BLOCKING)?SSH_TIMEOUT_INFINITE:SSH_TIMEOUT_NONBLOCKING);

            if (this->session_state == SSH_SESSION_STATE_ERROR){
                syslog(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
                return SSH_ERROR;
            }
        }
        uint32_t effectivelen = len > channel->remote_window ? channel->remote_window : len;
        if (effectivelen > maxpacketlen) {
            effectivelen = maxpacketlen;
        }

        /* stderr message has an extra field */
        this->out_buffer->out_uint8(SSH_MSG_CHANNEL_EXTENDED_DATA);
        this->out_buffer->out_uint32_be(channel->remote_channel);
        this->out_buffer->out_uint32_be(SSH2_EXTENDED_DATA_STDERR);
        this->out_buffer->out_uint32_be(effectivelen);
        this->out_buffer->out_blob(data, effectivelen);

        this->packet_send();

//        syslog(LOG_INFO,
//                "channel_write wrote %ld bytes", static_cast<long int>(effectivelen));

        channel->remote_window -= effectivelen;
        len -= effectivelen;
        data = data + effectivelen;
    }
    return static_cast<int>(origlen - len);
}

REDEMPTION_DIAGNOSTIC_POP
