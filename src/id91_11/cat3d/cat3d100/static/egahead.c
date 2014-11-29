#include <inttypes.h>

static unsigned char egahead_c3d[] = {
  0x00, 0x00, 0x00, 0x7b, 0x01, 0x00, 0x89, 0x01, 0x00, 0xa8, 0x01, 0x00,
  0x94, 0x06, 0x00, 0x49, 0x0a, 0x00, 0x0a, 0x0b, 0x00, 0x19, 0x0c, 0x00,
  0xdd, 0x0c, 0x00, 0x9c, 0x0d, 0x00, 0xc3, 0x0e, 0x00, 0x23, 0x10, 0x00,
  0xfc, 0x10, 0x00, 0x28, 0x12, 0x00, 0x57, 0x13, 0x00, 0x55, 0x14, 0x00,
  0x59, 0x15, 0x00, 0x59, 0x16, 0x00, 0xf3, 0x16, 0x00, 0x5d, 0x17, 0x00,
  0xb8, 0x18, 0x00, 0xc3, 0x62, 0x00, 0xc2, 0xa3, 0x00, 0xbc, 0xda, 0x00,
  0x02, 0x03, 0x01, 0xcc, 0x2b, 0x01, 0x40, 0x37, 0x01, 0x71, 0x3e, 0x01,
  0x44, 0x44, 0x01, 0x16, 0x59, 0x01, 0x6e, 0x70, 0x01, 0xb1, 0x84, 0x01,
  0xf4, 0x95, 0x01, 0xd3, 0xa8, 0x01, 0x8d, 0xbd, 0x01, 0x4c, 0xd0, 0x01,
  0x68, 0xe7, 0x01, 0x7b, 0xe7, 0x01, 0x2f, 0xe8, 0x01, 0xa9, 0xea, 0x01,
  0xe8, 0xed, 0x01, 0x5c, 0xf1, 0x01, 0xa7, 0xf5, 0x01, 0x02, 0xfa, 0x01,
  0x58, 0xfe, 0x01, 0x95, 0x02, 0x02, 0xb9, 0x06, 0x02, 0xff, 0x0a, 0x02,
  0x54, 0x0f, 0x02, 0xb9, 0x13, 0x02, 0x02, 0x18, 0x02, 0x5d, 0x1c, 0x02,
  0xa5, 0x20, 0x02, 0xde, 0x24, 0x02, 0x05, 0x29, 0x02, 0x46, 0x2d, 0x02,
  0x8f, 0x31, 0x02, 0xe3, 0x35, 0x02, 0x79, 0x50, 0x02, 0x88, 0x50, 0x02,
  0x9b, 0x52, 0x02, 0xca, 0x54, 0x02, 0xe7, 0x56, 0x02, 0x11, 0x59, 0x02,
  0xe2, 0x5a, 0x02, 0xd6, 0x5c, 0x02, 0xec, 0x5e, 0x02, 0xac, 0x60, 0x02,
  0x23, 0x62, 0x02, 0x77, 0x63, 0x02, 0xc1, 0x66, 0x02, 0xe5, 0x69, 0x02,
  0x04, 0x6d, 0x02, 0x3e, 0x70, 0x02, 0x70, 0x73, 0x02, 0x62, 0x77, 0x02,
  0x7c, 0x7b, 0x02, 0x39, 0x7f, 0x02, 0xaa, 0x82, 0x02, 0x1d, 0x85, 0x02,
  0x4d, 0x87, 0x02, 0xa7, 0x8a, 0x02, 0xe1, 0x8d, 0x02, 0x0f, 0x91, 0x02,
  0x55, 0x94, 0x02, 0x2d, 0x95, 0x02, 0x05, 0x96, 0x02, 0xcb, 0x96, 0x02,
  0x93, 0x97, 0x02, 0x7e, 0x98, 0x02, 0x5a, 0x99, 0x02, 0x45, 0x9a, 0x02,
  0x2d, 0x9b, 0x02, 0x0a, 0x9c, 0x02, 0x48, 0x9d, 0x02, 0x45, 0x9f, 0x02,
  0xe0, 0x9f, 0x02, 0x7b, 0xa0, 0x02, 0x9f, 0xa1, 0x02, 0xd2, 0xa2, 0x02,
  0x2e, 0xa6, 0x02, 0x73, 0xa9, 0x02, 0xc4, 0xac, 0x02, 0x10, 0xb0, 0x02,
  0xae, 0xb3, 0x02, 0xbf, 0xb7, 0x02, 0x2d, 0xbb, 0x02, 0xba, 0xbe, 0x02,
  0xaa, 0xc2, 0x02, 0x85, 0xc5, 0x02, 0xe0, 0xc7, 0x02, 0xd4, 0xca, 0x02,
  0xc7, 0xcd, 0x02, 0xbe, 0xcf, 0x02, 0x9e, 0xd2, 0x02, 0x23, 0xd4, 0x02,
  0x83, 0xd5, 0x02, 0xf6, 0xd6, 0x02, 0x7a, 0xd8, 0x02, 0xed, 0xd9, 0x02,
  0x5c, 0xdb, 0x02, 0xb4, 0xdc, 0x02, 0xcb, 0xdd, 0x02, 0xf9, 0xe1, 0x02,
  0x27, 0xe6, 0x02, 0x4d, 0xe9, 0x02, 0x5d, 0xec, 0x02, 0x0c, 0xef, 0x02,
  0x10, 0xf1, 0x02, 0x30, 0xf3, 0x02, 0x72, 0xf5, 0x02, 0x91, 0xf7, 0x02,
  0x41, 0xf9, 0x02, 0x67, 0xfd, 0x02, 0x72, 0xfd, 0x02, 0x84, 0x00, 0x03,
  0x08, 0x04, 0x03, 0x90, 0x07, 0x03, 0x87, 0x0d, 0x03, 0x8f, 0x11, 0x03,
  0x76, 0x15, 0x03, 0x41, 0x1b, 0x03, 0xdc, 0x1e, 0x03, 0x49, 0x22, 0x03,
  0xd7, 0x26, 0x03, 0x85, 0x2d, 0x03, 0x9a, 0x30, 0x03, 0xcb, 0x35, 0x03,
  0x97, 0x38, 0x03, 0x50, 0x3b, 0x03, 0xad, 0x3d, 0x03, 0x6c, 0x40, 0x03,
  0xc5, 0x43, 0x03, 0x2b, 0x47, 0x03, 0x23, 0x4b, 0x03, 0x1d, 0x4f, 0x03,
  0x76, 0x52, 0x03, 0xdc, 0x55, 0x03, 0x3d, 0x59, 0x03, 0xa6, 0x5c, 0x03,
  0xe9, 0x61, 0x03, 0xc0, 0x6d, 0x03, 0x82, 0x72, 0x03, 0x70, 0x78, 0x03,
  0x8d, 0x78, 0x03, 0xad, 0x78, 0x03, 0xd5, 0x78, 0x03, 0x0b, 0x7e, 0x03,
  0x28, 0x7f, 0x03, 0x39, 0x7f, 0x03, 0x8e, 0x7f, 0x03, 0xf9, 0x7f, 0x03,
  0x41, 0x80, 0x03, 0xb7, 0x80, 0x03, 0xfc, 0x80, 0x03, 0x26, 0x81, 0x03,
  0x61, 0x81, 0x03, 0xcd, 0x81, 0x03, 0x53, 0x82, 0x03, 0xab, 0x82, 0x03,
  0x2d, 0x83, 0x03, 0x89, 0x83, 0x03, 0xce, 0x83, 0x03, 0x25, 0x84, 0x03,
  0x85, 0x84, 0x03, 0xee, 0x84, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x47, 0x85, 0x03, 0xac, 0x85, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xfe, 0x85, 0x03, 0x76, 0x86, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xdb, 0x86, 0x03, 0x3f, 0x87, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x90, 0x87, 0x03, 0xf5, 0x87, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x47, 0x88, 0x03, 0x76, 0x88, 0x03, 0xca, 0x88, 0x03, 0x21, 0x89, 0x03,
  0x69, 0x89, 0x03, 0xc5, 0x89, 0x03, 0x0f, 0x8a, 0x03, 0x56, 0x8a, 0x03,
  0xa5, 0x8a, 0x03, 0x02, 0x8b, 0x03, 0x55, 0x8b, 0x03, 0xa0, 0x8b, 0x03,
  0xfb, 0x8b, 0x03, 0x40, 0x8c, 0x03, 0x9e, 0x8c, 0x03, 0xfc, 0x8c, 0x03,
  0x58, 0x8d, 0x03, 0xa7, 0x8d, 0x03, 0x00, 0x8e, 0x03, 0x54, 0x8e, 0x03,
  0x9e, 0x8e, 0x03, 0xdf, 0x8e, 0x03, 0x38, 0x8f, 0x03, 0x8c, 0x8f, 0x03,
  0xea, 0x8f, 0x03, 0x46, 0x90, 0x03, 0x92, 0x90, 0x03, 0xdb, 0x90, 0x03,
  0x0a, 0x91, 0x03, 0x39, 0x91, 0x03, 0x68, 0x91, 0x03, 0x97, 0x91, 0x03,
  0xc6, 0x91, 0x03, 0xf5, 0x91, 0x03, 0x24, 0x92, 0x03, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0x57, 0x92, 0x03, 0xf1, 0x92, 0x03, 0x78, 0x93, 0x03,
  0x0e, 0x94, 0x03, 0x9f, 0x94, 0x03, 0x15, 0x95, 0x03, 0x59, 0x95, 0x03,
  0xbb, 0x95, 0x03, 0x11, 0x96, 0x03, 0x7e, 0x96, 0x03, 0xd5, 0x96, 0x03,
  0x2c, 0x97, 0x03, 0xc7, 0x97, 0x03, 0x61, 0x98, 0x03, 0xfe, 0x98, 0x03,
  0x9d, 0x99, 0x03, 0x33, 0x9a, 0x03, 0xd0, 0x9a, 0x03, 0x66, 0x9b, 0x03,
  0x06, 0x9c, 0x03, 0xa7, 0x9c, 0x03, 0x06, 0x9d, 0x03, 0x77, 0x9d, 0x03,
  0xe2, 0x9d, 0x03, 0x7d, 0x9e, 0x03, 0xad, 0x9e, 0x03, 0x10, 0x9f, 0x03,
  0x5d, 0x9f, 0x03, 0xec, 0x9f, 0x03, 0x60, 0xa0, 0x03, 0xd2, 0xa0, 0x03,
  0x6a, 0xa1, 0x03, 0x04, 0xa2, 0x03, 0x99, 0xa2, 0x03, 0xff, 0xff, 0xff,
  0x34, 0xa3, 0x03, 0xb3, 0xa3, 0x03, 0x2a, 0xa4, 0x03, 0x6e, 0xa4, 0x03,
  0xd2, 0xa4, 0x03, 0x2f, 0xa5, 0x03, 0xad, 0xa5, 0x03, 0x25, 0xa6, 0x03,
  0x6a, 0xa6, 0x03, 0xd0, 0xa6, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x2d, 0xa7, 0x03, 0xac, 0xa8, 0x03, 0x1d, 0xaa, 0x03, 0x1b, 0xac, 0x03,
  0xfc, 0xad, 0x03, 0xf8, 0xaf, 0x03, 0xda, 0xb1, 0x03, 0xc4, 0xb3, 0x03,
  0xd4, 0xb5, 0x03, 0xba, 0xb7, 0x03, 0x75, 0xb9, 0x03, 0x62, 0xbb, 0x03,
  0x62, 0xbd, 0x03, 0x76, 0xbf, 0x03, 0x78, 0xc1, 0x03, 0x58, 0xc3, 0x03,
  0x55, 0xc5, 0x03, 0x4c, 0xc7, 0x03, 0x50, 0xc9, 0x03, 0x5c, 0xcb, 0x03,
  0xb8, 0xcd, 0x03, 0xa9, 0xde, 0x03, 0xd7, 0xef, 0x03
};

int32_t *EGAhead = (int32_t *)egahead_c3d;
