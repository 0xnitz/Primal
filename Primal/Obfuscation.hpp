#pragma once

#include <ntddk.h>

#define OBFUSCATION_KEY 0x25

template <size_t string_size>
struct ObfuscatedStringA {
    char data[string_size];

    constexpr ObfuscatedStringA(const char(&input)[string_size]) : data{} {
        for (size_t i = 0; i < string_size; i++) {
            data[i] = input[i] ^ OBFUSCATION_KEY;
        }
    }

    __forceinline const char* decrypt() {
        for (size_t i = 0; i < string_size; i++) {
            data[i] ^= OBFUSCATION_KEY;
        }

        return data;
    }
};

template <size_t string_size>
struct ObfuscatedStringW {
    wchar_t data[string_size];

    constexpr ObfuscatedStringW(const wchar_t(&input)[string_size]) : data{} {
        for (size_t i = 0; i < string_size; i++) {
            data[i] = input[i] ^ OBFUSCATION_KEY;
        }
    }

    __forceinline const wchar_t* decrypt() {
        for (size_t i = 0; i < string_size; i++) {
            data[i] ^= OBFUSCATION_KEY;
        }

        return data;
    }
};