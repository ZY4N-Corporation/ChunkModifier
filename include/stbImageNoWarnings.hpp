#pragma once

#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push 
#pragma GCC system_header
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION 
#define STB_IMAGE_STATIC
#include <stb_image.h>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>



#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop 
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
