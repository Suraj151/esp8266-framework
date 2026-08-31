/************************** Program Loader Interface ***************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 30th Aug 2026
******************************************************************************/

#include "ProgramLoaderInterface.h"

#ifdef ENABLE_PROGRAM_EXEC

#include "elf_loader/elf_loader.h"

ProgramLoaderInterface __i_program_loader;

/**
 * Memory an image can be read into and relocated from.
 */
void *ProgramLoaderInterface::allocImage(uint32_t _size)
{
    return esp_elf_malloc(_size, false);
}

/**
 * Release what allocImage() returned.
 */
void ProgramLoaderInterface::freeImage(void *_image)
{
    if (nullptr != _image)
    {
        esp_elf_free(_image);
    }
}

/**
 * The four byte ELF magic, checked before anything is relocated.
 */
bool ProgramLoaderInterface::isImageValid(const void *_image, uint32_t _size)
{
    if (nullptr == _image || _size < 4)
    {
        return false;
    }

    const uint8_t *bytes = (const uint8_t *)_image;
    return (0x7F == bytes[0] && 'E' == bytes[1] && 'L' == bytes[2] && 'F' == bytes[3]);
}

/**
 * Relocate an image into a runnable object.
 */
program_t ProgramLoaderInterface::load(const void *_image, uint32_t _size, int32_t &_err)
{
    _err = 0;

    if (!isImageValid(_image, _size))
    {
        _err = PDI_ERR_INVALID_ARG;
        return nullptr;
    }

    esp_elf_t *elf = (esp_elf_t *)esp_elf_malloc(sizeof(esp_elf_t), false);
    if (nullptr == elf)
    {
        _err = PDI_ERR_NO_MEM;
        return nullptr;
    }
    memset(elf, 0, sizeof(esp_elf_t));

    int ret = esp_elf_init(elf);
    if (ret < 0)
    {
        esp_elf_free(elf);
        _err = (int32_t)ret;
        return nullptr;
    }

    ret = esp_elf_relocate(elf, (const uint8_t *)_image);
    if (ret < 0)
    {
        esp_elf_deinit(elf);
        esp_elf_free(elf);
        _err = (int32_t)ret;
        return nullptr;
    }

    return (program_t)elf;
}

/**
 * Run the program to completion on the calling context.
 */
int32_t ProgramLoaderInterface::run(program_t _program)
{
    if (nullptr == _program)
    {
        return PDI_ERR_NULL_PTR;
    }

    return (int32_t)esp_elf_request((esp_elf_t *)_program, 0, 0, nullptr);
}

/**
 * Release everything load() took.
 */
void ProgramLoaderInterface::unload(program_t _program)
{
    if (nullptr == _program)
    {
        return;
    }

    esp_elf_deinit((esp_elf_t *)_program);
    esp_elf_free(_program);
}

/**
 * Stack the program's own context needs.
 */
uint32_t ProgramLoaderInterface::stackSize() const
{
    return PROGRAM_EXEC_STACK_SIZE;
}

#endif // ENABLE_PROGRAM_EXEC
