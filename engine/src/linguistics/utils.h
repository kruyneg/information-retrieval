#pragma once

#include <string>

namespace linguistics {

bool IsEnglish(const std::string& word);

void NormalizeYo(std::string& text);

std::string ToLower(const std::string& text);

bool IsLetter(char16_t c);

}  // namespace linguistics
