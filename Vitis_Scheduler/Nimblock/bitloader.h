#ifndef BITLOADER_H
#define BITLOADER_H

#include <iostream>
#include <vector>
#include <xil_types.h>

int init_filesystem();
int load_bitstream_to_memory(const char *BitstreamFile,
                             char **DestinationAddress, u32 *fileSize);

#endif // BITLOADER_H