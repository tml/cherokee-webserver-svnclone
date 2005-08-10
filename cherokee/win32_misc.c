/* -*- Mode: C; TABs are evil */

/* Cherokee
 *
 * Authors:
 *      Gisle Vanem <giva@bgnett.no>
 *
 * Copyright (C) 2001, 2002, 2003, 2004 Alvaro Lopez Ortega
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <assert.h>
#include <ctype.h>

#include "common-internal.h"
#include "server.h"
#include "ncpus.h"
#include "unix4win32.h"

#ifdef HAVE_OPENSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
# include <openssl/des.h>
#endif

#ifndef __GNUC__
#error "You must be joking"
#endif

#ifdef HAVE_IPV6
/* __declspec(dllexport) */ const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
#endif

#ifdef CHEROKEE_EMBEDDED
# define _ctor __attribute__ ((constructor))
#else
# define _ctor
#endif

static int     win_trace = 0;
static SOCKET  first_sock_num;
static WSADATA wsa_data;

/*
 * Constructor/DllMain
 */
static _ctor void win_init (void)
{
  const char *env = getenv ("CHEROKEE_TRACE");

  if (env)
  {
    win_trace = atoi (env);
    LoadLibrary ("exchndl.dll");   /* Dr MingW exception handler */
  }
  if (WSAStartup (MAKEWORD(1,1), &wsa_data) != 0)
  {
    PRINT_ERROR ("WSAStartup failed; %s\n", win_strerror(GetLastError()));
    exit (-1);
  }
  first_sock_num = socket (AF_INET, SOCK_STREAM, 0);
  assert (first_sock_num > 0);
  closesocket (first_sock_num);
}

#if !defined(CHEROKEE_EMBEDDED)
BOOL APIENTRY DllMain (HANDLE dll_handle, DWORD reason, LPVOID reserved)
{
  if (reason == DLL_PROCESS_ATTACH)
     win_init();

  (void) dll_handle;
  (void) reserved;
  return (TRUE);
}
#endif

int getdtablesize (void)
{
  return (wsa_data.iMaxSockets + first_sock_num);
}

ret_t set_system_fd_num_limit (uint32_t limit)
{
  (void) limit;
  return ret_error;
}

#undef random
int random (void)
{
  return rand();
}

#undef getgrnam
struct group *getgrnam (const char *group)
{
  return (NULL);
}

#undef crypt
char *crypt (const char *buf, const char *salt)
{
#ifdef HAVE_OPENSSL
  return DES_crypt (buf, salt);
#else
  (void) buf;
  (void) salt;
  SHOULDNT_HAPPEN;
  return ("No crypt");
#endif
}

struct tm *localtime_r (const time_t *time, struct tm *tm)
{
  struct tm *ret;

  if (!tm)
     return (NULL);
  ret = localtime (time);
  if (ret)
     *tm = *ret;
  return (ret);
}

/* ncpus.c */
int dcc_ncpus (int *ncpus)
{
  SYSTEM_INFO sys_info;

  memset (&sys_info, 0, sizeof(sys_info));
  GetSystemInfo (&sys_info);
  return_if_fail (sys_info.dwNumberOfProcessors > 0, ret_error);
  *ncpus = sys_info.dwNumberOfProcessors;
  return (ret_ok);
}

/* server.c */
ret_t cherokee_server_daemonize (cherokee_server_t *srv)
{
  SHOULDNT_HAPPEN;
  (void) srv;
  return ret_error;
}

#if defined(CHEROKEE_EMBEDDED) && 0
#include "handler_phpcgi.h"

ret_t cherokee_handler_phpcgi_new (cherokee_handler_t **hdl, void *cnt, cherokee_table_t *properties)
{
  SHOULDNT_HAPPEN;
  return ret_error;
}

ret_t cherokee_handler_phpcgi_init (cherokee_handler_t *hdl)
{
  SHOULDNT_HAPPEN;
  return ret_error;
}
#endif

/*
 * This function handles most Winsock errors we're able to produce.
 */
static char *get_winsock_error (int err, char *buf, size_t len)
{
  char *p;

  switch (err)
  {
    case WSAEINTR:
        p = "Call interrupted.";
        break;
    case WSAEBADF:
        p = "Bad file";
        break;
    case WSAEACCES:
        p = "Bad access";
        break;
    case WSAEFAULT:
        p = "Bad argument";
        break;
    case WSAEINVAL:
        p = "Invalid arguments";
        break;
    case WSAEMFILE:
        p = "Out of file descriptors";
        break;
    case WSAEWOULDBLOCK:
        p = "Call would block";
        break;
    case WSAEINPROGRESS:
    case WSAEALREADY:
        p = "Blocking call in progress";
        break;
    case WSAENOTSOCK:
        p = "Descriptor is not a socket.";
        break;
    case WSAEDESTADDRREQ:
        p = "Need destination address";
        break;
    case WSAEMSGSIZE:
        p = "Bad message size";
        break;
    case WSAEPROTOTYPE:
        p = "Bad protocol";
        break;
    case WSAENOPROTOOPT:
        p = "Protocol option is unsupported";
        break;
    case WSAEPROTONOSUPPORT:
        p = "Protocol is unsupported";
        break;
    case WSAESOCKTNOSUPPORT:
        p = "Socket is unsupported";
        break;
    case WSAEOPNOTSUPP:
        p = "Operation not supported";
        break;
    case WSAEAFNOSUPPORT:
        p = "Address family not supported";
        break;
    case WSAEPFNOSUPPORT:
        p = "Protocol family not supported";
        break;
    case WSAEADDRINUSE:
        p = "Address already in use";
        break;
    case WSAEADDRNOTAVAIL:
        p = "Address not available";
        break;
    case WSAENETDOWN:
        p = "Network down";
        break;
    case WSAENETUNREACH:
        p = "Network unreachable";
        break;
    case WSAENETRESET:
        p = "Network has been reset";
        break;
    case WSAECONNABORTED:
        p = "Connection was aborted";
        break;
    case WSAECONNRESET:
        p = "Connection was reset";
        break;
    case WSAENOBUFS:
        p = "No buffer space";
        break;
    case WSAEISCONN:
        p = "Socket is already connected";
        break;
    case WSAENOTCONN:
        p = "Socket is not connected";
        break;
    case WSAESHUTDOWN:
        p = "Socket has been shut down";
        break;
    case WSAETOOMANYREFS:
        p = "Too many references";
        break;
    case WSAETIMEDOUT:
        p = "Timed out";
        break;
    case WSAECONNREFUSED:
        p = "Connection refused";
        break;
    case WSAELOOP:
        p = "Loop??";
        break;
    case WSAENAMETOOLONG:
        p = "Name too long";
        break;
    case WSAEHOSTDOWN:
        p = "Host down";
        break;
    case WSAEHOSTUNREACH:
        p = "Host unreachable";
        break;
    case WSAENOTEMPTY:
        p = "Not empty";
        break;
    case WSAEPROCLIM:
        p = "Process limit reached";
        break;
    case WSAEUSERS:
        p = "Too many users";
        break;
    case WSAEDQUOT:
        p = "Bad quota";
        break;
    case WSAESTALE:
        p = "Something is stale";
        break;
    case WSAEREMOTE:
        p = "Remote error";
        break;
    case WSAEDISCON:
        p = "Disconnected";
        break;

    /* Extended Winsock errors */
    case WSASYSNOTREADY:
        p = "Winsock library is not ready";
        break;
    case WSANOTINITIALISED:
        p = "Winsock library not initalised";
        break;
    case WSAVERNOTSUPPORTED:
        p = "Winsock version not supported.";
        break;

    /* getXbyY() errors (already handled in herrmsg):
       Authoritative Answer: Host not found */
    case WSAHOST_NOT_FOUND:
        p = "Host not found";
        break;

    /* Non-Authoritative: Host not found, or SERVERFAIL */
    case WSATRY_AGAIN:
        p = "Host not found, try again";
        break;

    /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
    case WSANO_RECOVERY:
        p = "Unrecoverable error in call to nameserver";
        break;

    /* Valid name, no data record of requested type */
    case WSANO_DATA:
        p = "No data record of requested type";
        break;

    default:
        return (NULL);
  }
  strncpy (buf, p, len);
  buf [len-1] = '\0';
  return buf;
}

/*
 * A smarter strerror()
 */
char *win_strerror (int err)
{
  static char buf[512];   /* !! not thread-safe */
  DWORD  lang  = MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT);
  DWORD  flags = FORMAT_MESSAGE_FROM_SYSTEM |
                 FORMAT_MESSAGE_IGNORE_INSERTS |
                 FORMAT_MESSAGE_MAX_WIDTH_MASK;
  char  *p;

  if (err >= 0 && err < sys_nerr)
  {
    strncpy (buf, strerror(err), sizeof(buf)-1);
    buf [sizeof(buf)-1] = '\0';
  }
  else
  {
    if (!get_winsock_error (err, buf, sizeof(buf)) &&
        !FormatMessage (flags, NULL, err,
                        lang, buf, sizeof(buf)-1, NULL))
     sprintf (buf, "Unknown error %d (%#x)", err, err);
  }

  /* strip trailing '\r\n' or '\n'. */
  if ((p = strrchr(buf,'\n')) != NULL && (p - buf) >= 2)
     *p = '\0';
  if ((p = strrchr(buf,'\r')) != NULL && (p - buf) >= 1)
     *p = '\0';
  return (buf);
}


/*
 * A simple mmap() emulation.
 */
struct mmap_list {
       HANDLE            os_map;
       DWORD             size;
       const void       *file_ptr;
       struct mmap_list *next;
     };

static struct mmap_list *mmap_list0 = NULL;

static __inline struct mmap_list *
lookup_mmap (const void *file_ptr, size_t size)
{
  struct mmap_list *m;

  if (!file_ptr || size == 0)
     return (NULL);

  for (m = mmap_list0; m; m = m->next)
      if (m->file_ptr == file_ptr && m->size == (DWORD)size && m->os_map)
         return (m);
  return (NULL);
}

static __inline struct mmap_list *
add_mmap_node (const void *file_ptr, const HANDLE os_map, DWORD size)
{
  struct mmap_list *m = calloc (sizeof(*m), 1);

  if (!m)
     return (NULL);

  m->os_map   = os_map;
  m->file_ptr = file_ptr;
  m->size     = size;
  m->next     = mmap_list0;
  mmap_list0  = m;
  return (m);
}

static __inline struct mmap_list *
unlink_mmap (struct mmap_list *This)
{
  struct mmap_list *m, *prev, *next;

  for (m = prev = mmap_list0; m; prev = m, m = m->next)
  {
    if (m != This)
       continue;
    if (m == mmap_list0)
         mmap_list0 = m->next;
    else prev->next = m->next;
    next = m->next;
    free (m);
    return (next);
  }
  return (NULL);
}

/*
 * These functions should be protected with a critical section
 * to be on the safe side.
 */
void *win_mmap (int fd, size_t size, int prot)
{
  struct mmap_list *map = NULL;
  HANDLE os_handle, os_map;
  DWORD  os_prot;
  void  *file_ptr;

  if (fd < 0 || size == 0 || !(prot & (PROT_READ|PROT_WRITE)))
  {
    SetLastError (errno = EINVAL);
    return (MAP_FAILED);
  }

  /* todo:
     prot 0                -> PAGE_NOACCESS
     PROT_READ             -> PAGE_READONLY
     PROT_READ|PROT_WRITE  -> PAGE_READWRITE
     PROT_WRITE            -> PAGE_WRITECOPY
  */

  os_handle = (HANDLE) _get_osfhandle (fd);
  if (!os_handle)
     return (MAP_FAILED);

  os_prot = (prot == PROT_READ) ? PAGE_READONLY : PAGE_WRITECOPY;
  os_map = CreateFileMapping (os_handle, NULL, os_prot, 0, 0, NULL);
  if (!os_map)
     return (MAP_FAILED);

  os_prot = (prot == PROT_READ) ? FILE_MAP_READ : FILE_MAP_WRITE;
  file_ptr = MapViewOfFile (os_map, os_prot, 0, 0, 0);
  if (file_ptr)
  {
    map = add_mmap_node (file_ptr, os_map, size);
    if (!map)
    {
      FlushViewOfFile (file_ptr, size);
      UnmapViewOfFile (file_ptr);
    }
  }
  file_ptr = (map ? map->file_ptr : NULL);
  if (!file_ptr)
     CloseHandle (os_map);

  if (win_trace > 1)
     PRINT_DEBUG ("win_mmap(): fd %d, os_map %08lX, file_ptr %08lX, size %u, prot %d\n",
                  fd, (DWORD)os_map, (DWORD)file_ptr, size, prot);
  return (file_ptr);
}

int win_munmap (const void *file_ptr, size_t size)
{
  struct mmap_list *m = lookup_mmap (file_ptr, size);

  if (!m)
  {
    SetLastError (errno = EINVAL);
    return (-1);
  }
  FlushViewOfFile (m->file_ptr, m->size);
  UnmapViewOfFile ((void*)m->file_ptr);
  CloseHandle (m->os_map);
  unlink_mmap (m);
  return (0);
}

#if !defined(CHEROKEE_EMBEDDED)
/*
 * dlopen() emulation.
 */
static const char *last_func;
static DWORD       last_error;

void *win_dlopen (const char *dll_name, int flags)
{
  void *rc;

  last_func = "win_dlopen";
  rc = (void*) LoadLibrary (dll_name);
  if (rc)
       last_error = 0;
  else last_error = GetLastError();
  (void) flags;
  return (rc);
}

void *win_dlsym (const void *dll_handle, const char *func_name)
{
  HINSTANCE hnd = (HINSTANCE)dll_handle;
  void *rc;

  if (!hnd)
     hnd = GetModuleHandle (NULL);
  last_func = "win_dlsym";
  rc = (void*) GetProcAddress (hnd, func_name);
  if (rc)
       last_error = 0;
  else last_error = GetLastError();
  return (rc);
}

int win_dlclose (const void *dll_handle)
{
  HINSTANCE hnd = (HINSTANCE)dll_handle;

  last_func = "win_dlclose";
  if (!hnd)
     return (-1);

  if (FreeLibrary(hnd))
       last_error = 0;
  else last_error = GetLastError();
  return (last_error);
}

const char *win_dlerror (void)
{
  static char errbuf[1024];  /* !! not thread-safe */

  if (!last_error)
     return (NULL);
  snprintf (errbuf, sizeof(errbuf)-1, "%s(): %s", last_func, win_strerror(last_error));
  return (errbuf);
}
#endif  /* CHEROKEE_EMBEDDED */


/*
 * inet_ntop() + inet_pton() originally by Paul Vixie.
 * Modified for Win32/MingW by by Gisle Vanem.
 */

#define	IN6ADDRSZ     16
#define	INADDRSZ      4
#define	INT16SZ	      2

/* Set both incase user was dumb enough to use the original strerror()
 */
#define SET_ERRNO(e)  WSASetLastError (errno = (e))

static int         inet_pton4 (const char *src, u_char *dst);
static const char *inet_ntop4 (const u_char *src, char *dst, size_t size);

#ifdef HAVE_IPV6
static int         inet_pton6 (const char *src, u_char *dst);
static const char *inet_ntop6 (const u_char *src, char *dst, size_t size);
#endif

/*
 * Convert from presentation format (which usually means ASCII printable)
 *	to network format (which is usually some kind of binary format).
 * return:
 *	1 if the address was valid for the specified address family
 *	0 if the address wasn't valid (`dst' is untouched in this case)
 *	-1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *	Paul Vixie, 1996.
 */
int inet_pton (int af, const char *src, void *dst)
{
  switch (af)
  {
    case AF_INET:
         return inet_pton4(src, dst);
#ifdef HAVE_IPV6
    case AF_INET6:
         return inet_pton6(src, dst);
#endif
    default:
         SET_ERRNO (WSAEAFNOSUPPORT);
         return (-1);
  }
}

/*
 * Convert a network format address to presentation format.
 *
 * Returns pointer to presentation format address (`dst'),
 * Returns NULL on error (see errno).
 */
const char *inet_ntop (int af, const void *src, char *buf, size_t size)
{
  switch (af)
  {
    case AF_INET:
         return inet_ntop4 ((const u_char*)src, buf, size);
#ifdef HAVE_IPV6
    case AF_INET6:
         return inet_ntop6 ((const u_char*)src, buf, size);
#endif
    default:
         SET_ERRNO (EAFNOSUPPORT);
         return (NULL);
  }
}

/*
 * Like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *	1 if `src' is a valid dotted quad, else 0.
 * notice:
 *	does not touch `dst' unless it's returning 1.
 * author:
 *	Paul Vixie, 1996.
 */
static int inet_pton4 (const char *src, u_char *dst)
{
  static const char digits[] = "0123456789";
  int saw_digit, octets, ch;
  u_char tmp[INADDRSZ], *tp;

  saw_digit = 0;
  octets = 0;
  *(tp = tmp) = 0;
  while ((ch = *src++) != '\0')
  {
    const char *pch = strchr(digits, ch);

    if (pch)
    {
      u_int new = *tp * 10 + (pch - digits);

      if (new > 255)
         return (0);
      *tp = new;
      if (! saw_digit)
      {
        if (++octets > 4)
           return (0);
        saw_digit = 1;
      }
    }
    else if (ch == '.' && saw_digit)
    {
      if (octets == 4)
         return (0);
      *++tp = 0;
      saw_digit = 0;
    }
    else
      return (0);
  }
  if (octets < 4)
     return (0);

  memcpy (dst, tmp, INADDRSZ);
  return (1);
}


/*
 * Format an IPv4 address, more or less like inet_ntoa().
 *
 * Returns `dst' (as a const)
 * Note:
 *  - uses no statics
 *  - takes a u_char* not an in_addr as input
 */
static const char *inet_ntop4 (const u_char *src, char *dst, size_t size)
{
  const char *addr = inet_ntoa(*(struct in_addr*)src);

  if (strlen(addr) >= size)
  {
    SET_ERRNO (ENOSPC);
    return (NULL);
  }
  return strcpy(dst, addr);
}


#ifdef HAVE_IPV6
/*
 * Convert presentation level address to network order binary form.
 * return:
 *	1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *	(1) does not touch `dst' unless it's returning 1.
 *	(2) :: in a full address is silently ignored.
 * credit:
 *	inspired by Mark Andrews.
 * author:
 *	Paul Vixie, 1996.
 */
static int inet_pton6 (const char *src, u_char *dst)
{
  static const char xdigits[] = "0123456789ABCDEF";
  u_char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
  const char *curtok;
  int   ch, saw_xdigit;
  u_int val;

  memset((tp = tmp), 0, IN6ADDRSZ);
  endp = tp + IN6ADDRSZ;
  colonp = NULL;

  /* Leading :: requires some special handling. */
  if (*src == ':' && *++src != ':')
     return (0);

  curtok = src;
  saw_xdigit = 0;
  val = 0;
  while ((ch = *src++) != '\0')
  {
    const char *pch = strchr (xdigits, toupper(ch));

    if (pch)
    {
      val <<= 4;
      val |= (pch - xdigits);
      if (val > 0xffff)
         return (0);
      saw_xdigit = 1;
      continue;
    }
    if (ch == ':')
    {
      curtok = src;
      if (!saw_xdigit)
      {
        if (colonp)
           return (0);
        colonp = tp;
        continue;
      }
      if (tp + INT16SZ > endp)
         return (0);
      *tp++ = (u_char) (val >> 8) & 0xff;
      *tp++ = (u_char) val & 0xff;
      saw_xdigit = 0;
      val = 0;
      continue;
    }
    if (ch == '.' && ((tp + INADDRSZ) <= endp) &&
        inet_pton4(curtok, tp) > 0)
    {
      tp += INADDRSZ;
      saw_xdigit = 0;
      break;	/* '\0' was seen by inet_pton4(). */
    }
    return (0);
  }

  if (saw_xdigit)
  {
    if (tp + INT16SZ > endp)
       return (0);
    *tp++ = (u_char) (val >> 8) & 0xff;
    *tp++ = (u_char) val & 0xff;
  }
  if (colonp != NULL)
  {
    /*
     * Since some memmove()'s erroneously fail to handle
     * overlapping regions, we'll do the shift by hand.
     */
    const int n = tp - colonp;
    int i;

    for (i = 1; i <= n; i++)
    {
      endp[-i] = colonp[n - i];
      colonp[n - i] = 0;
    }
    tp = endp;
  }
  if (tp != endp)
     return (0);
  memcpy (dst, tmp, IN6ADDRSZ);
  return (1);
}

/*
 * Convert IPv6 binary address into presentation (printable) format.
 */
static const char *inet_ntop6 (const u_char *src, char *dst, size_t size)
{
  /*
   * Note that int32_t and int16_t need only be "at least" large enough
   * to contain a value of the specified size.  On some systems, like
   * Crays, there is no such thing as an integer variable with 16 bits.
   * Keep this in mind if you think this function should have been coded
   * to use pointer overlays.  All the world's not a VAX.
   */
  char  tmp [sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
  char *tp;
  struct {
    long base;
    long len;
  } best, cur;
  u_long words [IN6ADDRSZ / INT16SZ];
  int    i;

  /* Preprocess:
   *  Copy the input (bytewise) array into a wordwise array.
   *  Find the longest run of 0x00's in src[] for :: shorthanding.
   */
  memset(words, 0, sizeof(words));
  for (i = 0; i < IN6ADDRSZ; i++)
      words[i/2] |= (src[i] << ((1 - (i % 2)) << 3));

  best.base = -1;
  cur.base  = -1;
  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
  {
    if (words[i] == 0)
    {
      if (cur.base == -1)
        cur.base = i, cur.len = 1;
      else
        cur.len++;
    }
    else if (cur.base != -1)
    {
      if (best.base == -1 || cur.len > best.len)
         best = cur;
      cur.base = -1;
    }
  }
  if ((cur.base != -1) && (best.base == -1 || cur.len > best.len))
     best = cur;
  if (best.base != -1 && best.len < 2)
     best.base = -1;

  /* Format the result.
   */
  tp = tmp;
  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
  {
    /* Are we inside the best run of 0x00's?
     */
    if (best.base != -1 && i >= best.base && i < (best.base + best.len))
    {
      if (i == best.base)
         *tp++ = ':';
      continue;
    }

    /* Are we following an initial run of 0x00s or any real hex?
     */
    if (i != 0)
       *tp++ = ':';

    /* Is this address an encapsulated IPv4?
     */
    if (i == 6 && best.base == 0 &&
        (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
    {
      if (!inet_ntop4(src+12, tp, sizeof(tmp) - (tp - tmp)))
      {
        SET_ERRNO (ENOSPC);
        return (NULL);
      }
      tp += strlen(tp);
      break;
    }
    tp += sprintf (tp, "%lx", words[i]);
  }

  /* Was it a trailing run of 0x00's?
   */
  if (best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
     *tp++ = ':';
  *tp++ = '\0';

  /* Check for overflow, copy, and we're done.
   */
  if ((size_t)(tp - tmp) > size)
  {
    SET_ERRNO (ENOSPC);
    return (NULL);
  }
  return strcpy (dst, tmp);
}
#endif  /* HAVE_IPV6 */

