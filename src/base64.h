﻿#pragma once

#include <string>

std::string base64_encode(const char const*, unsigned int len);
std::string base64_decode(std::string const& s);