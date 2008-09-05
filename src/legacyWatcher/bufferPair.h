/*
 * bufferPair.h - manage a pair of buffers
 *
 * Copyright (C) 2007  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */
#ifndef BUFFERPAIR_H_FILE
#define BUFFERPAIR_H_FILE

#include <unistd.h> /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif

struct TransDuo;
struct BufferPair;

/*
 * Create a BufferPair from a TransDuo
 * Puts transformed in bufferPairFirst(), raw in bufferPairSecond()
 */
struct BufferPair *bufferPairFromTransDuo(struct TransDuo *);

/*
 * Create a BufferPair from a buffer created by bufferPairMarshal()
 */
struct BufferPair *bufferPairFromMarshaled(
        void const *buf, size_t buflen);

/*
 * Create a BufferPair from two buffers.
 */
struct BufferPair *bufferPairFromBuffers(
        void const *first, size_t firstLength,
        void const *second, size_t secondLength);

/*
 * Destroy a BufferPair
 */
void bufferPairDestroy(struct BufferPair *toDestroy);

/*
 * Turn a buffer pair into a single buffer that can be separated later.
 */
void const *bufferPairMarshal(struct BufferPair const *toMarshal);

/*
 * Make a copy of what bufferPairMarshal() returns.
 *
 * It is the caller's responsibility to free() the returned value.
 */
void *bufferPairMarshalDup(struct BufferPair const *toMarshal);

/*
 * The length of the buffer returned by bufferPairMarshal().
 */
size_t bufferPairMarshalLength(struct BufferPair const *toMarshal);

/*
 * Return the first buffer in the BufferPair
 */
void const *bufferPairFirst(struct BufferPair const *toGet);

/*
 * Return a copy of the first buffer in the BufferPair
 *
 * It is the caller's responsibility to free() the returned value.
 */
void *bufferPairFirstDup(struct BufferPair const *toGet);

/*
 * The length of the buffer returned by bufferPairFirst().
 */
size_t bufferPairFirstLength(struct BufferPair const *toGet);

/*
 * Return the second buffer in the BufferPair
 */
void const *bufferPairSecond(struct BufferPair const *toGet);

/*
 * Return a copy of the second buffer in the BufferPair
 *
 * It is the caller's responsibility to free() the returned value.
 */
void *bufferPairSecondDup(struct BufferPair const *toGet);

/*
 * The length of the buffer returned by bufferPairSecond()
 */
size_t bufferPairSecondLength(struct BufferPair const *toGet);

#ifdef __cplusplus
}
#endif

#endif /* BUFFERPAIR_H_FILE */
