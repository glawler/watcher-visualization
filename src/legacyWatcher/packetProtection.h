//
// packetProtection.h - provide source authentication for "packet"
// structures.
//
// Copyright 2006 Sparta Inc.
//
#ifndef PACKET_PROTCTION_H_FILE
#define PACKET_PROTCTION_H_FILE

#ifdef __cplusplus
extern "C" {
#endif

struct PacketProtection;

//
// Allocate and initialize packet protection If "fname" is null, then no
// protection will take place during "packetProtect()" and no
// verification will take place during "packetUnprotect()".
//
// fname - file holding key information.
//
//     Key information format:
//        <ManetAddr>|<key data>
//        <ManetAddr>|<key data>
//        <ManetAddr>|<key data>
//             :         :
//
//     '#' introduces a comment.
//
// Returns zero on success.
//
int packetProtectionInit(
        struct PacketProtection **pp,
        char const *fname);

//
// Clean up packet protection
//
// Returns zero on success.
//
void packetProtectionFini(struct PacketProtection *pp);

//
// Protect a packet.
//
// pp    - PacketProtection to use
//
// p_in  - Packet to protect
//
// p_out - Loaded with a new protected packet if packet protection
//         occures, set to null if packet protection does not occur.
//
// Returns zero on success, ENOENT on unknown source.
//
int packetProtect(
        struct PacketProtection const *pp, 
        packet **p_out,
        ManetAddr from,
        packet const *p_in);

//
// Unprotect a protected packet
//
// Returns zero on succes, ENOENT on unknown source address, EILSEQ on
// modification.
//
int packetUnprotect(struct PacketProtection const *pp, ManetAddr from, packet *p);

#ifdef __cplusplus
}
#endif

#endif // PACKET_PROTCTION_H_FILE
