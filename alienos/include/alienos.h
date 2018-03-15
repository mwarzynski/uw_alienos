
// Key presses representation as numbers.
// Range for ASCII: 0x20 - 0x7e.
#define KEY_UP 0x80
#define KEY_LEFT 0x81
#define KEY_DOWN 0x82
#define KEY_RIGHT 0x83
#define KEY_ENTER 0x0

// PT - segment for parameters passed from the operating system.
// Parameters are always of type int and are passed in binary form (4 bytes, little-endian).
// The size of the `PT_PARAMS` segment indicates how many parameters a given program
// needs (`p_memsz / 4`).
// Before the program starts, the operating system places parameter values in this segment.
// If the program does not have such a segment, it means that it does not accept parameters.
#define PT_PARAMS 0x60031337

