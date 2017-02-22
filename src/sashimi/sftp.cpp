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
   Copyright (c) 2003-2013 by Aris Adamantiadis
   This file contains code written by Nick Zitzmann
*/

#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "sashimi/libssh/libssh.h"

#include "sashimi/buffer.hpp"
#include "sashimi/channels.hpp"

#define LIBSFTP_VERSION 3

struct sftp_session_struct {
    ssh_session_struct * session;
    ssh_channel_struct * channel;
    int server_version;
    int client_version;
    int version;
    struct sftp_request_queue_struct * queue;
    uint32_t id_counter;
    int errnum;
    void **handles;
    struct sftp_ext_struct * ext;

    sftp_session_struct()
        : session(nullptr)
        , channel(nullptr)
        , server_version(0)
        , client_version(0)
        , version(0)
        , queue(nullptr)
        , id_counter(0)
        , errnum(0)
        , handles(nullptr)
        , ext(nullptr)
    {
    }
};

struct sftp_packet_struct {
    sftp_session_struct* sftp;
    uint8_t type;
    ssh_buffer_struct* payload;
};

struct sftp_dir_struct {
    sftp_session_struct * sftp;
    char *name;
    SSHString handle; /* handle to directory */
    ssh_buffer_struct* buffer; /* contains raw attributes from server which haven't been parsed */
    uint32_t count; /* counts the number of following attributes structures into buffer */
    int eof; /* end of directory listing */

    sftp_dir_struct()
        : sftp(nullptr)
        , name(nullptr)
        , handle(0)
        , buffer(nullptr)
        , count(0)
        , eof(0)
    {
    }
};

struct sftp_message_struct {
    sftp_session_struct * sftp;
    uint8_t packet_type;
    ssh_buffer_struct* payload;
    uint32_t id;

    sftp_message_struct()
        : sftp(nullptr)
        , packet_type(0)
        , payload(nullptr)
        , id(0)
    {
    }
};

/* this is a bunch of all data that could be into a message */
struct sftp_client_message_struct {
    sftp_session_struct* sftp;
    uint8_t type;
    uint32_t id;
    char *filename; /* can be "path" */
    uint32_t flags;
    struct sftp_attributes_struct* attr;
    SSHString handle;
    uint64_t offset;
    uint32_t len;
    int attr_num;
    ssh_buffer_struct* attrbuf; /* used by sftp_reply_attrs */
    SSHString data; /* can be newpath of rename() */
    ssh_buffer_struct* complete_message; /* complete message in case of retransmission*/
    char *str_data; /* cstring version of data */

    sftp_client_message_struct()
        : sftp(nullptr)
        , type(0)
        , id(0)
        , filename(nullptr) /* can be "path" */
        , flags(0)
        , attr(nullptr)
        , handle(0)
        , offset(0)
        , len(0)
        , attr_num(0)
        , attrbuf(nullptr) /* used by sftp_reply_attrs */
        , data(0) /* can be newpath of rename() */
        , complete_message(nullptr) /* complete message in case of retransmission*/
        , str_data(nullptr) /* cstring version of data */
    {
    }

};

struct sftp_request_queue_struct {
    sftp_request_queue_struct * next;
    sftp_message_struct * message;

    sftp_request_queue_struct()
        : next(nullptr)
        , message(nullptr)
    {
    }
};

/* SSH_FXP_MESSAGE described into .7 page 26 */
struct sftp_status_message_struct {
    uint32_t id;
    uint32_t status;
    SSHString error;
    SSHString lang;
    char *errormsg;
    char *langmsg;

    sftp_status_message_struct()
        : id(0)
        , status(0)
        , error(nullptr)
        , lang(nullptr)
        , errormsg(nullptr)
        , langmsg(nullptr)
    {
    }
};

struct sftp_attributes_struct {
    char *name;
    char *longname; /* ls -l output on openssh, not reliable else */
    uint32_t flags;
    uint8_t type;
    uint64_t size;
    uint32_t uid;
    uint32_t gid;
    char *owner; /* set if openssh and version 4 */
    char *group; /* set if openssh and version 4 */
    uint32_t permissions;
    uint64_t atime64;
    uint32_t atime;
    uint32_t atime_nseconds;
    uint64_t createtime;
    uint32_t createtime_nseconds;
    uint64_t mtime64;
    uint32_t mtime;
    uint32_t mtime_nseconds;
    SSHString acl;
    uint32_t extended_count;
    SSHString extended_type;
    SSHString extended_data;

    sftp_attributes_struct()
        : name(nullptr)
        , longname(nullptr)
        , flags(0)
        , type(0)
        , size(0)
        , uid(0)
        , gid(0)
        , owner(nullptr) /* set if openssh and version 4 */
        , group(nullptr) /* set if openssh and version 4 */
        , permissions(0)
        , atime64(0)
        , atime(0)
        , atime_nseconds(0)
        , createtime(0)
        , createtime_nseconds(0)
        , mtime64(0)
        , mtime(0)
        , mtime_nseconds(0)
        , acl(0)
        , extended_count(0)
        , extended_type(0)
        , extended_data(0)
    {}
};

/**
 * @brief SFTP statvfs structure.
 */
struct sftp_statvfs_struct {
    uint64_t f_bsize;   /** file system block size */
    uint64_t f_frsize;  /** fundamental fs block size */
    uint64_t f_blocks;  /** number of blocks (unit f_frsize) */
    uint64_t f_bfree;   /** free blocks in file system */
    uint64_t f_bavail;  /** free blocks for non-root */
    uint64_t f_files;   /** total file inodes */
    uint64_t f_ffree;   /** free file inodes */
    uint64_t f_favail;  /** free file inodes for to non-root */
    uint64_t f_fsid;    /** file system id */
    uint64_t f_flag;    /** bit mask of f_flag values */
    uint64_t f_namemax; /** maximum filename length */

    sftp_statvfs_struct()
        : f_bsize(0)
        , f_frsize(0)
        , f_blocks(0)
        , f_bfree(0)
        , f_bavail(0)
        , f_files(0)
        , f_ffree(0)
        , f_favail(0)
        , f_fsid(0)
        , f_flag(0)
        , f_namemax(0)
    {
    }

};

/**
 * @brief Start a new sftp session with an existing channel.
 *
 * @param session       The ssh session to use.
 * @param channel		An open session channel with subsystem already allocated
 *
 * @return              A new sftp session or nullptr on error.
 *
 * @see sftp_free()
 */
LIBSSH_API sftp_session_struct* sftp_new_channel(ssh_session_struct * session, ssh_channel channel);


/**
 * @brief Close and deallocate a sftp session.
 *
 * @param sftp          The sftp session handle to free.
 */
LIBSSH_API void sftp_free(sftp_session_struct* sftp);

/**
 * @brief Initialize the sftp session with the server.
 *
 * @param sftp          The sftp session to initialize.
 *
 * @return              0 on success, < 0 on error with ssh error set.
 *
 * @see sftp_new()
 */
LIBSSH_API int sftp_init(sftp_session_struct* sftp);

/**
 * @brief Get the last sftp error.
 *
 * Use this function to get the latest error set by a posix like sftp function.
 *
 * @param sftp          The sftp session where the error is saved.
 *
 * @return              The saved error (see server responses), < 0 if an error
 *                      in the function occured.
 *
 * @see Server responses
 */
LIBSSH_API int sftp_get_error(sftp_session_struct* sftp);

/**
 * @brief Get the count of extensions provided by the server.
 *
 * @param  sftp         The sftp session to use.
 *
 * @return The count of extensions provided by the server, 0 on error or
 *         not available.
 */
LIBSSH_API unsigned int sftp_extensions_get_count(sftp_session_struct* sftp);

/**
 * @brief Get the name of the extension provided by the server.
 *
 * @param  sftp         The sftp session to use.
 *
 * @param  indexn        The index number of the extension name you want.
 *
 * @return              The name of the extension.
 */
LIBSSH_API const char *sftp_extensions_get_name(sftp_session_struct* sftp, unsigned int indexn);

/**
 * @brief Get the data of the extension provided by the server.
 *
 * This is normally the version number of the extension.
 *
 * @param  sftp         The sftp session to use.
 *
 * @param  indexn        The index number of the extension data you want.
 *
 * @return              The data of the extension.
 */
LIBSSH_API const char *sftp_extensions_get_data(sftp_session_struct* sftp, unsigned int indexn);

/**
 * @brief Check if the given extension is supported.
 *
 * @param  sftp         The sftp session to use.
 *
 * @param  name         The name of the extension.
 *
 * @param  data         The data of the extension.
 *
 * @return 1 if supported, 0 if not.
 *
 * Example:
 *
 * @code
 * sftp_extension_supported(sftp, "statvfs@openssh.com", "2");
 * @endcode
 */
LIBSSH_API int sftp_extension_supported(sftp_session_struct* sftp, const char *name,
    const char *data);

/**
 * @brief Open a directory used to obtain directory entries.
 *
 * @param session       The sftp session handle to open the directory.
 * @param path          The path of the directory to open.
 *
 * @return              A sftp directory handle or nullptr on error with ssh and
 *                      sftp error set.
 *
 * @see                 sftp_readdir
 * @see                 sftp_closedir
 */
LIBSSH_API sftp_dir_struct* sftp_opendir(sftp_session_struct* session, const char *path);

/**
 * @brief Get a single file attributes structure of a directory.
 *
 * @param session      The sftp session handle to read the directory entry.
 * @param dir          The opened sftp directory handle to read from.
 *
 * @return             A file attribute structure or nullptr at the end of the
 *                     directory.
 *
 * @see                sftp_opendir()
 * @see                sftp_attribute_free()
 * @see                sftp_closedir()
 */
LIBSSH_API sftp_attributes_struct* sftp_readdir(sftp_session_struct* session, sftp_dir_struct* dir);

/**
 * @brief Tell if the directory has reached EOF (End Of File).
 *
 * @param dir           The sftp directory handle.
 *
 * @return              1 if the directory is EOF, 0 if not.
 *
 * @see                 sftp_readdir()
 */
LIBSSH_API int sftp_dir_eof(sftp_dir_struct* dir);

/**
 * @brief Get information about a file or directory.
 *
 * @param session       The sftp session handle.
 * @param path          The path to the file or directory to obtain the
 *                      information.
 *
 * @return              The sftp attributes structure of the file or directory,
 *                      nullptr on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API sftp_attributes_struct* sftp_stat(sftp_session_struct* session, const char *path);

/**
 * @brief Get information about a file or directory.
 *
 * Identical to sftp_stat, but if the file or directory is a symbolic link,
 * then the link itself is stated, not the file that it refers to.
 *
 * @param session       The sftp session handle.
 * @param path          The path to the file or directory to obtain the
 *                      information.
 *
 * @return              The sftp attributes structure of the file or directory,
 *                      nullptr on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API sftp_attributes_struct* sftp_lstat(sftp_session_struct* session, const char *path);

/**
 * @brief Get information about a file or directory from a file handle.
 *
 * @param file          The sftp file handle to get the stat information.
 *
 * @return              The sftp attributes structure of the file or directory,
 *                      nullptr on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API sftp_attributes_struct* sftp_fstat(struct sftp_file_struct* file);

/**
 * @brief Free a sftp attribute structure.
 *
 * @param file          The sftp attribute structure to free.
 */
LIBSSH_API void sftp_attributes_free(sftp_attributes_struct* file);

/**
 * @brief Close a directory handle opened by sftp_opendir().
 *
 * @param dir           The sftp directory handle to close.
 *
 * @return              Returns SSH_NO_ERROR or SSH_ERROR if an error occured.
 */
LIBSSH_API int sftp_closedir(sftp_dir_struct* dir);

/**
 * @brief Close an open file handle.
 *
 * @param file          The open sftp file handle to close.
 *
 * @return              Returns SSH_NO_ERROR or SSH_ERROR if an error occured.
 *
 * @see                 sftp_open()
 */
LIBSSH_API int sftp_close(sftp_file_struct* file);

/**
 * @brief Open a file on the server.
 *
 * @param session       The sftp session handle.
 *
 * @param file          The file to be opened.
 *
 * @param accesstype    Is one of O_RDONLY, O_WRONLY or O_RDWR which request
 *                      opening  the  file  read-only,write-only or read/write.
 *                      Acesss may also be bitwise-or'd with one or  more of
 *                      the following:
 *                      O_CREAT - If the file does not exist it will be
 *                      created.
 *                      O_EXCL - When  used with O_CREAT, if the file already
 *                      exists it is an error and the open will fail.
 *                      O_TRUNC - If the file already exists it will be
 *                      truncated.
 *
 * @param mode          Mode specifies the permissions to use if a new file is
 *                      created.  It  is  modified  by  the process's umask in
 *                      the usual way: The permissions of the created file are
 *                      (mode & ~umask)
 *
 * @return              A sftp file handle, nullptr on error with ssh and sftp
 *                      error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API sftp_file_struct* sftp_open(sftp_session_struct* session, const char *file, int accesstype,
    mode_t mode);

/**
 * @brief Make the sftp communication for this file handle non blocking.
 *
 * @param[in]  handle   The file handle to set non blocking.
 */
LIBSSH_API void sftp_file_set_nonblocking(sftp_file_struct* handle);

/**
 * @brief Make the sftp communication for this file handle blocking.
 *
 * @param[in]  handle   The file handle to set blocking.
 */
LIBSSH_API void sftp_file_set_blocking(sftp_file_struct* handle);

/**
 * @brief Write to a file using an opened sftp file handle.
 *
 * @param file          Open sftp file handle to write to.
 *
 * @param buf           Pointer to buffer to write data.
 *
 * @param count         Size of buffer in bytes.
 *
 * @return              Number of bytes written, < 0 on error with ssh and sftp
 *                      error set.
 *
 * @see                 sftp_open()
 * @see                 sftp_read()
 * @see                 sftp_close()
 */
LIBSSH_API ssize_t sftp_write(sftp_file_struct* file, const void *buf, size_t count);

/**
 * @brief Seek to a specific location in a file.
 *
 * @param file         Open sftp file handle to seek in.
 *
 * @param new_offset   Offset in bytes to seek.
 *
 * @return             0 on success, < 0 on error.
 */
LIBSSH_API int sftp_seek(sftp_file_struct* file, uint32_t new_offset);

/**
 * @brief Seek to a specific location in a file. This is the
 * 64bit version.
 *
 * @param file         Open sftp file handle to seek in.
 *
 * @param new_offset   Offset in bytes to seek.
 *
 * @return             0 on success, < 0 on error.
 */
LIBSSH_API int sftp_seek64(sftp_file_struct* file, uint64_t new_offset);

/**
 * @brief Report current byte position in file.
 *
 * @param file          Open sftp file handle.
 *
 * @return              The offset of the current byte relative to the beginning
 *                      of the file associated with the file descriptor. < 0 on
 *                      error.
 */
LIBSSH_API unsigned long sftp_tell(sftp_file_struct* file);

/**
 * @brief Report current byte position in file.
 *
 * @param file          Open sftp file handle.
 *
 * @return              The offset of the current byte relative to the beginning
 *                      of the file associated with the file descriptor. < 0 on
 *                      error.
 */
LIBSSH_API uint64_t sftp_tell64(sftp_file_struct* file);

/**
 * @brief Rewinds the position of the file pointer to the beginning of the
 * file.
 *
 * @param file          Open sftp file handle.
 */
LIBSSH_API void sftp_rewind(sftp_file_struct* file);

/**
 * @brief Unlink (delete) a file.
 *
 * @param sftp          The sftp session handle.
 *
 * @param file          The file to unlink/delete.
 *
 * @return              0 on success, < 0 on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API int sftp_unlink(sftp_session_struct* sftp, const char *file);

/**
 * @brief Remove a directoy.
 *
 * @param sftp          The sftp session handle.
 *
 * @param directory     The directory to remove.
 *
 * @return              0 on success, < 0 on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API int sftp_rmdir(sftp_session_struct* sftp, const char *directory);

/**
 * @brief Create a directory.
 *
 * @param sftp          The sftp session handle.
 *
 * @param directory     The directory to create.
 *
 * @param mode          Specifies the permissions to use. It is modified by the
 *                      process's umask in the usual way:
 *                      The permissions of the created file are (mode & ~umask)
 *
 * @return              0 on success, < 0 on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API int sftp_mkdir(sftp_session_struct* sftp, const char *directory, mode_t mode);

/**
 * @brief Rename or move a file or directory.
 *
 * @param sftp          The sftp session handle.
 *
 * @param original      The original url (source url) of file or directory to
 *                      be moved.
 *
 * @param newname       The new url (destination url) of the file or directory
 *                      after the move.
 *
 * @return              0 on success, < 0 on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API int sftp_rename(sftp_session_struct* sftp, const char *original, const  char *newname);

/**
 * @brief Set file attributes on a file, directory or symbolic link.
 *
 * @param sftp          The sftp session handle.
 *
 * @param file          The file which attributes should be changed.
 *
 * @param attr          The file attributes structure with the attributes set
 *                      which should be changed.
 *
 * @return              0 on success, < 0 on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API int sftp_setstat(sftp_session_struct* sftp, const char *file, sftp_attributes_struct* attr);

/**
 * @brief Change the file owner and group
 *
 * @param sftp          The sftp session handle.
 *
 * @param file          The file which owner and group should be changed.
 *
 * @param owner         The new owner which should be set.
 *
 * @param group         The new group which should be set.
 *
 * @return              0 on success, < 0 on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API int sftp_chown(sftp_session_struct* sftp, const char *file, uid_t owner, gid_t group);

/**
 * @brief Change permissions of a file
 *
 * @param sftp          The sftp session handle.
 *
 * @param file          The file which owner and group should be changed.
 *
 * @param mode          Specifies the permissions to use. It is modified by the
 *                      process's umask in the usual way:
 *                      The permissions of the created file are (mode & ~umask)
 *
 * @return              0 on success, < 0 on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API int sftp_chmod(sftp_session_struct* sftp, const char *file, mode_t mode);

/**
 * @brief Change the last modification and access time of a file.
 *
 * @param sftp          The sftp session handle.
 *
 * @param file          The file which owner and group should be changed.
 *
 * @param times         A timeval structure which contains the desired access
 *                      and modification time.
 *
 * @return              0 on success, < 0 on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API int sftp_utimes(sftp_session_struct* sftp, const char *file, const struct timeval *times);

/**
 * @brief Create a symbolic link.
 *
 * @param  sftp         The sftp session handle.
 *
 * @param  target       Specifies the target of the symlink.
 *
 * @param  dest         Specifies the path name of the symlink to be created.
 *
 * @return              0 on success, < 0 on error with ssh and sftp error set.
 *
 * @see sftp_get_error()
 */
LIBSSH_API int sftp_symlink(sftp_session_struct* sftp, const char *target, const char *dest);

/**
 * @brief Read the value of a symbolic link.
 *
 * @param  sftp         The sftp session handle.
 *
 * @param  path         Specifies the path name of the symlink to be read.
 *
 * @return              The target of the link, nullptr on error.
 *
 * @see sftp_get_error()
 */
LIBSSH_API char *sftp_readlink(sftp_session_struct* sftp, const char *path);

/**
 * @brief Get information about a mounted file system.
 *
 * @param  sftp         The sftp session handle.
 *
 * @param  path         The pathname of any file within the mounted file system.
 *
 * @return A statvfs structure or nullptr on error.
 *
 * @see sftp_get_error()
 */
LIBSSH_API sftp_statvfs_struct* sftp_statvfs(sftp_session_struct* sftp, const char *path);

/**
 * @brief Get information about a mounted file system.
 *
 * @param  file         An opened file.
 *
 * @return A statvfs structure or nullptr on error.
 *
 * @see sftp_get_error()
 */
LIBSSH_API sftp_statvfs_struct* sftp_fstatvfs(sftp_file_struct* file);

/**
 * @brief Free the memory of an allocated statvfs.
 *
 * @param  statvfs_o      The statvfs to free.
 */
LIBSSH_API void sftp_statvfs_free(sftp_statvfs_struct* statvfs_o);

/**
 * @brief Canonicalize a sftp path.
 *
 * @param sftp          The sftp session handle.
 *
 * @param path          The path to be canonicalized.
 *
 * @return              The canonicalize path, nullptr on error.
 */
LIBSSH_API char *sftp_canonicalize_path(sftp_session_struct* sftp, const char *path);

/**
 * @brief Get the version of the SFTP protocol supported by the server
 *
 * @param sftp          The sftp session handle.
 *
 * @return              The server version.
 */
LIBSSH_API int sftp_server_version(sftp_session_struct* sftp);

/**
 * @brief Create a new sftp server session.
 *
 * @param session       The ssh session to use.
 *
 * @param chan          The ssh channel to use.
 *
 * @return              A new sftp server session.
 */
LIBSSH_API sftp_session_struct* sftp_server_new(ssh_session_struct * session, ssh_channel chan);

/**
 * @brief Intialize the sftp server.
 *
 * @param sftp         The sftp session to init.
 *
 * @return             0 on success, < 0 on error.
 */
LIBSSH_API int sftp_server_init(sftp_session_struct* sftp);

/* this is not a public interface */
#define SFTP_HANDLES 256
sftp_packet_struct* sftp_packet_read(sftp_session_struct* sftp);
int sftp_packet_write(sftp_session_struct* sftp,uint8_t type, ssh_buffer_struct* payload);
void sftp_packet_free(sftp_packet_struct* packet);
int buffer_add_attributes(ssh_buffer_struct* buffer, sftp_attributes_struct* attr);
sftp_attributes_struct* sftp_parse_attr(sftp_session_struct* session, ssh_buffer_struct* buf,int expectname);
/* sftpserver.c */

LIBSSH_API sftp_client_message_struct* sftp_get_client_message(sftp_session_struct* sftp);
LIBSSH_API void sftp_client_message_free(sftp_client_message_struct* msg);
LIBSSH_API uint8_t sftp_client_message_get_type(sftp_client_message_struct* msg);
LIBSSH_API const char *sftp_client_message_get_filename(sftp_client_message_struct* msg);
LIBSSH_API void sftp_client_message_set_filename(sftp_client_message_struct* msg, const char *newname);
LIBSSH_API const char *sftp_client_message_get_data(sftp_client_message_struct* msg);
LIBSSH_API uint32_t sftp_client_message_get_flags(sftp_client_message_struct* msg);
LIBSSH_API int sftp_send_client_message(sftp_session_struct* sftp, sftp_client_message_struct* msg);

int sftp_reply_name(sftp_client_message_struct* msg, const char *name, sftp_attributes_struct* attr);
int sftp_reply_handle(sftp_client_message_struct* msg, const SSHString & handle);
int sftp_reply_attr(sftp_client_message_struct* msg, sftp_attributes_struct* attr);
int sftp_reply_status(sftp_client_message_struct* msg, uint32_t status, const char *message);
int sftp_reply_names_add(sftp_client_message_struct* msg, const char *file, const char *longname, sftp_attributes_struct* attr);
int sftp_reply_names(sftp_client_message_struct* msg);
int sftp_reply_data(sftp_client_message_struct* msg, const void *data, int len);
void sftp_handle_remove(sftp_session_struct* sftp, void *handle);

/* SFTP commands and constants */
enum {
    SSH_FXP_INIT = 1,
    SSH_FXP_VERSION = 2,
    SSH_FXP_OPEN = 3,
    SSH_FXP_CLOSE = 4,
    SSH_FXP_READ = 5,
    SSH_FXP_WRITE = 6,
    SSH_FXP_LSTAT = 7,
    SSH_FXP_FSTAT = 8,
    SSH_FXP_SETSTAT = 9,
    SSH_FXP_FSETSTAT = 10,
    SSH_FXP_OPENDIR = 11,
    SSH_FXP_READDIR = 12,
    SSH_FXP_REMOVE = 13,
    SSH_FXP_MKDIR = 14,
    SSH_FXP_RMDIR = 15,
    SSH_FXP_REALPATH = 16,
    SSH_FXP_STAT = 17,
    SSH_FXP_RENAME = 18,
    SSH_FXP_READLINK = 19,
    SSH_FXP_SYMLINK = 20,
    SSH_FXP_STATUS = 101,
    SSH_FXP_HANDLE = 102,
    SSH_FXP_DATA = 103,
    SSH_FXP_NAME = 104,
    SSH_FXP_ATTRS = 105,
    SSH_FXP_EXTENDED = 200,
    SSH_FXP_EXTENDED_REPLY = 201
};

/* attributes */
/* sftp draft is completely braindead : version 3 and 4 have different flags for same constants */
/* and even worst, version 4 has same flag for 2 different constants */
/* follow up : i won't develop any sftp4 compliant library before having a clarification */

enum {
    SSH_FILEXFER_ATTR_SIZE            = 0x00000001,
    SSH_FILEXFER_ATTR_UIDGID          = 0x00000002,
    SSH_FILEXFER_ATTR_PERMISSIONS     = 0x00000004,
    SSH_FILEXFER_ATTR_ACCESSTIME      = 0x00000008,
//    SSH_FILEXFER_ATTR_ACMODTIME  = 0x00000008,
    SSH_FILEXFER_ATTR_CREATETIME      = 0x00000010,
    SSH_FILEXFER_ATTR_MODIFYTIME      = 0x00000020,
    SSH_FILEXFER_ATTR_ACL             = 0x00000040,
    SSH_FILEXFER_ATTR_OWNERGROUP      = 0x00000080,
    SSH_FILEXFER_ATTR_SUBSECOND_TIMES = 0x00000100,
    SSH_FILEXFER_ATTR_EXTENDED        = 0x80000000
};

/* types */
#define SSH_FILEXFER_TYPE_REGULAR 1
#define SSH_FILEXFER_TYPE_DIRECTORY 2
#define SSH_FILEXFER_TYPE_SYMLINK 3
#define SSH_FILEXFER_TYPE_SPECIAL 4
#define SSH_FILEXFER_TYPE_UNKNOWN 5

/**
 * @name Server responses
 *
 * @brief Responses returned by the sftp server.
 * @{
 */

/** No error */
#define SSH_FX_OK 0
/** End-of-file encountered */
#define SSH_FX_EOF 1
/** File doesn't exist */
//#define SSH_FX_NO_SUCH_FILE 2
/** Permission denied */
//#define SSH_FX_PERMISSION_DENIED 3
/** Generic failure */
#define SSH_FX_FAILURE 4
/** Garbage received from server */
//#define SSH_FX_BAD_MESSAGE 5
/** No connection has been set up */
//#define SSH_FX_NO_CONNECTION 6
/** There was a connection, but we lost it */
//#define SSH_FX_CONNECTION_LOST 7
/** Operation not supported by the server */
//#define SSH_FX_OP_UNSUPPORTED 8
/** Invalid file handle */
//#define SSH_FX_INVALID_HANDLE 9
/** No such file or directory path exists */
//#define SSH_FX_NO_SUCH_PATH 10
/** An attempt to create an already existing file or directory has been made */
#define SSH_FX_FILE_ALREADY_EXISTS 11
/** We are trying to write on a write-protected filesystem */
//#define SSH_FX_WRITE_PROTECT 12
/** No media in remote drive */
//#define SSH_FX_NO_MEDIA 13

/** @} */

/* file flags */
#define SSH_FXF_READ 0x01
#define SSH_FXF_WRITE 0x02
//#define SSH_FXF_APPEND 0x04
#define SSH_FXF_CREAT 0x08
#define SSH_FXF_TRUNC 0x10
#define SSH_FXF_EXCL 0x20
//#define SSH_FXF_TEXT 0x40

/* rename flags */
#define SSH_FXF_RENAME_OVERWRITE  0x00000001
//#define SSH_FXF_RENAME_ATOMIC     0x00000002
//#define SSH_FXF_RENAME_NATIVE     0x00000004

//#define SFTP_OPEN SSH_FXP_OPEN
//#define SFTP_CLOSE SSH_FXP_CLOSE
//#define SFTP_READ SSH_FXP_READ
//#define SFTP_WRITE SSH_FXP_WRITE
//#define SFTP_LSTAT SSH_FXP_LSTAT
//#define SFTP_FSTAT SSH_FXP_FSTAT
//#define SFTP_SETSTAT SSH_FXP_SETSTAT
//#define SFTP_FSETSTAT SSH_FXP_FSETSTAT
//#define SFTP_OPENDIR SSH_FXP_OPENDIR
//#define SFTP_READDIR SSH_FXP_READDIR
//#define SFTP_REMOVE SSH_FXP_REMOVE
//#define SFTP_MKDIR SSH_FXP_MKDIR
//#define SFTP_RMDIR SSH_FXP_RMDIR
//#define SFTP_REALPATH SSH_FXP_REALPATH
//#define SFTP_STAT SSH_FXP_STAT
//#define SFTP_RENAME SSH_FXP_RENAME
//#define SFTP_READLINK SSH_FXP_READLINK
//#define SFTP_SYMLINK SSH_FXP_SYMLINK

/* openssh flags */
//#define SSH_FXE_STATVFS_ST_RDONLY 0x1 /* read-only */
//#define SSH_FXE_STATVFS_ST_NOSUID 0x2 /* no setuid */


/* file handler */
struct sftp_file_struct {
    sftp_session_struct * sftp;
    char *name;
    uint64_t offset;
    SSHString handle;
    int eof;
    int nonblocking;

    sftp_file_struct()
        : sftp(nullptr)
        , name(nullptr)
        , offset(0)
        , handle(0)
        , eof(0)
        , nonblocking(0)
    {
    }
};


struct sftp_ext_struct {
    unsigned int count;
    char **name;
    char **data;

    sftp_ext_struct()
        : count(0)
        , name(nullptr)
        , data(nullptr)
    {
    }
};

/* functions */
static int sftp_enqueue(sftp_session_struct* session, sftp_message_struct* msg);
static void sftp_set_error(sftp_session_struct* sftp, int errnum);


//int ssh_channel_request_sftp(ssh_session_struct * session, ssh_channel channel){
//    syslog(LOG_INFO, "%s ---", __FUNCTION__);
//    syslog(LOG_INFO, ">>>>>>>>>>> CHANNEL_REQUEST_SFTP %s", channel->show());
//    if(channel == nullptr) {
//        return SSH_ERROR;
//    }
//    return ssh_channel_request_subsystem(session, channel, "sftp");
//}


//sftp_session_struct* sftp_new(ssh_session_struct * session){
//  syslog(LOG_WARNING, "sftp_new");

//  sftp_session_struct * sftp = new sftp_session_struct;
//  sftp->ext = new sftp_ext_struct;

//  sftp->session = session;
//  // TODO: I'm providing nullptr instead of channel callbacks, not worse than uninitialized
//  sftp->channel = ssh_new_channel(session, nullptr);
//  if (sftp->channel == nullptr) {
//    delete sftp;
//    return nullptr;
//  }

//  if (ssh_sessionchannel_open(sftp->session, sftp->channel)) {
//    ssh_channel_free(sftp->session, sftp->channel);
//    delete sftp;

//    return nullptr;
//  }

//  if (ssh_channel_request_sftp(sftp->session, sftp->channel)) {
//    delete sftp;

//    return nullptr;
//  }

//  return sftp;
//}

sftp_session_struct* sftp_new_channel(ssh_session_struct * session, ssh_channel channel)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
    sftp_session_struct * sftp = new sftp_session_struct;
    sftp->session = session;
    sftp->channel = channel;
    sftp->ext = new sftp_ext_struct;

    return sftp;
}

sftp_session_struct* sftp_server_new(ssh_session_struct * session, ssh_channel chan)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
    sftp_session_struct * sftp = new sftp_session_struct;
    sftp->session = session;
    sftp->channel = chan;

    return sftp;
}

int sftp_server_init(sftp_session_struct* sftp)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
    ssh_session_struct * session = sftp->session;
    sftp_packet_struct* packet = nullptr;
    ssh_buffer_struct* reply = nullptr;
    uint32_t version;

    packet = sftp_packet_read(sftp);
    if (packet == nullptr) {
        syslog(LOG_WARNING, "========================================== done 1");
        return -1;
    }

    if (packet->type != SSH_FXP_INIT) {
        ssh_set_error(session->error, SSH_FATAL,
            "Packet read of type %d instead of SSH_FXP_INIT",
            packet->type);

        if (packet != nullptr) {
            delete packet->payload;
            delete packet;
        }
        syslog(LOG_WARNING, "========================================== done 2");
        return -1;
    }

    syslog(LOG_INFO, "Received SSH_FXP_INIT");

    version = packet->payload->in_uint32_be();
    syslog(LOG_INFO, "Client version: %d", version);
    sftp->client_version = version;

    if (packet != nullptr) {
        delete packet->payload;
        delete packet;
    }

    reply = new ssh_buffer_struct;

    reply->out_uint32_be(LIBSFTP_VERSION);

    if (sftp_packet_write(sftp, SSH_FXP_VERSION, reply) < 0) {
        delete reply;
        syslog(LOG_WARNING, "========================================== done 4");
        return -1;
    }
    delete reply;

    syslog(LOG_INFO, "Server version sent");

    if (version > LIBSFTP_VERSION) {
        sftp->version = LIBSFTP_VERSION;
    } else {
        sftp->version=version;
    }

    syslog(LOG_WARNING, "========================================== done 5");
    return 0;
}

int sftp_packet_write(sftp_session_struct* sftp, uint8_t type, ssh_buffer_struct* payload)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
    int size;

    payload->buffer_prepend_data(&type, sizeof(uint8_t));
    // TODO: check memory allocation

    size = htonl(payload->in_remain());
    payload->buffer_prepend_data(&size, sizeof(uint32_t));
    // TODO: check memory allocation

    // TODO packet_write client and packet write server like elsewhere
    size = ssh_channel_write_client(sftp->session, sftp->channel, payload->get_pos_ptr(),
      payload->in_remain());
    if (size < 0) {
        return -1;
    }
    else if(static_cast<uint32_t>(size) != payload->in_remain()) {
        syslog(LOG_INFO,
            "Had to write %d bytes, wrote only %d",
            static_cast<uint32_t>(payload->in_remain()),
            size);
    }

    return size;
}


/**
 * @brief Reads data from a channel.
 *
 * @param[in]  channel  The channel to read from.
 *
 * @param[in]  dest     The destination buffer which will get the data.
 *
 * @param[in]  count    The count of bytes to be read.
 *
 * @return              The number of bytes read, 0 on end of file or SSH_ERROR
 *                      on error. In nonblocking mode it Can return 0 if no data
 *                      is available or SSH_AGAIN.
 *
 * @warning This function may return less than count bytes of data, and won't
 *          block until count bytes have been read.
 * @warning The read function using a buffer has been renamed to
 *          channel_read_buffer().
 */
inline int ssh_channel_read_stdout(ssh_session_struct * session, ssh_channel channel, void *dest, uint32_t count)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
//    syslog(LOG_WARNING, "ssh_channel_read channel=%s", channel->show());
    uint32_t len;
    int rc;

    if(channel == nullptr) {
        return SSH_ERROR;
    }
    if(dest == nullptr) {
        ssh_set_error(session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return SSH_ERROR;
    }

    if (count == 0) {
        return 0;
    }

    /*
     * We may have problem if the window is too small to accept as much data
     * as asked
     */

    unsigned minimumsize = count - channel->stdout_buffer->in_remain();
    if (minimumsize > channel->local_window) {
        unsigned new_window = (minimumsize > WINDOWBASE) ? minimumsize : WINDOWBASE;
        if (new_window > channel->local_window){
            /* WINDOW_ADJUST packet needs a relative increment rather than an absolute
             * value, so we give here the missing bytes needed to reach new_window
             */
            session->out_buffer->out_uint8(SSH_MSG_CHANNEL_WINDOW_ADJUST);
            session->out_buffer->out_uint32_be(channel->remote_channel);
            session->out_buffer->out_uint32_be(new_window - channel->local_window);
            session->packet_send();
            channel->local_window = new_window - channel->local_window ;
        }
    }

    /* block reading until at least one byte has been read
     *  and ignore the trivial case count=0
     */
     // CGR: why do we reset count to 1 ? Didn't the external caller asked for a specific count ?
//    count = 1;

    rc = SSH_OK;

    if ((session->flags & SSH_SESSION_FLAG_BLOCKING)){
        while (channel->stdout_buffer->in_remain() < count) {
            if (channel->remote_eof) {
                break;
            }
            if (session->session_state == SSH_SESSION_STATE_ERROR) {
                break;
            }
            dopoll(session->ctx, SSH_TIMEOUT_INFINITE);
            if (session->session_state == SSH_SESSION_STATE_ERROR) {
                break;
            }
        }
    }
    else {
            struct timeval start;
            gettimeofday(&start, nullptr);
            int timeout = TIMEOUT_DEFAULT_MS;

            while(channel->stdout_buffer->in_remain() < count){
                if (channel->remote_eof) {
                    break;
                }
                if (session->session_state == SSH_SESSION_STATE_ERROR) {
                    break;
                }
                dopoll(session->ctx, timeout);
                if (session->session_state == SSH_SESSION_STATE_ERROR) {
                    break;
                }
                struct timeval now;
                gettimeofday(&now, nullptr);
                long ms =   (now.tv_sec - start.tv_sec) * 1000
                        + (now.tv_usec < start.tv_usec) * 1000
                        + (now.tv_usec - start.tv_usec) / 1000;

                if (ms >= timeout) {
                    rc = (channel->stdout_buffer->in_remain() >= count
                        || channel->remote_eof
                        || session->session_state == SSH_SESSION_STATE_ERROR) ? SSH_OK : SSH_AGAIN;
                    break;
                }
                timeout -= ms;
            }
    }
    if (rc == SSH_ERROR
    || session->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    if (channel->remote_eof && channel->stdout_buffer->in_remain() == 0) {
        return -1;
    }

    len = channel->stdout_buffer->in_remain();
    /* Read count bytes if len is greater, everything otherwise */
    len = (len > count ? count : len);

    memcpy(dest, channel->stdout_buffer->get_pos_ptr(), len);
    channel->stdout_buffer->in_skip_bytes(len);

    /* Authorize some buffering while userapp is busy */
    if ((channel->local_window < WINDOWLIMIT)
    && (WINDOWBASE > channel->local_window)) {
        /* WINDOW_ADJUST packet needs a relative increment rather than an absolute
         * value, so we give here the missing bytes needed to reach new_window
         */
        session->out_buffer->out_uint8(SSH_MSG_CHANNEL_WINDOW_ADJUST);
        session->out_buffer->out_uint32_be(channel->remote_channel);
        session->out_buffer->out_uint32_be(WINDOWBASE - channel->local_window);
        session->packet_send();
        channel->local_window = WINDOWBASE - channel->local_window ;
    }
    return len;
}


sftp_packet_struct* sftp_packet_read(sftp_session_struct* sftp)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
    unsigned char buffer[4096];
    uint32_t size;
    int r;

    sftp_packet_struct * packet = new sftp_packet_struct;
    packet->sftp = sftp;
    packet->payload = new ssh_buffer_struct;
    // TODO: check memory allocation

    syslog(LOG_WARNING, "A +++");
    r = 0;
    while (r < 4) {
        int res = ssh_channel_read_stdout(sftp->session, sftp->channel, buffer, 4-r);
        syslog(LOG_WARNING, "B +++ res = %d", res);
        if (res < 0) {
            delete packet->payload;
            delete packet;
            return nullptr;
        }
        r += res;
    }

    packet->payload->out_blob(buffer, r);
    size = packet->payload->in_uint32_be();
    r=ssh_channel_read_stdout(sftp->session, sftp->channel, buffer, 1);
    if (r <= 0) {
        /* TODO: check if there are cases where an error needs to be set here */
        delete packet->payload;
        delete packet;
        return nullptr;
    }
    packet->payload->out_blob(buffer, r);
    packet->type = packet->payload->in_uint8();
    size=size-1;
    while (size>0){
        r=ssh_channel_read_stdout(sftp->session, sftp->channel, buffer, sizeof(buffer)>size ? size:sizeof(buffer));

        if(r <= 0) {
            /* TODO: check if there are cases where an error needs to be set here */
            delete packet->payload;
            delete packet;
            return nullptr;
        }
        packet->payload->out_blob(buffer,r);
        size -= r;
    }
    syslog(LOG_WARNING, "G +++");
    return packet;
}

static void sftp_set_error(sftp_session_struct* sftp, int errnum)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
    if (sftp != nullptr) {
        sftp->errnum = errnum;
    }
}

/* Get the last sftp error */
int sftp_get_error(sftp_session_struct* sftp)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
    if (sftp == nullptr) {
        return -1;
    }

    return sftp->errnum;
}

static sftp_message_struct* sftp_get_message(sftp_packet_struct * packet)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);

    sftp_message_struct* msg = new sftp_message_struct;
    msg->payload = new ssh_buffer_struct;
    msg->sftp = packet->sftp;
    msg->packet_type = packet->type;

    if ((packet->type != SSH_FXP_STATUS) && (packet->type!=SSH_FXP_HANDLE)
    && (packet->type != SSH_FXP_DATA)
    && (packet->type != SSH_FXP_ATTRS)
    && (packet->type != SSH_FXP_NAME)
    && (packet->type != SSH_FXP_EXTENDED_REPLY)) {
        ssh_set_error(packet->sftp->session->error, SSH_FATAL,
            "Unknown packet type %d", packet->type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        return nullptr;
    }

    msg->id = packet->payload->in_uint32_le();

    syslog(LOG_INFO,
      "Packet with id %d type %d",
      msg->id,
      msg->packet_type);

    msg->payload->out_blob(packet->payload->get_pos_ptr(), packet->payload->in_remain());

    return msg;
}

static int sftp_read_and_dispatch(sftp_session_struct* sftp)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
    sftp_packet_struct* packet = nullptr;
    sftp_message_struct* msg = nullptr;

    packet = sftp_packet_read(sftp);
    if (packet == nullptr) {
        return -1; /* something nasty happened reading the packet */
    }

    msg = sftp_get_message(packet);
    if (packet != nullptr) {
        delete packet->payload;
        delete packet;
    }
    if (msg == nullptr) {
        return -1;
    }

    if (sftp_enqueue(sftp, msg) < 0) {
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        return -1;
    }

    return 0;
}

/* Initialize the sftp session with the server. */
int sftp_init(sftp_session_struct* sftp)
{
    syslog(LOG_WARNING, "--- %s", __FUNCTION__);
    char *ext_name = nullptr;
    char *ext_data = nullptr;

    ssh_buffer_struct* buffer = new ssh_buffer_struct;
    // TODO: check memory allocation

    buffer->out_uint32_le(LIBSFTP_VERSION);
    if (sftp_packet_write(sftp, SSH_FXP_INIT, buffer) < 0) {
        delete buffer;
        return -1;
    }
    delete buffer;

    sftp_packet_struct* packet = sftp_packet_read(sftp);
    if (packet == nullptr) {
        return -1;
    }

    if (packet->type != SSH_FXP_VERSION) {
        ssh_set_error(sftp->session->error, SSH_FATAL,
            "Received a %d messages instead of SSH_FXP_VERSION", packet->type);
        if (packet != nullptr) {
            delete packet->payload;
            delete packet;
        }
        return -1;
    }

    /* TODO: are we sure there are 4 bytes ready? */
    uint32_t version = packet->payload->in_uint32_be();
    syslog(LOG_INFO, "SFTP server version %d", version);

    // TODO: Horrible, fix that

    while (1) {
        if (sizeof(uint32_t) > packet->payload->in_remain()) {
            break;
        }

        uint32_t ext_name_len = packet->payload->in_uint32_be();
        if (ext_name_len > packet->payload->in_remain()) {
            break;
        }

        ext_name = new char[ext_name_len + 1];
        packet->payload->buffer_get_data(ext_name, ext_name_len);
        ext_name[ext_name_len] = 0;

        if (sizeof(uint32_t) > packet->payload->in_remain()) {
            break;
        }

        uint32_t ext_data_len = packet->payload->in_uint32_be();
        if (ext_data_len > packet->payload->in_remain()) {
            break;
        }

        ext_data =  new char[ext_data_len + 1];
        packet->payload->buffer_get_data(ext_data, ext_data_len);
        ext_data[ext_data_len] = 0;

        syslog(LOG_INFO, "SFTP server extension: %s, version: %s", ext_name, ext_data);

        char ** tmp = static_cast<char**>(realloc(sftp->ext->name, (sftp->ext->count+1) * sizeof(char *)));
        tmp[sftp->ext->count] = ext_name;
        sftp->ext->name = tmp;

        char ** tmp2 = static_cast<char**>(realloc(sftp->ext->data, (sftp->ext->count+1) * sizeof(char *)));
        tmp2[sftp->ext->count] = ext_data;
        sftp->ext->data = tmp2;

        sftp->ext->count++;
    }
    if (packet != nullptr) {
        delete packet->payload;
        delete packet;
    }
    sftp->version = sftp->server_version = version;

    return 0;
}

unsigned int sftp_extensions_get_count(sftp_session_struct* sftp)
{
    if (sftp == nullptr || sftp->ext == nullptr) {
        return 0;
    }

    return sftp->ext->count;
}

const char *sftp_extensions_get_name(sftp_session_struct* sftp, unsigned int idx)
{
    if (sftp == nullptr){
        return nullptr;
    }
    if (sftp->ext == nullptr || sftp->ext->name == nullptr) {
        ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return nullptr;
    }

    if (idx > sftp->ext->count) {
        ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return nullptr;
    }
    return sftp->ext->name[idx];
}

const char *sftp_extensions_get_data(sftp_session_struct* sftp, unsigned int idx)
{
    if (sftp == nullptr){
        return nullptr;
    }

    if (sftp->ext == nullptr || sftp->ext->name == nullptr) {
        ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return nullptr;
    }

    if (idx > sftp->ext->count) {
        ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return nullptr;
    }

    return sftp->ext->data[idx];
}

int sftp_extension_supported(sftp_session_struct* sftp, const char *name, const char *data)
{

    if (sftp == nullptr || name == nullptr || data == nullptr) {
        return 0;
    }

    int n = sftp_extensions_get_count(sftp);

    int i;
    for (i = 0; i < n; i++) {
        const char *ext_name = sftp_extensions_get_name(sftp, i);
        const char *ext_data = sftp_extensions_get_data(sftp, i);

        if (ext_name != nullptr
        && ext_data != nullptr
        && strcmp(ext_name, name) == 0
        && strcmp(ext_data, data) == 0) {
            return 1;
        }
    }

    return 0;
}

static int sftp_enqueue(sftp_session_struct* sftp, sftp_message_struct* msg)
{
    sftp_request_queue_struct * queue = new sftp_request_queue_struct;
    queue->message = msg;

    syslog(LOG_INFO, "Queued msg type %d id %d", msg->id, msg->packet_type);

    if(sftp->queue == nullptr) {
        sftp->queue = queue;
    }
    else {
        sftp_request_queue_struct * ptr = sftp->queue;
        while(ptr->next) {
            ptr = ptr->next; /* find end of linked list */
        }
        ptr->next = queue; /* add it on bottom */
    }
    return 0;
}

/*
 * Pulls of a message from the queue based on the ID.
 * Returns nullptr if no message has been found.
 */
static sftp_message_struct* sftp_dequeue(sftp_session_struct* sftp, uint32_t id)
{
  sftp_request_queue_struct* prev = nullptr;
  sftp_request_queue_struct* queue;
  sftp_message_struct* msg;

  if(sftp->queue == nullptr) {
    return nullptr;
  }

  queue = sftp->queue;
  while (queue) {
    if(queue->message->id == id) {
      /* remove from queue */
      if (prev == nullptr) {
        sftp->queue = queue->next;
      } else {
        prev->next = queue->next;
      }
      msg = queue->message;
      if (queue != nullptr) {
          delete queue;
      }
      syslog(LOG_INFO,
          "Dequeued msg id %d type %d",
          msg->id,
          msg->packet_type);
      return msg;
    }
    prev = queue;
    queue = queue->next;
  }

  return nullptr;
}

/*
 * Assigns a new SFTP ID for new requests and assures there is no collision
 * between them.
 * Returns a new ID ready to use in a request
 */
static inline uint32_t sftp_get_new_id(sftp_session_struct* session) {
    return ++session->id_counter;
}

static sftp_status_message_struct* parse_status_msg(sftp_message_struct* msg)
{
    if (msg->packet_type != SSH_FXP_STATUS) {
        ssh_set_error(msg->sftp->session->error, SSH_FATAL,
            "Not a ssh_fxp_status message passed in!");
        return nullptr;
    }

    sftp_status_message_struct * status = new sftp_status_message_struct;
    status->id = msg->id;
    status->status = msg->payload->in_uint32_be();

    if (sizeof(uint32_t) > msg->payload->in_remain()){
        if(msg->sftp->version >=3){
            delete status;
            ssh_set_error(msg->sftp->session->error, SSH_FATAL,
                "Invalid SSH_FXP_STATUS message");
            return nullptr;
        }
    }
    uint32_t error_len = msg->payload->in_uint32_be();
    if (error_len > msg->payload->in_remain()){
        if(msg->sftp->version >=3){
            delete status;
            ssh_set_error(msg->sftp->session->error, SSH_FATAL,
                "Invalid SSH_FXP_STATUS message");
            return nullptr;
        }
    }
    status->error = SSHString(error_len);
    msg->payload->buffer_get_data(status->error.data.get(), error_len);

    if (sizeof(uint32_t) > msg->payload->in_remain()){
        if(msg->sftp->version >=3){
            delete status;
            ssh_set_error(msg->sftp->session->error, SSH_FATAL,
                "Invalid SSH_FXP_STATUS message");
            return nullptr;
        }
    }
    uint32_t errormsg_len = msg->payload->in_uint32_be();
    if (errormsg_len > msg->payload->in_remain()){
        if(msg->sftp->version >=3){
            delete status;
            ssh_set_error(msg->sftp->session->error, SSH_FATAL,
                "Invalid SSH_FXP_STATUS message");
            return nullptr;
        }
    }
    msg->payload->buffer_get_data(status->errormsg, errormsg_len);
    status->errormsg[errormsg_len] = 0;

    char * tmp = new char[0];
    status->langmsg = tmp;

    return status;
}

static sftp_file_struct* parse_handle_msg(sftp_message_struct* msg)
{

    if(msg->packet_type != SSH_FXP_HANDLE) {
        ssh_set_error(msg->sftp->session->error, SSH_FATAL,
            "Not a ssh_fxp_handle message passed in!");
        return nullptr;
    }

    sftp_file_struct * file = new sftp_file_struct;

    if (sizeof(uint32_t) > msg->payload->in_remain()) {
        // ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    uint32_t tmp_handle_len = msg->payload->in_uint32_be();
    if (tmp_handle_len > msg->payload->in_remain()) {
        // ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }

    SSHString tmp_handle(tmp_handle_len);
    msg->payload->buffer_get_data(tmp_handle.data.get(),tmp_handle_len);

    file->handle = std::move(tmp_handle);

    file->sftp = msg->sftp;
    file->offset = 0;
    file->eof = 0;

    return file;
}

/* Open a directory */
sftp_dir_struct* sftp_opendir(sftp_session_struct* sftp, const char *path)
{
    sftp_message_struct* msg = nullptr;
    sftp_file_struct* file = nullptr;
    sftp_dir_struct* dir = nullptr;
    sftp_status_message_struct* status;
    ssh_buffer_struct* payload;
    uint32_t id;

    payload = new ssh_buffer_struct;
    // TODO: check memory allocation

    id = sftp_get_new_id(sftp);
    payload->out_uint32_le(id);

    if (sftp_packet_write(sftp, SSH_FXP_OPENDIR, payload) < 0) {
        delete payload;
        return nullptr;
    }
    delete payload;

    while (msg == nullptr) {
        if (sftp_read_and_dispatch(sftp) < 0) {
            /* something nasty has happened */
            return nullptr;
        }
        msg = sftp_dequeue(sftp, id);
    }

    switch (msg->packet_type) {
    case SSH_FXP_STATUS:
        status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (status == nullptr) {
            return nullptr;
        }
        sftp_set_error(sftp, status->status);
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED, "SFTP server: %s", status->errormsg);
        if (status != nullptr) {
            delete [] status->errormsg;
            delete [] status->langmsg;
            delete status;
        }
        return nullptr;

    case SSH_FXP_HANDLE:
        file = parse_handle_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (file != nullptr) {
            dir = new sftp_dir_struct;
            dir->sftp = sftp;
            size_t tmplen = strlen(path)+1;
            char * tmp = new char[tmplen];
            memcpy(tmp, path, tmplen);
            dir->name = tmp;
            if (dir->name == nullptr) {
              delete dir;
              delete file;
              return nullptr;
            }
            dir->handle = std::move(file->handle);
            memcpy(dir->handle.data.get(), file->handle.data.get(), file->handle.size);

            delete file;
        }
        return dir;

    default:
        ssh_set_error(sftp->session->error, SSH_FATAL,
          "Received message %d during opendir!", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    }

    return nullptr;
}


/*
 * Parse the attributes from a payload from some messages. It is coded on
 * baselines from the protocol version 4.
 * This code is more or less dead but maybe we need it in future.
 */
static sftp_attributes_struct* sftp_parse_attr_4(sftp_session_struct* sftp, ssh_buffer_struct* buf, int expectnames)
{
    int ok = 0;

    /* unused member variable */
    (void) expectnames;

    sftp_attributes_struct * attr = new sftp_attributes_struct();
    // TODO: check memory allocation

    /* This isn't really a loop, but it is like a try..catch.. */
    do {
        attr->flags = buf->in_uint32_be();

        if (attr->flags & SSH_FILEXFER_ATTR_SIZE) {
            attr->size = buf->in_uint64_be();
        }

        if (attr->flags & SSH_FILEXFER_ATTR_OWNERGROUP) {
            if (sizeof(uint32_t) > buf->in_remain()){
                break;
            }
            uint32_t owner_length = buf->in_uint32_be();
            if (owner_length > buf->in_remain()){
                break;
            }
            attr->owner =  new char[owner_length + 1];
            buf->buffer_get_data(attr->owner, owner_length);
            attr->owner[owner_length] = 0;

            if (sizeof(uint32_t) > buf->in_remain()){
                break;
            }
            uint32_t group_length = buf->in_uint32_be();
            if (group_length > buf->in_remain()){
                break;
            }
            attr->group =  new char[group_length + 1];
            buf->buffer_get_data(attr->group, group_length);
            attr->group[group_length] = 0;
        }

        if (attr->flags & SSH_FILEXFER_ATTR_PERMISSIONS) {
            attr->permissions = buf->in_uint32_be();

            /* FIXME on windows! */
            switch (attr->permissions & S_IFMT) {
            case S_IFSOCK:
            case S_IFBLK:
            case S_IFCHR:
            case S_IFIFO:
                attr->type = SSH_FILEXFER_TYPE_SPECIAL;
            break;
            case S_IFLNK:
                attr->type = SSH_FILEXFER_TYPE_SYMLINK;
            break;
            case S_IFREG:
                attr->type = SSH_FILEXFER_TYPE_REGULAR;
            break;
            case S_IFDIR:
                attr->type = SSH_FILEXFER_TYPE_DIRECTORY;
            break;
            default:
                attr->type = SSH_FILEXFER_TYPE_UNKNOWN;
            break;
            }
        }

        if (attr->flags & SSH_FILEXFER_ATTR_ACCESSTIME) {
            attr->atime64 = buf->in_uint64_be();
        }

        if (attr->flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES) {
            attr->atime_nseconds = buf->in_uint32_be();
        }

        if (attr->flags & SSH_FILEXFER_ATTR_CREATETIME) {
            attr->createtime = buf->in_uint64_be();
        }

        if (attr->flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES) {
            attr->createtime_nseconds = buf->in_uint32_be();
        }

        if (attr->flags & SSH_FILEXFER_ATTR_MODIFYTIME) {
            attr->mtime64 = buf->in_uint64_be();
        }

        if (attr->flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES) {
            attr->mtime_nseconds = buf->in_uint32_be();
        }

        if (attr->flags & SSH_FILEXFER_ATTR_ACL) {
            if (sizeof(uint32_t) > buf->in_remain()){
                break;
            }
            uint32_t acl_length = buf->in_uint32_be();
            if (acl_length > buf->in_remain()){
                break;
            }
            attr->acl = SSHString(acl_length);
            buf->buffer_get_data(attr->acl.data.get(), acl_length);
        }

        if (attr->flags & SSH_FILEXFER_ATTR_EXTENDED) {
            attr->extended_count = buf->in_uint32_be();

            while (attr->extended_count){
                if (sizeof(uint32_t) > buf->in_remain()){
                    break;
                }
                uint32_t extended_type_len = buf->in_uint32_be();
                if (extended_type_len > buf->in_remain()){
                    break;
                }
                attr->extended_type = SSHString(extended_type_len);
                buf->buffer_get_data(attr->extended_type.data.get(), extended_type_len);

                if (sizeof(uint32_t) > buf->in_remain()){
                    break;
                }
                uint32_t extended_data_len = buf->in_uint32_be();
                if (extended_data_len > buf->in_remain()){
                    break;
                }
                attr->extended_data = SSHString(extended_data_len);
                buf->buffer_get_data(attr->extended_data.data.get(), extended_data_len);

                attr->extended_count--;
            }

            if (attr->extended_count) {
                break;
            }
        }
        ok = 1;
    } while (0);

    if (ok == 0) {
        /* break issued somewhere */
        delete [] attr->owner;
        delete [] attr->group;
        delete attr;

        ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid ATTR structure");

        return nullptr;
    }
    return attr;
}

enum sftp_longname_field_e {
    SFTP_LONGNAME_PERM = 0,
    SFTP_LONGNAME_FIXME,
    SFTP_LONGNAME_OWNER,
    SFTP_LONGNAME_GROUP,
    SFTP_LONGNAME_SIZE,
    SFTP_LONGNAME_DATE,
    SFTP_LONGNAME_TIME,
    SFTP_LONGNAME_NAME,
};

static char *sftp_parse_longname(const char *longname,  enum sftp_longname_field_e longname_field)
{
    const char *q;
    size_t len, field = 0;
    char *x;

    const char * p = longname;
    /* Find the beginning of the field which is specified by sftp_longanme_field_e. */
    while(field != longname_field) {
        if(isspace(*p)) {
            field++;
            p++;
            while(*p && isspace(*p)) {
                p++;
            }
        }
        else {
            p++;
        }
    }

    q = p;
    while (!isspace(*q)) {
        q++;
    }

    /* There is no strndup on windows */
    len = q - p + 1;
    x =  new char[len];
    if (x == nullptr) {
        return nullptr;
    }
    snprintf(x, len, "%s", p);

    return x;
}

/* sftp version 0-3 code. It is different from the v4 */
/* maybe a paste of the draft is better than the code */
/*
        uint32   flags
        uint64   size           present only if flag SSH_FILEXFER_ATTR_SIZE
        uint32   uid            present only if flag SSH_FILEXFER_ATTR_UIDGID
        uint32   gid            present only if flag SSH_FILEXFER_ATTR_UIDGID
        uint32   permissions    present only if flag SSH_FILEXFER_ATTR_PERMISSIONS
        uint32   atime          present only if flag SSH_FILEXFER_ACCESSTIME
        uint32   mtime          present only if flag SSH_FILEXFER_ACCESSTIME
        uint32   extended_count present only if flag SSH_FILEXFER_ATTR_EXTENDED
        string   extended_type
        string   extended_data
        ...      more extended data (extended_type - extended_data pairs),
                   so that number of pairs equals extended_count              */
static sftp_attributes_struct* sftp_parse_attr_3(sftp_session_struct* sftp, ssh_buffer_struct* buf, int expectname)
{
    uint32_t flags = 0;
    int ok = 0;

    sftp_attributes_struct * attr = new sftp_attributes_struct();
    // TODO: check memory allocation

    /* This isn't really a loop, but it is like a try..catch.. */
    do {
      if (expectname) {
        if (sizeof(uint32_t) > buf->in_remain()){
            break;
        }
        uint32_t name_length = buf->in_uint32_be();
        if (name_length > buf->in_remain()){
            break;
        }
        attr->name = new char[name_length + 1];
        buf->buffer_get_data(attr->name, name_length);
        attr->name[name_length] = 0;

        syslog(LOG_INFO, "Name: %s", attr->name);

        if (sizeof(uint32_t) > buf->in_remain()){
            break;
        }
        uint32_t longname_length = buf->in_uint32_be();
        if (longname_length > buf->in_remain()){
            break;
        }
        attr->longname =  new char[longname_length + 1];
        buf->buffer_get_data(attr->longname, longname_length);
        attr->longname[longname_length] = 0;

        syslog(LOG_INFO, "Name: %s", attr->name);


        /* Set owner and group if we talk to openssh and have the longname */
        if (sftp->session->openssh) {
            attr->owner = sftp_parse_longname(attr->longname, SFTP_LONGNAME_OWNER);
            if (attr->owner == nullptr) {
                break;
            }

            attr->group = sftp_parse_longname(attr->longname, SFTP_LONGNAME_GROUP);
            if (attr->group == nullptr) {
                break;
            }
        }
    }

    attr->flags = buf->in_uint32_be();
    syslog(LOG_INFO,
        "Flags: %.8lx\n", static_cast<long unsigned int>(flags));

    if (flags & SSH_FILEXFER_ATTR_SIZE) {
      attr->size = buf->in_uint64_be();
      syslog(LOG_INFO,
          "Size: %llu\n",
          static_cast<long long unsigned int>(attr->size));
    }

    if (flags & SSH_FILEXFER_ATTR_UIDGID) {
      attr->uid = buf->in_uint32_be();
      attr->gid = buf->in_uint32_be();
    }

    if (flags & SSH_FILEXFER_ATTR_PERMISSIONS) {
      attr->permissions = buf->in_uint32_be();

      switch (attr->permissions & S_IFMT) {
        case S_IFSOCK:
        case S_IFBLK:
        case S_IFCHR:
        case S_IFIFO:
          attr->type = SSH_FILEXFER_TYPE_SPECIAL;
          break;
        case S_IFLNK:
          attr->type = SSH_FILEXFER_TYPE_SYMLINK;
          break;
        case S_IFREG:
          attr->type = SSH_FILEXFER_TYPE_REGULAR;
          break;
        case S_IFDIR:
          attr->type = SSH_FILEXFER_TYPE_DIRECTORY;
          break;
        default:
          attr->type = SSH_FILEXFER_TYPE_UNKNOWN;
          break;
      }
    }

    if (flags & SSH_FILEXFER_ATTR_ACCESSTIME) {
      attr->atime = buf->in_uint32_be();
      attr->mtime = buf->in_uint32_be();
    }

    if (flags & SSH_FILEXFER_ATTR_EXTENDED) {
        attr->extended_count = buf->in_uint32_be();

        while (attr->extended_count){
            if (sizeof(uint32_t) > buf->in_remain()){
                break;
            }
            uint32_t extended_type_len = buf->in_uint32_be();
            if (extended_type_len > buf->in_remain()){
                break;
            }
            attr->extended_type = SSHString(extended_type_len);
            buf->buffer_get_data(attr->extended_type.data.get(), extended_type_len);

            if (sizeof(uint32_t) > buf->in_remain()){
                break;
            }
            uint32_t extended_data_len = buf->in_uint32_be();
            if (extended_data_len > buf->in_remain()){
                break;
            }
            attr->extended_data = SSHString(extended_data_len);
            buf->buffer_get_data(attr->extended_data.data.get(), extended_data_len);

            attr->extended_count--;
        }
        if (attr->extended_count) {
            break;
        }
    }
    ok = 1;
  } while (0);

  if (!ok) {
    /* break issued somewhere */
    delete [] attr->name;
    delete [] attr->longname;
    delete [] attr->owner;
    delete [] attr->group;
    delete attr;

    ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid ATTR structure");

    return nullptr;
  }

  /* everything went smoothly */
  return attr;
}

/* FIXME is this really needed as a public function? */
int buffer_add_attributes(ssh_buffer_struct* buffer, sftp_attributes_struct* attr)
{
    uint32_t flags = (attr ? attr->flags : 0);

    flags &= (SSH_FILEXFER_ATTR_SIZE
            | SSH_FILEXFER_ATTR_UIDGID
            | SSH_FILEXFER_ATTR_PERMISSIONS
            | SSH_FILEXFER_ATTR_ACCESSTIME);

    buffer->out_uint32_be(flags);

    if (attr) {
        if (flags & SSH_FILEXFER_ATTR_SIZE) {
            // TODO: see error checking
            buffer->out_uint64_be(attr->size);
        }

        if (flags & SSH_FILEXFER_ATTR_UIDGID) {
            buffer->out_uint32_be(attr->uid);
            buffer->out_uint32_be(attr->gid);
        }

        if (flags & SSH_FILEXFER_ATTR_PERMISSIONS) {
            buffer->out_uint32_be(attr->permissions);
            return -1;
        }

        if (flags & SSH_FILEXFER_ATTR_ACCESSTIME) {
            buffer->out_uint32_be(attr->atime);
            buffer->out_uint32_be(attr->mtime);
        }
    }

    return 0;
}


sftp_attributes_struct* sftp_parse_attr(sftp_session_struct* sftp, ssh_buffer_struct* buf, int expectname)
{
    switch(sftp->version) {
    case 4:
        return sftp_parse_attr_4(sftp, buf, expectname);
    case 3:
    case 2:
    case 1:
    case 0:
        return sftp_parse_attr_3(sftp, buf, expectname);
    default:
        ssh_set_error(sftp->session->error, SSH_FATAL,
          "Version %d unsupported by client", sftp->server_version);
        return nullptr;
    }
}

/* Get the version of the SFTP protocol supported by the server */
int sftp_server_version(sftp_session_struct* sftp)
{
    return sftp->server_version;
}

/* Get a single file attributes structure of a directory. */
sftp_attributes_struct* sftp_readdir(sftp_session_struct* sftp, sftp_dir_struct* dir)
{
    sftp_message_struct* msg = nullptr;
    sftp_status_message_struct* status;
    sftp_attributes_struct* attr;
    ssh_buffer_struct* payload;
    uint32_t id;

    if (dir->buffer == nullptr) {
        payload = new ssh_buffer_struct;
        // TODO: check memory allocation

        id = sftp_get_new_id(sftp);
        payload->out_uint32_be(id);

        payload->out_uint32_be(dir->handle.size);
        payload->out_blob(dir->handle.data.get(), dir->handle.size);

        if (sftp_packet_write(sftp, SSH_FXP_READDIR, payload) < 0) {
            delete payload;
            return nullptr;
        }
        delete payload;

        syslog(LOG_INFO, "Sent a ssh_fxp_readdir with id %d", id);

        while (msg == nullptr) {
            if (sftp_read_and_dispatch(sftp) < 0) {
                /* something nasty has happened */
                return nullptr;
            }
            msg = sftp_dequeue(sftp, id);
        }

        switch (msg->packet_type){
        case SSH_FXP_STATUS:
            status = parse_status_msg(msg);
            if (msg != nullptr) {
                delete msg->payload;
                delete msg;
            }
            if (status == nullptr) {
              return nullptr;
            }
            sftp_set_error(sftp, status->status);
            switch (status->status) {
              case SSH_FX_EOF:
                dir->eof = 1;
                if (status != nullptr) {
                    delete [] status->errormsg;
                    delete [] status->langmsg;
                    delete status;
                }
                return nullptr;
              default:
                break;
            }

            ssh_set_error(sftp->session->error, SSH_FATAL,
                "Unknown error status: %d", status->status);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
            return nullptr;
        case SSH_FXP_NAME:
            dir->count = msg->payload->in_uint32_be();
            dir->buffer = msg->payload;
            msg->payload = nullptr;
            if (msg != nullptr) {
                delete msg->payload;
                delete msg;
            }
        break;
        default:
            ssh_set_error(sftp->session->error, SSH_FATAL,
                "Unsupported message back %d", msg->packet_type);
            if (msg != nullptr) {
                delete msg->payload;
                delete msg;
            }
            return nullptr;
        }
    }

    /* now dir->buffer contains a buffer and dir->count != 0 */
    if (dir->count == 0) {
        ssh_set_error(sftp->session->error, SSH_FATAL,
            "Count of files sent by the server is zero, which is invalid, or "
            "libsftp bug");
        return nullptr;
    }

    syslog(LOG_INFO, "Count is %d", dir->count);

    attr = sftp_parse_attr(sftp, dir->buffer, 1);
    if (attr == nullptr) {
        ssh_set_error(sftp->session->error, SSH_FATAL,
            "Couldn't parse the SFTP attributes");
        return nullptr;
    }

    dir->count--;
    if (dir->count == 0) {
        delete dir->buffer;
        dir->buffer = nullptr;
    }
    return attr;
}

/* Tell if the directory has reached EOF (End Of File). */
int sftp_dir_eof(sftp_dir_struct* dir)
{
    return dir->eof;
}

static int sftp_handle_close(sftp_session_struct* sftp, const SSHString & handle)
{
    sftp_status_message_struct* status;
    sftp_message_struct* msg = nullptr;
    ssh_buffer_struct* buffer = nullptr;
    uint32_t id;

    buffer = new ssh_buffer_struct;
    // TODO: check memory allocation

    id = sftp_get_new_id(sftp);
    buffer->out_uint32_le(id);

    buffer->out_uint32_be(handle.size);
    buffer->out_blob(handle.data.get(), handle.size);

    if (sftp_packet_write(sftp, SSH_FXP_CLOSE ,buffer) < 0) {
        delete buffer;
        return -1;
    }
    delete buffer;

    while (msg == nullptr) {
        if (sftp_read_and_dispatch(sftp) < 0) {
            /* something nasty has happened */
            return -1;
        }
        msg = sftp_dequeue(sftp,id);
    }

    switch (msg->packet_type) {
    case SSH_FXP_STATUS:
        status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if(status == nullptr) {
            return -1;
        }
        sftp_set_error(sftp, status->status);
        switch (status->status) {
        case SSH_FX_OK:
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
            return 0;
        default:
        break;
        }

        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
          "SFTP server: %s", status->errormsg);
        if (status != nullptr) {
            delete [] status->errormsg;
            delete [] status->langmsg;
            delete status;
        }
        return -1;
    default:
        ssh_set_error(sftp->session->error, SSH_FATAL,
          "Received message %d during sftp_handle_close!", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    }

    return -1;
}

/* Close an open file handle. */
int sftp_close(sftp_file_struct* file)
{
    int err = SSH_NO_ERROR;

    delete [] file->name;
    file->name = nullptr;
    err = sftp_handle_close(file->sftp, file->handle);
    /* FIXME: check server response and implement errno */
    delete file;
    file = nullptr;

    return err;
}

/* Close an open directory. */
int sftp_closedir(sftp_dir_struct* dir)
{
    int err = SSH_NO_ERROR;

    delete [] dir->name;
    dir->name = nullptr;
    err = sftp_handle_close(dir->sftp, dir->handle);

    /* FIXME: check server response and implement errno */
    delete dir->buffer;
    delete dir;
    dir = nullptr;

    return err;
}

/* Open a file on the server. */
sftp_file_struct* sftp_open(sftp_session_struct* sftp, const char *file, int flags, mode_t mode)
{
    uint32_t id;

    ssh_buffer_struct* buffer = new ssh_buffer_struct;
    // TODO: check memory allocation

    sftp_attributes_struct attr;
    attr.permissions = mode;
    attr.flags = SSH_FILEXFER_ATTR_PERMISSIONS;

    uint32_t sftp_flags =
        /* if any of the other flag is set, READ should not be set initialy */
           (flags == O_RDONLY)      * SSH_FXF_READ
        | ((flags & O_WRONLY) != 0) * SSH_FXF_WRITE
        | ((flags & O_RDWR)   != 0) * (SSH_FXF_WRITE | SSH_FXF_READ)
        | ((flags & O_CREAT)  != 0) * SSH_FXF_CREAT
        | ((flags & O_TRUNC)  != 0) * SSH_FXF_TRUNC
        | ((flags & O_EXCL)   != 0) * SSH_FXF_EXCL;

    syslog(LOG_INFO,"Opening file %s with sftp flags %x", file, sftp_flags);
    id = sftp_get_new_id(sftp);
    buffer->out_uint32_le(id);

    // TODO: check memory allocation

    buffer->out_uint32_be(strlen(file));
    buffer->out_blob(file, strlen(file));

    buffer->out_uint32_be(sftp_flags);
    buffer_add_attributes(buffer, &attr);
    if (sftp_packet_write(sftp, SSH_FXP_OPEN, buffer) < 0) {
        delete buffer;
        return nullptr;
    }
    delete buffer;

    sftp_message_struct* msg = nullptr;
    while (msg == nullptr) {
        if (sftp_read_and_dispatch(sftp) < 0) {
          /* something nasty has happened */
          return nullptr;
        }
        msg = sftp_dequeue(sftp, id);
    }

    switch (msg->packet_type) {
    case SSH_FXP_STATUS:
    {
        sftp_status_message_struct* status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (status == nullptr) {
        return nullptr;
        }
        sftp_set_error(sftp, status->status);
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
          "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
    }
    return nullptr;
    case SSH_FXP_HANDLE:
    {
        sftp_file_struct* handle = parse_handle_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        return handle;
    }
    default:
    {
        ssh_set_error(sftp->session->error, SSH_FATAL,
          "Received message %d during open!", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    }
    }

  return nullptr;
}

void sftp_file_set_nonblocking(sftp_file_struct* handle){
    handle->nonblocking=1;
}

void sftp_file_set_blocking(sftp_file_struct* handle){
    handle->nonblocking=0;
}

ssize_t sftp_write(sftp_file_struct* file, const void *buf, size_t count)
{
    sftp_session_struct* sftp = file->sftp;
    sftp_message_struct* msg = nullptr;
    sftp_status_message_struct* status;

    ssh_buffer_struct* buffer = new ssh_buffer_struct;
    // TODO: check memory allocation

    SSHString datastring(static_cast<uint32_t>(count));
    memcpy(datastring.data.get(), buf, count);

    uint32_t id = sftp_get_new_id(file->sftp);

    // TODO: See error support
    buffer->out_uint32_le(id);

    buffer->out_uint32_be(file->handle.size);
    buffer->out_blob(file->handle.data.get(), file->handle.size);

    buffer->out_uint64_be(file->offset);

    buffer->out_uint32_be(datastring.size);
    buffer->out_blob(datastring.data.get(), datastring.size);

    int packetlen = buffer->in_remain();
    int len = sftp_packet_write(file->sftp, SSH_FXP_WRITE, buffer);
    delete buffer;
    if (len < 0) {
        return -1;
    } else  if (len != packetlen) {
        syslog(LOG_INFO,
            "Could not write as much data as expected");
    }

    while (msg == nullptr) {
        if (sftp_read_and_dispatch(file->sftp) < 0) {
          /* something nasty has happened */
          return -1;
        }
        msg = sftp_dequeue(file->sftp, id);
    }

    switch (msg->packet_type) {
    case SSH_FXP_STATUS:
        status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (status == nullptr) {
            return -1;
        }
        sftp_set_error(sftp, status->status);
        switch (status->status) {
        case SSH_FX_OK:
            file->offset += count;
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
            return count;
        default:
          break;
        }
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
          "SFTP server: %s", status->errormsg);
        file->offset += count;
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
        return -1;
    default:
        ssh_set_error(sftp->session->error, SSH_FATAL,
            "Received message %d during write!", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        return -1;
    }
}

/* Seek to a specific location in a file. */
int sftp_seek(sftp_file_struct* file, uint32_t new_offset)
{
    if (file == nullptr) {
        return -1;
    }

    file->offset = new_offset;
    file->eof = 0;

    return 0;
}

int sftp_seek64(sftp_file_struct* file, uint64_t new_offset) {
    if (file == nullptr) {
        return -1;
    }

    file->offset = new_offset;
    file->eof = 0;

    return 0;
}

/* Report current byte position in file. */
unsigned long sftp_tell(sftp_file_struct* file) {
    return file->offset;
}
/* Report current byte position in file. */
uint64_t sftp_tell64(sftp_file_struct* file) {
    return file->offset;
}

/* Rewinds the position of the file pointer to the beginning of the file.*/
void sftp_rewind(sftp_file_struct* file) {
    file->offset = 0;
    file->eof = 0;
}

/* code written by Nick */
int sftp_unlink(sftp_session_struct* sftp, const char *file) {
    sftp_status_message_struct* status = nullptr;
    sftp_message_struct* msg = nullptr;
    ssh_buffer_struct* buffer;
    uint32_t id;

    buffer = new ssh_buffer_struct;
    // TODO: check memory allocation

    id = sftp_get_new_id(sftp);
    buffer->out_uint32_le(id);
    buffer->out_uint32_be(strlen(file));
    buffer->out_blob(file, strlen(file));

    if (sftp_packet_write(sftp, SSH_FXP_REMOVE, buffer) < 0) {
        delete buffer;
        return -1;
    }
    delete buffer;

    while (msg == nullptr) {
        if (sftp_read_and_dispatch(sftp)) {
            return -1;
        }
        msg = sftp_dequeue(sftp, id);
    }

    if (msg->packet_type == SSH_FXP_STATUS) {
        /* by specification, this command's only supposed to return SSH_FXP_STATUS */
        status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (status == nullptr) {
            return -1;
        }
        sftp_set_error(sftp, status->status);
        switch (status->status) {
        case SSH_FX_OK:
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
            return 0;
        default:
            break;
        }

        /*
         * The status should be SSH_FX_OK if the command was successful, if it
         * didn't, then there was an error
         */
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
            "SFTP server: %s", status->errormsg);
        if (status != nullptr) {
            delete [] status->errormsg;
            delete [] status->langmsg;
            delete status;
        }
        return -1;
    }
    else {
        ssh_set_error(sftp->session->error,SSH_FATAL,
            "Received message %d when attempting to remove file", msg->packet_type);
            if (msg != nullptr) {
                delete msg->payload;
                delete msg;
            }
    }

    return -1;
}

/* code written by Nick */
int sftp_rmdir(sftp_session_struct* sftp, const char *directory)
{
    sftp_status_message_struct* status = nullptr;
    sftp_message_struct* msg = nullptr;
    ssh_buffer_struct* buffer;
    uint32_t id;

    buffer = new ssh_buffer_struct;
    id = sftp_get_new_id(sftp);
    buffer->out_uint32_le(id);
    uint32_t size = strlen(directory);
    buffer->out_uint32_be(size);
    buffer->out_blob(directory, size);

    if (sftp_packet_write(sftp, SSH_FXP_RMDIR, buffer) < 0) {
        delete buffer;
        return -1;
    }
    delete buffer;

    while (msg == nullptr) {
        if (sftp_read_and_dispatch(sftp) < 0) {
            return -1;
        }
        msg = sftp_dequeue(sftp, id);
    }

  /* By specification, this command returns SSH_FXP_STATUS */
    if (msg->packet_type == SSH_FXP_STATUS) {
        status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (status == nullptr) {
            return -1;
        }
        sftp_set_error(sftp, status->status);
        switch (status->status) {
        case SSH_FX_OK:
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
            return 0;
        default:
            break;
        }
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
            "SFTP server: %s", status->errormsg);
        if (status != nullptr) {
            delete [] status->errormsg;
            delete [] status->langmsg;
            delete status;
        }
        return -1;
    }
    else {
        ssh_set_error(sftp->session->error, SSH_FATAL,
            "Received message %d when attempting to remove directory",
            msg->packet_type);
            if (msg != nullptr) {
                delete msg->payload;
                delete msg;
            }
      }

    return -1;
}

/* Code written by Nick */
int sftp_mkdir(sftp_session_struct* sftp, const char *directory, mode_t mode)
{
  sftp_status_message_struct* status = nullptr;
  sftp_message_struct* msg = nullptr;
  sftp_attributes_struct* errno_attr = nullptr;
  ssh_buffer_struct* buffer;
  uint32_t id;

  buffer = new ssh_buffer_struct;
  sftp_attributes_struct attr;
  attr.permissions = mode;
  attr.flags = SSH_FILEXFER_ATTR_PERMISSIONS;

  id = sftp_get_new_id(sftp);
  buffer->out_uint32_le(id);
  uint32_t size = strlen(directory);
  buffer->out_uint32_be(size);
  buffer->out_blob(directory, size);

  buffer_add_attributes(buffer, &attr);
  if (sftp_packet_write(sftp, SSH_FXP_MKDIR, buffer) < 0) {
    delete buffer;
    return -1;
  }
  delete buffer;

  while (msg == nullptr) {
    if (sftp_read_and_dispatch(sftp) < 0) {
      return -1;
    }
    msg = sftp_dequeue(sftp, id);
  }

  /* By specification, this command only returns SSH_FXP_STATUS */
  if (msg->packet_type == SSH_FXP_STATUS) {
    status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    if (status == nullptr) {
      return -1;
    }
    sftp_set_error(sftp, status->status);
    switch (status->status) {
      case SSH_FX_FAILURE:
        /*
         * mkdir always returns a failure, even if the path already exists.
         * To be POSIX conform and to be able to map it to EEXIST a stat
         * call is needed here.
         */
        errno_attr = sftp_lstat(sftp, directory);
        if (errno_attr != nullptr) {
          delete errno_attr;
          errno_attr = nullptr;
          sftp_set_error(sftp, SSH_FX_FILE_ALREADY_EXISTS);
        }
        break;
      case SSH_FX_OK:
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
        return 0;
      default:
        break;
    }
    /*
     * The status should be SSH_FX_OK if the command was successful, if it
     * didn't, then there was an error
     */
    ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
        "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
    return -1;
  } else {
    ssh_set_error(sftp->session->error, SSH_FATAL,
        "Received message %d when attempting to make directory",
        msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
  }

  return -1;
}

/* code written by nick */
int sftp_rename(sftp_session_struct* sftp, const char *original, const char *newname) {
  sftp_status_message_struct* status = nullptr;
  sftp_message_struct* msg = nullptr;
  ssh_buffer_struct* buffer;
  uint32_t id;

  buffer = new ssh_buffer_struct;
  id = sftp_get_new_id(sftp);
  buffer->out_uint32_le(id);
  uint32_t len1 = strlen(original);
  buffer->out_uint32_be(len1);
  buffer->out_blob(original, len1);
  uint32_t len2 = strlen(newname);
  buffer->out_uint32_be(len2);
  buffer->out_blob(newname, len2);

  /* POSIX rename atomically replaces newpath, we should do the same
   * only available on >=v4 */
   // TODO: check code below why not _be ?
  if (sftp->version>=4){
    buffer->out_uint32_le(SSH_FXF_RENAME_OVERWRITE);
  }

  if (sftp_packet_write(sftp, SSH_FXP_RENAME, buffer) < 0) {
    delete buffer;
    return -1;
  }
  delete buffer;

  while (msg == nullptr) {
    if (sftp_read_and_dispatch(sftp) < 0) {
      return -1;
    }
    msg = sftp_dequeue(sftp, id);
  }

  /* By specification, this command only returns SSH_FXP_STATUS */
  if (msg->packet_type == SSH_FXP_STATUS) {
    status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    if (status == nullptr) {
      return -1;
    }
    sftp_set_error(sftp, status->status);
    switch (status->status) {
      case SSH_FX_OK:
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
        return 0;
      default:
        break;
    }
    /*
     * Status should be SSH_FX_OK if the command was successful, if it didn't,
     * then there was an error
     */
    ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
        "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
    return -1;
  } else {
    ssh_set_error(sftp->session->error, SSH_FATAL,
        "Received message %d when attempting to rename",
        msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
  }

  return -1;
}

/* Code written by Nick */
/* Set file attributes on a file, directory or symbolic link. */
int sftp_setstat(sftp_session_struct* sftp, const char *file, sftp_attributes_struct* attr) {
  uint32_t id;
  ssh_buffer_struct* buffer;
  sftp_message_struct* msg = nullptr;
  sftp_status_message_struct* status = nullptr;

  buffer = new ssh_buffer_struct;
  id = sftp_get_new_id(sftp);
  buffer->out_uint32_le(id);

  uint32_t len1 = strlen(file);
  buffer->out_uint32_be(len1);
  buffer->out_blob(file, len1);

  buffer_add_attributes(buffer, attr);
  if (sftp_packet_write(sftp, SSH_FXP_SETSTAT, buffer) < 0) {
    delete buffer;
    return -1;
  }
  delete buffer;

  while (msg == nullptr) {
    if (sftp_read_and_dispatch(sftp) < 0) {
      return -1;
    }
    msg = sftp_dequeue(sftp, id);
  }

  /* By specification, this command only returns SSH_FXP_STATUS */
  if (msg->packet_type == SSH_FXP_STATUS) {
    status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    if (status == nullptr) {
      return -1;
    }
    sftp_set_error(sftp, status->status);
    switch (status->status) {
      case SSH_FX_OK:
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
        return 0;
      default:
        break;
    }
    /*
     * The status should be SSH_FX_OK if the command was successful, if it
     * didn't, then there was an error
     */
    ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
        "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
    return -1;
  } else {
    ssh_set_error(sftp->session->error, SSH_FATAL,
        "Received message %d when attempting to set stats", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
  }

  return -1;
}

/* Change the file owner and group */
int sftp_chown(sftp_session_struct* sftp, const char *file, uid_t owner, gid_t group) {
    sftp_attributes_struct attr;
    attr.uid = owner;
    attr.gid = group;
    attr.flags = SSH_FILEXFER_ATTR_UIDGID;
    return sftp_setstat(sftp, file, &attr);
}

/* Change permissions of a file */
int sftp_chmod(sftp_session_struct* sftp, const char *file, mode_t mode) {
    sftp_attributes_struct attr;
    attr.permissions = mode;
    attr.flags = SSH_FILEXFER_ATTR_PERMISSIONS;

    return sftp_setstat(sftp, file, &attr);
}

/* Change the last modification and access time of a file. */
int sftp_utimes(sftp_session_struct* sftp, const char *file, const struct timeval *times) {
    sftp_attributes_struct attr;

    attr.atime = times[0].tv_sec;
    attr.atime_nseconds = times[0].tv_usec;

    attr.mtime = times[1].tv_sec;
    attr.mtime_nseconds = times[1].tv_usec;
    attr.flags = SSH_FILEXFER_ATTR_ACCESSTIME
               | SSH_FILEXFER_ATTR_MODIFYTIME
               | SSH_FILEXFER_ATTR_SUBSECOND_TIMES;

  return sftp_setstat(sftp, file, &attr);
}

int sftp_symlink(sftp_session_struct* sftp, const char *target, const char *dest) {
  sftp_status_message_struct* status = nullptr;
  sftp_message_struct* msg = nullptr;
  ssh_buffer_struct* buffer;
  uint32_t id;

  if (sftp == nullptr)
    return -1;
  if (target == nullptr || dest == nullptr) {
    ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
    return -1;
  }

  buffer = new ssh_buffer_struct;
  SSHString target_s(target);
  SSHString dest_s(dest);

  id = sftp_get_new_id(sftp);
  buffer->out_uint32_le(id);
  if (sftp->session->openssh) {
    /* TODO check for version number if they ever fix it. */
    buffer->out_uint32_be(target_s.size);
    buffer->out_blob(target_s.data.get(), target_s.size);
    buffer->out_uint32_be(dest_s.size);
    buffer->out_blob(dest_s.data.get(), dest_s.size);

  }
  else {
    /* TODO check for version number if they ever fix it. */
    buffer->out_uint32_be(dest_s.size);
    buffer->out_blob(dest_s.data.get(), dest_s.size);
    buffer->out_uint32_be(target_s.size);
    buffer->out_blob(target_s.data.get(), target_s.size);
  }

  if (sftp_packet_write(sftp, SSH_FXP_SYMLINK, buffer) < 0) {
    delete buffer;
    return -1;
  }
  delete buffer;

  while (msg == nullptr) {
    if (sftp_read_and_dispatch(sftp) < 0) {
      return -1;
    }
    msg = sftp_dequeue(sftp, id);
  }

  /* By specification, this command only returns SSH_FXP_STATUS */
  if (msg->packet_type == SSH_FXP_STATUS) {
    status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    if (status == nullptr) {
      return -1;
    }
    sftp_set_error(sftp, status->status);
    switch (status->status) {
      case SSH_FX_OK:
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
        return 0;
      default:
        break;
    }
    /*
     * The status should be SSH_FX_OK if the command was successful, if it
     * didn't, then there was an error
     */
    ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
        "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
    return -1;
  } else {
    ssh_set_error(sftp->session->error, SSH_FATAL,
        "Received message %d when attempting to set stats", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
  }

  return -1;
}

char *sftp_readlink(sftp_session_struct* sftp, const char *path)
{
    sftp_status_message_struct* status = nullptr;
    sftp_message_struct* msg = nullptr;
    char *lnk;

    if (sftp == nullptr){
        return nullptr;
    }
    if (path == nullptr) {
      ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
      return nullptr;
    }
    if (sftp->version < 3){
      ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,"sftp version %d does not support sftp_readlink",sftp->version);
        return nullptr;
    }
    ssh_buffer_struct* buffer = new ssh_buffer_struct;

    SSHString path_s(path);

    uint32_t id = sftp_get_new_id(sftp);
    buffer->out_uint32_le(id);

    buffer->out_uint32_be(path_s.size);
    buffer->out_blob(path_s.data.get(), path_s.size);

    if (sftp_packet_write(sftp, SSH_FXP_READLINK, buffer) < 0) {
        delete buffer;
        return nullptr;
    }
    delete buffer;

  while (msg == nullptr) {
    if (sftp_read_and_dispatch(sftp) < 0) {
      return nullptr;
    }
    msg = sftp_dequeue(sftp, id);
  }

  switch (msg->packet_type)
  {
  case SSH_FXP_NAME:
  {
    /* we don't care about "count" */
    /* ignored = */ msg->payload->in_uint32_be();
    /* we only care about the file name string */

    if (sizeof(uint32_t) > msg->payload->in_remain()) {
        // ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    uint32_t link_s_len = msg->payload->in_uint32_be();
    if (link_s_len > msg->payload->in_remain()) {
        // ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    SSHString link_s(link_s_len);
    msg->payload->buffer_get_data(link_s.data.get(),link_s_len);

        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    lnk =  new char[link_s.size + 1];
    memcpy(lnk, link_s.data.get(), link_s.size);
    lnk[link_s.size] = 0;
    return lnk;
  }
  case SSH_FXP_STATUS:
  {
      /* bad response (error) */
      status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
      if (status == nullptr) {
        return nullptr;
      }
      ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
          "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
      break;
  }
  default: /* this shouldn't happen */
    ssh_set_error(sftp->session->error, SSH_FATAL,
       "Received message %d when attempting to set stats", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
  }

  return nullptr;
}

sftp_statvfs_struct* sftp_statvfs(sftp_session_struct* sftp, const char *path)
{
    sftp_status_message_struct* status = nullptr;
    sftp_message_struct* msg = nullptr;

    if (sftp == nullptr){
        return nullptr;
    }
    if (path == nullptr) {
        ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return nullptr;
    }
    if (sftp->version < 3){
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
            "sftp version %d does not support sftp_statvfs",sftp->version);
        return nullptr;
    }

    ssh_buffer_struct* buffer = new ssh_buffer_struct;
    SSHString ext("statvfs@openssh.com");

    SSHString pathstr(path);

    uint32_t id = sftp_get_new_id(sftp);
    buffer->out_uint32_le(id);

    buffer->out_uint32_be(ext.size);
    buffer->out_blob(ext.data.get(), ext.size);

    buffer->out_uint32_be(pathstr.size);
    buffer->out_blob(pathstr.data.get(), pathstr.size);

    if (sftp_packet_write(sftp, SSH_FXP_EXTENDED, buffer) < 0) {
        delete buffer;
        return nullptr;
    }
    delete buffer;

    while (msg == nullptr) {
        if (sftp_read_and_dispatch(sftp) < 0) {
          return nullptr;
        }
        msg = sftp_dequeue(sftp, id);
    }

    switch (msg->packet_type)
    {
    case SSH_FXP_EXTENDED_REPLY:
    {
        sftp_statvfs_struct * statvfs = new sftp_statvfs_struct;
        statvfs->f_bsize = msg->payload->in_uint64_be();
        statvfs->f_frsize = msg->payload->in_uint64_be();
        /* number of blocks (unit f_frsize) */
        statvfs->f_blocks = msg->payload->in_uint64_be();
        /* free blocks in file system */
        statvfs->f_bfree = msg->payload->in_uint64_be();
        /* free blocks for non-root */
        statvfs->f_bavail = msg->payload->in_uint64_be();
        /* total file inodes */
        statvfs->f_files = msg->payload->in_uint64_be();
        /* free file inodes */
        statvfs->f_ffree = msg->payload->in_uint64_be();
        /* free file inodes for to non-root */
        statvfs->f_favail = msg->payload->in_uint64_be();
        /* file system id */
        statvfs->f_fsid = msg->payload->in_uint64_be();
        /* bit mask of f_flag values */
        statvfs->f_flag = msg->payload->in_uint64_be();
        /* maximum filename length */
        statvfs->f_namemax = msg->payload->in_uint64_be();

        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        return statvfs;
    }
    case SSH_FXP_STATUS: /* bad response (error) */
    {
        status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (status == nullptr) {
          return nullptr;
        }
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
            "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
        break;
    }
    default:
        ssh_set_error(sftp->session->error, SSH_FATAL,
            "Received message %d when attempting to get statvfs", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    }
    return nullptr;
}

sftp_statvfs_struct* sftp_fstatvfs(sftp_file_struct* file) {
    sftp_status_message_struct* status = nullptr;
    sftp_message_struct* msg = nullptr;
    sftp_session_struct* sftp;
    ssh_buffer_struct* buffer;
    uint32_t id;

    if (file == nullptr) {
        return nullptr;
    }
    sftp = file->sftp;

    buffer = new ssh_buffer_struct;
    id = sftp_get_new_id(sftp);
    buffer->out_uint32_le(id);

    buffer->out_uint32_be(strlen("fstatvfs@openssh.com"));
    buffer->out_blob("fstatvfs@openssh.com", strlen("fstatvfs@openssh.com"));

    buffer->out_uint32_be(file->handle.size);
    buffer->out_blob(file->handle.data.get(), file->handle.size);

    if (sftp_packet_write(sftp, SSH_FXP_EXTENDED, buffer) < 0) {
        delete buffer;
        return nullptr;
    }
    delete buffer;

    while (msg == nullptr) {
        if (sftp_read_and_dispatch(sftp) < 0) {
            return nullptr;
        }
        msg = sftp_dequeue(sftp, id);
    }

    switch (msg->packet_type)
    {
    case SSH_FXP_EXTENDED_REPLY:
    {
        sftp_statvfs_struct * statvfs = new sftp_statvfs_struct;
        statvfs->f_bsize = msg->payload->in_uint64_be();
        statvfs->f_frsize = msg->payload->in_uint64_be();
        /* number of blocks (unit f_frsize) */
        statvfs->f_blocks = msg->payload->in_uint64_be();
        /* free blocks in file system */
        statvfs->f_bfree = msg->payload->in_uint64_be();
        /* free blocks for non-root */
        statvfs->f_bavail = msg->payload->in_uint64_be();
        /* total file inodes */
        statvfs->f_files = msg->payload->in_uint64_be();
        /* free file inodes */
        statvfs->f_ffree = msg->payload->in_uint64_be();
        /* free file inodes for to non-root */
        statvfs->f_favail = msg->payload->in_uint64_be();
        /* file system id */
        statvfs->f_fsid = msg->payload->in_uint64_be();
        /* bit mask of f_flag values */
        statvfs->f_flag = msg->payload->in_uint64_be();
        /* maximum filename length */
        statvfs->f_namemax = msg->payload->in_uint64_be();
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        return statvfs;
    }
    case SSH_FXP_STATUS:
    { /* bad response (error) */
        status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (status == nullptr) {
            return nullptr;
        }
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
            "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
        break;
    }
    default:
        ssh_set_error(sftp->session->error, SSH_FATAL,
            "Received message %d when attempting to set stats", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    }
    return nullptr;
}

void sftp_statvfs_free(sftp_statvfs_struct* statvfs) {
  if (statvfs == nullptr) {
    return;
  }

  delete statvfs;
  statvfs = nullptr;
}

/* another code written by Nick */
char *sftp_canonicalize_path(sftp_session_struct* sftp, const char *path)
{
    sftp_status_message_struct* status = nullptr;
    sftp_message_struct* msg = nullptr;
    char *cname;

    if (sftp == nullptr){
        return nullptr;
    }

    if (path == nullptr) {
        ssh_set_error(sftp->session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return nullptr;
    }

    ssh_buffer_struct* buffer = new ssh_buffer_struct;

    SSHString pathstr(path);

    uint32_t id = sftp_get_new_id(sftp);
    buffer->out_uint32_le(id);

    buffer->out_uint32_be(pathstr.size);
    buffer->out_blob(pathstr.data.get(), pathstr.size);

    if (sftp_packet_write(sftp, SSH_FXP_REALPATH, buffer) < 0) {
        delete buffer;
        return nullptr;
    }
    delete buffer;

    while (msg == nullptr) {
        if (sftp_read_and_dispatch(sftp) < 0) {
            return nullptr;
        }
        msg = sftp_dequeue(sftp, id);
    }

    switch (msg->packet_type)
    {
    case SSH_FXP_NAME:
    {
        /* we don't care about "count" */
        /* ignored = */ msg->payload->in_uint32_be();
        /* we only care about the file name string */

        if (sizeof(uint32_t) > msg->payload->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t name_len = msg->payload->in_uint32_be();
        if (name_len > msg->payload->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        SSHString name(name_len);
        msg->payload->buffer_get_data(name.data.get(),name_len);

        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }

        cname =  new char[name.size + 1];
        memcpy(cname, name.data.get(), name.size);
        cname[name.size] = 0;
        return cname;
    }
    case SSH_FXP_STATUS:
    { /* bad response (error) */
        status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (status == nullptr) {
          return nullptr;
        }
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
            "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
        break;
    }
    default: // this shouldn't happen
        ssh_set_error(sftp->session->error, SSH_FATAL,
            "Received message %d when attempting to set stats", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    }
    return nullptr;
}

static sftp_attributes_struct* sftp_xstat(sftp_session_struct* sftp, const char *path, int param)
{
    sftp_status_message_struct* status = nullptr;
    sftp_message_struct* msg = nullptr;
    ssh_buffer_struct* buffer;

    buffer = new ssh_buffer_struct;

    SSHString pathstr(path);

    uint32_t id = sftp_get_new_id(sftp);
    buffer->out_uint32_le(id);

    buffer->out_uint32_be(pathstr.size);
    buffer->out_blob(pathstr.data.get(), pathstr.size);

    if (sftp_packet_write(sftp, param, buffer) < 0) {
        delete buffer;
        return nullptr;
    }
    delete buffer;

    while (msg == nullptr) {
        if (sftp_read_and_dispatch(sftp) < 0) {
          return nullptr;
        }
        msg = sftp_dequeue(sftp, id);
    }

    switch (msg->packet_type)
    {
    case SSH_FXP_ATTRS:
    {
        sftp_attributes_struct* attr = sftp_parse_attr(sftp, msg->payload, 0);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        return attr;
    }

    case SSH_FXP_STATUS:
    {
        status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
        if (status == nullptr) {
          return nullptr;
        }
        sftp_set_error(sftp, status->status);
        ssh_set_error(sftp->session->error, SSH_REQUEST_DENIED,
            "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }
        return nullptr;
    }
    default:
        ssh_set_error(sftp->session->error, SSH_FATAL,
          "Received mesg %d during stat()", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    }
    return nullptr;
}

sftp_attributes_struct* sftp_stat(sftp_session_struct* session, const char *path) {
  return sftp_xstat(session, path, SSH_FXP_STAT);
}

sftp_attributes_struct* sftp_lstat(sftp_session_struct* session, const char *path) {
  return sftp_xstat(session, path, SSH_FXP_LSTAT);
}

sftp_attributes_struct* sftp_fstat(sftp_file_struct* file) {
  sftp_status_message_struct* status = nullptr;
  sftp_message_struct* msg = nullptr;
  ssh_buffer_struct* buffer;
  uint32_t id;

  buffer = new ssh_buffer_struct;
  id = sftp_get_new_id(file->sftp);
  buffer->out_uint32_le(id);

  buffer->out_uint32_be(file->handle.size);
  buffer->out_blob(file->handle.data.get(), file->handle.size);

  if (sftp_packet_write(file->sftp, SSH_FXP_FSTAT, buffer) < 0) {
    delete buffer;
    return nullptr;
  }
  delete buffer;

  while (msg == nullptr) {
    if (sftp_read_and_dispatch(file->sftp) < 0) {
      return nullptr;
    }
    msg = sftp_dequeue(file->sftp, id);
  }

  if (msg->packet_type == SSH_FXP_ATTRS){
    return sftp_parse_attr(file->sftp, msg->payload, 0);
  } else if (msg->packet_type == SSH_FXP_STATUS) {
    status = parse_status_msg(msg);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }
    if (status == nullptr) {
      return nullptr;
    }
    ssh_set_error(file->sftp->session->error, SSH_REQUEST_DENIED,
        "SFTP server: %s", status->errormsg);
            if (status != nullptr) {
                delete [] status->errormsg;
                delete [] status->langmsg;
                delete status;
            }

    return nullptr;
  }
  ssh_set_error(file->sftp->session->error, SSH_FATAL,
      "Received msg %d during fstat()", msg->packet_type);
        if (msg != nullptr) {
            delete msg->payload;
            delete msg;
        }

  return nullptr;
}


sftp_client_message_struct* sftp_get_client_message(sftp_session_struct* sftp) {
    sftp_packet_struct* packet;
    ;
    ssh_buffer_struct* payload;

    sftp_client_message_struct * msg = new struct sftp_client_message_struct;

    packet = sftp_packet_read(sftp);
    payload = packet->payload;
    msg->type = packet->type;
    msg->sftp = sftp;

    /* take a copy of the whole packet */
    msg->complete_message = new ssh_buffer_struct;
    msg->complete_message->out_blob(payload->get_pos_ptr(), payload->in_remain());
    msg->id = payload->in_uint32_le();

    switch(msg->type) {
    case SSH_FXP_CLOSE: case SSH_FXP_READDIR: case SSH_FXP_READ:
    case SSH_FXP_WRITE: case SSH_FXP_REMOVE: case SSH_FXP_RMDIR:
    case SSH_FXP_OPENDIR: case SSH_FXP_READLINK: case SSH_FXP_REALPATH:
    case SSH_FXP_RENAME: case SSH_FXP_SYMLINK: case SSH_FXP_MKDIR:
    case SSH_FXP_SETSTAT: case SSH_FXP_FSETSTAT: case SSH_FXP_LSTAT:
    case SSH_FXP_STAT: case SSH_FXP_OPEN: case SSH_FXP_FSTAT:
    break;
    default:
        ssh_set_error(sftp->session->error, SSH_FATAL,
                "Received unhandled sftp message %d\n", msg->type);
        if (msg != nullptr) {
            delete [] msg->filename;
            if (msg->attr != nullptr) {
                delete [] msg->attr->name;
                delete [] msg->attr->longname;
                delete [] msg->attr->group;
                delete [] msg->attr->owner;
                delete msg->attr;
            }
            delete [] msg->complete_message;
            delete [] msg->str_data;
            delete msg;
        }
        return nullptr;
    }

    if (sizeof(uint32_t) > payload->in_remain()) {
        // ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    uint32_t tmp_len = payload->in_uint32_be();
    if (tmp_len > payload->in_remain()) {
        // ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    SSHString tmp(tmp_len);
    payload->buffer_get_data(tmp.data.get(),tmp_len);

    switch(msg->type) {
    case SSH_FXP_CLOSE:
    case SSH_FXP_READDIR:
        msg->handle = std::move(tmp);
    break;
    case SSH_FXP_READ:
        msg->handle = std::move(tmp);

        msg->offset = payload->in_uint64_be();
        msg->len = payload->in_uint32_be();
    break;
    case SSH_FXP_WRITE:
    {
        msg->handle = std::move(tmp);
        msg->offset = payload->in_uint64_be();

        if (sizeof(uint32_t) > payload->in_remain()) {
          // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t tmp_data_len = payload->in_uint32_be();
        if (tmp_data_len > payload->in_remain()) {
          // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        SSHString tmp_data(tmp_data_len);
        payload->buffer_get_data(tmp_data.data.get(),tmp_data_len);

        msg->data = std::move(tmp_data);
    }
    break;
    case SSH_FXP_REMOVE:
    case SSH_FXP_RMDIR:
    case SSH_FXP_OPENDIR:
    case SSH_FXP_READLINK:
    case SSH_FXP_REALPATH:
    {
        msg->filename = new char[tmp.size + 1];
        memcpy(msg->filename, tmp.data.get(), tmp.size);
        msg->filename[tmp.size] = 0;
    }
    break;
    case SSH_FXP_RENAME:
    case SSH_FXP_SYMLINK:
    {
        msg->filename =  new char[tmp.size + 1];
        memcpy(msg->filename, tmp.data.get(), tmp.size);
        msg->filename[tmp.size] = 0;

        if (sizeof(uint32_t) > payload->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t tmp_data_len = payload->in_uint32_be();
        if (tmp_data_len > payload->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        SSHString tmp_data(tmp_data_len);
        payload->buffer_get_data(tmp_data.data.get(),tmp_data_len);
        msg->data = std::move(tmp_data);
    }
    break;
    case SSH_FXP_MKDIR:
    case SSH_FXP_SETSTAT:
    {
        msg->filename =  new char[tmp.size + 1];
        memcpy(msg->filename, tmp.data.get(), tmp.size);
        msg->filename[tmp.size] = 0;
        msg->attr = sftp_parse_attr(sftp, payload, 0);
    }
    break;
    case SSH_FXP_FSETSTAT:
    {
        msg->handle = std::move(tmp);
        msg->attr = sftp_parse_attr(sftp, payload, 0);
    }
    break;
    case SSH_FXP_LSTAT:
    case SSH_FXP_STAT:
    {
        msg->filename =  new char[tmp.size + 1];
        memcpy(msg->filename, tmp.data.get(), tmp.size);
        msg->filename[tmp.size] = 0;
        if(sftp->version > 3) {
            msg->flags = payload->in_uint32_be();
        }
    }
    break;
    case SSH_FXP_OPEN:
    {
        msg->filename =  new char[tmp.size + 1];
        memcpy(msg->filename, tmp.data.get(), tmp.size);
        msg->filename[tmp.size] = 0;

        msg->flags = payload->in_uint32_be();
        msg->attr = sftp_parse_attr(sftp, payload, 0);
    }
    break;
    case SSH_FXP_FSTAT:
    {
        msg->handle = std::move(tmp);
        msg->flags = payload->in_uint32_be();
    }
    break;
    default:
    // Should already have been excluded by previous code
    break;
    }
    if (packet != nullptr) {
        delete packet->payload;
        delete packet;
    }
    return msg;
}

/* Send an sftp client message. Can be used in cas of proxying */
int sftp_send_client_message(sftp_session_struct* sftp, sftp_client_message_struct* msg){
  return sftp_packet_write(sftp, msg->type, msg->complete_message);
}

uint8_t sftp_client_message_get_type(sftp_client_message_struct* msg){
	return msg->type;
}

const char *sftp_client_message_get_filename(sftp_client_message_struct* msg){
	return msg->filename;
}

void sftp_client_message_set_filename(sftp_client_message_struct* msg, const char *newname){
	delete [] msg->filename;
	size_t tmplen = strlen(newname) + 1;
	char * tmp = new char [tmplen];
	memcpy(tmp, newname, tmplen);
	msg->filename = tmp;
}

const char *sftp_client_message_get_data(sftp_client_message_struct* msg){
	if (msg->str_data == nullptr){
    msg->str_data =  new char[msg->data.size + 1];
    memcpy(msg->str_data, msg->data.data.get(), msg->data.size);
    msg->str_data[msg->data.size] = 0;
	}
	return msg->str_data;
}

uint32_t sftp_client_message_get_flags(sftp_client_message_struct* msg){
	return msg->flags;
}


//9.4.  Name Response

//   The SSH_FXP_NAME response has the following format:

//       byte   SSH_FXP_NAME
//       uint32 request-id
//       uint32 count
//       repeats count times:
//           string     filename [UTF-8]
//           ATTRS      attrs
//       bool end-of-list [optional]

//   count
//      The number of names returned in this response, and the 'filename'
//      and 'attrs' field repeat 'count' times.

//   filename
//      A file name being returned (for SSH_FXP_READDIR, it will be a
//      relative name within the directory, without any path components;
//      for SSH_FXP_REALPATH it will be an absolute path name.)

//   attrs
//      The attributes of the file as described in Section ''File
//      Attributes''.

//   end-of-list
//      If this field is present and true, there are no more entries to be
//      read.  This field should either be omitted or be true unless the
//      request is SSH_FXP_READDIR.

int sftp_reply_name(sftp_client_message_struct* msg, const char *name, sftp_attributes_struct* attr) {
  ssh_buffer_struct* out = new ssh_buffer_struct;
  if (out == nullptr) {
    return -1;
  }

  if (name == nullptr) {
    delete out;
    return -1;
  }

  // TODO: include both lines below in error check
  out->out_uint32_le(msg->id);
  out->out_uint32_be(1);

  out->out_uint32_be(strlen(name));
  out->out_blob(name, strlen(name));

  /* The protocol is broken here between 3 & 4 */
  if (buffer_add_attributes(out, attr) < 0
  || sftp_packet_write(msg->sftp, SSH_FXP_NAME, out) < 0) {
    delete out;
    return -1;
  }

  delete out;
  return 0;
}

int sftp_reply_handle(sftp_client_message_struct* msg, const SSHString & handle)
{
  ssh_buffer_struct* out;

  out = new ssh_buffer_struct;
  if (out == nullptr) {
    return -1;
  }

  out->out_uint32_le(msg->id);
  msg->attrbuf->out_uint32_be(handle.size);
  msg->attrbuf->out_blob(handle.data.get(), handle.size);

  if (sftp_packet_write(msg->sftp, SSH_FXP_HANDLE, out) < 0) {
    delete out;
    return -1;
  }
  delete out;
  return 0;
}

int sftp_reply_attr(sftp_client_message_struct* msg, sftp_attributes_struct* attr)
{
  ssh_buffer_struct* out;

  out = new ssh_buffer_struct;
  if (out == nullptr) {
    return -1;
  }

  out->out_uint32_le(msg->id);
  if (buffer_add_attributes(out, attr) < 0 ||
      sftp_packet_write(msg->sftp, SSH_FXP_ATTRS, out) < 0) {
    delete out;
    return -1;
  }
  delete out;

  return 0;
}

int sftp_reply_names_add(sftp_client_message_struct* msg, const char *file, const char *longname, sftp_attributes_struct* attr)
{
    if (msg->attrbuf == nullptr) {
        msg->attrbuf = new ssh_buffer_struct;
        if (msg->attrbuf == nullptr) {
          return -1;
        }
    }

    if (file == nullptr) {
        return -1;
    }

    msg->attrbuf->out_uint32_be(strlen(file));
    msg->attrbuf->out_blob(file, strlen(file));

    if (longname == nullptr) {
        return -1;
    }

    msg->attrbuf->out_uint32_be(strlen(longname));
    msg->attrbuf->out_blob(longname, strlen(longname));

    if (buffer_add_attributes(msg->attrbuf, attr) < 0) {
        return -1;
    }
    msg->attr_num++;
    return 0;
}

int sftp_reply_names(sftp_client_message_struct* msg) {
  ssh_buffer_struct* out;

  out = new ssh_buffer_struct;
  if (out == nullptr) {
    delete msg->attrbuf;
    return -1;
  }

  out->out_uint32_le(msg->id);
  out->out_uint32_be(msg->attr_num);

  out->out_blob(msg->attrbuf->get_pos_ptr(), msg->attrbuf->in_remain());
  if (sftp_packet_write(msg->sftp, SSH_FXP_NAME, out) < 0) {
    delete out;
    delete msg->attrbuf;
    return -1;
  }

  delete out;
  delete msg->attrbuf;

  msg->attr_num = 0;
  msg->attrbuf = nullptr;

  return 0;
}

int sftp_reply_status(sftp_client_message_struct* msg, uint32_t status, const char *message) {
  ssh_buffer_struct* out;

  out = new ssh_buffer_struct;
  if (out == nullptr) {
    return -1;
  }

  // TODO: add error check
  out->out_uint32_le(msg->id);
  out->out_uint32_be(status);
  out->out_uint32_be(message?strlen(message):0);
  out->out_blob(message?message:"", message?strlen(message):0);

  out->out_uint32_be(0);
  if (sftp_packet_write(msg->sftp, SSH_FXP_STATUS, out) < 0) {
    delete out;
    return -1;
  }

  delete out;
  return 0;
}

int sftp_reply_data(sftp_client_message_struct* msg, const void *data, int len) {
  // TOD: replace with static buffer ?
  ssh_buffer_struct* out = new ssh_buffer_struct;
  if (out == nullptr) {
    return -1;
  }

  // TODO: add error check
  out->out_uint32_le(msg->id);
  out->out_uint32_be(len);

  out->out_blob(reinterpret_cast<const uint8_t*>(data), len);
  if (sftp_packet_write(msg->sftp, SSH_FXP_DATA, out) < 0) {
    delete out;
    return -1;
  }
  delete out;
  return 0;
}


void sftp_handle_remove(sftp_session_struct* sftp, void *handle) {
    int i;
    for (i = 0; i < SFTP_HANDLES; i++) {
        if (sftp->handles[i] == handle) {
            sftp->handles[i] = nullptr;
            break;
        }
    }
}

inline int sftp_data_get_type(sftp_client_message_struct* data) {
    return data->type;
}
/* vim: set ts=4 sw=4 et cindent: */
