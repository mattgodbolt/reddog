#ifndef _HARDWARE_H
#define _HARDWARE_H

// Hardware defines
#define HW_BASE 0xa05f8000
#define rSetRegister(a,b) (*(volatile Uint32 *)(HW_BASE + (a))) = (b)

// Background colour
#define HW_BG_COLOUR			0x40
// Alpha threshold
#define HW_ALPHA_THRESHOLD		0x48
// Culling parameter
#define HW_CULLING_PARAMETER	0x78
// Fog table colour
#define HW_FOG_TABLE_COLOUR		0xb0
// Fog vertex colour
#define HW_FOG_VERTEX_COLOUR	0xb4
// Fog density
#define HW_FOG_DENSITY			0xb8
// Colour clamp maximum
#define HW_COLCLAMP_MAX			0xbc
// Colour clamp minimum
#define HW_COLCLAMP_MIN			0xc0
// Palette mode
#define HW_PALETTE_MODE			0x108
// Fog tablebase
#define HW_FOG_TABLE			0x200

#endif
