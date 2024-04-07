/*****************************************************************************/
/* implode                                Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Implode function of PKWARE Data Compression library                       */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who              Comment                                  */
/* --------  ----  ---              -------                                  */
/* 11.04.03  1.00  Ladislav Zezula  First version of implode.c               */
/* 02.05.03  1.00  Ladislav Zezula  Stress test done                         */
/* 12.06.03  1.00  ShadowFlare      Fixed error in WriteCmpData that caused  */
/*                                  different output to be produced          */
/*****************************************************************************/

#ifndef ABYSS_IMPLODE_H
#define ABYSS_IMPLODE_H

//-----------------------------------------------------------------------------
// Defines

#define CMP_BUFFER_SIZE 36312 // Size of compression buffer
#define EXP_BUFFER_SIZE 12596 // Size of decompress buffer

#define CMP_BINARY 0 // Binary compression
#define CMP_ASCII  1 // Ascii compression

#define CMP_NO_ERROR         0
#define CMP_INVALID_DICTSIZE 1
#define CMP_INVALID_MODE     2
#define CMP_BAD_DATA         3
#define CMP_ABORT            4

//-----------------------------------------------------------------------------
// Define calling convention

// #define PKEXPORT
#define PKEXPORT __cdecl // Use for normal __cdecl calling
// #define PKEXPORT  __stdcall
// #define PKEXPORT  __fastcall

//-----------------------------------------------------------------------------
// Public functions

#ifdef __cplusplus
extern "C" {
#endif

unsigned int PKEXPORT explode(unsigned int(PKEXPORT *read_buf)(char *buf, unsigned int *size, void *param),
                              void(PKEXPORT *write_buf)(char *buf, unsigned int *size, void *param), char *work_buf,
                              void *param);

// The original name "crc32" was changed to "crc32pk" due
// to compatibility with zlib
unsigned long PKEXPORT crc32pk(char *buffer, unsigned int *size, unsigned long *old_crc);

#endif // ABYSS_IMPLODE_H
